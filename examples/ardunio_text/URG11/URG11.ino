#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192,168,3,18); //欲访问的地址
uint16_t serverPort =54768;         //服务器端口号

//USBHost usb;//初始化usb

WiFiClient client; //声明一个客户端对象，用于与服务器进行连接

void setup()
{   
    
    Serial.begin(19200);
    Serial.println();
    //while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  //SERIAL_PORT_MONITOR.println(" Program started");
    //WiFi.mode(WIFI_STA);
//    WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }
    //Serial.println("Connected");
    //Serial.print("IP Address:");
    //Serial.println(WiFi.localIP());
    if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
        //Serial.println("访问成功");
        //Serial.println(client.status());
        client.print("Hello world!");  
    }
    else
    {
        //Serial.println("访问失败");
        client.stop(); //关闭客户端
    }
    
}

int decode(const char code[],int byte){
int value = 0;
int i;
for(i = 0; i < byte;i++){
value <<= 6;
value &= ~0x3f;
value |= code[i]-0x30;
}
return value;
}





void loop()
{
    static String sendData = "MD0540054001000\n";
    static int data = 0;
    //Serial.println("尝试访问服务器");
    
        //Serial.println("访问成功");
        //Serial.println(client.status());

        if(client.available()) //如果已连接或有收到的未读取的数据
        {
           char c = client.read();
           data = decode("c",4);
           client.println(data);
           Serial.println(data);
                
        }
        if(client.connected())
        {
          client.println(sendData);
        }
        if (!client.connected()) 
        {
          client.println("disconnecting.");
          client.stop();
        }
            
            
        
    
    
    delay(5000);
}
