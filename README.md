#### Esp 8266 rc rx 2.4G 8 channel receiver
Uses EEE 802.11 Probe Response Packets to receive Multiwii Serial Protocol http://www.multiwii.com/wiki/index.php?title=Multiwii_Serial_Protocol 
MSP_SET_RAW_RC commands. Wifi network works in promiscuous mode allowing unassociated low latency transmission for best possible delivery and range.
Receiver forwards MSP messages to serial port to be consumed by flight controller.
See Esp 8266 rc tx for transmitter part.  
##### Hardware
1. esp8266 Esp-07 module with external antenna connection
##### Confirmed range
So far tested with 1/2 dipole antenna
~600m in clear sight with no packet loss
~400m in flight.
  
 
 