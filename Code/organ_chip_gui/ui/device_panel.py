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

        # ===============
        # HEATER SETPOINT
        # ===============
        self.temp_frame = ctk.CTkFrame(self)
        self.temp_frame.pack(pady=5, fill="x")


        self.temp_entry = ctk.CTkEntry(
            self.temp_frame,
            placeholder_text="Temp",
            width=80
        )
        self.temp_entry.pack(side="left", padx=5)
        
        self.temp_button = ctk.CTkButton(
            self.temp_frame, 
            text="Set",
            command=self.set_temp,
            width=60
        )
        self.temp_button.pack(side="left", padx=5)


        # heater on/off button
        self.temp_btn = ctk.CTkButton(
                    self.temp_frame,
                    text="Toggle Heater",
                    command=self.toggle_temp,
                    width=120
        ) 
        self.temp_btn.pack(side="left", padx=5)

        # heater state indication
        self.temp_on = False
        
        self.temp_state = ctk.CTkLabel(
            self.temp_frame,
            text="OFF"
                )
        self.temp_state.pack(side="left", padx=10)

        # default temperature
        #self.temp_entry.insert(0, "0") # set to 0, should turn this on when out of incubator

        # send initial temperature
        self.device.send("T,0")


        # ===========
        # PUMP 1 ROW
        # ===========
        self.pump1_frame = ctk.CTkFrame(self)
        self.pump1_frame.pack(pady=5, fill="x")

        # entry box for pump flow setpoint
        self.pump1_flow = ctk.CTkEntry(
            self.pump1_frame,
            placeholder_text="Flow",
            width=80
        )
        self.pump1_flow.pack(side="left", padx=5)

        # enter button for text entry (pump flow setpoint)
        self.pump1_set = ctk.CTkButton(
            self.pump1_frame,
            text="Set",
            command=self.set_pump1_flow,
            width=60
        )
        self.pump1_set.pack(side="left", padx=5)


        # toggle pump on / off button
        self.pump1_btn = ctk.CTkButton(
            self.pump1_frame,
            text="Toggle Pump 1",
            command=self.toggle_pump1,
            width=120
        )
        self.pump1_btn.pack(side="left", padx=5)

        # pump 1 state indication
        
        self.pump1_on = False
        
        self.pump1_state = ctk.CTkLabel(
            self.pump1_frame,
            text="OFF"
                )

        self.pump1_state.pack(side="left", padx=10)


        # ===========
        # PUMP 2 ROW
        # ===========
        self.pump2_frame = ctk.CTkFrame(self)
        self.pump2_frame.pack(pady=5, fill="x")

        # entry box for pump flow setpoint
        self.pump2_flow = ctk.CTkEntry(
            self.pump2_frame,
            placeholder_text="Flow",
            width=80
        )
        self.pump2_flow.pack(side="left", padx=5)
       
        # enter button for text entry (pump flow setpoint)
        self.pump2_set = ctk.CTkButton(
            self.pump2_frame,
            text="Set",
            command=self.set_pump2_flow,
            width=60
        )
        self.pump2_set.pack(side="left", padx=5)

        # toggle pump on / off button
        self.pump2_btn = ctk.CTkButton(
            self.pump2_frame,
            text="Toggle Pump 2",
            command=self.toggle_pump2,
            width=120
        )
        self.pump2_btn.pack(side="left", padx=5)

        self.pump2_on = False
        # this needs to be fixed so that it shows on or off depending on if you're clicking the button
        self.pump2_state = ctk.CTkLabel(
            self.pump2_frame,
            text="OFF"
        )
        self.pump2_state.pack(side="left", padx=10)


        # ===========
        # PUMP 3 ROW
        # ===========
        self.pump3_frame = ctk.CTkFrame(self)
        self.pump3_frame.pack(pady=5, fill="x")

        # entry box for pump flow setpoint
        self.pump3_flow = ctk.CTkEntry(
            self.pump3_frame,
            placeholder_text="Flow",
            width=80
        )
        self.pump3_flow.pack(side="left", padx=5)

        # enter button for text entry (pump flow setpoint)
        self.pump3_set = ctk.CTkButton(
            self.pump3_frame,
            text="Set",
            command=self.set_pump3_flow,
            width=60
        )
        self.pump3_set.pack(side="left", padx=5)


        # toggle pump on / off button
        self.pump3_btn = ctk.CTkButton(
            self.pump3_frame,
            text="Toggle Pump 3",
            command=self.toggle_pump3,
            width=120
        )
        self.pump3_btn.pack(side="left", padx=5)

        self.pump3_on = False

        # this needs to be fixed so that it shows on or off depending on if you're clicking the button
        self.pump3_state = ctk.CTkLabel(
            self.pump3_frame,
            text="OFF"
        )
        self.pump3_state.pack(side="left", padx=10)

    #  ================================================================================= #


    # UI ACTIONS (connect this to the arduino code)
    def set_temp(self, event=None):
        try:
            val = float(self.temp_entry.get())
            self.device.send(f"T,{val}")
        except ValueError:
            print("Invalid temperature")

    def toggle_temp(self):
        self.temp_on = not self.temp_on
        self.device.send(f"T,{int(self.temp_on)}")
        if (self.temp_on == 0):
            
                self.temp_state.configure(text = "OFF")
                
        else:
                self.temp_state.configure(text = "ON")


    # PUMP 1
    def set_pump1_flow(self):
        try:
            flow = float(self.pump1_flow.get())
            self.device.send(f"F1,{flow}")        # send to Arduino
        except ValueError:
            print("Invalid flow value")

    def toggle_pump1(self):
        self.pump1_on = not self.pump1_on
        self.device.send(f"P1,{int(self.pump1_on)}")
        if (self.pump1_on == 0):
            
                self.pump1_state.configure(text = "OFF")
                
        else:
                self.pump1_state.configure(text = "ON")

                
    # PUMP 2
    def set_pump2_flow(self):
        try:
            flow = float(self.pump2_flow.get())
            self.device.send(f"F2,{flow}")        # send to Arduino
        except ValueError:
            print("Invalid flow value")

    def toggle_pump2(self):
        self.pump2_on = not self.pump2_on
        self.device.send(f"P2,{int(self.pump2_on)}")
        if (self.pump2_on == 0):
            
                self.pump2_state.configure(text = "OFF")
                
        else:
                self.pump2_state.configure(text = "ON")
                
    # PUMP 3
    def set_pump3_flow(self):
        try:
            flow = float(self.pump3_flow.get())
            self.device.send(f"F3,{flow}")       # send to Arduino
        except ValueError:
            print("Invalid flow value")
 
    def toggle_pump3(self):
        self.pump3_on = not self.pump3_on
        self.device.send(f"P3,{int(self.pump3_on)}")
        
        if (self.pump3_on == 0):
            
                self.pump3_state.configure(text = "OFF")
                
        else:
                self.pump3_state.configure(text = "ON")

    # LIVE UPDATES FOR TEMPERATURE
    def update_temp(self, value):
        self.temp_label.configure(text=f"Temp: {value} °C")