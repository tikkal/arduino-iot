#include <Stepper.h>

#define MIN_STEPS 0
#define MAX_STEPS 600
#define BUMP 10
#define DELAY 500
#define SPEED 50
#define SERIAL_ENABLED true
   
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
  
  // Set the speed of the motor to 30 RPMs
  stepper.setSpeed(SPEED);

  // Initialize gauge to min position.
  InitPosition();
}

void loop()
{
  for (int pct = 10; pct <= 90; pct += BUMP) {
    SetPosition(pct);
    delay(DELAY);
  }
  for (int pct = 80; pct >= 20; pct -= BUMP) {
    SetPosition(pct);
    delay(DELAY);
  }
}
