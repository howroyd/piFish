#include <Arduino.h>

#define VERSION  "2.0"

#define I2C_SLAVE_ADDRESS 0x69

#define REG_SIZE 9

#define fet_pump      0
#define fet_lights    1
#define fet_heater    2
#define fet_air       3
#define i2c_scl       4
#define pwm_red       8
#define i2c_sda       6
#define pwm_green     7
#define pwm_blue      5
//RST reset (pull low > 700ns)
#define fet_fan       9
#define led_heartbeat 10

#include <TinyWireS.h>
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

class Blinky {
  public:
    Blinky(const int pin, const unsigned long period) {
      _pin = pin;
      _period = period;
    }
    Blinky(const int pin) {
      _pin = pin;
      _period = 1000;
    }
    void init(void) {
      pinMode(_pin, OUTPUT);
      _time_last = millis();
    }
    void update(void) {
      unsigned long now = millis();
      if (_counter > 0) {
        if ((now - _time_last) > _period / 10) {
          blinky(_pin);
          _counter--;
          _time_last = now;
        }
      }
      else {
        if ((now - _time_last) > _period) {
          blinky(_pin);
          _time_last = now;
        }
      }
    }
    void set_counter(const int number) {
      _counter = number;
    }
    // Toggle an LED
    static const inline void blinky(const int pin) {
      digitalWrite(pin, !digitalRead(pin));
    }
  private:
    int _pin;
    unsigned long _time_last;
    unsigned long _period;
    int _counter;
};

Blinky blinky(led_heartbeat);

volatile uint8_t reg_ptr = 0;
void receiveEvent(int);
void requestEvent(void);

void updateRegister(void);

byte pwmRegister[] = { 128, 128, 128, 1, 1, 1, 1, 1 }; // PWV values with respect to pinRegister
const byte pinRegister[] = { pwm_red , pwm_green, pwm_blue, fet_fan, fet_pump, fet_lights, fet_heater, fet_air }; // Pins in use in order
byte statusRegister = 0b00000000; // Toggle on/off a bit for each pin on/off
byte i2cOutRegister[REG_SIZE];
volatile byte i2cReceiveRegister[3];
volatile bool newData;

void setup() {
  for (int a = 0; a < 8; a++) {
    pinMode(pinRegister[a], OUTPUT);
  }

  blinky.init();

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);

  statusRegister = ~statusRegister;
}

void loop() {
  static unsigned long last_hs;
  static unsigned long last_ms;
  static unsigned long last_ls;
  unsigned long now = millis();

  // High speed loop //
  if ( (now - last_hs) > 10) {
    last_hs = now;

    // Blink the onboard LED
    blinky.update();

    if (newData) {
      updateRegister();
      blinky.set_counter(10);
    }
  }

  // Medium speed loop //
  if ((now - last_ms) > 100) {
    last_ms = now;
  }

  // Low speed loop //
  if ((now - last_ls) > 1000) {
    last_ls = now;

    for (int a = 0; a < 8; a++) {
      if bitRead(statusRegister, a) {
        if (a < 3) analogWrite(pinRegister[a], pwmRegister[a]);
        else digitalWrite(pinRegister[a], bitRead(statusRegister, a));
      }
      else {
        digitalWrite(pinRegister[a], LOW);
      }
    }
  }

  TinyWireS_stop_check();
}

void receiveEvent(uint8_t howMany)
{
  // Sanity check
  if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE) return;

  if (howMany == 1) {
    uint8_t ptr = TinyWireS.receive();
    if (ptr < REG_SIZE) reg_ptr = ptr;
    return;
  }

  for (int a = 0; a < howMany; a++) {
    if ( a < 3) {
      i2cReceiveRegister[a] = TinyWireS.receive();
    }
    else {
      TinyWireS.receive();  // if we receive more data than allowed just throw it away
    }
  }

  newData = true;
}

void requestEvent()
{
  TinyWireS.send(i2cOutRegister[reg_ptr]);
}

void updateRegister(void) {
  if (i2cReceiveRegister[0] < REG_SIZE) {
    pwmRegister[i2cReceiveRegister[0]] = i2cReceiveRegister[1];
    statusRegister = i2cReceiveRegister[2];

    for (int a = 0; a < 8; a++) {
      i2cOutRegister[a] = pwmRegister[a];
    }

    i2cOutRegister[8] = statusRegister;
  }

  newData = false;
}

