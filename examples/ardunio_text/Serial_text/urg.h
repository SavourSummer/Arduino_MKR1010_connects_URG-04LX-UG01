#if !defined(__URG_H__)
#define __URG_H__

#include "Usb.h"


#define URG_SIO_RESET                  0 /* Reset the port */
#define URG_SIO_MODEM_CTRL             1 /* Set the modem control register */
#define URG_SIO_SET_FLOW_CTRL          2 /* Set flow control register */
#define URG_SIO_SET_BAUD_RATE          3 /* Set baud rate */
#define URG_SIO_SET_DATA               4 /* Set the data characteristics of the port */
#define URG_SIO_GET_MODEM_STATUS       5 /* Retrieve current value of modem status register */
#define URG_SIO_SET_EVENT_CHAR         6 /* Set the event character */
#define URG_SIO_SET_ERROR_CHAR         7 /* Set the error character */

#define URG_SIO_RESET_SIO              0
#define URG_SIO_RESET_PURGE_RX         1
#define URG_SIO_RESET_PURGE_TX         2

#define URG_SIO_SET_DATA_PARITY_NONE   (0x0 << 8 )
#define URG_SIO_SET_DATA_PARITY_ODD    (0x1 << 8 )
#define URG_SIO_SET_DATA_PARITY_EVEN   (0x2 << 8 )
#define URG_SIO_SET_DATA_PARITY_MARK   (0x3 << 8 )
#define URG_SIO_SET_DATA_PARITY_SPACE  (0x4 << 8 )
#define URG_SIO_SET_DATA_STOP_BITS_1   (0x0 << 11)
#define URG_SIO_SET_DATA_STOP_BITS_15  (0x1 << 11)
#define URG_SIO_SET_DATA_STOP_BITS_2   (0x2 << 11)
#define URG_SIO_SET_BREAK              (0x1 << 14)

#define URG_SIO_SET_DTR_MASK           0x1
#define URG_SIO_SET_DTR_HIGH           ( 1 | ( URG_SIO_SET_DTR_MASK  << 8))
#define URG_SIO_SET_DTR_LOW            ( 0 | ( URG_SIO_SET_DTR_MASK  << 8))
#define URG_SIO_SET_RTS_MASK           0x2
#define URG_SIO_SET_RTS_HIGH           ( 2 | ( URG_SIO_SET_RTS_MASK << 8 ))
#define URG_SIO_SET_RTS_LOW            ( 0 | ( URG_SIO_SET_RTS_MASK << 8 ))

#define URG_SIO_DISABLE_FLOW_CTRL      0x0
#define URG_SIO_RTS_CTS_HS             (0x1 << 8)
#define URG_SIO_DTR_DSR_HS             (0x2 << 8)
#define URG_SIO_XON_XOFF_HS            (0x4 << 8)

#define URG_SIO_CTS_MASK               0x10
#define URG_SIO_DSR_MASK               0x20
#define URG_SIO_RI_MASK                0x40
#define URG_SIO_RLSD_MASK              0x80

#define bmREQ_URG_OUT                USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_URG_IN         USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE







/*设备描述符结构 */
class URG : public USBDeviceConfig, public UsbConfigXtracter {
    static const uint8_t epDataInIndex; // 数据输入端点索引DataIn endpoint index
    static const uint8_t epDataOutIndex; //中断输出端点索引 DataOUT endpoint index
    static const uint8_t epInterruptInIndex; //中断进入端点索引 InterruptIN  endpoint index

    
    USBHost *pUsb;
    uint8_t bAddress; //地址
    uint8_t bConfNum; //配置编号 configuration number
    uint8_t bNumIface; //配置的接口数 number of interfaces in the configuration
    uint8_t bNumEP; // 配置中的EP数total number of EP in the configuration
    uint32_t qNextPollTime; //下一次的令牌时间 next poll time
    volatile bool bPollEnable; //令牌启用标志 poll enable flag
    volatile bool ready; //设备就绪致使器device ready indicator
    

    EpInfo epInfo[2];
     void PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr);
    
  public:
    URG(USBHost *pusb);

    //可能CPP需要用到的东西
    uint8_t SetData(uint16_t databm);
    //发送和接受数据方法 Methods for recieving and sending data
    uint32_t RcvData(uint16_t *bytes_rcvd, uint8_t *dataptr);
    //int urg_open(&urg, URG_SERIAL, connect_device, connect_baudrate);;
    //int urg_start_measurement(&urg, URG_DISTANCE, 1, 0);
    //int urg_get_distance(&urg, length_data, NULL);
    //void urg_close(&urg);
    //USB设备设置实现 USBDeviceConfig implementation
     uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
     uint32_t Release();
     uint32_t Poll();
    
    virtual uint32_t GetAddress() {
      return bAddress;
    };

    // UsbConfigXtracter implementation


        // UsbConfigXtracter implementation
        void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);

    virtual bool isReady() {
      return ready;
    };

};
#endif
