import tkinter as tk
import subprocess
import threading

class BlinkSim:
    def __init__(self, root):
        self.root = root
        self.canvas = tk.Canvas(root, width=200, height=200, bg='white')
        self.canvas.pack()
        
        # Draw our "Virtual LED"
        self.led = self.canvas.create_oval(50, 50, 150, 150, fill='gray')
        
        # Start the C program (make sure to compile it to 'blink.exe' or './blink' first)
        self.process = subprocess.Popen(['./blink'], stdout=subprocess.PIPE, text=True)
        
        # Start a thread to read the pipe without freezing the GUI
        threading.Thread(target=self.read_pipe, daemon=True).start()

    def read_pipe(self):
        for line in iter(self.process.stdout.readline, ''):
            command = line.strip()
            if command == "LED_ON":
                self.root.after(0, lambda: self.canvas.itemconfig(self.led, fill='red'))
            elif command == "LED_OFF":
                self.root.after(0, lambda: self.canvas.itemconfig(self.led, fill='gray'))

root = tk.Tk()
sim = BlinkSim(root)
root.mainloop()