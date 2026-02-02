import threading
from PIL import Image, ImageDraw
import pystray
from pystray import MenuItem as item
from ..utils.paths import get_resource_path

class SystemTray:
    def __init__(self, app_name, on_show, on_quit):
        self.app_name = app_name
        self.on_show = on_show
        self.on_quit = on_quit
        self.icon = None
        self.thread = None

    def _create_default_icon(self):
        """Generates a fallback icon if the .png is missing."""
        img = Image.new('RGB', (64, 64), color='black')
        d = ImageDraw.Draw(img)
        # Draw a cyan square
        d.rectangle([16, 16, 48, 48], fill="cyan")
        return img

    def _load_icon_image(self):
        """Tries to load icon.png, falls back to generator."""
        icon_path = get_resource_path("assets/icon.png")
        try:
            return Image.open(icon_path)
        except Exception:
            return self._create_default_icon()

    def _run_tray(self):
        image = self._load_icon_image()
        
        # Define Menu
        menu = (
            item('Show Settings', self._action_show),
            item('Quit', self._action_quit)
        )

        self.icon = pystray.Icon("macropad_host", image, self.app_name, menu)
        self.icon.run()

    def _action_show(self, icon, item):
        # pystray runs on its own thread, so we call the callback
        if self.on_show:
            self.on_show()

    def _action_quit(self, icon, item):
        self.stop()
        if self.on_quit:
            self.on_quit()

    def start(self):
        """Starts the tray icon in a separate daemon thread."""
        self.thread = threading.Thread(target=self._run_tray, daemon=True)
        self.thread.start()

    def stop(self):
        """Stops the tray icon."""
        if self.icon:
            self.icon.stop()
