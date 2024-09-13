/*****************************************************************************************
 * Note that this script has been rewritten based on BVSwitch 
 * code to be used on the dual CMOS imaging rig (PNI284) to control
 * upto 4 LEDs simultaneously. 
 * This revised code resets the LED sequence back to the first LED after a pause to start 
 *  the next sequence from the first LED.
 * 'turn_off_all_LEDs': This function ensures to turn off all LEDs 
 *  after a certain time elapsed (allOffThreshold, e.g., 500ms) from the most recent pulse. 
 *  Note that keeping the debounceDelay to be 20ms is critical for accurate LED pulsing 
 *  with pulse trains with varying pulse widths (17.5~100ms). 
 * 9/11/2024 Junchol Park
 *****************************************************************************************/

/***************************************************************
 * This code allows one to toggle and strobe TTL logic driven 
 * illumination sources using anArduino microcontroller 
 * (up to a maximum of 9) in any order.
 ****************************************************************/

/***************************************************************
 * Select the strobe TTL logic used by your illumination source. 
 * Here, the macro LED_ON corresponds to the TTL signal level used 
 * to turn the illumination source on while LED_OFF corresponds
 * to the TTL signal level used to turn the illumination source off.
 * These values are used to set the logic state of Arduino digital
 * output pins
 ****************************************************************/

 /***************************************************************
 * Cases: (ASCII character input through serial port)
 *  c => (clear): disable interrupts for delay pin -> turn off all LEDs -> free dynamic memory
 *  t => (test): turn off all LEDs -> 10ms delay -> grab toggle index -> turn on indexed LED
 *  s => (strobe): read available bytes -> allocate memory of LED order array and initiate values -> read order -> strobe LEDs
 *  a => reset LED in turn
 *  f => flush serial port
 *  M => check serial comm status
 *  d => check Stim_Delay_pin state
 *  x => turn off all LEDs
 ****************************************************************/

/***************************************************************
 * Serial Port inputs:
 *  incoming_value => case
 *  ledToggleIndex => index for LED to be tested (for case == t only)
 *  numberOfElements => from LED sequence
 ****************************************************************/

 /***************************************************************
 * Pins:
 *  LED_array: {LED pins}
 *  Stim_Delay_pin: strobe signal
 *  Exposure_Output_pin: output of exposure signal
 ****************************************************************/
 
#define LED_ON HIGH
#define LED_OFF LOW

//Define how many sources will be strobed
#define LENGTH_LED_ARRAY 4

// Global variables for serial communication and strobing
volatile int incoming_value;
int *LED_order = NULL;
volatile int LED_array[] = {2, 3, 4, 5}; // Order is Blue, Lime, Green, Red LEDs
volatile int Stim_Delay_pin = 20;        // Strobe signal
volatile int Stim_Delay_pin_off = 21;    // Another pin for strobe signal
volatile int ledToggleIndex = 0;
volatile int numberOfElements = 0;       // Number of LED transitions
volatile int nextLED = 0;                // Current LED in the sequence
volatile bool ledOnFlag = false;         // Flag to track if LED should be turned on
volatile bool ledOffFlag = false;        // Flag to track if LED should be turned off
volatile bool strobeActive = false;      // Flag to track if strobe sequence is active
volatile bool allLEDsOffFlag = false;    // Flag to ensure all LEDs are turned off only once

// Timing variables for debouncing and turning off LEDs after inactivity
unsigned long lastOnInterruptTime = 0;
unsigned long lastOffInterruptTime = 0;
unsigned long lastStrobeTime = 0;
const unsigned long debounceDelay = 20;     // Debounce delay (20ms) KEEP IT 20ms! 
const unsigned long pauseThreshold = 5000;  // Pause threshold (5 seconds)
const unsigned long allOffThreshold = 500;  // Time after which all LEDs turn off automatically (0.5 seconds)

// Setup function to initialize pins and interrupts
void setup() {
  Serial.begin(9600);  // Initialize serial communication

  // Initialize all LED pins as OUTPUT and set them to LOW (off)
  for (int i = 0; i < LENGTH_LED_ARRAY; i++) {
    pinMode(LED_array[i], OUTPUT);
    digitalWrite(LED_array[i], LED_OFF);
  }

  // Initialize strobe pins
  pinMode(Stim_Delay_pin, INPUT);
  pinMode(Stim_Delay_pin_off, INPUT);

  Serial.println("Setup complete");
}

