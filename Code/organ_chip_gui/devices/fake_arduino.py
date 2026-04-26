import threading
import random
import time

# used for creating fake arduino for testing

class FakeArduinoDevice:
    def __init__(self, name):
        self.name = name
        self.callbacks = []

        threading.Thread(target=self.loop, daemon=True).start()

    def send(self, cmd):
        print(f"[{self.name}] {cmd}")

    def register_callback(self, cb):
        self.callbacks.append(cb)

    def loop(self):
        while True:
            temp = round(random.uniform(20, 40), 2)

            for cb in self.callbacks:
                cb(temp)

            time.sleep(1)
