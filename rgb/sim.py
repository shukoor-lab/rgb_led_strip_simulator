import tkinter as tk
import subprocess
import threading

# SET YOUR SIMULATOR CAPACITY HERE
SIM_N = 5

class DynamicRGBSim:
    def __init__(self, root):
        self.root = root
        self.root.title(f"Simulator ({SIM_N} slots)")
        
        # Calculate window size based on LED count
        spacing = 50
        width = (SIM_N * spacing) + 50
        self.canvas = tk.Canvas(root, width=width, height=120, bg='#111')
        self.canvas.pack()
        
        self.leds = []
        for i in range(SIM_N):
            x = 30 + (i * spacing)
            l = self.canvas.create_oval(x, 40, x+30, 70, fill='black', outline='#333')
            self.leds.append(l)
        
        self.process = subprocess.Popen(['./logic'], stdout=subprocess.PIPE)
        threading.Thread(target=self.read_data, daemon=True).start()

    def read_data(self):
        while True:
            byte = self.process.stdout.read(1)
            if byte == b'S':
                # 1. Read the 'N' byte sent by C
                c_count_raw = self.process.stdout.read(1)
                if not c_count_raw: continue
                c_count = c_count_raw[0]
                
                # 2. Read the RGB data (exactly c_count * 3 bytes)
                raw_payload = self.process.stdout.read(c_count * 3)
                
                if len(raw_payload) == c_count * 3:
                    # Logic to handle mismatches
                    for i in range(SIM_N):
                        if i < c_count:
                            # Extract data from C payload
                            r, g, b = raw_payload[i*3 : i*3+3]
                            color = f'#{r:02x}{g:02x}{b:02x}'
                        else:
                            # Case 1: C has fewer LEDs than Sim. Turn rest black.
                            color = 'black'
                        
                        self.update_ui(i, color)

    def update_ui(self, index, color):
        self.root.after(0, lambda: self.canvas.itemconfig(self.leds[index], fill=color))

root = tk.Tk()
sim = DynamicRGBSim(root)
root.mainloop()