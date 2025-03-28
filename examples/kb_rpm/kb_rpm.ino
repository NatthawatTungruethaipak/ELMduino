#include "BluetoothSerial.h"
#include "ELMduino.h"
BluetoothSerial SerialBT;
#define ELM_PORT SerialBT
#define DEBUG_PORT Serial
ELM327 myELM327;
typedef enum { ENG_RPM} obd_pid_states;
obd_pid_states obd_state = ENG_RPM;

float rpm = 0;
float rounded;
String str;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
Adafruit_8x16minimatrix matrix;
void setup()
{
#if LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif

    DEBUG_PORT.begin(115200);
    // SerialBT.setPin("1234");
    ELM_PORT.begin("ArduHUD", true);

    matrix.begin(0x70);
    matrix.setRotation(1);
    matrix.setTextSize(1);
    matrix.setTextColor(LED_ON);
    matrix.setTextWrap(false);

    if (!ELM_PORT.connect("OBDII"))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }

    if (!myELM327.begin(ELM_PORT, false, 2000))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    Serial.println("Connected to ELM327");
}

void printStringOnMatrix(String str) {
  int x = 0;  // X position for printing characters
  for (int i = 0; i < str.length(); i++) {
    if (str.charAt(i) == '.') {
      matrix.drawPixel(x - 2, 6, 1);  // Draw small dot for decimal point
    } else {
      matrix.setCursor(x, 0);
      matrix.print(str.charAt(i));  // Print individual character
      x += 8;  // Move right for next character
    }
  }
}

void drawHighFiveK() {
  // "k" will be at the last 3 pixels on the x-axis (columns 13, 14, 15)
  
  // Vertical line on the left side of "k" (x=13)
  matrix.drawPixel(13, 2, 1);  // Third part of "k"
  matrix.drawPixel(13, 3, 1);  // Fourth part of "k"
  matrix.drawPixel(13, 4, 1);  // Bottom part of "k"
  matrix.drawPixel(13, 5, 1);  // Top part of "k"
  matrix.drawPixel(13, 6, 1);  // Second part of "k"
  
  // Diagonal line for the "k" connecting middle of right side (x=14)
  matrix.drawPixel(14, 5, 1);  // Lower diagonal part of "k"
  
  // Diagonal line for the "k" connecting lower part (x=15)
  matrix.drawPixel(15, 4, 1);  // Lower part of "k"
  matrix.drawPixel(15, 6, 1);  // Lower part of "k"
}

void loop()
{
  switch (obd_state)
  {
    case ENG_RPM:
    {
      rpm = myELM327.rpm();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        Serial.print("rpm: ");
        Serial.println(rpm);
        rounded = roundf((rpm / 1000) * 10) / 10;
        str = String(rounded, 1);
        matrix.clear();
        printStringOnMatrix(str);
        drawHighFiveK();
        matrix.writeDisplay();
        delay(60);  // Delay between updates (adjust as needed)
        obd_state = ENG_RPM;
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        obd_state = ENG_RPM;
      }
      break;
    }
  }
}
