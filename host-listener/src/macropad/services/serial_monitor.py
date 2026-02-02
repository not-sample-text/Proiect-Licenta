import threading
import serial
import time
import serial.tools.list_ports

class SerialMonitor(threading.Thread):
    """
    Runs in the background. Listens to the hardware.
    Pushes events to a thread-safe Queue.
    """
    def __init__(self, message_queue):
        super().__init__()
        self.queue = message_queue
        self.running = False
        self.serial_port = None
        self.port_name = None
        self._stop_event = threading.Event()

    def get_available_ports(self):
        """Static helper to list COM ports."""
        return [p.device for p in serial.tools.list_ports.comports()]

    def connect(self, port_name):
        """Attempts to open the serial connection."""
        if self.serial_port and self.serial_port.is_open:
            self.disconnect()
        
        self.port_name = port_name
        try:
            self.serial_port = serial.Serial(port_name, 115200, timeout=1)
            self.queue.put(("STATUS", f"Connected to {port_name}"))
            return True
        except Exception as e:
            self.queue.put(("ERROR", f"Connection Failed: {e}"))
            return False

    def disconnect(self):
        """Safely closes the connection."""
        if self.serial_port:
            try:
                self.serial_port.close()
            except Exception:
                pass
            self.serial_port = None
        self.queue.put(("STATUS", "Disconnected"))

    def run(self):
        """The main loop of the background thread."""
        self.running = True
        while not self._stop_event.is_set():
            if self.serial_port and self.serial_port.is_open:
                try:
                    if self.serial_port.in_waiting:
                        # Read line, decode, strip whitespace
                        line = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            # Send raw data to the Brain
                            self.queue.put(("DATA", line))
                except serial.SerialException as e:
                    self.queue.put(("ERROR", f"Serial connection lost: {e}"))
                    self.disconnect()
                except Exception as e:
                    self.queue.put(("ERROR", f"Read Error: {e}"))
            else:
                # If not connected, just sleep a bit to save CPU
                time.sleep(0.5)
            
            # Fast poll to keep latency low
            time.sleep(0.01)

    def stop(self):
        """Stops the thread gracefully."""
        self._stop_event.set()
        self.disconnect()
