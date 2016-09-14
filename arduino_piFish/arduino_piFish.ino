#include <Wire.h>

char addr = 0x69;

char pinRed = 9;
char pinGreen = 10;
char pinBlue = 11;

int red = 50;
int green = 25;
int blue = 50;

String inString = "";

void setup() {
  // put your setup code here, to run once:
  pinMode(pinRed, OUTPUT);
  pinMode(pinGreen, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  Wire.begin(addr);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(pinRed, red);
  analogWrite(pinGreen, green);
  analogWrite(pinBlue, blue);

  Serial.print(red, DEC);
  Serial.print(", ");
  Serial.print(green, DEC);
  Serial.print(", ");
  Serial.print(blue, DEC);
  Serial.println();


  while (Serial.available() > 0) {
    if (Serial.read() == '%') {
      int _red = Serial.parseInt();
      int _green = Serial.parseInt();
      int _blue = Serial.parseInt();
      if (Serial.read() == '\n') {
        red = constrain(_red, 0, 255);
        green = constrain(_green, 0, 255);
        blue = constrain(_blue, 0, 255);
      }
    }
  }
}

void receiveEvent(int howMany) {
  delay(5);
  int reg = Wire.read(); // Ignored
  int _red = Wire.read();
  int _green = Wire.read();
  int _blue = Wire.read();
  
  if (_red != -1 && _green != -1 && blue != -1) {
    red = constrain(_red, 0, 255);
    green = constrain(_green, 0, 255);
    blue = constrain(_blue, 0, 255);
  }
}

void requestEvent() {
  char buf[4] = {0x00, (char)red, (char)green, (char)blue};
  Wire.write(buf, 4);
}
