#include "Wire.h"
#include "PubSubClient.h"
#include "WiFi.h"
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <MFRC522.h>

const char *ssid = "HTEronet_93119D";
const char *password = "4BRL38YX7T";
const char *mqtt_server = "broker.emqx.io";
void callback(char *topic, byte *message, unsigned int length);
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

float temperature = 0;
#define vrata 4
#define roleteGore 25
#define roleteDolje 20
#define rasvjeta 4
#define output 24
#define DHT11PIN 5
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

DHT dht(DHT11PIN, DHT11);

void setup()
{
  dht.begin();
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.begin(9600);

  pinMode(vrata, OUTPUT);
  pinMode(roleteGore, OUTPUT);
  pinMode(roleteDolje, OUTPUT);
  pinMode(rasvjeta, OUTPUT);
  pinMode(output, OUTPUT);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected())
  {
    while (!client.connected())
    {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("Ante"))
      {
        Serial.println("connected");
        client.subscribe("soba101/wc");
        client.subscribe("hotel_purs/rolete_gore");
        client.subscribe("hotel_purs/rolete_dolje");
        client.subscribe("hotel_purs/vrata");
        client.subscribe("hotel_purs/output1");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000)
  {

    lastMsg = now;
    temperature = dht.readTemperature();
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    // client.publish("esp32/temperature", dht.readTemperature());

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
      String rfid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      rfid.toUpperCase();
      rfid.trim();
      Serial.print("RFID:");
      Serial.println(rfid);
      delay(1000);
      // client.publish("esp32/rfid", rfid);
    }
  }
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "soba101/wc")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(rasvjeta, HIGH);
    }
    else if (messageTemp == "off")
    {
      Serial.println("[off]");
      digitalWrite(rasvjeta, LOW);
    }
  }
  else if (String(topic) == "hotel_purs/rolete_gore")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(roleteGore, HIGH);
      delay(5000);
      digitalWrite(roleteGore, LOW);
    }
    else if (messageTemp == "off")
    {
      Serial.println("off");
      digitalWrite(roleteGore, LOW);
    }
  }
  else if (String(topic) == "hotel_purs/rolete_dolje")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(roleteDolje, HIGH);
      delay(5000);
      digitalWrite(roleteDolje, LOW);
    }
    else if (messageTemp == "off")
    {
      Serial.println("off");
      digitalWrite(roleteDolje, LOW);
    }
  }
  else if (String(topic) == "hotel_purs/vrata")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "open")
    {
      Serial.println("OTKLJUČANO");
      digitalWrite(vrata, HIGH);
      delay(3000);
      digitalWrite(vrata, LOW);
    }
    else
    {
      Serial.println("ZAKLJUČANO");
      digitalWrite(vrata, LOW);
    }
  }
}
