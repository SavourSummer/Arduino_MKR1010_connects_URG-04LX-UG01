



#include <SPI.h>
#include <WiFiNINA.h>
#include <Usb.h>


USBHost usb; //需要包含Usb.h的库才有效，


void setup() {
  // put your setup code here, to run once:
    if(usb.Init())
    delay(200);
    UHD_Init();
    USB_send("BM\n", 3);
}

void loop() {
  // put your main code here, to run repeatedly:
    usb.Task();
    if(usb.getUsbTaskState() == USB_STATE_RUNNING )
    {
        uint32_t  rcode;
        char strbuf[] = "PP\n";
        
        
    }
}
