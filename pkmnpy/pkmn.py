from pynput.keyboard import Key, Controller

keyboard = Controller()

keyboard.press('z')
keyboard.release('z')