
#define BLYNK_TEMPLATE_ID "TMPL3nTKexrHc"
#define BLYNK_TEMPLATE_NAME "LoRa"
#define BLYNK_AUTH_TOKEN "RQXkBMmEryEUPige-xGTr-9ooGGDMmGC"

#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>

#define ss 15
#define rst 16
#define dio0 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *ssid = "Galaxy S";
const char *password = "Frequency";
const char *blynkAuthToken = "RQXkBMmEryEUPige-xGTr-9ooGGDMmGC";

ESP8266WebServer server(80);

String receivedTemperature;
String receivedHumidity;
int receivedSoilMoisture; // Variable to hold received soil moisture data

BlynkTimer timer;

const int relayPin = 2; // GPIO pin to which the relay is connected

void setup()
{

  Serial.begin(115200);
  delay(100);
   digitalWrite(relayPin,HIGH);
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(blynkAuthToken, ssid, password);

  pinMode(relayPin, OUTPUT); // Set relay pin as output

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Set up a Blynk timer to send data to Blynk every 5 seconds
  timer.setInterval(5000L, sendSensorDataToBlynk);
}

void loop()
{
 
  server.handleClient();
  Blynk.run();
  timer.run();

  int pos2, pos3, pos4; // Add pos4 for soil moisture position
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    Serial.print("Received LoRa packet: ");
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);

    while (LoRa.available())
    {
      Serial.print((char)LoRa.read());
    }

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    pos2 = LoRaData.indexOf('@');
    pos3 = LoRaData.indexOf('$');
    pos4 = LoRaData.indexOf('%'); // Find the position of the '%' symbol

    receivedTemperature = LoRaData.substring(pos2 + 1, pos3);
    receivedHumidity = LoRaData.substring(pos3 + 1, pos4); // Extract humidity until '%'
    receivedSoilMoisture = LoRaData.substring(pos4 + 1).toInt(); // Extract soil moisture after '%'

    Serial.print(F("Received Temperature: "));
    Serial.print(receivedTemperature);
    Serial.println(F("°C"));

    Serial.print(F("Received Humidity = "));
    Serial.print(receivedHumidity);
    Serial.println(F("%"));

    Serial.print(F("Received Soil Moisture = "));
    Serial.println(receivedSoilMoisture);

    Blynk.virtualWrite(V1, receivedTemperature.toFloat());
    Blynk.virtualWrite(V2, receivedHumidity.toFloat());
    Blynk.virtualWrite(V3, receivedSoilMoisture); // Send soil moisture to Blynk V3

    Serial.println();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Received Temperature: ");
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print(receivedTemperature);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Received Humidity: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(receivedHumidity);
    display.print(" %");
    display.display();
    delay(1500);
  }
}

void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(receivedTemperature, receivedHumidity));
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(String receivedTemp, String receivedHum)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  ptr += "<title>ESP8266 Weather Report</title>\n";
  ptr += "<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
  ptr += "body{margin-top: 50px;}\n";
  ptr += "h1 {margin: 50px auto 30px;}\n";
  ptr += ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  ptr += ".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
  ptr += ".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr += ".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
  ptr += ".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
  ptr += ".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr += ".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
  ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
  ptr += ".data{padding: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";

  ptr += "<div id=\"webpage\">\n";

  ptr += "<h1>YCCE Weather Report</h1>\n";
  ptr += "<div class=\"data\">\n";

  // Display received temperature
  ptr += "<div class=\"side-by-side temperature-icon\">\n";
  ptr += "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  ptr += "width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
  ptr += "<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
  ptr += "c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
  ptr += "c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
  ptr += "c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
  ptr += "c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
  ptr += "</svg>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"side-by-side temperature-text\">Received Temperature</div>\n";
  ptr += "<div class=\"side-by-side temperature\">";
  ptr += receivedTemp;
  ptr += "<span class=\"superscript\">°C</span></div>\n";

  ptr += "</div>\n";

  // Display received humidity
  ptr += "<div class=\"data\">\n";
  ptr += "<div class=\"side-by-side humidity-icon\">\n";
  ptr += "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  ptr += "width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
  ptr += "<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
  ptr += "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
  ptr += "</svg>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"side-by-side humidity-text\">Received Humidity</div>\n";
  ptr += "<div class=\"side-by-side humidity\">";
  ptr += receivedHum;
  ptr += "<span class=\"superscript\">%</span></div>\n";

  ptr += "</div>\n";

  // Display received soil moisture
  ptr += "<div class=\"data\">\n";
  ptr += "<div class=\"side-by-side humidity-icon\">\n";
  ptr += "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  ptr += "width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
  ptr += "<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
  ptr += "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
  ptr += "</svg>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"side-by-side humidity-text\">Received Soil Moisture</div>\n";
  ptr += "<div class=\"side-by-side humidity\">";
  ptr += receivedSoilMoisture;
  ptr += "</div>\n";

  ptr += "</div>\n";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void sendSensorDataToBlynk()
{
  Blynk.virtualWrite(V1, receivedTemperature.toFloat());
  Blynk.virtualWrite(V2, receivedHumidity.toFloat());
  Blynk.virtualWrite(V3, receivedSoilMoisture); // Send soil moisture to Blynk V3
}

// Blynk callback function to handle button press
BLYNK_WRITE(V4)
{
  int relayState = param.asInt(); // Get the state of the button (1 for ON, 0 for OFF)
  digitalWrite(relayPin, relayState); // Set the relay state accordingly
}
