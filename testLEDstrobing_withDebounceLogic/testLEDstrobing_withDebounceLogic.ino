// Pin assignments for the LEDs
int LED_array[] = {2, 3, 4, 5};  // Pins for Blue, Lime, Green, Red LEDs
int LED_order[] = {0, 1, 2, 3};        // Fixed sequence: 0 (Blue), 2 (Green) -> '1', '3'

// Global variables for handling the sequence
volatile int nextLED = 0;        // Index for the next LED in the sequence
volatile int numberOfElements = 4;  // Number of LEDs in the fixed sequence
volatile bool ledOnFlag = false;  // Flag to track if LED should be turned on
volatile bool ledOffFlag = false; // Flag to track if LED should be turned off
volatile bool strobeActive = false; // Flag to track if strobe sequence is active
volatile bool allLEDsOffFlag = false; // Flag to ensure all LEDs are turned off only once

// Pins for external TTL signals
int Stim_Delay_pin = 20;         // Pin for external strobe-on signal
int Stim_Delay_pin_off = 21;     // Pin for external strobe-off signal

// Separate debounce timing variables for each ISR
unsigned long lastOnInterruptTime = 0;
unsigned long lastOffInterruptTime = 0;
unsigned long lastStrobeTime = 0;   // Time when the last strobe event occurred
const unsigned long debounceDelay = 10;  // Adjust debounce delay to 20ms
const unsigned long pauseThreshold = 5000; // Pause threshold (e.g., 5 seconds)
const unsigned long allOffThreshold = 500; // Threshold to turn off all LEDs (e.g., 1 second)

// Setup function to initialize pins and interrupts
void setup() {
  Serial.begin(9600);

  // Initialize all LED pins as OUTPUT and set them to LOW (off)
  for (int i = 0; i < 4; i++) {
    pinMode(LED_array[i], OUTPUT);
    digitalWrite(LED_array[i], LOW);  // Turn off all LEDs
  }

  // Set up pins for strobing signals
  pinMode(Stim_Delay_pin, INPUT);  // Pin for external strobe-on signal
  pinMode(Stim_Delay_pin_off, INPUT);  // Pin for external strobe-off signal

  // Attach interrupts for strobing process
  attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin), strobe_on_ISR, RISING);  // Turn on LED when signal goes from Low to High
  attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin_off), strobe_off_ISR, FALLING);  // Turn off LED when signal goes from High to Low
  Serial.println("Interrupts enabled");
}

// Main loop to handle LED strobing and serial output
void loop() {
  unsigned long currentTime = millis();
  
  // Handle turning on the LED
  if (ledOnFlag) {
    ledOnFlag = false;  // Clear flag
    strobe_on_LEDs();  // Call the function to handle the LED
    strobeActive = true; // Strobe is active when a LED turns on
    lastStrobeTime = currentTime;  // Update the last strobe time
    allLEDsOffFlag = false;  // Reset the flag, because a new strobe started
  }

  // Handle turning off the LED
  if (ledOffFlag) {
    ledOffFlag = false;  // Clear flag
    strobe_off_LEDs();  // Call the function to handle the LED
    lastStrobeTime = currentTime;  // Update the last strobe time
  }

  // Check if the strobe sequence has paused beyond the threshold and reset the sequence
  if (strobeActive && (currentTime - lastStrobeTime) > pauseThreshold) {
    Serial.println("Pause detected, resetting sequence.");
    nextLED = 0;        // Reset to the first LED in the sequence
    strobeActive = false; // Set strobeActive to false, indicating no active strobe
  }

  // Check if time threshold has passed since the last "on" event to turn off all LEDs
  if (!allLEDsOffFlag && (currentTime - lastOnInterruptTime) > allOffThreshold) {
    turn_off_all_LEDs();  // Turn off all LEDs after the threshold time
    allLEDsOffFlag = true; // Set the flag to prevent repeated calls
  }
}

// Interrupt Service Routine (ISR) for turning on the LED
void strobe_on_ISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastOnInterruptTime > debounceDelay) {  // Debounce for turning on
    ledOnFlag = true;  // Set flag to indicate LED should be turned on
    lastOnInterruptTime = currentTime;  // Update the last interrupt time for "on"
  }
}

// Interrupt Service Routine (ISR) for turning off the LED
void strobe_off_ISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastOffInterruptTime > debounceDelay) {  // Debounce for turning off
    ledOffFlag = true;  // Set flag to indicate LED should be turned off
    lastOffInterruptTime = currentTime;  // Update the last interrupt time for "off"
  }
}

// Function to turn on the next LED in the sequence
void strobe_on_LEDs() {
  // Turn on the current LED
  digitalWrite(LED_array[LED_order[nextLED]], HIGH);
  Serial.print("Turning ON LED: ");
  Serial.println(LED_order[nextLED]);  // Debug: Print the current LED being turned on
}

// Function to turn off the current LED and move to the next one
void strobe_off_LEDs() {
  // Turn off the current LED
  digitalWrite(LED_array[LED_order[nextLED]], LOW);
  Serial.print("Turning OFF LED: ");
  Serial.println(LED_order[nextLED]);  // Debug: Print the current LED being turned off

  // Move to the next LED in the sequence
  nextLED++;

  // Wrap around if we've reached the end of the sequence
  if (nextLED >= numberOfElements) {
    nextLED = 0;  // Go back to the first LED
  }
}

// Function to turn off all LEDs
void turn_off_all_LEDs() {
  Serial.println("Turning OFF all LEDs due to inactivity.");
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_array[i], LOW);  // Turn off all LEDs
  }
}
