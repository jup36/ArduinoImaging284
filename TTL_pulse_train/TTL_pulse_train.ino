// Define the pin for the TTL output
const int ttlPin = 5;
// Define the frequency in Hertz
const int frequency = 10; // 10Hz
// Calculate the delay time in microseconds (for a 50% duty cycle)
const unsigned long pulseDelay = 1000000 / (2 * frequency); // Microseconds

void setup() {
  // Set the TTL pin as an output
  pinMode(ttlPin, OUTPUT);
}

void loop() {
  // Generate the pulse
  digitalWrite(ttlPin, LOW);   // Set the TTL pin high
  delayMicroseconds(pulseDelay); // Wait for half the period
  digitalWrite(ttlPin, LOW);    // Set the TTL pin low
  delayMicroseconds(pulseDelay); // Wait for the other half of the period
}
