#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "Zheng si hao";
char pass[] = "56390088";
int status = WL_IDLE_STATUS;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to network :");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println("You're connected to the network");
  Serial.println("-----------");
  printData();
  Serial.println("-----------");
}
void loop()
{
  delay(10000);
  printData();
  Serial.println("----------");
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
