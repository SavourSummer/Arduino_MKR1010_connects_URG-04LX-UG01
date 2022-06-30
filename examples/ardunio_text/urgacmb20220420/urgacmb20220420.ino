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




char ssid[] = "vision";
char pass[] = "vision422";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192, 168, 1, 102); //欲访问的地址
uint16_t serverPort = 54768;        //服务器端口号



const uint16_t szBuf = 4000, szPack = 64;
uint8_t buf[szBuf];


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
  uint8_t rcode;
  uint16_t rcvd, cnt;

  UsbH.Task();
  while (Acm.isReady())
  {

    //client.print("step 1");
    if (client.available())
    {
      String line = client.readString();
      //client.write(line.c_str());
      /* sending to the phone */
      rcode = Acm.SndData(line.length(), (uint8_t*) line.c_str());
      //client.print("step 2");
      //delay(30);
      if (rcode)
        ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
    } //if(Serial.available()...

    //static uint8_t buf[66];
    //      const uint16_t szBuf = 1024;
    //      uint8_t buf[szBuf];
    //szPack = 64;
    rcvd = szPack;
    rcode = Acm.RcvData(&rcvd, buf);
    uint16_t idx = rcvd;
    if (rcvd > 0)
    {
      int bFinding = 100;
      int p = 0;
      while (bFinding>0)
      {
        --bFinding;
        rcvd = szBuf - idx;
        if (rcvd > szPack) rcvd = szPack;
        rcode = Acm.RcvData(&rcvd, &buf[idx]);
        idx += rcvd;
        while (p < idx-1)
        {
          if (buf[p] == '\n')
          {
            ++p;
            if (buf[p] == '\n')
            {
              bFinding = 0;
              break;
            }
          }
          ++p;
        } // while p
      } // while bFinding
    }
    rcvd = idx;
    //if (rcvd > 0)
    //client.print("step 3");

    cnt = 0;
    //    while (rcvd > 0)
    //    {
    //      if (rcvd > szPack)
    //        client.write(buf + cnt, szPack);
    //      else
    //      {
    //        client.write(buf + cnt, rcvd);
    //        break;
    //      }
    //      cnt += szPack;
    //      rcvd -= szPack;
    //    }

    //      if (rcode && rcode != USB_ERRORFLOW)
    //        ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

    //      rcvd = idx;

    if (rcvd)
    { //more than zero bytes received
      //      for (uint16_t i = 0; i < rcvd; i++)
      //      {
      //        client.write(buf[i]); //printing on the screen
      //      }
      client.write(buf, rcvd);
      //client.print("step 4");
    }
    //delay(10);

  }



}
