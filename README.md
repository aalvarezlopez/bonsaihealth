# bonsaihealth
Take care of your bonsai. Measure light level, temperature and soil humedity of your Bonsai and its enviroment. Enable external pump when watering is needed.
# Installation
For the installation pk2cmd should be installed. '''make program'' compile, link a program the board using pk2cmd. Also, 
pickit2 or other programmer should be connected to the board and the board turn on.
#Wifi configuration
Connect the device trhough usb to a PC and open a terminal. Typing 'm' the menu will be print and all available options.

To connect the device to a WLAN and open a socket type 'w' and enter. Now all commands will be send to the ESP8266 and
the reply from ESP8266 also print in this terminal. Next commands should be send to the ESP8266
```
AT+CWMODE=3
AT+CWJAP="ESSID","password"
AT+CIFSR
AT+CIPMUX=1
AT+CIPSERVER=1,80
```
