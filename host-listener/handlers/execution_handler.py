import subprocess
import os
import sys
import webbrowser
import shlex
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
            pass
        else:
            logger.warning(f"Unknown action type '{action_type}' for '{label}'")

    @staticmethod
    def _launch_app(path, label):
        logger.info(f"Launching APP [{label}]: {path}")
        
        try:
            # Safe cross-platform execution without shell=True
            if sys.platform == 'win32':
                subprocess.Popen(path, creationflags=subprocess.CREATE_NO_WINDOW)
            else:
                subprocess.Popen(shlex.split(path))
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
            # Smart Venv Detection (Cross-Platform)
            script_dir = os.path.dirname(path)
            venv_exe = None
            
            for venv_name in ["venv", ".venv", "env", ".env"]:
                if sys.platform == 'win32':
                    # Fixed: Using python.exe instead of pythonw.exe to prevent stdout crashing
                    possible_exe = os.path.join(script_dir, venv_name, "Scripts", "python.exe")
                else:
                    possible_exe = os.path.join(script_dir, venv_name, "bin", "python3")
                
                if os.path.exists(possible_exe):
                    venv_exe = possible_exe
                    break

            if venv_exe:
                logger.info(f"Detected local venv. Routing through: {venv_exe}")
                cmd = [venv_exe, path]
            else:
                logger.info("No local venv found. Using global python.")
                # Fixed: Using python instead of pythonw
                cmd = ["python", path] if sys.platform == 'win32' else ["python3", path]

        elif ext in (".sh", ".bash"):
            cmd = ["bash", path]
        elif ext in (".bat", ".cmd"):
            # Explicit interpreter bypasses shell=True vulnerability while ensuring execution
            cmd = ["cmd.exe", "/c", path]
        elif ext == ".ps1":
            cmd = ["powershell", "-ExecutionPolicy", "Bypass", "-File", path]
        elif ext == ".js":
            cmd = ["node", path]
        elif ext == ".ahk":
            cmd = ["AutoHotkey.exe", path]
        elif ext == ".scpt":
            cmd = ["osascript", path]
        else:
            # Strict 3-Way OS Fallback Support
            if sys.platform == 'win32':
                cmd = ["cmd.exe", "/c", "start", '""', path]
            elif sys.platform == 'darwin':
                cmd = ["open", path]
            else:
                # Handles Linux/Debian environments seamlessly
                cmd = ["xdg-open", path]
        
        try:
            # Execute with terminal suppression on Windows, standard Popen on Unix-likes
            if sys.platform == 'win32':
                subprocess.Popen(cmd, creationflags=subprocess.CREATE_NO_WINDOW)
            else:
                subprocess.Popen(cmd)
        except Exception as e:
            logger.error(f"Failed to run SCRIPT '{path}': {e}")
