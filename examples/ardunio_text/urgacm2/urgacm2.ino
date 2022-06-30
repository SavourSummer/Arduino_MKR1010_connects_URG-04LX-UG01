#include <usbhub.h>
#include <cdcacm.h>
#include <SPI.h>
#include <WiFiNINA.h>
#define SerialDebug Serial1

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
USBHost UsbH;
USBHub Hub(&UsbH);
ACMAsyncOper  AsyncOper;
ACM           Acm(&UsbH, &AsyncOper);
char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192, 168, 137, 1); //欲访问的地址
uint16_t serverPort = 54768;        //服务器端口号
WiFiClient client;
bool doPipeConfig = false;
bool usbConnected = false;

//SAMD21 datasheet pg 836. ADDR location needs to be aligned. 
uint8_t bufBk0[64] __attribute__ ((aligned (4))); //Bank0
uint8_t bufBk1[64] __attribute__ ((aligned (4))); //Bank1

void setup()
{
  //bFirst = true;
  //vid = pid = 0;
  SerialDebug.begin(115200);
  if (UsbH.Init()) {
    
    while (1); //halt
  }
  USB_SetHandler(&CUSTOM_UHD_Handler);
  delay( 200 );

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

  
}

void loop()
{
  //Note that Task() polls a hub if present, and we want to avoid polling.
  //So these conditions carry out enumeration only, and then stop running.
  //The idea is that except for enumeration (and release) this loop should 
  //be quiescent. 
  if (doPipeConfig || (!usbConnected && (UsbH.getUsbTaskState() != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE))) {
    UsbH.Task();
  } else if (usbConnected && (UsbH.getUsbTaskState() != USB_STATE_RUNNING)){
    UsbH.Task();
  }
  
  if (usbConnected && (UsbH.getUsbTaskState() == USB_STATE_RUNNING) ) {
    
      if (doPipeConfig) {
        //There is a chance that a disconnect interrupt may happen in the middle of this
        //and result in instability. Various tests here on usbConnected to hopefully
        //reduce the chance of it.

         uint8_t rcode;
  if (client.available()) {
        String line = client.readString();
        client.write(line.c_str());
        /* sending to the phone */
        rcode = Acm.SndData(line.length(), (uint8_t*) line.c_str());
        if (rcode)
          ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
      }
        
        uint32_t epAddr = Acm.GetEpAddress();
        doPipeConfig = false;
        uint16_t rcvd;
        while (usbConnected && (USB->HOST.HostPipe[Acm.GetEpAddress()].PCFG.bit.PTYPE != 0x03)) {
          UsbH.Task(); 
          Acm.RcvData(&rcvd,  bufBk0);
                if (rcvd) { //more than zero bytes received
        
      }
          
        }
        USB->HOST.HostPipe[epAddr].BINTERVAL.reg = 0x01;//Zero here caused bus resets.
        usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg = (uint32_t)bufBk0;
        usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg = (uint32_t)bufBk1;
        usb_pipe_table[epAddr].HostDescBank[0].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES;
        usb_pipe_table[epAddr].HostDescBank[1].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_256_BYTES; 
        USB->HOST.HostPipe[epAddr].PCFG.bit.PTOKEN = tokIN;
        USB->HOST.HostPipe[epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY; 
        uhd_unfreeze_pipe(epAddr); //launch the transfer
        USB->HOST.HostPipe[epAddr].PINTENSET.reg = 0x3; //Enable pipe interrupts

        
      }
    
  } else {
    USB_SetHandler(&CUSTOM_UHD_Handler);
    USB->HOST.HostPipe[Acm.GetEpAddress()].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
}


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


void handleBank0(uint32_t epAddr){
  int rcvd = uhd_byte_count0(epAddr);
  String dataString = "";
  for (int i = 0; i < rcvd; i++) {
   //for regular MIDI searching for nonzero in the data and then
   //reading in chunks of four bytes seems to work well. 
   //Sysex would require a different strategy though.
    if (bufBk0[i] > 0) {
      dataString = String(bufBk0[i]);
      i++;
      dataString = dataString + String(bufBk0[i]);
      i++;
      dataString = dataString + String(bufBk0[i]);
      i++;
      dataString = dataString + String(bufBk0[i]);
      client.print(dataString);
      client.print("|");
    }
  }      
  if (dataString != "") {
    SerialDebug.println("");
  }
  uhd_ack_in_received0(epAddr);
  uhd_ack_in_ready0(epAddr);
}

void handleBank1(uint32_t epAddr){
  int rcvd = uhd_byte_count1(epAddr);
  String dataString = "";
  for (int i = 0; i < rcvd; i++) {
    if (bufBk1[i] > 0) {
      dataString = String(bufBk1[i]);
      i++;
      dataString = dataString + String(bufBk1[i]);
      i++;
      dataString = dataString + String(bufBk1[i]);
      i++;
      dataString = dataString + String(bufBk1[i]);
      client.print(dataString);
      client.print("|");
    }
  } 
  if (dataString != "") {
    client.println("");
  }
  uhd_ack_in_received1(epAddr);
  uhd_ack_in_ready1(epAddr);
}
