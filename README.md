# Arduino_MKR1010_connects_URG-04LX-UG01
Arduino_MKR1010_connects_URG-04LX-UG01
this repository is base on https://github.com/gdsports/USB_Host_Library_SAMD . what I do is add a line in Usb.cpp ,because 
Arduino MKR was missing packet when Hokuyo URG-04LX-UG01 was sending.
![K_(Y7P}G3G49{9I17@HKNT0](https://user-images.githubusercontent.com/76558384/163400838-2b286054-4b6d-492f-bb02-b8bc03d25833.png)

# About example
urgacm can receive complete packet form URG-04LX-UG01,but it sends a charater form buf every time until the number of packet equal 64.
urgacmAP1 sends whole buf to PC.


Pay attention to，it is a client in urgacm but server in urgacmAP and urgacmAP1.
