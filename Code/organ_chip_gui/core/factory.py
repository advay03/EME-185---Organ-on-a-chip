from core.config import USE_FAKE, N_FAKE_DEVICES
from devices.fake_arduino import FakeArduinoDevice
from devices.arduino_device import ArduinoDevice
from comms.serial_utils import find_arduinos

# used for creating fake arduino for testing

def create_devices():
    print("USE_FAKE =", USE_FAKE)

    if USE_FAKE:
        return [FakeArduinoDevice(f"Device {i+1}") for i in range(N_FAKE_DEVICES)]

    else:
        devices = []
        for i, (port, name) in enumerate(find_arduinos()):
            devices.append(ArduinoDevice(port, name))
        return devices
