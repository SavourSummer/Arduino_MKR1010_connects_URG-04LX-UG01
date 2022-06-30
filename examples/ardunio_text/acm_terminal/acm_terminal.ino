#include <cdcacm.h>
#include <SPI.h>
#include <WiFiNINA.h>

#include "pgmstrings.h"

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

  LINE_CODING	lc;
  lc.dwDTERate	= 19200;
  lc.bCharFormat	= 0;
  lc.bParityType	= 0;
  lc.bDataBits	= 8;

  rcode = pacm->SetLineCoding(&lc);

  if (rcode)
    ErrorMessage<uint8_t>(PSTR("SetLineCoding"), rcode);

  return rcode;
}




char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192,168,3,18); //欲访问的地址
uint16_t serverPort =54768;         //服务器端口号

//USBHost usb;//初始化usb

WiFiClient client; //声明一个客户端对象，用于与服务器进行连接



USBHost     UsbH;
//USBHub     Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           AcmSerial(&UsbH, &AsyncOper);

void setup()
{
  SerialDebug.begin( 19200 );
  SerialDebug.println("Start");

  if (UsbH.Init())
    SerialDebug.println("USB host failed to initialize");

  delay( 200 );
  SerialDebug.println("USB Host init OK");
  WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }
    if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
      client.print("Hello world!");   
    }
}

void loop()
{
  
  UsbH.Task();
  
  if( AcmSerial.isReady()) {
    uint8_t rcode;
    int bytesIn;
    char buf[64];

    /* reading the keyboard */
    if((bytesIn = SerialDebug.available()) > 0) {
      bytesIn = SerialDebug.readBytes(buf, min(bytesIn, sizeof(buf)));
      if (bytesIn > 0) {
        /* sending to USB CDC ACM */
        rcode = AcmSerial.SndData(strlen(buf), (uint8_t*)buf);
        
        if (rcode)
          ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
      }
    }

    /* reading USB CDC ACM */
    /* buffer size must be greater or equal to max.packet size */
    /* it it set to 64 (largest possible max.packet size) here, can be tuned down
       for particular endpoint */
    uint16_t rcvd = sizeof(buf);
    rcode = AcmSerial.RcvData(&rcvd, (uint8_t *)buf);
    
    if (rcode && rcode != USB_ERRORFLOW)
      ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

    if( rcvd ) { //more than zero bytes received
      SerialDebug.write(buf, rcvd);
    }
  }
    
    if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
      client.print("Hello world!");   
    while (client.connected() || client.available()) //如果已连接或有收到的未读取的数据
        {
          client.write(rcode);
          
    }
    else
    {
        //Serial.println("访问失败");
        client.stop(); //关闭客户端
    }
}
