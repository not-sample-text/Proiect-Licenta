import tkinter as tk
from tkinter import filedialog, ttk, scrolledtext
import pystray
from pystray import MenuItem as item
from PIL import Image, ImageDraw
import threading
import serial
import serial.tools.list_ports
import json
import os
import shutil
import sys
import platform
from pathlib import Path
import subprocess
import time

# --- CONFIGURATION ---
APP_NAME = "Macropad Listener"
CONFIG_FILENAME = "config.json"
BACKUP_FILENAME = "config.backup.json"

# --- HELPER CLASSES ---

class ConfigManager:
    """Handles File I/O: User Documents vs Internal Cache"""
    def __init__(self):
        self.os_type = platform.system()
        self.app_dir = self._get_app_directory()
        self.cache_path = self.app_dir / CONFIG_FILENAME
        self.backup_path = self.app_dir / BACKUP_FILENAME
        self.settings_path = self.app_dir / "settings.ini"
        
        # Ensure app directory exists
        self.app_dir.mkdir(parents=True, exist_ok=True)
        
        self.config_data = None
        self.source_path = self._load_source_path()

    def _get_app_directory(self):
        home = Path.home()
        if self.os_type == "Windows":
            return home / "AppData" / "Local" / "Macropad_Host"
        elif self.os_type == "Darwin": # macOS
            return home / "Library" / "Application Support" / "Macropad_Host"
        else: # Linux
            return home / ".config" / "macropad_host"

    def _load_source_path(self):
        if self.settings_path.exists():
            try:
                with open(self.settings_path, "r") as f:
                    path = Path(f.read().strip())
                    return path if path.exists() else None
            except:
                return None
        return None

    def save_source_path(self, path):
        self.source_path = Path(path)
        with open(self.settings_path, "w") as f:
            f.write(str(self.source_path))

    def sync(self):
        """Checks User File vs Cache. Updates Cache if User File is newer."""
        status = "Ready"
        
        if not self.source_path or not self.source_path.exists():
            if self.cache_path.exists():
                self.load_cache()
                return "Running on Cache (Source Missing)"
            return "Setup Required: Please locate config.json"

        try:
            # Check timestamps
            should_update = False
            if not self.cache_path.exists():
                should_update = True
            elif self.source_path.stat().st_mtime > self.cache_path.stat().st_mtime:
                should_update = True

            if should_update:
                if self.cache_path.exists():
                    shutil.copy2(self.cache_path, self.backup_path)
                shutil.copy2(self.source_path, self.cache_path)
                status = "Config Synced from Source"
            else:
                status = "Config Up-to-Date"

            self.load_cache()
        except Exception as e:
            status = f"Sync Error: {e}"

        return status

    def load_cache(self):
        if self.cache_path.exists():
            with open(self.cache_path, "r", encoding='utf-8') as f:
                self.config_data = json.load(f)

    def get_command(self, layer_idx, key_id):
        """Parses the JSON to find the command for L{x}:{key}"""
        if not self.config_data: return None
        try:
            # Access the layers array
            layers = self.config_data.get("layers", [])
            if layer_idx >= len(layers): return None
            
            # Access specific key
            key_data = layers[layer_idx]["keys"].get(key_id)
            if key_data:
                return key_data.get("value")
        except:
            return None
        return None

class ActionExecutor:
    """Runs the actual commands on the OS"""
    @staticmethod
    def run(command):
        if not command: return "Empty Command"
        
        system = platform.system()
        try:
            # 1. Web URL
            if command.startswith("http"):
                import webbrowser
                webbrowser.open(command)
                return f"Opened URL: {command}"

            # 2. File/Script/App
            # Clean up path quotes just in case
            command = command.replace('"', '')

            if system == "Windows":
                os.startfile(command)
            elif system == "Darwin": # macOS
                subprocess.call(('open', command))
            else: # Linux
                subprocess.call(('xdg-open', command))
                
            return f"Executed: {command}"
        except Exception as e:
            return f"Execution Failed: {e}"

