import customtkinter as ctk

class DevicePanel(ctk.CTkFrame):
    def __init__(self, master, device):
        super().__init__(master)

        self.configure(width=250, height=300)

        self.pack_propagate(False)
        self.device = device

        # register callback for live updates
        self.device.register_callback(self.update_temp)

        # UI
        self.title = ctk.CTkLabel(self, text=device.name, font=("Arial", 16)) 
        self.title.pack(pady=5)

        self.temp_label = ctk.CTkLabel(self, text="Temp: -- °C")
        self.temp_label.pack(pady=5)

        # heater setpoint
        self.temp_slider = ctk.CTkSlider(self, from_=20, to=60, command=self.set_temp) # slider
        #self.temp_entry = ctk.CTkEntry(self, placeholder_text ="Temperature Setpoint")
        self.temp_slider.pack(pady=5)

        # pump flow
        self.flow_label = ctk.CTkLabel(self, text="Flow: --")
        self.flow_label.pack(pady=5)
        self.flow_slider = ctk.CTkSlider(self, from_=0, to=1, command=self.set_flow)
        self.flow_slider.pack(pady=5)

        # pump 1 toggle
        self.pump1_btn = ctk.CTkButton(self, text="Toggle Pump 1", command=self.toggle_pump1)
        self.pump1_btn.pack(pady=5)

        self.pump1_on = False
        self.pump1_state = ctk.CTkLabel(self, text="Pump 1: OFF")
        self.pump1_state.pack(pady=5)

        # pump 2 toggle
        self.pump2_btn = ctk.CTkButton(self, text="Toggle Pump 2", command=self.toggle_pump2)
        self.pump2_btn.pack(pady=5)

        self.pump2_on = False
        self.pump2_state = ctk.CTkLabel(self, text="Pump 2: OFF")
        self.pump2_state.pack(pady=5)

        # pump 3 toggle
        self.pump3_btn = ctk.CTkButton(self, text="Toggle Pump 3", command=self.toggle_pump3)
        self.pump3_btn.pack(pady=5)


        self.pump3_on = False
        self.pump3_state = ctk.CTkLabel(self, text="Pump 3: OFF")
        self.pump3_state.pack(pady=5)


    # UI ACTIONS (connect this to the arduino code)
    def set_temp(self, val):
        self.device.send(f"T,{val}")

    def set_flow(self, val):
        self.device.send(f"F,{val}")

    def toggle_pump1(self):
        self.pump1_on = not self.pump1_on
        self.device.send(f"P1,{int(self.pump1_on)}")

    def toggle_pump2(self):
        self.pump2_on = not self.pump2_on
        self.device.send(f"P2,{int(self.pump2_on)}")

    def toggle_pump3(self):
        self.pump3_on = not self.pump3_on
        self.device.send(f"P3,{int(self.pump3_on)}")

    # LIVE UPDATES FOR TEMPERATURE
    def update_temp(self, value):
        self.temp_label.configure(text=f"Temp: {value} °C")