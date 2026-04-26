import serial
import threading
from core.config import BAUD_RATE

class ArduinoDevice:
    def __init__(self, port, name):

        self.port = port
        self.name = name

        self.serial = serial.Serial(port, 9600, timeout=1) # opens serial connection, between python and arduino

        self.running = True
        self.latest_temp = "--"

        self.callbacks = []

        # start read thread immediately
        self.thread = threading.Thread(target=self.read_loop, daemon=True)
        self.thread.start()

    def send(self, cmd): # sends all commands to arduino
        try:
            self.serial.write((cmd + "\n").encode())
        except:
            pass

    def register_callback(self, func):
        self.callbacks.append(func)

    def read_loop(self): # reads incoming data from arduino continuosly 
        while self.running:
            try:
                if self.serial.in_waiting:
                    line = self.serial.readline().decode().strip()

                    # expected: TEMP,36.5
                    if line.startswith("TEMP"):
                        value = line.split(",")[1]
                        self.latest_temp = value # stores data

                        for cb in self.callbacks: # to gui
                            cb(value)

            except:
                pass
