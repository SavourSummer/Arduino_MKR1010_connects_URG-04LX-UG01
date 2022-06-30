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

WiFiServer server(10940);
//WiFiClient client; //声明一个客户端对象，用于与服务器进行连接

const uint16_t szBufAcm = 4000, szPack = 66, szBufWifi = 128;
uint8_t buf_acm[szBufAcm], buf_wifi[szBufWifi]; // 保存USB和wifi接收的数据

USBHost     UsbH;
//USBHub     Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           Acm(&UsbH, &AsyncOper);

void setup()
{
  if (WiFi.status() == WL_NO_MODULE) 
  {
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {  }
  while(true)
  {
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED)
    {
      break;
      //Serial.print(".");
    }
    delay(100);
  }
  server.begin();
  
  // initial USB
  UsbH.Init();
  delay( 200 );
  UsbH.Task();

  while (!Acm.isReady())
  {
    UsbH.Task();
    delay(50);
  }

}

void loop()
{
  uint8_t rcode;
  uint16_t rcvd, cnt;
  
  WiFiClient client = server.available(); // listen for incoming clients
  while (client && client.connected())
  {
	// read wifi data
    if (client.available())
    {
      int rcvd_wifi = client.read(buf_wifi, szBufWifi);
      Acm.SndData(rcvd_wifi, buf_wifi);
      //delay(20);
    }
    
    if (!Acm.isReady())
      client.print("USB down!");
    delay(10);
    // read USB data
    rcvd = szPack;
    rcode = Acm.RcvData(&rcvd, buf_acm);
    uint16_t idx = rcvd;
    if (rcvd > 0)
    {
      unsigned long ts = millis(); // starting time
      int p = 0; 
      while (millis() - ts < 100)
      {
        rcvd = szBufAcm - idx;
        if (rcvd > szPack) rcvd = szPack;
        rcode = Acm.RcvData(&rcvd, &buf_acm[idx]);
//        if (rcvd <= 0)
//          continue;
        idx += rcvd;
        while (p < idx - 1)
        {
          if (buf_acm[p] == '\n')
          {
            ++p;
            if (buf_acm[p] == '\n')
            {
              client.println("double n found!");
              ts -= 100;
              break;
            }
          }
          ++p;
        } // while p
      } // while bFinding

      // send wifi
      rcvd = idx;
      //client.println(rcvd);
      int nWrite = client.write(buf_acm, rcvd);
      //client.println(nWrite);
    } // rcvd > 0
  } // client.connected()
}//loop
