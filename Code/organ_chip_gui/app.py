import customtkinter as ctk
from core.factory import create_devices
from ui.device_panel import DevicePanel
import time

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        # title + background
        self.title("Organ-on-Chip Control GUI")
        self.geometry("1000x600")
        
        # run time
        self.start_time = time.time()

        self.top_bar = ctk.CTkFrame(self)
        self.top_bar.pack(fill="x", padx=10, pady=(10, 0))

        self.runtime_label = ctk.CTkLabel(self.top_bar, text="Runtime: 0s")
        self.runtime_label.pack(side="left", padx=10)

        # container
        self.container = ctk.CTkFrame(self)
        self.container.pack(fill="both", expand=True, padx=10, pady=10)
        # self.container.configure(fg_color="gray") # background color


        # set up
        self.devices = []
        self.panels = []
        self.load_devices()
        self.update_runtime()

    def load_devices(self):
        print("LOAD_DEVICES CALLED") # troubleshooting
        self.devices = create_devices() # factory.py to create and store devices (for fake and real arduino)

        print("DEVICE COUNT:", len(self.devices)) # troubleshooting

        columns = 4  # change this to however many panels per row you want

        for i, device in enumerate(self.devices):
            print("CREATING PANEL FOR:", device.name)  # troubleshooting

            panel = DevicePanel(self.container, device)  # creates UI for device

            # convert index into grid position
            row = i // columns
            col = i % columns

            panel.grid(
                row=row,
                column=col,
                padx=10,
                pady=10,
                sticky="nsew"
            )

            # allow resizing behavior
            self.container.grid_columnconfigure(col, weight=1)
            self.container.grid_rowconfigure(row, weight=1)

            self.panels.append(panel)

        """
        for i, device in enumerate(self.devices):
            print("CREATING PANEL FOR:", device.name) # troubleshooting
            panel = DevicePanel(self.container, device) # creates ui for all serial connections established
            #panel.grid(row=0, column=i, padx=10, pady=10, sticky="nsew")
            self.container.grid_rowconfigure(0, minsize=400)
            self.container.grid_columnconfigure(i, weight=1)
            self.panels.append(panel)
        """
    
    def update_runtime(self):
        elapsed = int(time.time() - self.start_time)

        minutes = elapsed // 60
        seconds = elapsed % 60

        self.runtime_label.configure(
            text=f"Runtime: {minutes:02d}:{seconds:02d}"
        )

        self.after(1000, self.update_runtime)  # update every 1 sec



    