import json
import shutil
from pathlib import Path
from ..utils.paths import get_app_data_dir

CONFIG_FILENAME = "config.json"
BACKUP_FILENAME = "config.backup.json"
SETTINGS_FILENAME = "settings.ini"

class ConfigManager:
    def __init__(self):
        self.app_dir = get_app_data_dir()
        
        # Define paths
        self.cache_path = self.app_dir / CONFIG_FILENAME
        self.backup_path = self.app_dir / BACKUP_FILENAME
        self.settings_path = self.app_dir / SETTINGS_FILENAME
        
        # Ensure the directory exists
        self.app_dir.mkdir(parents=True, exist_ok=True)
        
        self.data = {}
        self.source_path = self._load_source_path_pointer()

    def _load_source_path_pointer(self):
        """Reads the 'pointer' file (settings.ini) to find where the user keeps their config."""
        if self.settings_path.exists():
            try:
                with open(self.settings_path, "r") as f:
                    path_str = f.read().strip()
                    if path_str:
                        return Path(path_str)
            except Exception:
                pass 
        return None

    def set_source_path(self, path_str):
        """Updates the pointer to the user's config file."""
        self.source_path = Path(path_str)
        with open(self.settings_path, "w") as f:
            f.write(str(self.source_path))

    def sync_and_load(self):
        """
        Logic:
        1. If source exists and is newer than cache -> Copy Source to Cache.
        2. Load Cache.
        3. If no Cache, fail gracefully.
        """
        status_msg = "Ready"

        # 1. Sync Logic
        if self.source_path and self.source_path.exists():
            should_update = False
            
            if not self.cache_path.exists():
                should_update = True
            elif self.source_path.stat().st_mtime > self.cache_path.stat().st_mtime:
                should_update = True
            
            if should_update:
                try:
                    # Backup existing cache if it exists
                    if self.cache_path.exists():
                        shutil.copy2(self.cache_path, self.backup_path)
                    
                    shutil.copy2(self.source_path, self.cache_path)
                    status_msg = "Synced config from source."
                except Exception as e:
                    status_msg = f"Sync failed: {e}"
            else:
                status_msg = "Config is up to date."
        elif self.source_path:
            status_msg = f"Source missing ({self.source_path}). Using cache."

        # 2. Load Logic
        if self.cache_path.exists():
            try:
                with open(self.cache_path, "r", encoding='utf-8') as f:
                    self.data = json.load(f)
            except json.JSONDecodeError as e:
                return f"Error: Cache corrupted ({e})"
        else:
            return "Error: No config loaded. Please select a source file."

        return status_msg

    def get_command(self, layer_idx, key_id):
        """
        Safe retrieval of commands.
        structure: layers[i] -> keys -> key_id -> value
        """
        try:
            layers = self.data.get("layers", [])
            if 0 <= layer_idx < len(layers):
                key_data = layers[layer_idx].get("keys", {}).get(key_id)
                if key_data:
                    return key_data.get("value")
        except Exception:
            pass
        return None
