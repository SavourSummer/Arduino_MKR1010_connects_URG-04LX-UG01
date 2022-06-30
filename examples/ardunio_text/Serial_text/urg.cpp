#include "urg.h"

const uint8_t URG::epDataInIndex = 1;
const uint8_t URG::epDataOutIndex = 2;
const uint8_t URG::epInterruptInIndex = 3;
/* constructor */


URG::URG(USBHost *p) :

pUsb(p),
bAddress(0),
bNumEP(1){
        for(uint8_t i = 0; i < 4; i++) 
        {
                epInfo[i].epAddr = 0;
                epInfo[i].maxPktSize = (i) ? 0 : 8;
                epInfo[i].bmSndToggle = 0;
                epInfo[i].bmRcvToggle = 0;
                epInfo[i].bmNakPower = (i==epDataInIndex) ? USB_NAK_NOWAIT: USB_NAK_MAX_POWER;
        }
        if(pUsb)
                pUsb->RegisterDeviceClass(this);
}










uint32_t URG::Init(uint32_t parent, uint32_t port, uint32_t lowspeed) 
{
        const uint8_t constBufSize = sizeof (USB_DEVICE_DESCRIPTOR);

        uint8_t buf[constBufSize];
        USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
        uint8_t rcode;
        UsbDeviceDefinition *p = NULL;
        EpInfo *oldep_ptr = NULL;

        uint8_t num_of_conf; // number of configurations

        AddressPool &addrPool = pUsb->GetAddressPool();

        USBTRACE("URG Init\r\n");

        if(bAddress) {
        USBTRACE("URG CLASS IN USE??\r\n");
                return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
                }
        // Get pointer to pseudo device with address 0 assigned
        p = addrPool.GetUsbDevicePtr(0);

        if(!p) {
        USBTRACE("URG NO ADDRESS??\r\n");
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
                }
        if(!p->epinfo) {
                USBTRACE("epinfo\r\n");
                return USB_ERROR_EPINFO_IS_NULL;
        }

        // Save old pointer to EP_RECORD of address 0
        oldep_ptr = p->epinfo;

        // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
        p->epinfo = epInfo;

        p->lowspeed = lowspeed;

        // Get device descriptor
        rcode = pUsb->getDevDescr(0, 0, sizeof (USB_DEVICE_DESCRIPTOR), buf);

        // Restore p->epinfo
        p->epinfo = oldep_ptr;

        if(rcode) {
                goto FailGetDevDescr;
        }
    

        // Allocate new address according to device class
        bAddress = addrPool.AllocAddress(parent, false, port);

        if(!bAddress)
                return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;

        // Extract Max Packet Size from the device descriptor
        epInfo[0].maxPktSize = udd->bMaxPacketSize0;

        // Assign new address to the device
        rcode = pUsb->setAddr(0, 0, bAddress);

        if(rcode) {
                p->lowspeed = false;
                addrPool.FreeAddress(bAddress);
                bAddress = 0;
                USBTRACE2("setAddr:", rcode);
                return rcode;
        }

        USBTRACE2("Addr:", bAddress);

        p->lowspeed = false;

        p = addrPool.GetUsbDevicePtr(bAddress);

        if(!p)
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

        p->lowspeed = lowspeed;

        num_of_conf = udd->bNumConfigurations;

        // Assign epInfo to epinfo pointer
        rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);

        if(rcode)
                goto FailSetDevTblEntry;

        USBTRACE2("NC:", num_of_conf);

        for(uint8_t i = 0; i < num_of_conf; i++) {
                ConfigDescParser < 0xFF, 0xFF, 0xFF, CP_MASK_COMPARE_ALL> confDescrParser(this);//不同cdcacm，在136行

                // This interferes with serial output, and should be opt-in for debugging.
                //HexDumper<USBReadParser, uint32_t, uint32_t> HexDump;
                //rcode = pUsb->getConfDescr(bAddress, 0, i, &HexDump);
                //if(rcode)
                //        goto FailGetConfDescr;

                rcode = pUsb->getConfDescr(bAddress, 0, i, &confDescrParser);

                if(rcode)
                        goto FailGetConfDescr;

                if(bNumEP > 1)
                        break;
        } // for

        if(bNumEP < 2)
                return USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

        USBTRACE2("NumEP:", bNumEP);

        // Assign epInfo to epinfo pointer
        rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);

        USBTRACE2("Conf:", bConfNum);

        // Set Configuration Value
        rcode = pUsb->setConf(bAddress, 0, bConfNum);

        if(rcode)
                goto FailSetConfDescr;

        

        if(rcode)
                goto FailOnInit;

        USBTRACE("URG configured\r\n");

        ready = true;
        return 0;

FailGetDevDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetDevDescr();
        goto Fail;
#endif

FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
        NotifyFailSetDevTblEntry();
        goto Fail;
#endif

FailGetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetConfDescr();
        goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailSetConfDescr();
        goto Fail;
#endif

FailOnInit:
#ifdef DEBUG_USB_HOST
        USBTRACE("OnInit:");

Fail:
        NotifyFail(rcode);
#endif
        Release();
        return rcode;
}

void URG::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto __attribute__((unused)), const USB_ENDPOINT_DESCRIPTOR *pep) 
{
        ErrorMessage<uint8_t > (PSTR("Conf.Val"), conf);
        ErrorMessage<uint8_t > (PSTR("Iface Num"), iface);
        ErrorMessage<uint8_t > (PSTR("Alt.Set"), alt);

        bConfNum = conf;

        uint8_t index;

        if((pep->bmAttributes & bmUSB_TRANSFER_TYPE) == USB_TRANSFER_TYPE_INTERRUPT && (pep->bEndpointAddress & 0x80) == 0x80)
                index = epInterruptInIndex;
        else if((pep->bmAttributes & bmUSB_TRANSFER_TYPE) == USB_TRANSFER_TYPE_BULK)
                index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
        else
                return;

        // Fill in the endpoint info structure
        epInfo[index].epAddr = (pep->bEndpointAddress & 0x0F);
        epInfo[index].maxPktSize = (uint8_t)pep->wMaxPacketSize;
        epInfo[index].bmSndToggle = 0;
        epInfo[index].bmRcvToggle = 0;

        bNumEP++;

        PrintEndpointDescriptor(pep);
}



uint32_t URG::Release() {
        pUsb->GetAddressPool().FreeAddress(bAddress);

        bAddress = 0;
        bNumEP = 1;
        qNextPollTime = 0;
        bPollEnable = false;
        ready = false;
        return 0;
}

/*int8_t URG::SetBaudRate(uint32_t baud) 
{

       return rv;
}

uint8_t URG::SetData(uint16_t databm) {
        
        return rv;
}*/

void URG::PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr) {
        Notify(PSTR("Endpoint descriptor:"), 0x80);
        Notify(PSTR("\r\nLength:\t\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bLength, 0x80);
        Notify(PSTR("\r\nType:\t\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bDescriptorType, 0x80);
        Notify(PSTR("\r\nAddress:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bEndpointAddress, 0x80);
        Notify(PSTR("\r\nAttributes:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bmAttributes, 0x80);
        Notify(PSTR("\r\nMaxPktSize:\t"), 0x80);
        D_PrintHex<uint16_t > (ep_ptr->wMaxPacketSize, 0x80);
        Notify(PSTR("\r\nPoll Intrv:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bInterval, 0x80);
        Notify(PSTR("\r\n"), 0x80);
}
