#include "Wire.h"
#include "PubSubClient.h"
#include "WiFi.h"
#include <DHT.h>
#include <DHT_U.h>

const char *ssid = "757863";
const char *password = "pavkovic98";
const char *mqtt_server = "broker.emqx.io";
void callback(char *topic, byte *message, unsigned int length);
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

float temperature = 0;
#define vrata 5
#define roleteGore 22
#define roleteDolje 20
#define rasvjeta 23
#define output 24
#define DHT11PIN 16
//#define DHTTYPE DHT11
DHT dht(DHT11PIN, DHT11);

void setup()
{
  dht.begin();
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
      if (client.connect("ESP32Client"))
      {
        Serial.println("connected");
        client.subscribe("hotel_purs/rasvjeta");
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

    /* temperature = sht.getTemperature();
     char tempString[8];
     dtostrf(temperature, 1, 2, tempString);
     Serial.print("Temperature: ");
     Serial.println(tempString);
     client.publish("esp32/temperature", tempString);

 */
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

  if (String(topic) == "hotel_purs/rasvjeta")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(rasvjeta, HIGH);
    }
    else if (messageTemp == "off")
    {
      Serial.println("off");
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
