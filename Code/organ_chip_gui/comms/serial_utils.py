from serial.tools import list_ports

def find_arduinos():
    ports = list_ports.comports()
    return [(p.device, f"Arduino {i+1}") for i, p in enumerate(ports)]
