// Pin assignments for the LEDs
int LED_array[] = {2, 3, 4, 5};  // Pins for Blue, Lime, Green, Red LEDs
int LED_order[] = {0, 2, 1, 3};        // Fixed sequence: 0 (Blue), 2 (Green) -> '1', '3'

// Global variables for handling the sequence
volatile int nextLED = 0;        // Index for the next LED in the sequence
volatile int numberOfElements = 4;  // Number of LEDs in the fixed sequence
volatile int start_delay = 1;    // Flag to control delay before strobing
volatile bool ledOnFlag = false;  // Flag to track if LED should be turned on
volatile bool ledOffFlag = false; // Flag to track if LED should be turned off

// Pins for external TTL signals
int Stim_Delay_pin = 20;         // Pin for external strobe-on signal
int Stim_Delay_pin_off = 21;     // Pin for external strobe-off signal

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
  // Handle turning on the LED
  if (ledOnFlag) {
    ledOnFlag = false;  // Clear flag
    strobe_on_LEDs();  // Call the function to handle the LED
  }

  // Handle turning off the LED
  if (ledOffFlag) {
    ledOffFlag = false;  // Clear flag
    strobe_off_LEDs();  // Call the function to handle the LED
  }
}

// Interrupt Service Routine (ISR) for turning on the LED
void strobe_on_ISR() {
  ledOnFlag = true;  // Set flag to indicate LED should be turned on
}

// Interrupt Service Routine (ISR) for turning off the LED
void strobe_off_ISR() {
  ledOffFlag = true;  // Set flag to indicate LED should be turned off
}

// Function to turn on the next LED in the sequence
void strobe_on_LEDs() {
  if (start_delay == 1) {
    delayMicroseconds(100);  // Introduce a 10ms delay (adjustable)
    start_delay = 0;  // Disable delay for the next cycle
  }

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

  // Re-enable delay for the next strobe cycle
  start_delay = 1;
}
