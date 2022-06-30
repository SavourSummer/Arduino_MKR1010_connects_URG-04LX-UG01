#include <cdcacm.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <stdio.h>
#include <usbhub.h>
#include "pgmstrings.h"

// On SAMD boards where the native USB port is also the serial console, use
// Serial1 for the serial console. This applies to all SAMD boards except for
// Arduino Zero and M0 boards.
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
#define SerialDebug SERIAL_PORT_MONITOR
#else
#define SerialDebug Serial1
#endif

#define Is_uhd_in_received0(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT0) == USB_HOST_PINTFLAG_TRCPT0)
#define Is_uhd_in_received1(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT1) == USB_HOST_PINTFLAG_TRCPT1)
#define uhd_ack_in_received0(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT0
#define uhd_ack_in_received1(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT1
#define uhd_byte_count0(p)                        usb_pipe_table[p].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT
#define uhd_byte_count1(p)                        usb_pipe_table[p].HostDescBank[1].PCKSIZE.bit.BYTE_COUNT
#define Is_uhd_in_ready0(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_BK0RDY) == USB_HOST_PSTATUS_BK0RDY)  
#define uhd_ack_in_ready0(p)                       USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY
#define Is_uhd_in_ready1(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_BK1RDY) == USB_HOST_PSTATUS_BK1RDY)  
#define uhd_ack_in_ready1(p)                       USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK1RDY
#define uhd_current_bank(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_CURBK) == USB_HOST_PSTATUS_CURBK)  
#define Is_uhd_toggle(p)                          ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_DTGL) == USB_HOST_PSTATUS_DTGL)  
#define Is_uhd_toggle_error0(p)                   usb_pipe_table[p].HostDescBank[0].STATUS_PIPE.bit.DTGLER
#define Is_uhd_toggle_error1(p)                   usb_pipe_table[p].HostDescBank[1].STATUS_PIPE.bit.DTGLER



uint8_t bufBk0[64] __attribute__ ((aligned (4))); //Bank0
uint8_t bufBk1[64] __attribute__ ((aligned (4))); //Bank1
bool doPipeConfig = false;
bool usbConnected = false;


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
USBHub     Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           Acm(&UsbH, &AsyncOper);

void setup()
{
   SerialDebug.begin(9600);
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

USB_SetHandler(&CUSTOM_UHD_Handler);

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


/*
void pipeConfig(uint32_t addr, uint32_t epAddr) {
    SerialDebug.println("pipeConfig called");
    //UHD_Pipe_Alloc Pipe Configuration Datasheet 31.8.6
    //Note usb_pipe_table and DESCADDR done by UHD_Init() already.
    USB->HOST.HostPipe[epAddr].PCFG.reg = 0; //clear
    USB->HOST.HostPipe[epAddr].PCFG.bit.PTYPE = 0x3; //pipe enabled as BULK and frozen
    USB->HOST.HostPipe[epAddr].PCFG.bit.BK = USB_HOST_NB_BK_1; //dual bank
    USB->HOST.HostPipe[epAddr].BINTERVAL.reg = 0x01;//Zero here caused bus resets.
    USB->HOST.HostPipe[epAddr].PCFG.bit.PTOKEN = USB_HOST_PCFG_PTOKEN_IN;
    USB->HOST.HostPipe[epAddr].PSTATUSSET.reg  = USB_HOST_PSTATUSSET_BK0RDY;
    //Not sure why this is here. It's in UHD_Init() as well. Maybe to clear it.
    memset((uint8_t *)&usb_pipe_table[epAddr], 0, sizeof(usb_pipe_table[epAddr]));
    usb_pipe_table[epAddr].HostDescBank[0].CTRL_PIPE.bit.PDADDR = addr;
    usb_pipe_table[epAddr].HostDescBank[0].CTRL_PIPE.bit.PEPNUM = epAddr;
    usb_pipe_table[epAddr].HostDescBank[0].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
    //End Pipe Configuration
}
*/
void CUSTOM_UHD_Handler(void)
{
  uint32_t epAddr = Acm.GetEpAddress();
  if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DCONN) {
    
    doPipeConfig = true;
    usbConnected = true;
  } else if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DDISC) {
    
    usbConnected = false;
    USB->HOST.HostPipe[epAddr].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
  UHD_Handler();
  uhd_freeze_pipe(epAddr);

  //Both banks full and bank1 is oldest, so process first. 
  if (Is_uhd_in_received0(epAddr) && Is_uhd_in_received1(epAddr) && uhd_current_bank(epAddr)) {
    handleBank1(epAddr);
  }
  if(Is_uhd_in_received0(epAddr)) {
    handleBank0(epAddr);
  } 
  if (Is_uhd_in_received1(epAddr)) {
    handleBank1(epAddr);
  }
  uhd_unfreeze_pipe(epAddr);    
}
void handleBank0(uint32_t epAddr){
  int rcvd = uhd_byte_count0(epAddr);
  String dataString = "";
  for (int i = 0; i < rcvd; i++) {
   //for regular MIDI searching for nonzero in the data and then
   //reading in chunks of four bytes seems to work well. 
   //Sysex would require a different strategy though.
    if (bufBk0[i] > 0) {
      dataString = String(bufBk0[i],HEX);
      i++;
      dataString = dataString + String(bufBk0[i],HEX);
      i++;
      dataString = dataString + String(bufBk0[i],HEX);
      i++;
      dataString = dataString + String(bufBk0[i],HEX);
      client.print(dataString);
      client.print("|");
    }
  }      
  if (dataString != "") {
    client.println("");
  }
  uhd_ack_in_received0(epAddr);
  uhd_ack_in_ready0(epAddr);
}

