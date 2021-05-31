#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Ticker.h>
Ticker blinker;
WiFiServer server(8888);
const char *ssid = "Moon Lamp";      //Enter your wifi SSID
const char *password = "moonlampV1"; //Enter your wifi Password
int R = 0, G = 0, B = 0;
int count_1 = 0, count_2 = 0;
boolean m = false;
String Buf_Wheel;
String brightness;
void setup()
{
  pinMode(5, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  EEPROM.begin(512);
  blinker.attach(1, Timer_Count);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("");
  Serial.println("Wifi Mode AP");
  for (int i = 0; i < 10; i++)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  Read_eeprom();
  server.begin();
}

void Timer_Count()
{
  count_2++;
}
//=======================================================================
//
//=======================================================================
void colorConverter(String hex_rgb, int Brightness)
{
  unsigned long rgb = (long)strtol(&hex_rgb[1], NULL, 16);
  byte red = rgb >> 16;
  byte green = (rgb & 0x00ff00) >> 8;
  byte blue = (rgb & 0x0000ff);
  rgb = 0;
  rgb |= red << 16;
  rgb |= blue << 8;
  rgb |= green;
  int r = map((int)red, 255, 0, Brightness, 0);
  int g = map((int)green, 255, 0, Brightness, 0);
  int b = map((int)blue, 255, 0, Brightness, 0);
  analogWrite(4, 255 - r);
  analogWrite(2, 255 - g);
  analogWrite(0, 255 - b);
}

void Read_eeprom()
{
  if (EEPROM.read(0) != 0)
  { //neu duu lieu doc ra tu EEPROM khac 0 thi doc du lieu
    for (int i = 0; i < 10; ++i)
    { //32 o nho dau tieu la chua ten mang wifi SSID
      Buf_Wheel += char(EEPROM.read(i));
    }
    Serial.print("Color: ");
    Serial.println(Buf_Wheel);
    for (int i = 10; i < 20; ++i)
    { //o nho tu 32 den 96 la chua PASSWORD
      brightness += char(EEPROM.read(i));
    }
    Serial.print("Brightness: ");
    Serial.println(brightness);
    colorConverter(Buf_Wheel, brightness.toInt());
  }
}
void loop()
{
  if (digitalRead(5) == 0)
  {
    count_1++;
    delay(500);
    if (count_1 >= 10)
    {
      count_1 = 0;
      for (int i = 0; i < 20; ++i)
      {
        EEPROM.write(i, 0); //xoa bo nho EEPROM
      }
      EEPROM.commit();
      ESP.restart();
    }
  }
  else
    count_1 = 0;

  WiFiClient client = server.available();
  client.setNoDelay(1);
  if (client)
  {
    if (client.connected())
    {
      Serial.println("Client Connected");
    }
    while (client.connected())
    {
      while (client.available() > 0)
      {
        // read data from the connected client
        String buf = client.readStringUntil('\n');
        Serial.println(buf);
        if (buf.startsWith("%"))
        {
          buf = buf.substring(1, buf.length());
          brightness = buf;
          colorConverter(Buf_Wheel, brightness.toInt());
          Serial.println(buf);
        }
        else
        {
          Buf_Wheel = buf;
          colorConverter(Buf_Wheel, brightness.toInt());
        }
        count_2 = 0;
        m = true;
      }
      if (WiFi.softAPgetStationNum() == 0)
        client.stop();

      if (count_2 >= 3 && m == true)
      {
        m = false;
        for (int i = 0; i < 20; ++i)
        {
          EEPROM.write(i, 0); //xoa bo nho EEPROM
        }
        for (int i = 0; i < Buf_Wheel.length(); ++i)
        {
          EEPROM.write(i, Buf_Wheel[i]);
        }
        for (int i = 0; i < brightness.length(); ++i)
        {
          EEPROM.write(10 + i, brightness[i]);
        }
        EEPROM.commit();
        Serial.println("Write EEPROM");
        count_2 = 0;
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
