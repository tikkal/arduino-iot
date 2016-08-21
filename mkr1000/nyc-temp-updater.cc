#include <Stepper.h>
#include <SPI.h>
#include <WiFi101.h>

// For serial line. Set to false if not debugging over usb.
#define SERIAL_ENABLED false

// For stepper
#define MIN_STEPS 0
#define MAX_STEPS 600
#define DELAY 500
#define SPEED 30
#define SWEEP_MIN 15
#define SWEEP_BUMP 5
#define SWEEP_MAX 72
#define TEMP_F_MAX 100
#define TEMP_F_MIN 0

// For wifi
char ssid[] = "ssid";      // your network SSID (name)
char pass[] = "pass";        // your network password
int wifi_status = WL_IDLE_STATUS;
char server[] = "thisisper.com";
unsigned long lastConnectionTime = 0; 
const unsigned long shortInterval = 5L * 1000L;
const unsigned long longInterval = 60L * 1000L;
unsigned long postingInterval = shortInterval;
String http_input = "";
WiFiClient wifi_client;

   
// Create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(MAX_STEPS, 2, 3, 4, 5);
int g_pct = 0;

void ClearDisplay() {
  // Do nothing, no lcd display attached yet.
}

void PrintDisplay(String message) {
  if (SERIAL_ENABLED) {
    Serial.println(message);
  }
}

void StepAndDisplay(int steps, int pct) {
  String message = String("Stepping ") + String(steps) 
      + String(" steps to reach position pct ") + String(pct);
  PrintDisplay(message);
  stepper.step(steps);
  g_pct = pct;
  delay(DELAY);
}

void MinPosition() {
  StepAndDisplay(-MAX_STEPS, 0);
}

void MaxPosition() {
  StepAndDisplay(MAX_STEPS, 100);
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
  float steps = delta_pct * 0.01 * MAX_STEPS;
  StepAndDisplay((int) steps, next_pct);
}

float TempFToStepperPctPos(float temp_f) {
  if (temp_f >= TEMP_F_MAX) {
    return SWEEP_MAX;
  } else if (temp_f <= TEMP_F_MIN) {
    return SWEEP_MIN;
  }
  // Calculate fraction of the range we want.
  float fraction = (temp_f - TEMP_F_MIN) / (TEMP_F_MAX - TEMP_F_MIN);

  // Translate it to a stepper pct position, with sweep min/max as boundaries.
  return fraction * (SWEEP_MAX - SWEEP_MIN) + SWEEP_MIN;
}

void SetTempF(float temp_f) {
  int pct = (int) TempFToStepperPctPos(temp_f);
  SetPosition(pct);  
}

// Init position by forcing max/min.
void InitPosition() {
  MaxPosition();
  MinPosition();
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

// Set dial at 0, 25, 50, 75, 100 to visually check calibration.
void CheckCalibration() {
  for (float temp_f = 25.0; temp_f <= 100.0; temp_f += 25.0) {
    SetTempF(temp_f);
    delay(DELAY);
  }
  for (float temp_f = 75.0; temp_f >= 0.0; temp_f -= 25.0) {
    SetTempF(temp_f);
    delay(DELAY);
  }
}

void httpRequest() {
  // Close any connection before send a new request.
  // This will free the socket on the WiFi shield
  wifi_client.stop();

  // if there's a successful connection:
  if (wifi_client.connect(server, 80)) {
    // send the HTTP request:
    wifi_client.println("GET /tide/weather.txt HTTP/1.1");
    wifi_client.println("Host: www.arduino.cc");
    wifi_client.println("User-Agent: ArduinoWiFi/1.1");
    wifi_client.println("Connection: close");
    wifi_client.println();

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

    // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    PrintDisplay("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // Attempt to connect to Wifi network
  while (wifi_status != WL_CONNECTED) {
    PrintDisplay(ssid);
    wifi_status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  
  // Report wifi status to serial port after connected.
  printWifiStatus();
  
  stepper.setSpeed(SPEED);
  InitPosition();
  CheckCalibration();

  // Make first http request.
  httpRequest();  
}

void loop() {
    // Build input buffer of data read from wifi client.
  while (wifi_client.available()) {
    char c = wifi_client.read();
    http_input = http_input + String(c);
  }
  
  // If enough time has passed since last connection,
  // then refresh. Switch to 'long interval' after first
  // fetch.
  if (millis() - lastConnectionTime > postingInterval) {
    String content = ExtractHttpContent(http_input);
    ClearDisplay();
    PrintDisplay(content);
    SetTempF(content.toFloat());
    http_input = "";
    postingInterval = longInterval;
    httpRequest();
  }
}
