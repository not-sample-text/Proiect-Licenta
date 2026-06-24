import subprocess
import os
import webbrowser
from utils.logger import get_logger

logger = get_logger("ExecutionHandler")

class ExecutionHandler:
    @staticmethod
    def execute(action):
        if not action or not isinstance(action, dict):
            return
        
        action_type = action.get("type")
        value = action.get("value")
        label = action.get("label", "Unknown")

        if not value:
            return

        if action_type == "APP":
            ExecutionHandler._launch_app(value, label)
        elif action_type == "SCRIPT":
            ExecutionHandler._run_script(value, label)
        elif action_type in ("KEY", "SHORTCUT"):
            # Handled directly by ESP32 HID over USB/BLE, ignored by host daemon
            pass
        else:
            logger.warning(f"Unknown action type '{action_type}' for '{label}'")

    @staticmethod
    def _launch_app(path, label):
        logger.info(f"Launching APP [{label}]: {path}")
        
        try:
            # shell=True allows OS to resolve PATH binaries automatically
            subprocess.Popen(path, shell=True)
        except Exception as e:
            logger.error(f"Failed to launch APP '{path}': {e}")

    @staticmethod
    def _run_script(path, label):
        logger.info(f"Running SCRIPT [{label}]: {path}")
        
        if path.startswith("http://") or path.startswith("https://"):
            webbrowser.open(path)
            return

        if not os.path.exists(path):
            logger.error(f"Script path does not exist: {path}")
            return

        ext = os.path.splitext(path)[1].lower()
        cmd = []

        if ext == ".py":
            cmd = ["python3", path] if os.name != 'nt' else ["python", path]
        elif ext in (".sh", ".bash"):
            cmd = ["bash", path]
        elif ext in (".bat", ".cmd"):
            cmd = [path]
        elif ext == ".ps1":
            cmd = ["powershell", "-ExecutionPolicy", "Bypass", "-File", path]
        elif ext == ".js":
            cmd = ["node", path]
        elif ext == ".ahk":
            cmd = ["AutoHotkey.exe", path]
        elif ext == ".scpt":
            cmd = ["osascript", path]
        else:
            # Fallback to default OS execution handler
            if os.name == 'nt':
                cmd = ["cmd", "/c", "start", '""', path]
            else:
                cmd = ["xdg-open", path]
        
        try:
            subprocess.Popen(cmd)
        except Exception as e:
            logger.error(f"Failed to run SCRIPT '{path}': {e}")
