#include <Wire.h>

#define VERSION  "1.0"
#define I2C_ADDR 0x69

// Onboard LED on Arduino Nano connected to D13
const char led_board = 13;

// Mains input relay (sprung, normally off) to entire system
//   prevents any mains output if there is no power to Arduino
const char relay_safety = 12;

// Structure to contain which output is connected to which pin
typedef struct
{
  unsigned char rgb_red;
  unsigned char rgb_green;
  unsigned char rgb_blue;
  unsigned char fan;
  unsigned char relay_pump;
  unsigned char relay_lights;
  unsigned char relay_heater;
  unsigned char relay_air;
} Pins;

// Structure to hold RGB values (16million colours)
typedef struct
{
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colour;

// Structure to hold output relay states
typedef struct
{
  bool pump;
  bool lights;
  bool heater;
  bool air;
} RelayState;

// Small function to blink the onboard light at 1Hz
static const inline void blinky(void);

// Initialise structures with default values
const Pins pin = {A0, A1, A3, A6, 3, 4, 5, 6};
Colour colour = {50, 50, 50};
unsigned char fan_speed = 255;
RelayState relay_state = {true, false, true, true};

// Create I2C register of references to structure members
//   First value is a random to prevent erroneous I2C data
//   Possibly a dodgy pointer cast?
unsigned char* reg[] = {0, &colour.red, &colour.green,
                        &colour.blue, &fan_speed,
                        (unsigned char*)&relay_state.pump,
                        (unsigned char*)&relay_state.lights,
                        (unsigned char*)&relay_state.heater,
                        (unsigned char*)&relay_state.air};


/*  SETUP  */
void setup() {
  // Ensure mains input if off
  pinMode(relay_safety, OUTPUT);
  digitalWrite(relay_safety, HIGH);

  // Initialise I2C and Serial
  Wire.begin(I2C_ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);

  Serial.println(sprintf("PiFish Version %s\nI2C address 0x%02X", VERSION, I2C_ADDR));

  // Setup output pin modes
  pinMode(pin.rgb_red, OUTPUT);
  pinMode(pin.rgb_green, OUTPUT);
  pinMode(pin.rgb_blue, OUTPUT);
  pinMode(pin.fan, OUTPUT);
  pinMode(pin.relay_pump, OUTPUT);
  pinMode(pin.relay_heater, OUTPUT);
  pinMode(pin.relay_air, OUTPUT);
  pinMode(pin.relay_lights, OUTPUT);

  pinMode(led_board, OUTPUT);
  pinMode(2, INPUT); // Always 5V due to PCB wiring, high impedence

  // All ok, activate mains relay
  digitalWrite(relay_safety, LOW);
}


/*  MAIN LOOP  */
void loop() {
  // Write PWM values
  analogWrite(pin.rgb_red, colour.red);
  analogWrite(pin.rgb_green, colour.green);
  analogWrite(pin.rgb_blue, colour.blue);
  analogWrite(pin.fan, fan_speed);

  // Write relay states
  digitalWrite(pin.relay_pump, relay_state.pump);
  digitalWrite(pin.relay_heater, relay_state.heater);
  digitalWrite(pin.relay_air, relay_state.air);
  digitalWrite(pin.relay_lights, relay_state.lights);

  // Print time & data to Serial
  Serial.print(millis());
  for (int x = 0 ; x < 9 ; x++) {
    Serial.print(',');
    Serial.print(*reg[x]);
  }
  Serial.println();

  // Blink the onboard LED
  blinky();

  // Sleep for a bit to save a bit of energy
  //   Not sure if this actually helps!
  delay(1000);
}

// Incoming I2C data
void receiveEvent(int howMany) {
  // First byte will be a registry pointer
  byte ptr = Wire.read();

  // Second byte will be the value
  byte val = Wire.read();

  // Parse the data
  switch (ptr) {
    case 1:
    case 2:
    case 3:
    case 4:
      // PWM value 0-255
      *reg[ptr] = (byte)((((byte)val >= 0) && ((byte)val <= 255)) ? val : 0);
      break;
    default:
      // Boolean state
      *reg[ptr] = (bool)val;
      break;
  }
  return;
}

// I2C request for data
void requestEvent() {
  // Send the whole register
  for (int x = 0 ; x < 9 ; x++) {
    Wire.write((byte)*reg[x]);
  }
  return;
}

// Toggle an LED
static const inline void blinky(void) {
  digitalWrite(led_board, !digitalRead(led_board));
  return;
}
