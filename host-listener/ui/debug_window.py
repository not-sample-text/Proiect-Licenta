import tkinter as tk
from tkinter import scrolledtext
import threading
from utils.logger import log_queue

class DebugWindow:
    def __init__(self):
        self.root = None
        self.text_area = None
        self.is_open = False

    def _build_and_run(self):
        self.root = tk.Tk()
        self.root.title("ApexPad Debug Console")
        self.root.geometry("600x400")
        self.root.configure(bg="#1e1e1e")
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        self.text_area = scrolledtext.ScrolledText(
            self.root, 
            state='disabled', 
            bg="#1e1e1e", 
            fg="#00ff00", 
            font=("Consolas", 10)
        )
        self.text_area.pack(expand=True, fill='both', padx=5, pady=5)

        self.is_open = True
        self.root.after(100, self._poll_queue)
        self.root.mainloop()

    def _poll_queue(self):
        if not self.is_open:
            return
            
        while not log_queue.empty():
            msg = log_queue.get_nowait()
            self.text_area.config(state='normal')
            self.text_area.insert(tk.END, msg + "\n")
            self.text_area.see(tk.END)
            self.text_area.config(state='disabled')
            
        self.root.after(100, self._poll_queue)

    def _on_close(self):
        self.is_open = False
        self.root.destroy()

    def show(self):
        if self.is_open:
            return
        # Spawning Tkinter in its own isolated thread to prevent blocking Pystray
        threading.Thread(target=self._build_and_run, daemon=True).start()
