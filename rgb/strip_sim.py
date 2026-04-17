import tkinter as tk
import subprocess
import threading
import math

# Configuration
SIM_N = 81 
LED_RADIUS = 5
SPACING = 15  # Uniform distance between all LEDs

# Calculate Circle Radius based on spacing to keep it uniform with the tail
# We want 40 segments (from 41 to 81) to cover ~320 degrees
# Radius = (Total Arc Length) / (Arc Angle in Radians)
ARC_ANGLE = 1.8 * math.pi 
CIRCLE_RADIUS = (40 * SPACING-5) / ARC_ANGLE

class SigmaSim:
    def __init__(self, root):
        self.root = root
        self.root.title("Sigma LED Strip Simulator")
        
        # Dynamic window size
        self.canvas = tk.Canvas(root, width=900, height=500, bg='#111')
        self.canvas.pack()
        
        self.leds = []
        self.setup_shape()
        
        # Start C process
        self.process = subprocess.Popen(['./logic'], stdout=subprocess.PIPE)
        threading.Thread(target=self.read_data, daemon=True).start()

    def setup_shape(self):
        # Junction point where index 40 and the start of circle (41) meet
        junc_x = 350 
        junc_y = 150

        # 1. Straight Line (0 to 40) -> 41 LEDs
        for i in range(41):
            x = junc_x + (40 - i) * SPACING
            y = junc_y
            self.leds.append(self.create_led(x, y))

        # 2. Circle Loop (41 to 80) -> 40 LEDs
        # Center is below the junction
        center_x = junc_x
        center_y = junc_y + CIRCLE_RADIUS
        
        start_angle = -math.pi / 2 # Top of the circle
        
        for i in range(40):
            # Calculate angle for LEDs 41 through 80
            # Dividing by 40 ensures index 80 is the last physical LED
            angle = start_angle - (i / 40) * ARC_ANGLE
            
            x = center_x + CIRCLE_RADIUS * math.cos(angle)
            y = center_y + CIRCLE_RADIUS * math.sin(angle)
            
            self.leds.append(self.create_led(x, y))

    def create_led(self, x, y):
        return self.canvas.create_oval(
            x - LED_RADIUS, y - LED_RADIUS, 
            x + LED_RADIUS, y + LED_RADIUS, 
            fill='black', outline='#444'
        )

    def read_data(self):
        while True:
            byte = self.process.stdout.read(1)
            if byte == b'S':
                count_byte = self.process.stdout.read(1)
                if not count_byte: break
                c_count = count_byte[0]
                
                # Read exactly what C sends
                raw_payload = self.process.stdout.read(c_count * 3)
                if len(raw_payload) == c_count * 3:
                    # Update only the LEDs we have data for
                    for i in range(len(self.leds)):
                        if i < c_count:
                            r, g, b = raw_payload[i*3 : i*3+3]
                            color = f'#{r:02x}{g:02x}{b:02x}'
                        else:
                            color = 'black'
                        self.update_ui(i, color)

    def update_ui(self, index, color):
        self.root.after(0, lambda: self.canvas.itemconfig(self.leds[index], fill=color))

if __name__ == "__main__":
    root = tk.Tk()
    sim = SigmaSim(root)
    root.mainloop()