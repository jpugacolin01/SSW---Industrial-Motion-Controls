// ME195B Industrial Motion Control Project by Jose Puga
// Senior Project Design

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Nextion.h>
#include <AccelStepper.h>

#define STEP_PIN 2  // Define the STEP pin
#define DIR_PIN 3   // Define the DIR pin

const int stepsPerRevolution = 200;  // Assuming a standard stepper motor
const int microsteps = 16;  // Assuming driver is set to 1/16 microstepping
const int stepsPerMM = (stepsPerRevolution * microsteps) / 8; // Steps per mm calculation

bool moving = false;  // Track if the motor is moving
bool direction = true;  // true for clockwise, false for counterclockwise
unsigned long lastStepTime = 0;  // Time tracking for steps
int stepDelay = 1000;  // Initial step delay in microseconds, adjust as needed
unsigned long targetSteps = 0;  // Target steps to move for special button action
unsigned long stepsTaken = 0; 

// Declare the Nextion components
NexButton b0 = NexButton(1, 1, "b0"); // Page 0, ID 1, Button for forward
NexButton bt0 = NexButton(1, 2, "bt0"); // Page 0, ID 2, Button for backward
NexSlider h0 = NexSlider(1, 3, "h0"); // Page 0, ID 3, Slider for speed
NexButton b1 = NexButton(1,5,"b1"); // Page 1, ID, Button to STOP motors. 
NexButton b2 = NexButton(1,6,"b2"); // Page 1, ID 6, Button to Automatically Run Motors from starting position to 800 mm at MAX SPEED. 

// List of all the touch event listeners
NexTouch *nex_listen_list[] = {
    &b0,
    &bt0,
    &h0,
    &b1,
    &b2,
    NULL  // End of list
};

void b0PopCallback(void *ptr) {
    moving = true;  // Start or continue the motor
}

void bt0PopCallback(void *ptr) {
    direction = !direction;  // Toggle direction
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);  // Apply the direction change
}

void h0PopCallback(void *ptr) {
    uint32_t speed;
    h0.getValue(&speed);
    // Convert speed from range 0-100 to delay time between steps, from slow to fast
    stepDelay = map(speed, 0, 100, 2000, 50);  // Now mapping speed from 0-100
}

void b1PopCallback(void *ptr) {
  moving = false; 
}

void b2PopCallback(void *ptr) {
    moving = true;  // Start the motor
    stepDelay = 50;  // Set to maximum speed
    targetSteps = stepsPerMM * 800;  // Calculate steps for 800 mm
}


void setup() {

    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    Serial.begin(9600); // Start serial communication with computer for debugging
    Serial2.begin(9600); // Start serial communication on Serial2 port with Nextion display
    Serial.println("STARTING UP....");

    nexInit(); // Initialize communication with Nextion display

    // Attach the callback functions
    b0.attachPop(b0PopCallback, &b0);
    bt0.attachPop(bt0PopCallback, &bt0);
    h0.attachPop(h0PopCallback, &h0);
    b1.attachPop(b1PopCallback,&b1);
    b2.attachPop(b2PopCallback, &b2);

  
}

void loop() {
    nexLoop(nex_listen_list); // Process Nextion touch events

     if (moving) {
        unsigned long currentTime = micros();
        if (currentTime - lastStepTime >= stepDelay) {
            digitalWrite(STEP_PIN, HIGH);  // Make one step
            delayMicroseconds(10);  // Minimum high time for most stepper drivers
            digitalWrite(STEP_PIN, LOW);
            lastStepTime = currentTime;  // Reset the last step time
            stepsTaken++; 

        } 

            // Check if the special movement is complete
            if (targetSteps > 0 && stepsTaken >= targetSteps) {
                moving = false;  // Stop the motor
                stepsTaken = 0;  // Reset the step counter
                targetSteps = 0;  // Reset the target steps
        }
    }

}