void handleBank1(uint32_t epAddr){
  int rcvd = uhd_byte_count1(epAddr);
  String dataString = "";
  for (int i = 0; i < rcvd; i++) {
    if (bufBk1[i] > 0) {
      dataString = String(bufBk1[i],HEX);
      i++;
      dataString = dataString + String(bufBk1[i],HEX);
      i++;
      dataString = dataString + String(bufBk1[i],HEX);
      i++;
      dataString = dataString + String(bufBk1[i],HEX);
      client.print(dataString);
      
    }
  } 
  if (dataString != "") {
   client.println("");
  }
  uhd_ack_in_received1(epAddr);
  uhd_ack_in_ready1(epAddr);
}
void loop()
{
  
    
    if (doPipeConfig || (!usbConnected && (UsbH.getUsbTaskState() != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE))) {
    UsbH.Task();
  } else if (usbConnected && (UsbH.getUsbTaskState() != USB_STATE_RUNNING)){
    UsbH.Task();
  }
    if (usbConnected && (UsbH.getUsbTaskState() == USB_STATE_RUNNING) )  {
      uint8_t rcode;

      if (client.available()) {
        String line = client.readString();
        client.write(line.c_str());
        /* sending to the phone */
        rcode = Acm.SndData(line.length(), (uint8_t*) line.c_str());
        if (rcode)
          ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
      } //if(Serial.available()...
 
      if (doPipeConfig) {
      //delay(50);
      uint32_t epAddr = Acm.GetEpAddress();
      static uint8_t buf[256];
      uint16_t rcvd = 256;
      doPipeConfig = false;
      while (usbConnected && (USB->HOST.HostPipe[Acm.GetEpAddress()].PCFG.bit.PTYPE != 0x03)) {
          UsbH.Task(); 
      rcode = Acm.RcvData(&rcvd, buf);
      if (rcode && rcode != USB_ERRORFLOW)
        ErrorMessage<uint8_t>(PSTR("Ret"), rcode);
        
      if (rcvd) { //more than zero bytes received
        for (uint16_t i = 0; i < rcvd; i++) {
          client.write((char) buf[i]); //printing on the screen
        }
      }
      
      }
        /*usb_pipe_table[epAddr].HostDescBank[0].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
        usb_pipe_table[epAddr].HostDescBank[1].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
        usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg = (uint32_t)bufBk0;
        usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg = (uint32_t)bufBk1; 
        */
      //delay(10);
   
    USB->HOST.HostPipe[epAddr].BINTERVAL.reg = 0x01;
        usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg = (uint32_t)bufBk0;
        usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg = (uint32_t)bufBk1;
    
    usb_pipe_table[epAddr].HostDescBank[0].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
    usb_pipe_table[epAddr].HostDescBank[1].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
     USB->HOST.HostPipe[epAddr].PCFG.bit.PTOKEN = tokOUT;
        USB->HOST.HostPipe[epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY; 
        uhd_unfreeze_pipe(epAddr); //launch the transfer
        USB->HOST.HostPipe[epAddr].PINTENSET.reg = 0x3; //Enable pipe interrupts
    
    //End Pipe Configuration
    }
    }
    else {
    USB_SetHandler(&CUSTOM_UHD_Handler);
    USB->HOST.HostPipe[Acm.GetEpAddress()].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
    }
  
  
}
