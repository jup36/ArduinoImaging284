// Define the pin numbers for output
const int outputPin1 = 2;
const int outputPin2 = 3;
const int outputPin3 = 4;
const int outputPin4 = 5;

// Variable for the frequency
int frequency = 10;  // Frequency in Hz, initial value set to 50 Hz

void setup() {
  // Set all output pins as outputs
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  pinMode(outputPin3, OUTPUT);
  pinMode(outputPin4, OUTPUT);
  
  // Optional: Immediately turn all pins off if not needed initially
  turnAllPinsOff();
}

void loop() {
  // If you want to stop toggling and just turn them off, uncomment the next line
  turnAllPinsOff();

  // Toggle logic here, comment or remove if you permanently want pins off
  //togglePins();
}

void togglePins() {
  long halfPeriod = 500 / frequency;  // Calculate half period time in milliseconds
  
  // Set all output pins high
  digitalWrite(outputPin1, HIGH);
  digitalWrite(outputPin2, HIGH);
  digitalWrite(outputPin3, HIGH);
  digitalWrite(outputPin4, HIGH);
  delay(halfPeriod);  // Wait for half the period
  
  // Set all output pins low
  digitalWrite(outputPin1, LOW);
  digitalWrite(outputPin2, LOW);
  digitalWrite(outputPin3, LOW);
  digitalWrite(outputPin4, LOW);
  delay(halfPeriod);  // Wait for half the period
}

void turnAllPinsOff() {
  digitalWrite(outputPin1, LOW);
  digitalWrite(outputPin2, LOW);
  digitalWrite(outputPin3, LOW);
  digitalWrite(outputPin4, LOW);
}
