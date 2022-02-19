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
#define vratarfid 26
#define roleteGore 15
#define roleteDolje 2
#define rasvjetasoba 13
#define templed 12
#define rasvjetawc 14
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
  pinMode(vratarfid, OUTPUT);
  pinMode(roleteGore, OUTPUT);
  pinMode(roleteDolje, OUTPUT);
  pinMode(rasvjetawc, OUTPUT);
  pinMode(templed, OUTPUT);
  pinMode(rasvjetasoba, OUTPUT);

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
        client.subscribe("hotelpurs/soba101");
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
    client.publish("soba101/temp", tempString);

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

      if (rfid == "49 11 01 BA")
      {
        Serial.println("OTKLJUČANO");
        digitalWrite(vratarfid, HIGH);
        delay(3000);
        digitalWrite(vratarfid, LOW);
      }
      else
      {
        Serial.println("ZAKLJUČANO");
        digitalWrite(vratarfid, LOW);
      }
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

  if (String(topic) == "hotelpurs/soba101")
  {
    if (messageTemp == "wcon")
    {
      Serial.println("wcon");
      digitalWrite(rasvjetawc, HIGH);
    }
    else if (messageTemp == "wcoff")
    {
      Serial.println("wcoff");
      digitalWrite(rasvjetawc, LOW);
    }
    else if (messageTemp == "sobaon")
    {
      Serial.println("sobaon");
      digitalWrite(rasvjetasoba, HIGH);
    }
    else if (messageTemp == "sobaoff")
    {
      Serial.println("[off]");
      digitalWrite(rasvjetasoba, LOW);
    }
    else if (messageTemp == "rolup")
    {
      Serial.println("up");
      digitalWrite(roleteGore, HIGH);
      delay(5000);
      digitalWrite(roleteGore, LOW);
    }
    else if (messageTemp == "roldown")
    {
      Serial.println("down");
      digitalWrite(roleteDolje, HIGH);
      delay(5000);
      digitalWrite(roleteDolje, LOW);
    }
    else if (messageTemp == "open")
    {
      Serial.println("OTKLJUČANO");
      digitalWrite(vrata, HIGH);
      delay(3000);
      digitalWrite(vrata, LOW);
    }
    else if (messageTemp == "tempup")
    {
      Serial.println("up");
      for (int i = 0; i < 5; i++)
      {
        digitalWrite(templed, HIGH);
        delay(500);
        digitalWrite(templed, LOW);
        delay(500);
      }
    }
    else if (messageTemp == "tempdown")
    {
      Serial.println("down");
      for (int i = 0; i < 5; i++)
      {
        digitalWrite(templed, HIGH);
        delay(200);
        digitalWrite(templed, LOW);
        delay(200);
      }
    }
    else
    {
      Serial.println("ZAKLJUČANO");
      digitalWrite(vrata, LOW);
    }
  }
}