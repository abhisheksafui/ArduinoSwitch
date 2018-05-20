# ArduinoSwitch

Arduino Library for easy management of digital Switch operations. Lets you create a 
switch with  
  1. input pin, 
  2. switch active mode, and 
  3. callback function that needs to be called if switch is pressed 

After creating a switch completely forget about it and let the ArduinoSwitch::update()
function, that needs to be called in your loop() frequently, manage all your
digital switches. 

It takes care of switch debounce operations and continuous press operations.
Switch press is marked in interrupt and then debounce and other switch operations 
are performed in your loop() using milliseconds. So dont delay your loop more than
your switch debounce time.

ACTIVE_HIGH switches MUST be pulled down by external resistor. 
For ACTIVE_LOW switches you may skip as internal pullup is used.

Required Libraries:

https://github.com/abhisheksafui/ArduinoList.git
