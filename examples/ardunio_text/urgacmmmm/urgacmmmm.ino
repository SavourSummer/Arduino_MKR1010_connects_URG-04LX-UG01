#include <cdcacm.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <usbhub.h>
#include "pgmstrings.h"
//#include <clocks.h>
// On SAMD boards where the native USB port is also the serial console, use
// Serial1 for the serial console. This applies to all SAMD boards except for
// Arduino Zero and M0 boards.
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
#define SerialDebug SERIAL_PORT_MONITOR
#else
#define SerialDebug Serial1
#endif

class ACMAsyncOper : public CDCAsyncOper
{
  public:
    uint8_t OnInit(ACM *pacm);
};

uint8_t ACMAsyncOper::OnInit(ACM *pacm)
{
  uint8_t rcode;
  // Set DTR = 1 RTS=1
  rcode = pacm->SetControlLineState(3);

  if (rcode)
  {
    ErrorMessage<uint8_t>(PSTR("SetControlLineState"), rcode);
    return rcode;
  }

  LINE_CODING  lc;
  lc.dwDTERate  = 115200;
  lc.bCharFormat  = 0;
  lc.bParityType  = 0;
  lc.bDataBits  = 8;

  rcode = pacm->SetLineCoding(&lc);

  if (rcode)
    ErrorMessage<uint8_t>(PSTR("SetLineCoding"), rcode);

  return rcode;
}




char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192, 168, 137, 1); //欲访问的地址
uint16_t serverPort = 54768;        //服务器端口号



WiFiClient client; //声明一个客户端对象，用于与服务器进行连接



USBHost     UsbH;
//USBHub     Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           Acm(&UsbH, &AsyncOper);

void setup()
{
  // SerialDebug.begin(19200);
  // SerialDebug.println("Start");

  if (UsbH.Init())
    //SerialDebug.println("USB host failed to initialize");

    delay( 200 );
  //SerialDebug.println("USB Host init OK");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    //Serial.print(".");
  }



if (client.connect(serverIP, serverPort)) //若是没有连接上延迟100毫秒后连接
  {
    client.print("Hello world!");
  }

    else
  {
    client.stop();
  }
  
  //client.connect(serverIP, serverPort); //若是没有连接上延迟100毫秒后连接
  //client.print("Hello world!");

}


void loop()
{
  
    
    UsbH.Task();
    if (Acm.isReady()) {
      uint8_t rcode;

      if (client.available()) {
        String line = client.readString();
        //client.write(line.c_str());
        /* sending to the phone */
        rcode = Acm.SndData(line.length(), (uint8_t*) line.c_str());
        if (rcode)
          ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
      } //if(Serial.available()...

      delay(30);//发送一个命令到接收到第一个包需要30ms

      static uint8_t buf[66];
      uint16_t rcvd = 64;
      rcode = Acm.RcvData(&rcvd, buf);
      if (rcode && rcode != USB_ERRORFLOW)
        ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

      if (rcvd) { //more than zero bytes received
        for (uint16_t i = 0; i < rcvd; i++) {
          client.write((char) buf[i]); //printing on the screen
        }
        //     client.write(buf,128);
      }
      //delay(10);
      
    }
    
  
  
}
