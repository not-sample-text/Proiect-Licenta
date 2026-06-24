import threading
from core.config_manager import ConfigManager
from core.serial_manager import SerialManager
from ui.tray_icon import ApexTrayIcon
from utils.logger import get_logger

logger = get_logger("MainApp")

def main():
    logger.info("Booting ApexPad Background Daemon...")
    
    # 1. Initialize core logic
    config_mgr = ConfigManager()
    config_mgr.locate_and_load()
    serial_mgr = SerialManager(config_mgr)
    
    # 2. Spin up the Serial Listener in a background thread
    listener_thread = threading.Thread(target=serial_mgr.start_daemon, daemon=True)
    listener_thread.start()
    
    # 3. Launch the System Tray UI on the main thread (Blocking)
    tray = ApexTrayIcon(serial_mgr, config_mgr)
    tray.run()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.info("Shutdown signal received. Exiting gracefully.")
