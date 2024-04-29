#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

#define DHTPIN 2         // pin where the DHT22 is connected
#define SOIL_MOISTURE_PIN A0  // analog pin where the soil moisture sensor is connected

#define DHT_TYPE DHT11

DHT dht(DHTPIN, DHT_TYPE);

#define ss 15
#define rst 16
#define dio0 4


String LoRaMessage = "";
int counter = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);


  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1);
  }
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);


  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  Serial.println("");

  Serial.print("Sending packet: ");
  Serial.println(counter);

  LoRaMessage = String(counter) + "@" + String(t) + "$" + String(h) + "%" + String(soilMoisture) ;
  // send packet
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();

  counter++;



  delay(1500);
}
