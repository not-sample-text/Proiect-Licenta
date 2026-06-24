import logging
import queue

# A thread-safe queue to pass logs to our GUI debug window
log_queue = queue.Queue()

class QueueHandler(logging.Handler):
    def emit(self, record):
        try:
            msg = self.format(record)
            log_queue.put(msg)
        except Exception:
            self.handleError(record)

def get_logger(name):
    logger = logging.getLogger(name)
    if not logger.handlers:
        logger.setLevel(logging.DEBUG)
        
        formatter = logging.Formatter('[%(levelname)s] %(name)s: %(message)s')
        
        # Standard console output
        ch = logging.StreamHandler()
        ch.setLevel(logging.DEBUG)
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        
        # GUI Queue output
        qh = QueueHandler()
        qh.setLevel(logging.DEBUG)
        qh.setFormatter(formatter)
        logger.addHandler(qh)
        
    return logger
