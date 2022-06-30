#include <SPI.h>
#include <WiFiNINA.h>
#include <RTCZero.h>

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;

const IPAddress serverIP(192, 168, 43, 9); //欲访问的地址
uint16_t serverPort = 54768;        //服务器端口号

RTCZero rtc;

WiFiClient client; //声明一个客户端对象，用于与服务器进行连接

void setup()
{
  Serial.begin(115200);
  Serial.println();
  rtc.begin();

    
  //WiFi.mode(WIFI_STA);
  //    WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
  unsigned long epoch;

  int numberOfTries = 0, maxTries = 6;

  do {

    epoch = WiFi.getTime();

    numberOfTries++;

  }

  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {

    Serial.print("NTP unreachable!!");

    

  }

  else {

    Serial.print("Epoch received: ");

    Serial.println(epoch);

    rtc.setEpoch(epoch);

    Serial.println();

  }

}

void loop()
{
  Serial.println("尝试访问服务器");
  if (client.connect(serverIP, serverPort)) //尝试访问目标地址
  {
    Serial.println("访问成功");
    Serial.println(client.status());
    client.print("Hello world!");                    //向服务器发送数据
    while (client.connected() || client.available()) //如果已连接或有收到的未读取的数据
    {
      if (client.available() || Serial.available()) //如果有数据可读取
      {
        String line = client.readStringUntil('\n'); //读取数据到换行符
        Serial.print("读取到数据：");
        Serial.println(line);
        
        String line1 = Serial.readStringUntil('\n');
        Serial.write(line.c_str());
        
        
        client.write(line.c_str()); //将收到的数据回发
        client.write(line1.c_str());
        
      }
    }
    Serial.println("关闭当前连接");
    client.stop(); //关闭客户端
  }
  else
  {
    Serial.println("访问失败");
    client.stop(); //关闭客户端
  }
  delay(5000);
}
