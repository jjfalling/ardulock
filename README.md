ArduLock
========

ArduLock takes input from a 4x4 keypad, gives feedback using a rgb led and uses a relay or servo to open a door. 
The pin is stored in eeprom and can be changed using the keypad.


Instructions.

When the device is ready to use, the rgb led will flash red-green-blue-white(ish). The led will be red when its ready to accept input. 

To unlock, enter pin and press *. Light will turn green if the correct pin is entered or flash red if not. 
The default pin is 12345.

Press # to clear the temp memory. Light will flash green once.

To change pin, enter the current pin followed by A. The led will turn blue. Enter a new pin of up to 10 numbers followed by * (do not use 10 0s). If the pin was successfully change the led will flash blue before turning red. 


If you forget your pin, your will need to clear the eeprom. The eeprom_clear example provided with the Arduino IDE will do the trick. 