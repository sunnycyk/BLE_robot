BLE_robot
=========

Arduino compatible program for [RedBearLab BLE Nano](http://redbearlab.com/blenano/) for controlling [Rapiro](http://www.rapiro.com) with other BLE Device(e.g. mobile device)

UUID Setting
============
Service UUID:                          "713D0000-503E-4C75-BA94-3148F18D941E"

Characterstic RX UUID:                 "713D0002-503E-4C75-BA94-3148F18D941E"

Characterstic TX UUID:                 "713D0003-503E-4C75-BA94-3148F18D941E"

Program
=======
Basically, the program will forward any bytes that received from BLE to Serial connection, 
so you can send anything to RedBearLab BLE Nano and works on any device which accepts serial commands.  For Rapiro with standard firmware, the basic commands are #M0 - #M9.   (Please note that the buffer size is limited so if you are working for any command string that is longer than the buffer size, you may have to adjust it in the code)


