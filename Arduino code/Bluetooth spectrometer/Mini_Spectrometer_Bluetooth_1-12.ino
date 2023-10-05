// Imperial College London - Department of Chemistry
// Mini Spectrometer - Bluetooth
// Nick Brooks 2023
// Version NB1.12 5-10-23

#include <Adafruit_AS7341.h>
#include <ArduinoBLE.h>

/************* BLE UUIDs ******************/

BLEService spectrometerService("9ce6c285-27c0-4225-a406-39a1f545b62d"); // BLE Spectrometer Service

BLEUnsignedIntCharacteristic spectrometerLED1("a1fefc52-bed5-4e1e-8b51-69f71e1b79f5", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic spectrometerLED2("2d69768c-3668-4dfd-82b5-01d1f75b917f", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic spectrometerGain("9ec03f4a-5013-48af-99f6-581b976540d1", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic spectrometerExposure("62b16ea6-4210-44bd-b0e0-adac3abc6744", BLERead | BLEWrite);
BLECharacteristic spectrometerDetector("63b118cc-7df1-498c-be35-dfb15534f55c", BLERead | BLENotify, 16);


/************* AS7341 variables ******************/

Adafruit_AS7341 as7341;

const long interval = 20;           // minimum time between updates (milliseconds)
unsigned long previousMillis = 0;     // previous loop time

int val;

byte sensorData[16];

const int transmissionLedPin = 9; // pin to use for the transmission LED
const int fluorescenceLedPin = 5; // pin to use for the UV LED

// variables to hold sensor values.
uint16_t readings[12];
uint16_t stepSize = 3596; //This sets the resolution of the exposure time (see AS7341 documentation for more info)
/*
 10ms - 3596
 15ms -  5395
 20ms -  7193
 25ms -  8992
 30ms -  10790
 35ms -  12589
 40ms -  14387
 45ms -  16186
 50ms -  17985
 */


/********* Functions to run the spectrometer and LEDs *********/

void spectrometerInfo (void)
{
  Serial.println("Imperial College London - Department of Chemistry");
  Serial.println("Mini Spectrometer Bluetooth - Nick Brooks");
  Serial.println("Version NB1.12 - 05-10-23");
}

void setFluorescenceBrightness (int brightness) {
  analogWrite(fluorescenceLedPin, brightness);
  Serial.print("Fluorescence LED brightness: "); Serial.println(constrain(int(brightness),0,255));
  spectrometerLED2.writeValue(constrain(int(brightness),0,255));
}

void setTransmissionBrightness (int brightness) {
  analogWrite(transmissionLedPin, brightness);
  Serial.print("Transmission LED brightness: "); Serial.println(constrain(int(brightness),0,255));
  spectrometerLED1.writeValue(constrain(int(brightness),0,255));
}

void sensorRead(void) {
  bool timeOutFlag = timeOutCheck();
  if(as7341.checkReadingProgress() || timeOutFlag )
  { 
    digitalWrite(LED_BUILTIN, HIGH);
    if(timeOutFlag)
    {} //Recover/restart
    
    as7341.getAllChannels(readings);

    sensorData[0] = lowByte(readings[0]);
    sensorData[1] = highByte(readings[0]);
    sensorData[2] = lowByte(readings[1]);
    sensorData[3] = highByte(readings[1]);
    sensorData[4] = lowByte(readings[2]);
    sensorData[5] = highByte(readings[2]);
    sensorData[6] = lowByte(readings[3]);
    sensorData[7] = highByte(readings[3]);
    sensorData[8] = lowByte(readings[6]);
    sensorData[9] = highByte(readings[6]);
    sensorData[10] = lowByte(readings[7]);
    sensorData[11] = highByte(readings[7]);
    sensorData[12] = lowByte(readings[8]);
    sensorData[13] = highByte(readings[8]);
    sensorData[14] = lowByte(readings[9]);
    sensorData[15] = highByte(readings[9]);
    
    spectrometerDetector.writeValue(sensorData,16);
    as7341.startReading();
  }
  else {
        digitalWrite(LED_BUILTIN, LOW);
  }
}

bool timeOutCheck()
{
  return false;
}

void setExposure (int exposure) {
  as7341.setATIME(int((exposure/(0.00278*(stepSize+1))-1)));
  Serial.print (("Integration time: "));
  Serial.print ((1+long(as7341.getATIME()))*(1+long(as7341.getASTEP()))*0.00278);
  Serial.println ((" ms"));
  exposure = int((1+long(as7341.getATIME()))*(1+long(as7341.getASTEP()))*0.00278);
  spectrometerExposure.writeValue(round(((1+long(as7341.getATIME()))*(1+long(as7341.getASTEP()))*0.00278)));
}

void setGain (int gainIn) {
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
        spectrometerGain.writeValue(0);
        break;
      case AS7341_GAIN_1X:
        Serial.println(F("1x"));
        spectrometerGain.writeValue(1);
        break;
      case AS7341_GAIN_2X:
        Serial.println(F("2x"));
        spectrometerGain.writeValue(2);
        break;
      case AS7341_GAIN_4X:
        Serial.println(F("4x"));
        spectrometerGain.writeValue(3);
        break;
      case AS7341_GAIN_8X:
        Serial.println(F("8x"));
        spectrometerGain.writeValue(4);
        break;
      case AS7341_GAIN_16X:
        Serial.println(F("16x"));
        spectrometerGain.writeValue(5);
        break;
      case AS7341_GAIN_32X:
        Serial.println(F("32x"));
        spectrometerGain.writeValue(6);
        break;
      case AS7341_GAIN_64X:
        Serial.println(F("64x"));
        spectrometerGain.writeValue(7);
        break;
      case AS7341_GAIN_128X:
        Serial.println(F("128x"));
        spectrometerGain.writeValue(8);
        break;
      case AS7341_GAIN_256X:
        Serial.println(F("256x"));
        spectrometerGain.writeValue(9);
        break;
      case AS7341_GAIN_512X:
        Serial.println(F("512x"));
        spectrometerGain.writeValue(10);
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

void setup() {
  pinMode(9, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  delay(2000);
  
  Serial.println("Mini Spectrometer Serial Started");
  spectrometerInfo();
    if (!as7341.begin()){
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }


   // begin BLE initialization
   
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  Serial.println("Bluetooth Started");

  // set advertised local name and service UUID for spectrometer functions:
  BLE.setLocalName("Mini Spectrometer");
  BLE.setDeviceName("Mini Spectrometer");
  BLE.setAdvertisedService(spectrometerService);

  // add the characteristic to the service
  spectrometerService.addCharacteristic(spectrometerLED1);
  spectrometerService.addCharacteristic(spectrometerLED2);
  spectrometerService.addCharacteristic(spectrometerGain);
  spectrometerService.addCharacteristic(spectrometerExposure);
  spectrometerService.addCharacteristic(spectrometerDetector);

  // add service
  BLE.addService(spectrometerService);


  // set the initial value for the characeristic:

  as7341.setASTEP(stepSize);
  setGain(7);
  setExposure(100);
  
  setFluorescenceBrightness(0);
  setTransmissionBrightness(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Spectrometer Peripheral Started");
}


void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {

      sensorRead();
      
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (spectrometerLED1.written()) {
        val = spectrometerLED1.value();
        setTransmissionBrightness (val);
      }

      if (spectrometerLED2.written()) {
        val = spectrometerLED2.value();
        setFluorescenceBrightness (val);
      }

      if (spectrometerGain.written()) {
        val = spectrometerGain.value();
          setGain (val);
      }

      if (spectrometerExposure.written()) {
        val = spectrometerExposure.value();
        setExposure(val);
      }

      unsigned long currentMillis = millis();

      while (currentMillis - previousMillis < interval) {
        currentMillis = millis();
        delay(10);
      }

      previousMillis = currentMillis;          
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
