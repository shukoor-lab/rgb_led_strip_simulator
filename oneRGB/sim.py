import tkinter as tk
import subprocess
import threading

class RGBSim:
    def __init__(self, root):
        self.root = root
        self.root.title("AVR RGB Simulator")
        self.canvas = tk.Canvas(root, width=300, height=300, bg='#222')
        self.canvas.pack()
        
        # The Virtual RGB LED
        self.led = self.canvas.create_oval(100, 100, 200, 200, fill='black', outline='white')
        
        # Start your compiled C program
        # Note: Use 'logic.exe' on Windows or './logic' on Mac/Linux
        self.process = subprocess.Popen(['./logic'], stdout=subprocess.PIPE)
        
        threading.Thread(target=self.read_data, daemon=True).start()

    def read_data(self):
        while True:
            # Look for the 'C' header
            byte = self.process.stdout.read(1)
            if byte == b'C':
                # Read exactly 3 bytes for R, G, and B
                rgb = self.process.stdout.read(3)
                if len(rgb) == 3:
                    r, g, b = rgb[0], rgb[1], rgb[2]
                    color_hex = f'#{r:02x}{g:02x}{b:02x}'
                    
                    # Update the GUI safely from the thread
                    self.root.after(0, lambda c=color_hex: self.canvas.itemconfig(self.led, fill=c))

root = tk.Tk()
sim = RGBSim(root)
root.mainloop()