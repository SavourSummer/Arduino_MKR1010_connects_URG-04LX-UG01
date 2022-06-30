# Arduino_MKR1010_connects_URG-04LX-UG01
## Description
Arduino MKR1010 as USB host connects URG-04LX-UG01 with OTG cable.
If you want more details ，the paper can provide help.
this repository is base on https://github.com/gdsports/USB_Host_Library_SAMD . what I do is add a line in Usb.cpp ,because 
Arduino MKR was missing packet when Hokuyo URG-04LX-UG01 was sending.
   ![K_(Y7P}G3G49{9I17@HKNT0](https://user-images.githubusercontent.com/76558384/163400838-2b286054-4b6d-492f-bb02-b8bc03d25833.png)

## About example
urgacm can receive complete packet form URG-04LX-UG01,but it sends a charater form buf every time until the number of packet equal 64.
   urgacmAP1 sends whole buf to PC.
the urgacmb20220421_servertext_DEMO.mp4 is recorded by urgacmb20220421_servertext program and UrgLidarTester.

UrgLidarTester is coded by jingyu Lin ，the professor of Guangdong University of Technology .

   Pay attention to，it is a client in urgacm but server in urgacmAP and urgacmAP1.
## How to use
### First
   The +5V pin of Arduino MKR 1010 should connect +5 sourse , and the GND pin connect GND. Just do this and you should premise the voltage of +5V pin is 5V at least when MKR is connecting URG-04LX-UG01,otherwise it can't connect WiFi or receive data of URG. 
      ![170042574f835dfae6c6e260403b7a5](https://user-images.githubusercontent.com/76558384/163567429-cf72a55b-7517-4054-92b4-cf4f0116438e.jpg)

### Second
   Upload example to MKR.If MKR is USBhost,you should click Reset button twice then the yellow LED of MKR flashes . In this time ,you can upload code when set correct port.
### Finally
   Set correct WiFi ssid and pass ,sure ,server need net port.
