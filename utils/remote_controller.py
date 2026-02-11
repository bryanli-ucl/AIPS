import os
from pynput import keyboard
import numpy as np

def on_press(key) -> None:
    if hasattr(key, "char"):
        print(f"Pressed: {key.char}")
    else:
        print(f"Pressed Unknown {key}")

with keyboard.Listener(on_press=on_press) as listener:
    listener.join()

