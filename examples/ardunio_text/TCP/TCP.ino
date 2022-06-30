/*
 * 以下是TCP报头头部的数据结构。
 */
 #include <RTCZero.h>
typedef struct TCP_Header      //TCP头部
{
  unsigned int sourse_port; //unsigned int 是2字节大小。源端口
  unsigned int aim_port;    //目的端口
  unsigned long num;        //unsigned long 是4字节大小。序号
  unsigned long ACK_num;        //确认号
  byte         data_offset; //数据偏移，本是4位。byte是1字节大小
  byte         baoliu;      //保留位，本是6位大小。
  bool         URG;         //紧急位，当URG=1，表示需要紧急传送
  bool         ACK;         //当ACK=1，确认号才有效；
  bool         PSH;         //推送,当PSH=1，希望不再等缓存满后再交付
  bool         RST;        //复位，当RST=1，表示需要重新连接。
  bool         SYN;        //同步。SYN=1且ACK=0，表示这是一个希望连接的报文，
                           //若同意连接ACK置1
  bool         FIN;        //终止。Fin=1,表示发送方发送完毕解除连接                  
  unsigned int win;         //窗口。表示接受方允许发送方发送的数据量
  unsigned int check_sum;      //检验和
  unsigned int URG_P;       //紧急指针，表示需要紧急传送数据的位置
  //以下还可以添加时间戳之类的数据类型。暂时不设定
   
  
}TCP_H;
