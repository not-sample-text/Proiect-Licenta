import sys
import os
from pathlib import Path

def get_resource_path(relative_path):
    """ 
    Get absolute path to resource, works for dev and for PyInstaller.
    
    PyInstaller creates a temp folder and stores path in _MEIPASS.
    """
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)

def get_app_data_dir(app_name="Macropad_Host"):
    """
    Returns the standard OS location for storing user data.
    Windows: %LOCALAPPDATA%
    Mac: ~/Library/Application Support
    Linux: ~/.config
    """
    home = Path.home()
    if sys.platform == "win32":
        return home / "AppData" / "Local" / app_name
    elif sys.platform == "darwin":
        return home / "Library" / "Application Support" / app_name
    else:
        return home / ".config" / app_name.lower()
