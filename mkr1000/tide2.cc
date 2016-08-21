/*
 Adapted from: http://arduino.cc/en/Tutorial/WifiWebClientRepeating by Tom Igoe
 */

#include <SPI.h>
#include <WiFi101.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

// Set up display
#define TFT_SCLK   2
#define TFT_CS     3
#define TFT_RST    4 
#define TFT_DC     5
#define TFT_MOSI   8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
 
char ssid[] = "ssid";      // your network SSID (name)
char pass[] = "pass";        // your network password
int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// Serial toggle for debugging
bool serial_enabled = false;

// server address:
char server[] = "***REMOVED***";
unsigned long lastConnectionTime = 0; 

// The first refresh is on 'short interval', subsequent fetches
// are on the long interval.
const unsigned long shortInterval = 5L * 1000L;
const unsigned long longInterval = 60L * 1000L;
unsigned long postingInterval = shortInterval;
String input = "";

// Text buffer for display.

void ClearDisplay() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
}

void PrintDisplay(String buffer) {
  tft.setTextColor(ST7735_WHITE);
  tft.setTextWrap(true);
  tft.println(buffer);
  if (serial_enabled) {
    Serial.println(buffer);
  }
}

void setup() {
  // Initialize serial and wait for port to open:
  if (serial_enabled) {
    Serial.begin(9600);
  
    // Will hang if cannot reach serial port.
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only.
    }
  }
  
  // Use initializer for the 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  PrintDisplay("Hello world");
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    PrintDisplay("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    PrintDisplay(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  // You're connected now, so print out the status:
  printWifiStatus();

  // Make first http request.
  httpRequest();
}

String ExtractHttpContent(String raw) {
  String ret = raw;
  ret.trim();

  // Find first blank line from HTTP response.
  String marker("\r\n\r\n");
  int marker_idx = ret.indexOf(marker);
  if (marker_idx >= 0) {
    ret = ret.substring(marker_idx + marker.length());
  }
  int maxchar = 256;
  if (ret.length() > maxchar) {
    ret = ret.substring(0, maxchar);
  }
  return ret;
}

void loop() {
  // Build the buffer.
  while (client.available()) {
    char c = client.read();
    input = input + String(c);
  }
  
  // If enough time has passed since last connection,
  // then refresh. Switch to 'long interval' after first
  // fetch.
  if (millis() - lastConnectionTime > postingInterval) {
    String content = ExtractHttpContent(input);
    ClearDisplay();
    PrintDisplay(content);
    input = "";
    postingInterval = longInterval;
    httpRequest();
  }
}

// This method makes a HTTP connection to the server.
void httpRequest() {
  // Close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP request:
    client.println("GET /tide/current.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  String message = "SSID: " + String(WiFi.SSID()) + String("\n");
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  message += String("IP Addresss: ") + String(ip) + String("\n");
 
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  message += "signal strength (RSSI): " + String(rssi) + String(" dBm") + String("\n");
  PrintDisplay(message);
}
