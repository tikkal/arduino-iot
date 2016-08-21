#include <Stepper.h>
#include <SPI.h>
#include <WiFi101.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

// Stepper gauge config
#define MIN_STEPS 0
#define MAX_STEPS 600
#define BUMP 10
#define DELAY 500
#define SPEED 50
#define SERIAL_ENABLED true

// Display config
#define TFT_SCLK   2
#define TFT_CS     3
#define TFT_RST    4 
#define TFT_DC     5
#define TFT_MOSI   8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
 
char ssid[] = "ssid";      // your network SSID (name)
char pass[] = "pass";        // your network password
int wifi_status = WL_IDLE_STATUS;
char server[] = "thisisper.com";
unsigned long lastConnectionTime = 0; 
const unsigned long shortInterval = 5L * 1000L;
const unsigned long longInterval = 60L * 1000L;
unsigned long postingInterval = shortInterval;
String input = "";
WiFiClient client;

// Create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(MAX_STEPS, 2, 3, 4, 5);

int g_pct = 0;

void StepAndDisplay(int steps, int pct) {
  String message = String("Stepping ") + String(steps) 
      + String(" steps to reach position pct ") + String(pct);
  Serial.println(message);
  stepper.step(steps);
  g_pct = pct;
  delay(DELAY);
}

void MinPosition() {
  StepAndDisplay(MAX_STEPS, 0);
}

void MaxPosition() {
  StepAndDisplay(-MAX_STEPS, 100);
}

void SetPosition(int next_pct) {
  if (next_pct <= 0) {
    MinPosition();
    return;
  }
  if (next_pct >= 100) {
    MaxPosition();
    return;
  }
  int delta_pct = next_pct - g_pct;
  float steps = delta_pct * 0.01 * MAX_STEPS * -1;
  StepAndDisplay((int) steps, next_pct);
}

// Init position by forcing max/min.
void InitPosition() {
  MaxPosition();
  MinPosition();
}

void ClearDisplay() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
}

void PrintDisplay(String buffer) {
  tft.setTextColor(ST7735_WHITE);
  tft.setTextWrap(true);
  tft.println(buffer);
  if (SERIAL_ENABLED) {
    Serial.println(buffer);
  }
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

void setup()
{
  if (SERIAL_ENABLED) {
    Serial.begin(9600);
  
    // Can hang if cannot reach serial port.
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only.
    }
    Serial.println("Serial port connected.");
  }

  // Use initializer for the 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  PrintDisplay("Hello world");
  
  // Set the speed of the motor and init position.
  stepper.setSpeed(SPEED);
  InitPosition();
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    PrintDisplay("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (wifi_status != WL_CONNECTED) {
    PrintDisplay(ssid);
    wifi_status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  // Print wifi status after connected and make first http request.
  printWifiStatus();
  httpRequest();  
}

void loop() {
  // Build input buffer of data read from wifi client.
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

void httpRequest() {
  // Close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP request:
    client.println("GET /tide/weather.txt HTTP/1.1");
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
