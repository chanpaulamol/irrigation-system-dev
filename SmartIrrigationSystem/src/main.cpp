#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

const char *ssid = "Chan";
const char *password = "chanpaul";
const char *mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

SimpleDHT11 dht11(D5);

long now = millis();
long lastMeasure = 0;
String macAddr = "";

void setup_wifi()
{
  delay(10);
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
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddr.c_str()))
    {
      Serial.println("connected");
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

void setup()
{
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(macAddr.c_str());
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;
    int err = SimpleDHTErrSuccess;

    byte temperature = 0;
    byte humidity = 0;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      Serial.print("DHT11 read failed,error is");
      Serial.println(err);
      delay(1000);
      return;
    }

    static char temperatureTemp[7];
    static char humidityHum[7];
    dtostrf(temperature, 4, 2, temperatureTemp);
    dtostrf(humidity, 4, 2, humidityHum);

    float soilMoisture = (temperature - 20) * humidity / 100;
    // Print the data
    Serial.println("Temperature");
    Serial.println(temperatureTemp);
    Serial.println("Temperature");
    Serial.println(humidityHum);
    Serial.println("Soil Moisture");
    Serial.println(soilMoisture);

    // Publish temperature, humidity, and soil moisture data to MQTT
    client.publish("smartirrigation/temperature", temperatureTemp);
    client.publish("smartirrigation/humidity", humidityHum);
    client.publish("smartirrigation/soilmoisture", String(soilMoisture).c_str());
  }
}
