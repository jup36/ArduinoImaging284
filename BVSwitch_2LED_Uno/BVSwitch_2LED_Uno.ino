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
#define LENGTH_LED_ARRAY 2

//Global variables
volatile int incoming_value;            // value for case to run
int *LED_order = NULL;                  // declare pointer for dynamic memory allocation
volatile int LED_array[] = {8, 9, 10, 11}; // Order is blue lime green red {14,15}; //{DAC0,DAC1}; {52,51};//{8,10,12,6}; //these pins have LEDs attached to them.  (DAC0 and DAC1 are true analog output pins / referred to as DAC1 and DAC2 on the board)
//volatile int Exposure_Output_pin = 4;   //this pin outputs the exposure signal read from the camera. // not actually used, the expose out signal is sent for the Behavioral nidq in he matlab routine for loop 
volatile int Stim_Delay_pin = 2;       // strobe signal (CMOS expose out), on arduino UNO only pins 2 and 3 can be used as interrupt so exposure out needs to bre on those pins
volatile int Stim_Delay_pin_off = 3;   // on the old set-up, this is just dooubling exposure out, not sure why we need two different pins for that
volatile int Stim_Delay_state = 0;      // stores state from read to Stim_Delay_pin
volatile int ledToggleIndex = 0;        // stores index incoming from serial port for testing
volatile int numberOfElements = 0;      // number of LED transitions
volatile int nextLED = 0;               // LED in turn
volatile int start_delay = 1;           // set delay on or off for when LEDs are strobed, allows for interrupts (when on)


void setup()      // Set mode and state of LED, Delay and Exposure pins
{
  //SerialUSB.begin(9600); //for DUE
  Serial.begin(9600); //for UNO?
  for (int i=0; i<LENGTH_LED_ARRAY; i++) // *(int i=0; i<1; i++)*     // Set all LED pins to output with off states
  {
    pinMode(LED_array[i], OUTPUT);
    digitalWrite(LED_array[i], LED_OFF);
  }
  pinMode(Stim_Delay_pin, INPUT);
  //pinMode(Exposure_Output_pin, OUTPUT);
  //digitalWrite(Exposure_Output_pin, LED_OFF);

}


