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
uint16_t usblen;
char* hokuyo_data = (char*) malloc(2200);
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
  lc.dwDTERate  = 19200;
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

const IPAddress serverIP(192,168,137,1); //欲访问的地址
uint16_t serverPort = 54768;        //服务器端口号



WiFiClient client; //声明一个客户端对象，用于与服务器进行连接



USBHost     UsbH;
//USBHub     Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           Acm(&UsbH, &AsyncOper);

void setup()
{
  SerialDebug.begin( 19200 );
  SerialDebug.println("Start");

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
  client.connect(serverIP, serverPort); //若是没有连接上延迟100毫秒后连接
  client.print("Hello world!");

}






void usb_setup() {
  if (UsbH.Init() == -1)
    Serial.println(PSTR("OSCOKIRQ failed to assert"));

  delay(200);
}
//修改了usb_send的 const char
void usb_send(const char* data, uint16_t len) {
  while (!Acm.isReady()) {
    UsbH.Task();
    delay(20);
  };

  uint8_t rcode;
  if (Acm.isReady()) {
    //    Serial.print("send ");
    //    Serial.println(data);
    delay(100);
    rcode = Acm.SndData(len, (uint8_t*) data);
    if (rcode)
      ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
    delay(100);
  }
}

void usb_receive(char* buf, uint16_t* rcvd) {
  while (!Acm.isReady()) {
    UsbH.Task();
    delay(20);
  };

  uint8_t rcode;
  if (Acm.isReady()) {
    rcode = Acm.RcvData(rcvd, (uint8_t*) buf);
    if (rcode && rcode != 0x04)
      ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

     // for (uint16_t i = 0; i < *rcvd; i++) {
     //       client.print((char)buf[i]);
     //   }

    //    Serial.print("received ");
    //    Serial.println(*rcvd);
    //    for (uint16_t i = 0; i < *rcvd; i++) {
    //      Serial.print((char) buf[i]);
    //    }
    //    Serial.println("");
  }
}

void hokuyo_on() {
  usb_send("BM\n", 3);
  usblen = 8;
  usb_receive(hokuyo_data, &usblen);
}

void hokuyo_off() {
  usb_send("QT\n", 3);
  usblen = 8;
  usb_receive(hokuyo_data, &usblen);
}

//#define GD_SEGMENT_LEN 64*34 //must not be larger than HOKUYO_DATA_LENGTH
#define GD_SEGMENT_LEN 64*34 //must not be larger than HOKUYO_DATA_LENGTH
/*void hokuyo_capture_send() {
  bool reach_end = false;
  usb_send("GD0044072501\n", 13);

  do {
    usblen = GD_SEGMENT_LEN;
    usb_receive(hokuyo_data, &usblen);

    if (usblen == 0) {
      continue;
    } else if (usblen < GD_SEGMENT_LEN) {
      reach_end = true;
    }

  //    dataptr = (const unsigned char*) hokuyo_data;
  //    wifi_run(1);

    wifi_send(hokuyo_data, usblen);
    delay(100);
  } while (!reach_end);

  //  memset(hokuyo_data, 0, 2200);

  }*/






void loop()
{

  while (!client.connected()) //连接WiFi,若是没有连接继续。
  {
    client.connect(serverIP, serverPort);//若是没有连接上延迟100毫秒后连接
    client.print("Hello world!");
    delay(100);

  }
  if (client.connected())
  {
    while (client.available()) //若是接受到服务端发送的命令，当然也可以一个一个字符接受然后发送，雷达可识别
    {
      String line = client.readString();
      client.write(line.c_str());
      usb_send(line.c_str(), line.length());


      bool reach_end = false;

      do {
        usblen = 300;
        usb_receive(hokuyo_data, (uint16_t)3000);
        delay(40);
        if (usblen == 0) {
          continue;
       } else if (usblen < 3000) {
          reach_end = true;
        }

        //    dataptr = (const unsigned char*) hokuyo_data;
        //    wifi_run(1);

        client.write(hokuyo_data, 300);
        delay(10);
      } while (!reach_end);
    }

  }
}
