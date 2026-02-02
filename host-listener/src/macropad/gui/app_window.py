import tkinter as tk
from tkinter import ttk, scrolledtext, filedialog
import queue
import threading

# Import our custom modules
from ..core.config import ConfigManager
from ..core.executor import CommandExecutor
from ..services.serial_monitor import SerialMonitor
from ..services.tray_icon import SystemTray

APP_NAME = "Macropad Host"

class AppWindow:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title(APP_NAME)
        self.root.geometry("600x450")
        
        # 1. Initialize Core & Services
        self.config = ConfigManager()
        self.msg_queue = queue.Queue()
        self.serial_monitor = SerialMonitor(self.msg_queue)
        
        # 2. Setup System Tray
        # We pass our own methods as callbacks so the tray can control this window
        self.tray = SystemTray(
            app_name=APP_NAME,
            on_show=self.show_window,
            on_quit=self.quit_app
        )
        
        # 3. Build UI Elements
        self._setup_ui()
        
        # 4. Override Window "X" Button
        self.root.protocol("WM_DELETE_WINDOW", self.hide_window)

        # 5. Start Background Threads
        self.tray.start()
        self.serial_monitor.start()
        
        # 6. Initial Sync & Start Heartbeat
        self.log(self.config.sync_and_load())
        self.root.after(100, self.process_queue)

    def _setup_ui(self):
        """Constructs the visual layout."""
        # --- Top Bar: Connection ---
        top_frame = tk.Frame(self.root, pady=10)
        top_frame.pack(fill=tk.X, padx=10)
        
        tk.Label(top_frame, text="COM Port:").pack(side=tk.LEFT)
        
        self.port_combo = ttk.Combobox(top_frame, width=15)
        self.port_combo.pack(side=tk.LEFT, padx=5)
        self.port_combo['values'] = self.serial_monitor.get_available_ports()
        if self.port_combo['values']:
            self.port_combo.current(0)
            
        btn_refresh = tk.Button(top_frame, text="↻", width=3, command=self.refresh_ports)
        btn_refresh.pack(side=tk.LEFT)
        
        self.btn_connect = tk.Button(top_frame, text="Connect", command=self.toggle_connection, bg="#dddddd")
        self.btn_connect.pack(side=tk.LEFT, padx=10)

        # --- Middle Bar: Config ---
        cfg_frame = tk.LabelFrame(self.root, text="Configuration", pady=5, padx=5)
        cfg_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.lbl_path = tk.Label(cfg_frame, text=str(self.config.source_path or "No source selected"), fg="blue", anchor="w")
        self.lbl_path.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        tk.Button(cfg_frame, text="Browse...", command=self.browse_config).pack(side=tk.RIGHT)
        tk.Button(cfg_frame, text="Sync Now", command=self.force_sync).pack(side=tk.RIGHT, padx=5)

        # --- Bottom: Console ---
        self.console = scrolledtext.ScrolledText(self.root, state='disabled', height=15)
        self.console.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

    # --- Logic: The Heartbeat ---
    def process_queue(self):
        """
        Polls the queue every 50ms for messages from background threads.
        This keeps the GUI responsive.
        """
        try:
            while True:
                # Get message without blocking
                msg_type, content = self.msg_queue.get_nowait()
                
                if msg_type == "STATUS":
                    self.log(f"[Status] {content}")
                    if "Connected" in content:
                        self.btn_connect.config(text="Disconnect", bg="#aaffaa")
                    elif "Disconnected" in content:
                        self.btn_connect.config(text="Connect", bg="#dddddd")
                        
                elif msg_type == "ERROR":
                    self.log(f"[Error] {content}")
                    
                elif msg_type == "DATA":
                    self.handle_macro_trigger(content)
                    
                self.msg_queue.task_done()
        except queue.Empty:
            pass
        finally:
            # Schedule next check
            self.root.after(50, self.process_queue)

    # --- Logic: The Protocol Parser (102 -> Action) ---
    def handle_macro_trigger(self, data):
        """
        Parses the 3-digit code from ESP32.
        Format: "LCR" (Layer, Col, Row) e.g. "102"
        """
        data = data.strip()
        
        # Validation: Must be exactly 3 digits
        if len(data) == 3 and data.isdigit():
            layer = int(data[0])
            col = int(data[1])
            row = int(data[2])
            
            # Reconstruct Key ID (e.g., "C0R2") to match config.json
            key_id = f"C{col}R{row}"
            
            self.log(f"Trigger: Layer {layer} | Key {key_id}")
            
            # Fetch Command
            cmd = self.config.get_command(layer, key_id)
            
            # Execute Command
            success, msg = CommandExecutor.execute(cmd)
            self.log(f"  -> {msg}")
            
        else:
            self.log(f"[Raw Data] {data}")

    # --- User Actions ---
    def refresh_ports(self):
        ports = self.serial_monitor.get_available_ports()
        self.port_combo['values'] = ports
        if ports: self.port_combo.current(0)

    def toggle_connection(self):
        if self.btn_connect['text'] == "Connect":
            port = self.port_combo.get()
            if port:
                self.serial_monitor.connect(port)
        else:
            self.serial_monitor.disconnect()

    def browse_config(self):
        path = filedialog.askopenfilename(filetypes=[("JSON", "*.json")])
        if path:
            self.config.set_source_path(path)
            self.lbl_path.config(text=path)
            self.force_sync()

    def force_sync(self):
        msg = self.config.sync_and_load()
        self.log(msg)

    def log(self, msg):
        self.console.configure(state='normal')
        self.console.insert(tk.END, msg + "\n")
        self.console.see(tk.END)
        self.console.configure(state='disabled')

    # --- Window Management ---
    def show_window(self):
        self.root.after(0, self.root.deiconify)

    def hide_window(self):
        self.root.withdraw()
        # Optional: Show a notification bubble here if desired

    def quit_app(self):
        # Graceful shutdown
        self.serial_monitor.stop()
        self.tray.stop()
        self.root.destroy()

    def run(self):
        self.root.mainloop()
