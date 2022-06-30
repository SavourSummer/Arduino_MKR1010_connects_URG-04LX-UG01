#include "cdcftdi.h"
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
#define PIN_USB_HOST_ENABLE

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192,168,3,18); //欲访问的地址
uint16_t serverPort =54768;         //服务器端口号
WiFiClient client; //声明一个客户端对象，用于与服务器进行连接
class FTDIAsync : public FTDIAsyncOper
{
public:
    uint8_t OnInit(FTDI *pftdi);
};

uint8_t FTDIAsync::OnInit(FTDI *pftdi)
{
    uint8_t rcode = 0;

    rcode = pftdi->SetBaudRate(115200);

    if (rcode)
    {
        ErrorMessage<uint8_t>(PSTR("SetBaudRate"), rcode);
        return rcode;
    }
    rcode = pftdi->SetFlowControl(FTDI_SIO_DISABLE_FLOW_CTRL);

    if (rcode)
        ErrorMessage<uint8_t>(PSTR("SetFlowControl"), rcode);

    return rcode;
}

USBHost          UsbH;
//USBHub         Hub(&UsbH);
FTDIAsync        FtdiAsync;
FTDI             Ftdi(&UsbH, &FtdiAsync);

void setup()
{
  SerialDebug.begin( 115200 );
  SerialDebug.println("Start");

  if (UsbH.Init())
      SerialDebug.println("USB host did not start.");

  delay( 200 );
  WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }
}

void loop()
{
    UsbH.Task();
    
    if( UsbH.getUsbTaskState() == USB_STATE_RUNNING )
    {
        if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
      client.print("Hello world!");                    //向服务器发送数据
        while (client.connected() || client.available()) //如果已连接或有收到的未读取的数据
        {
        uint32_t  rcode;
        char strbuf[] = "PP\n";
        //char strbuf[] = "The quick brown fox jumps over the lazy dog";
        //char strbuf[] = "This string contains 61 character to demonstrate FTDI buffers"; //add one symbol to it to see some garbage
       

        rcode = Ftdi.SndData(strlen(strbuf), (uint8_t*)strbuf);

  if (rcode)
            ErrorMessage<uint8_t>(PSTR("SndData"), rcode);

        delay(50);

        uint8_t  buf[64];

        for (uint8_t i=0; i<64; i++)
            buf[i] = 0;

        uint8_t rcvd = 64;
        rcode = Ftdi.RcvData(&rcvd, buf);
        client.write(rcode);
        if (rcode && rcode != USB_ERRORFLOW)
            ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

        // The device reserves the first two bytes of data
        //   to contain the current values of the modem and line status registers.
        if (rcvd > 2)
            SerialDebug.print((char*)(buf+2));

        delay(10);
    }
    }
    }
}
