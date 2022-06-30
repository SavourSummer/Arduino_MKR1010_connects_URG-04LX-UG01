#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int keIndex = 0;
int status = WL_IDLE_STATUS;
unsigned int localPort = 2390; 
char packetBuffer[255];
char ReplyBuffer[] = "acknowledged";
IPAddress sever(192,168,3,18);
unsigned int severport = 8044;
WiFiClient client;

//int connect（sever,severport);

void setup() 
{
  Serial.begin(9600);
  
  while(!Serial);

  while(status != WL_CONNECTED)
  {
    Serial.print("Attempting to connetc to network:");
    Serial.println(ssid);
    status = WiFi.begin(ssid,pass);

    delay(10000);
  }
  Serial.println("You're connected to the network");
  Serial.println("-----------");
  printData();
  Serial.println("-----------");
  Serial.println(client.status());
  
    client.connect(sever,severport);
    Serial.println(client.status());
    Serial.print("waitting");
    delay(10000);
  
  if(client.connected())
  {
    Serial.print("connett to sever");
    Serial.println(client.status());
    Serial.println(sever);
    Serial.println(client.remoteIP());
    Serial.println(client.remotePort());
    client.print("hellow");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(client.available() || client.connected())
  {
    int c = client.read();
    Serial.write(c);
    
    client.write(c);
    client.print("hellow");
  }
  delay(10000);
  if(!client.connected())
  {
    Serial.println("disconnecting from server.");
    client.println("disconnected ");
  }
    

}

void printData()
{
  Serial.println("Board Information:");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address:");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID:");
  Serial.println(WiFi.SSID());

  long rssi = WiFi.RSSI();
  Serial.print("singnal streng (RSSI):");
  byte encryption = WiFi.encryptionType();
  Serial.print("Encrytion Type:");
  Serial.println(encryption, HEX);
  Serial.println();

}
