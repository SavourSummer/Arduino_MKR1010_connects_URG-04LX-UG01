#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192,168,3,18); //欲访问的地址
uint16_t serverPort =54768;         //服务器端口号



WiFiClient client; //声明一个客户端对象，用于与服务器进行连接

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  
  WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }

}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("尝试访问服务器");
  if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
      client.print("Hello world!");            
      while(client.connected() ||client.available())
      {
        if (client.available()|| Serial.available()) //如果有数据可读取
            {
         
         String line = client.readStringUntil('\0');
         
         Serial.write(line.c_str());//serial的print只能打印一个字符，wirte定义长度后可打印字符串
         client.write(line.c_str()); //在没有定义长度时将收到的数据回发,但是一次只能发一个字符。而不是字符串
         Serial.write("L");//string字符串可以一次发送字符直到字符为空
         client.write("L");
          
         /*
         char c = client.read();
         Serial.write(c);//写入333字符可以打印出333,但是却是一个循环出来每个3带有L
         Serial.println("L");
         */
            }
      }
        Serial.println("关闭当前连接");
        client.stop(); //关闭客户端
    }
    else
    {
      Serial.println("访问失败");
      client.stop();
    }

}