void loop()     // runs code defined by the case provided through serial port
{ 
  while (Serial.available() > 0) //(SerialUSB.available()>0)     // run only when receiving data
  {
    incoming_value = Serial.read(); //SerialUSB.read();      // read value from native port (IMPORTANT!!!!!!!!!!)
    
    switch (incoming_value)
    {
    
    case 99:  // c    (clear: disable interrupts for delay pin -> turn off all LEDs -> free dynamic memory)
    
      detachInterrupt(digitalPinToInterrupt(Stim_Delay_pin));
      detachInterrupt(digitalPinToInterrupt(Stim_Delay_pin_off));
      
      turn_off_all_LEDs();
      
      if (NULL != LED_order)
      {
        free(LED_order);
        LED_order = NULL;
      }
      Serial.print("Interrupts disabled");  //SerialUSB.print("Interrupts disabled"); 
      
      break;

    
    case 116:  // t   (test: turn off all LEDs -> 10ms delay -> grab toggle index -> toggle on indexed LED)
    
      // Turn off all LEDs first
      //SerialUSB.println("Toggle all LEDs OFF");
      turn_off_all_LEDs();

      delay(10);
      
      //SerialUSB.println("Toggle next LED ON");
      ledToggleIndex = Serial.read() - 49; //SerialUSB.read() - 49;   // ASCII equivalent for .read - 1;
      //SerialUSB.println(LED_array[ledToggleIndex], DEC);
      digitalWrite(LED_array[ledToggleIndex], LED_ON); // Note that UNO doesn't have a DAC (digital-to-analog conversion channels, just use digital channels)
      //analogWrite(LED_array[ledToggleIndex], 255);  // toggle analog ON
      Serial.print(ledToggleIndex+1,DEC); //"SerialUSB.print(ledToggleIndex+1,DEC);
      
      break;


    case 120:  // x   (All Off)
      Serial.println("Toggle all LEDs OFF"); //SerialUSB.println("Toggle all LEDs OFF");
      turn_off_all_LEDs();
      break;


    case 115:  // s   (Strobe: read available bytes -> allocate memory of LED order array and initiate values -> read order -> strobe LEDs)
    
      delay(10);
      
      // read number of available bytes
      numberOfElements = Serial.available(); //SerialUSB.available();
      numberOfElements = numberOfElements - 1;    // offset needed in for loop

      // allocate array using malloc and initalize all elements to 0
      LED_order = (int *) malloc(numberOfElements * sizeof(int));
      for (int counter = 0; counter < numberOfElements; counter++)   {
        LED_order[counter] = 0;
      }

      // Read in strobe order
      for (int counter = 0; counter < numberOfElements; counter++)   {
        LED_order[counter] = Serial.read() - 49; //SerialUSB.read() - 49;   // ACSII equivalent for .read - 1
      }

      // Print LED_order to serial port
      Serial.println("LED_order is:"); //SerialUSB.println("LED_order is:");
      for (int counter = 0; counter < numberOfElements; counter++)   {
         Serial.println(LED_order[counter], DEC); //SerialUSB.println(LED_order[counter], DEC);
      }
       // CJ: FROM HERE I JUST REPLACED ALL serialUSB by Serial
      // Print Pin order to serial port
      Serial.println("Pin order");
      for (int counter = 0; counter < numberOfElements; counter++)   {
        Serial.println(LED_array[LED_order[counter]], DEC);
      }

      //  Strobe LEDs (Stim_Delay_pin's used as interrupts)
      nextLED = 0;
      //digitalWrite(Exposure_Output_pin,LED_OFF);
      digitalWrite(digitalPinToInterrupt(Stim_Delay_pin),LOW);
      digitalWrite(digitalPinToInterrupt(Stim_Delay_pin_off),LOW);
      start_delay=1;
      attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin), strobe_on_LEDs, RISING);   // LED On when signal goes from Low to High
      attachInterrupt(digitalPinToInterrupt(Stim_Delay_pin_off), strobe_off_LEDs, FALLING);   // LED Off when signal goes from High to Low
      //attachInterrupt(0, strobe_on_LEDs, RISING);
      //attachInterrupt(1, strobe_off_LEDs, FALLING);
      Serial.print("Interrupts enabled");
      break;  

    case 114:  //r   (Low reset for delay pin)
      digitalWrite(Stim_Delay_pin, LOW);
      Serial.print("Stim Trigger Reset");  
      break;

    case 82:  //R    (High reset for delay pin)
      digitalWrite(Stim_Delay_pin, HIGH);
      Serial.print("Stim Trigger Reset");  
      break;

    case 97:  //a    (Reset LED state)
      Serial.print("State reset");
      nextLED=0;
      break;

    case 102:  //f   (Flush serial port / complete transmission of outgoing serial data)
      Serial.flush();
      Serial.println("Flush");
      break;

    case 77:  //M    (legacy from Matt's code to check comm status)
      Serial.println("I'm ready to get the data");
      break;

    case 100:  //d   (Stim Delay read)
      Stim_Delay_state=digitalRead(Stim_Delay_pin);
      if (Stim_Delay_state==1)
      {
        Serial.print("Ready");
      }
      else
      {
        Serial.print("Wait");
      }
      break;
    }
  }
}


void strobe_on_LEDs()   // start delay (if needed) and allow interrupts -> turn on next LED in turn
{
  if (start_delay==1){
    interrupts();
    delayMicroseconds(0); //5000 originally
    start_delay=0;
  }
  //SerialUSB.println(LED_array[LED_order[nextLED]]);
  //digitalWrite(LED_array[LED_order[nextLED]], LED_ON);
  digitalWrite(LED_array[LED_order[nextLED]], LED_ON); 
  //analogWrite(LED_array[LED_order[nextLED]], 255);
  //digitalWrite(Exposure_Output_pin,LED_ON);
}


void strobe_off_LEDs()    // turn off LED in turn -> select next LED in turn
{
  //digitalWrite(LED_array[LED_order[nextLED]], LED_OFF);
  digitalWrite(LED_array[LED_order[nextLED]], LED_OFF);
  //digitalWrite(Exposure_Output_pin,LED_OFF);
  //analogWrite(LED_array[LED_order[nextLED]], 0);

  if (nextLED < (numberOfElements - 1))   // hack: going to end-1 as matlab is sending an empty element
  {
    nextLED++;
  }
  else if (nextLED == (numberOfElements - 1))
  {
    nextLED = 0;
  }
}


void turn_off_all_LEDs()    // Turn off LEDs and Exposure pins
{
  for (int i=0; i < LENGTH_LED_ARRAY; i++)
  {
  digitalWrite(LED_array[i], LED_OFF);
  
  //analogWrite(LED_array[i], 0);
  }
  //digitalWrite(Exposure_Output_pin,LED_OFF);
}
