import os
import subprocess
import sys
import webbrowser
import shlex

class CommandExecutor:
    """
    Stateless class responsible purely for executing system actions.
    """
    
    @staticmethod
    def execute(cmd_str):
        """
        Parses and runs a command string.
        Returns: (Success: bool, Message: str)
        """
        if not cmd_str:
            return False, "Empty command"

        cmd_str = str(cmd_str).strip()
        
        try:
            # 1. Handle URLs
            if cmd_str.startswith("http://") or cmd_str.startswith("https://"):
                webbrowser.open(cmd_str)
                return True, f"Opened URL: {cmd_str}"

            # 2. Handle System Commands / Files
            # Strip quotes for safety (Windows hates quotes around file paths in startfile)
            clean_cmd = cmd_str.replace('"', '')

            if sys.platform == "win32":
                os.startfile(clean_cmd)
            elif sys.platform == "darwin":
                subprocess.call(('open', clean_cmd))
            else:
                subprocess.call(('xdg-open', clean_cmd))
            
            return True, f"Executed: {clean_cmd}"

        except FileNotFoundError:
            return False, f"File not found: {cmd_str}"
        except Exception as e:
            return False, f"Execution failed: {e}"