class SerialListener(threading.Thread):
    """Background thread to listen for L{x}:{key}"""
    def __init__(self, gui_log_callback, config_manager):
        super().__init__()
        self.gui_log = gui_log_callback
        self.cfg = config_manager
        self.running = True
        self.serial_port = None
        self.port_name = None

    def connect(self, port_name):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        self.port_name = port_name
        try:
            self.serial_port = serial.Serial(port_name, 115200, timeout=1)
            self.gui_log(f"Connected to {port_name}")
            return True
        except Exception as e:
            self.gui_log(f"Connection Failed: {e}")
            return False

    def run(self):
        while self.running:
            if self.serial_port and self.serial_port.is_open:
                try:
                    if self.serial_port.in_waiting:
                        line = self.serial_port.readline().decode('utf-8').strip()
                        if line:
                            self.handle_data(line)
                except Exception as e:
                    self.gui_log(f"Serial Error: {e}")
                    self.serial_port.close()
            else:
                time.sleep(1) # Wait before checking again
            time.sleep(0.01) # Fast polling

    def handle_data(self, data):
        # Protocol: L{layer}:{key} -> e.g., "L2:C0R1"
        if data.startswith("L") and ":" in data:
            try:
                parts = data.split(":")
                layer_idx = int(parts[0][1:]) # "L2" -> 2
                key_id = parts[1]             # "C0R1"
                
                self.gui_log(f"Trigger: Layer {layer_idx} Key {key_id}")
                
                # Get command from JSON
                cmd = self.cfg.get_command(layer_idx, key_id)
                if cmd:
                    result = ActionExecutor.run(cmd)
                    self.gui_log(f" -> {result}")
                else:
                    self.gui_log(" -> No command mapped.")
            except Exception as e:
                self.gui_log(f"Parse Error: {e}")
        else:
            # Log other messages (debug info from ESP)
            self.gui_log(f"Device: {data}")

    def stop(self):
        self.running = False
        if self.serial_port:
            self.serial_port.close()

class MacropadApp:
    def __init__(self):
        self.config = ConfigManager()
        self.icon = None
        self.root = tk.Tk()
        self.setup_gui()
        
        # Override X button
        self.root.protocol("WM_DELETE_WINDOW", self.hide_window)

        # Start Listener Thread
        self.listener = SerialListener(self.log, self.config)
        self.listener.start()

        # Initial Sync
        self.log(self.config.sync())

        # Start Tray
        threading.Thread(target=self.setup_tray, daemon=True).start()
        
        # Auto-hide on startup if configured (optional, showing for now)
        # self.root.withdraw()

    def setup_gui(self):
        self.root.title(APP_NAME)
        self.root.geometry("600x450")
        
        # 1. Top Bar: Port Selection
        top_frame = tk.Frame(self.root, pady=10)
        top_frame.pack(fill=tk.X, padx=10)
        
        tk.Label(top_frame, text="COM Port:").pack(side=tk.LEFT)
        self.port_combo = ttk.Combobox(top_frame, width=15)
        self.port_combo.pack(side=tk.LEFT, padx=5)
        
        refresh_btn = tk.Button(top_frame, text="Refresh", command=self.refresh_ports)
        refresh_btn.pack(side=tk.LEFT)
        
        connect_btn = tk.Button(top_frame, text="Connect", command=self.connect_serial, bg="#dddddd")
        connect_btn.pack(side=tk.LEFT, padx=5)

        # 2. Config Section
        cfg_frame = tk.LabelFrame(self.root, text="Configuration Source", pady=5, padx=5)
        cfg_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.path_label = tk.Label(cfg_frame, text=str(self.config.source_path or "Not Set"), fg="blue")
        self.path_label.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        browse_btn = tk.Button(cfg_frame, text="Browse...", command=self.browse_config)
        browse_btn.pack(side=tk.RIGHT)
        
        sync_btn = tk.Button(cfg_frame, text="Force Sync", command=self.force_sync)
        sync_btn.pack(side=tk.RIGHT, padx=5)

        # 3. Console
        self.console = scrolledtext.ScrolledText(self.root, state='disabled', height=15)
        self.console.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.refresh_ports()

    def setup_tray(self):
        # Draw a simple icon (Blue Square)
        img = Image.new('RGB', (64, 64), color='black')
        d = ImageDraw.Draw(img)
        d.rectangle([16, 16, 48, 48], fill="cyan")
        
        menu = (item('Show Settings', self.show_window), item('Quit', self.quit_app))
        self.icon = pystray.Icon("macropad", img, APP_NAME, menu)
        self.icon.run()

    # --- GUI ACTIONS ---

    def refresh_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)

    def connect_serial(self):
        port = self.port_combo.get()
        if port:
            self.listener.connect(port)

    def browse_config(self):
        path = filedialog.askopenfilename(filetypes=[("JSON", "*.json")])
        if path:
            self.config.save_source_path(path)
            self.path_label.config(text=path)
            self.force_sync()

    def force_sync(self):
        msg = self.config.sync()
        self.log(msg)

    def log(self, msg):
        self.console.configure(state='normal')
        self.console.insert(tk.END, msg + "\n")
        self.console.see(tk.END)
        self.console.configure(state='disabled')

    def show_window(self, icon=None, item=None):
        self.root.after(0, self.root.deiconify)

    def hide_window(self):
        self.root.withdraw()

    def quit_app(self, icon=None, item=None):
        self.listener.stop()
        if self.icon: self.icon.stop()
        self.root.destroy()

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    MacropadApp().run()
