// Mini-Spectrometer Version 1 Arduino Firmware
// Imperial College London - Department of Chemistry
// Mini Spectrometer
// Nick Brooks 2021
// Version NB1.3 30-1-21

#include <Adafruit_AS7341.h>

Adafruit_AS7341 as7341;

const byte numChars = 32;
char receivedChars[numChars];

// variables to hold the parsed data
char messageFromPC = 0;
int integerFromPC = 0;
float floatFromPC = 0.0;

// variables to hold values.
uint16_t readings[12];
uint16_t stepSize = 9999; //This sets the resolution of the exposure time (see AS7341 documentation for more info)

boolean newData = false;

/********* Functions to run the spectrometer and LEDs *********/

void spectrometerInfo (void)
{
  Serial.println("Imperial College London - Department of Chemistry");
  Serial.println("Mini Spectrometer - Nick Brooks");
  Serial.println("Version NB1.3 - 30-1-21");
}

void setFluorescenceBrightness (float brightness) {
  analogWrite(5, brightness);
  Serial.print("Fluorescence LED brightness: "); Serial.println(constrain(int(brightness),0,255));
}

void setTransmissionBrightness (float brightness) {
  analogWrite(9, brightness);
  Serial.print("Transmission LED brightness: "); Serial.println(constrain(int(brightness),0,255));
}


/********* Functions to run the AS7341 sensor *********/

void sensorRead(void) {
  digitalWrite(LED_BUILTIN, HIGH);
    if (!as7341.readAllChannels(readings)){
    Serial.println("Error reading all channels!");
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
    digitalWrite(LED_BUILTIN, LOW);

    Serial.print("ADC0/F1 415nm : ");
    Serial.println(readings[0]);
    Serial.print("ADC1/F2 445nm : ");
    Serial.println(readings[1]);
    Serial.print("ADC2/F3 480nm : ");
    Serial.println(readings[2]);
    Serial.print("ADC3/F4 515nm : ");
    Serial.println(readings[3]);
    Serial.print("ADC0/F5 555nm : ");

  /*
  // we skip the first set of duplicate clear/NIR readings
  Serial.print("ADC4/Clear-");
  Serial.println(readings[4]);
  Serial.print("ADC5/NIR-");
  Serial.println(readings[5]);
  */

    Serial.println(readings[6]);
    Serial.print("ADC1/F6 590nm : ");
    Serial.println(readings[7]);
    Serial.print("ADC2/F7 630nm : ");
    Serial.println(readings[8]);
    Serial.print("ADC3/F8 680nm : ");
    Serial.println(readings[9]);
    Serial.print("ADC4/Clear    : ");
    Serial.println(readings[10]);
    Serial.print("ADC5/NIR      : ");
    Serial.println(readings[11]);
}

void setExposure (float exposure) {
  as7341.setATIME(int((exposure*1000)/(2.78*10000)));
  Serial.print (("Integration time: "));
  Serial.print ((1+long(as7341.getATIME()))*(1+long(as7341.getASTEP()))*0.00278);
  Serial.println ((" ms"));
}

void setGain (float gainIn) {
  switch (int(gainIn)) {
    case 0:
      as7341.setGain(AS7341_GAIN_0_5X);    // 0.5x gain
      break;
    case 1:
      as7341.setGain(AS7341_GAIN_1X);      // 1x gain
      break;
    case 2:
      as7341.setGain(AS7341_GAIN_2X);   // 2x gain
      break;
    case 3:
      as7341.setGain(AS7341_GAIN_4X);   // 4x gain
      break;
    case 4:
      as7341.setGain(AS7341_GAIN_8X);    // 8x gain
      break;
    case 5:
      as7341.setGain(AS7341_GAIN_16X);      // 16x gain
      break;
    case 6:
      as7341.setGain(AS7341_GAIN_32X);   // 32x gain
      break;
    case 7:
      as7341.setGain(AS7341_GAIN_64X);   // 64x gain
      break;
    case 8:
      as7341.setGain(AS7341_GAIN_128X);      // 128x gain
      break;
    case 9:
      as7341.setGain(AS7341_GAIN_256X);   // 256x gain
      break;
    case 10:
      as7341.setGain(AS7341_GAIN_512X);   // 512x gain
      break;
    }
    Serial.print  (F("Gain: "));
    as7341_gain_t gainOut = as7341.getGain();
    switch(gainOut)
    {
      case AS7341_GAIN_0_5X:
        Serial.println(F("0.5x"));
        break;
      case AS7341_GAIN_1X:
        Serial.println(F("1x"));
        break;
      case AS7341_GAIN_2X:
        Serial.println(F("2x"));
        break;
      case AS7341_GAIN_4X:
        Serial.println(F("4x"));
        break;
      case AS7341_GAIN_8X:
        Serial.println(F("8x"));
        break;
      case AS7341_GAIN_16X:
        Serial.println(F("16x"));
        break;
      case AS7341_GAIN_32X:
        Serial.println(F("32x"));
        break;
      case AS7341_GAIN_64X:
        Serial.println(F("64x"));
        break;
      case AS7341_GAIN_128X:
        Serial.println(F("128x"));
        break;
      case AS7341_GAIN_256X:
        Serial.println(F("256x"));
        break;
      case AS7341_GAIN_512X:
        Serial.println(F("512x"));
        break;
    }
}

void displaySensorDetails(void) {
  Serial.println  (F("Sensor: AS7341"));
    Serial.println  (F("Driver Ver: 1"));
    Serial.println  (F("Unique ID: 7341"));
    Serial.println  (F("Max Value: 65535"));
    Serial.println  (F("Min Value: 0"));
    Serial.println  (F("Resolution: 1"));
}


/********* Functions to handle serial communication with the computer *********/

// Receive data and extract the section delimited by <> marks

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

// Split received line from computer into command and data

void parseData() {      // split the data into its parts

  char* atofIndx;

  //First character represents command
  messageFromPC = receivedChars[0];

  //Remainder of received data converted to number
  atofIndx = receivedChars + 1;
  floatFromPC = atof(atofIndx);

}

// Process commands received from the computer

void showParsedData() {
  /*
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("Float ");
    Serial.println(floatFromPC);
  */

  if (messageFromPC == 'R') {
    sensorRead();
  }

  else if (messageFromPC == 'D'){
    displaySensorDetails();
  }

  else if (messageFromPC == 'F'){
    setFluorescenceBrightness (floatFromPC);
  }

  else if (messageFromPC == 'T'){
    setTransmissionBrightness (floatFromPC);
  }

  else if (messageFromPC == 'E'){
    setExposure (floatFromPC);
  }

  else if (messageFromPC == 'G'){
    setGain (floatFromPC);
  }

  else if (messageFromPC == 'I'){
      spectrometerInfo();
  }

  else {
    Serial.println("?");
  }

}


/********* Setup and loop functions run directly by the Arduino *********/

void setup() {
  pinMode(9, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  // Wait for communication with the host computer serial monitor
  while (!Serial) {
    delay(1);
  }

  spectrometerInfo();

  if (!as7341.begin()){
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }

  as7341.setATIME(int(1/(2.78*(stepSize+1))));
  as7341.setASTEP(stepSize);
  as7341.setGain(AS7341_GAIN_64X);
}


void loop() {
  recvWithStartEndMarkers();
  if (newData == true) {
    parseData();
    showParsedData();
    newData = false;
  }
}