// Main loop to handle serial commands and strobe sequence
void loop(){     // runs code defined by the case provided through serial port
  unsigned long currentTime = millis();
  
  // Handle serial communication
  while (Serial.available() > 0) {
    incoming_value = Serial.read();
     
    switch (incoming_value){
      case 99:  // 'c' - clear: disable interrupts and turn off all LEDs
        detachInterrupt(digitalPinToInterrupt(Stim_Delay_pin));
        detachInterrupt(digitalPinToInterrupt(Stim_Delay_pin_off));
        turn_off_all_LEDs();
        if (LED_order != NULL) {
          free(LED_order);
          LED_order = NULL;
        }
        Serial.println("Interrupts disabled");
        break;

      case 116:  // 't' - test: turn on a specific LED for testing
        turn_off_all_LEDs();  // Turn off all LEDs first
        delay(10);  // Short delay
        ledToggleIndex = Serial.read() - 49;
        digitalWrite(LED_array[ledToggleIndex], LED_ON);
        Serial.print("Toggled LED ON: ");
        Serial.println(ledToggleIndex + 1, DEC);
        break;

      case 120:  // 'x' - turn off all LEDs
        turn_off_all_LEDs();
        Serial.println("All LEDs turned off");
        break;


      case 115:  // 's' - start strobe sequence from MATLAB
        delay(10);
        numberOfElements = Serial.available() - 1;  // Get number of available bytes minus 1
        LED_order = (int *) malloc(numberOfElements * sizeof(int));  // Allocate memory for LED order
        for (int i = 0; i < numberOfElements; i++) {
          LED_order[i] = Serial.read() - 49;  // Read in the strobe order
        }

        nextLED = 0;
        strobeActive = true;
        attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin), strobe_on_ISR, RISING);
        attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin_off), strobe_off_ISR, FALLING);
        Serial.println("Strobe sequence started");
        break;

      case 114:  // 'r' - reset trigger pin to LOW
        digitalWrite(Stim_Delay_pin, LOW);
        Serial.println("Stim Trigger Reset");
        break;

      case 82:  // 'R' - reset trigger pin to HIGH
        digitalWrite(Stim_Delay_pin, HIGH);
        Serial.println("Stim Trigger Reset to HIGH");
        break;

      case 97:  // 'a' - reset LED sequence
        nextLED = 0;
        Serial.println("LED sequence reset");
        break;

      case 102:  // 'f' - flush serial port
        Serial.flush();
        Serial.println("Serial port flushed");
        break;

      case 100:  // 'd' - read strobe signal pin
        int strobeState = digitalRead(Stim_Delay_pin);
        Serial.println(strobeState == HIGH ? "Ready" : "Wait");
        break;
    }
  }
  // Handle LED strobing and automatic turn-off due to inactivity
  if (ledOnFlag) {
    ledOnFlag = false;
    strobe_on_LEDs();
    strobeActive = true;
    lastStrobeTime = currentTime;
    allLEDsOffFlag = false;
  }

  if (ledOffFlag) {
    ledOffFlag = false;
    strobe_off_LEDs();
    lastStrobeTime = currentTime;
  }

  // Reset sequence after a pause to start the next sequence from the first LED
  if (strobeActive && (currentTime - lastStrobeTime) > pauseThreshold) {
    nextLED = 0;
    strobeActive = false;
    Serial.println("Pause detected, sequence reset");
  }

  // Turn off all LEDs after inactivity
  if (!allLEDsOffFlag && (currentTime - lastOnInterruptTime) > allOffThreshold) {
    turn_off_all_LEDs();
    allLEDsOffFlag = true;
    Serial.println("Turning off all LEDs due to inactivity");
  }
}


// ISR for strobe ON signal (Must not use any serial commands as they slow things down!)
void strobe_on_ISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastOnInterruptTime > debounceDelay) {
    ledOnFlag = true;
    lastOnInterruptTime = currentTime;
  }
}

// ISR for strobe OFF signal (Must not use any serial commands as they slow things down!)
void strobe_off_ISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastOffInterruptTime > debounceDelay) {
    ledOffFlag = true;
    lastOffInterruptTime = currentTime;
  }
}

// Function to turn on the next LED in the sequence (Must not use any serial commands as they slow things down!)
void strobe_on_LEDs() {
  digitalWrite(LED_array[LED_order[nextLED]], LED_ON);
  //Serial.print("Turning ON LED: "); // Use of any serial commands slow thing down, which causes inability to modulate the LED pulse widths
  //Serial.println(LED_order[nextLED]);
}

// Function to turn off the current LED and move to the next one (Must not use any serial commands as they slow things down!)
void strobe_off_LEDs() {
  digitalWrite(LED_array[LED_order[nextLED]], LED_OFF);
  //Serial.print("Turning OFF LED: "); // Use of any serial commands slow thing down, which causes inability to modulate the LED pulse widths
  //Serial.println(LED_order[nextLED]);

  // Move to the next LED in the sequence
  nextLED++;
  if (nextLED >= numberOfElements) {
    nextLED = 0;
  }
}

// Function to turn off all LEDs
void turn_off_all_LEDs() {
  for (int i = 0; i < LENGTH_LED_ARRAY; i++) {
    digitalWrite(LED_array[i], LED_OFF);
  }
  Serial.println("All LEDs turned off");
}
