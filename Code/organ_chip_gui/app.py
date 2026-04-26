import customtkinter as ctk
from core.factory import create_devices
from ui.device_panel import DevicePanel

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        # title + background
        self.title("Organ-on-Chip Control GUI")
        self.geometry("1000x800")
        
        self.container = ctk.CTkFrame(self)
        self.container.pack(fill="both", expand=True, padx=10, pady=10)
        # self.container.configure(fg_color="gray") # background color

        # set up
        self.devices = []
        self.panels = []
        self.load_devices()

    def load_devices(self):
        print("LOAD_DEVICES CALLED") # troubleshooting
        self.devices = create_devices() # factory.py to create and store devices (for fake and real arduino)

        print("DEVICE COUNT:", len(self.devices)) # troubleshooting

        for i, device in enumerate(self.devices):
            print("CREATING PANEL FOR:", device.name) # troubleshooting
            panel = DevicePanel(self.container, device) # creates ui for all serial connections established
            panel.grid(row=0, column=i, padx=10, pady=10, sticky="n")
            self.panels.append(panel)