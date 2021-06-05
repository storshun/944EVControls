
#include "OneWire.h"
#include "Stream.h"
#include <iostream>
#include <string>
//#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include "MCUFRIEND_kbv.h"
#include <DallasTemperature.h>
#include <interior_light_off.h>
#include <interior_light_on.h>

//CLIMATE CONTROL SYSTEM BY RANDALL LOWE ©2020
#define INT_TEMP_SENSOR 39
#define EXT_TEMP_SENSOR 40
#define ACPWMPIN 24
#define FANPWMPIN 25


#define MINPRESSURE 500
#define MAXPRESSURE 1000
//const int TS_LEFT=924,TS_RT=141,TS_TOP=135,TS_BOT=928; //Portrait
//LANDSCAPE CALIBRATION     320 x 240

////////////LCD SETUP
#define LCD_CS A3    // Chip Select goes to Analog 3
#define LCD_CD A2    // Command/Data goes to Analog 2
#define LCD_WR A1    // LCD Write goes to Analog 1
#define LCD_RD A0    // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

//COLOR VALUES
#define BLACK 0x0000
#define BLUE 0x001F
#define LIGHTBLUE 0x049A
#define RED 0xF800
#define ORANGE 0xFC40
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define DARKGRAY 0x4208
#define DARKGRAY2 0x4228
#define MEDGRAY 0xB596
#define LIGHTGRAY 0xAD55

//LIGHTING DEFINES
#define MINLUMENS 250
#define PHOTOSENSOR_PIN A13
#define INTERIORLIGHT_PIN 25
#define HEADLAMP_PIN 26

//SCREEN SCALE
//ILI9488
#define PXWIDTH 480
#define PXHEIGHT 320
float widthScalar = 0.0;
float heightScalar = 0.0;
const int STATICWIDTH = 320;
const int STATCIHEIGHT = 240;
//UI LAYOUT IS SETUP AT A 320x240 RESOLUTION AND THEN SCALED UP/DOWN AS NEEDED.
/*
  TouchScreen.h GFX Calibration
  Not possible to diagnose Touch pins on ARM or ESP32
  ID = 0x9488

  cx=92 cy=143 cz=293  X, Y, Pressure
  cx=504 cy=138 cz=503  X, Y, Pressure
  cx=927 cy=138 cz=541  X, Y, Pressure
  cx=97 cy=549 cz=257  X, Y, Pressure
  cx=915 cy=542 cz=459  X, Y, Pressure
  cx=100 cy=882 cz=231  X, Y, Pressure
  cx=505 cy=913 cz=400  X, Y, Pressure
  cx=924 cy=912 cz=478  X, Y, Pressure

*** COPY-PASTE from Serial Terminal:
  const int XP=6,XM=A2,YP=A1,YM=7; //320x480 ID=0x9488
  const int TS_LEFT=927,TS_RT=113,TS_TOP=939,TS_BOT=78;

  PORTRAIT  CALIBRATION     320 x 480
  x = map(p.x, LEFT=927, RT=113, 0, 320)
  y = map(p.y, TOP=939, BOT=78, 0, 480)

  LANDSCAPE CALIBRATION     480 x 320
  x = map(p.y, LEFT=939, RT=78, 0, 480)
  y = map(p.x, TOP=113, BOT=927, 0, 320)


  ATTEMPT 2 - SUCCESS
  TFT   OS Uno  TEENSY
  Gnd   GND     GND
  3v3   3V3     3.3v
  CS    A3      17 (A3)
  RS    A2      16 (A2)
  WR    A1      15 (A1)
  RD    A0      14 (A0)
  RST   RST     18 (A4)
  LED   GND     GND
  DB0   D8      D8
  DB1   D9      D9
  DB2   D10     D10
  DB3   D11     D11
  DB4   D4      D4
  DB5   D13     D5
  DB6   D6      D6
  DB7   D7      D7
*/

//const int XP = 6, XM = A2, YP = A1, YM = 7;                       //480x320 ili9488
//const int TS_LEFT = 135, TS_RT = 928, TS_TOP = 141, TS_BOT = 924; //Landscape

const int XP = 6, XM = A2, YP = A1, YM = 7; //320x480 ID=0x9488
const int TS_LEFT = 927, TS_RT = 113, TS_TOP = 939, TS_BOT = 92;

//UTFT

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;
uint16_t xpos, ypos;
uint16_t *pXpos = &xpos;
uint16_t *pYpos = &ypos;

//TEMP SETUP
//Create an instance of Onewire to communicate with any OneWire devices (not just Maxim/Dallas temp ICs)
OneWire InteriorSensor(INT_TEMP_SENSOR);
OneWire ExteriorSensor(EXT_TEMP_SENSOR);

//Pass our oneWire reference to Dallas Temperature
DallasTemperature InteriorSensors(&InteriorSensor);
DallasTemperature ExteriorSensors(&ExteriorSensor);

//Software button input debounce - NO LONGER USED AS NO PHYSICAL BUTTONS
// int counter = 0;
// int *counterP = &counter;
// const int debounce_count = 12;
// int current_state = LOW;
// int *current_stateP = &current_state;
// long cTime = 0;
// long *cTimeP = &cTime;

//GLOBAL CLIMATE CONTROL ENABLED/DISABLED
bool bCCEnabled = true; // bCCEnabled false means the system is off, true means it is on. Pressing any modal button (fan, AC, or OFF) will enable it.
bool *pCCEnabled = &bCCEnabled;
//int OFF_LED_PIN = 52;

//FAN CONTROL
int fanOn = 0;        //Simple state tracker for the "fan" motor. 0 - 4 Power Levels
int *pFanOn = &fanOn; //Pointer for fanOn
float fanDutyCycle = 0.00;
float *pfanDutyCycle = &fanDutyCycle;

//A/C Compressor Request On/Off
bool acCompReq = false;        //Simple state tracker for the "A/C" (LED)
bool *pACCompReq = &acCompReq; //Pointer for acCompReq

//Temp Control
//Clamp to 60F and 80F, default to 70F. Need to make this retain temperature setting at some point.
int desiredTemp = 70;
int *pDesiredTemp = &desiredTemp;
int hotClamp = 80;  //This clamps the heat to 80F max
int coldClamp = 60; //This clamps the cold to 60F min
int *hotMax = &hotClamp;
int *coldMin = &coldClamp;
const std::string upOutline("GREEN");
const std::string upFill("MAGENTA");

//EXTERIOR TEMP
float currentTempExt = 0.0; //Init the outside temp thermistor
int uintCurrentExtTemp = 50;
int *pCurrentExtTemp = &uintCurrentExtTemp;
int lastExtMeasuredTemp = 0;
int *pLastExtMeasuredTemp = &lastExtMeasuredTemp;

//INTERIOR TEMP
float currentIntTemp = 76.0;                //Init the interior temp sensor to an arbitrary value
int uintCurrentIntTemp = 250;               //Whole number needed for other operations (Initialized to a garbage number for the purpose of initializing the home screen.)
int *pCurrentIntTemp = &uintCurrentIntTemp; //pointer for reading the interior temp
int lastIntMeasuredTemp = 0;                //store the last interior temperature to reduce sampling.
int *pLastIntMeasuredTemp = &lastIntMeasuredTemp;

//CLIMATE CONTROL MODE
int climateSystemMode = 0;                    //0 = off, 1 = cool, 2 = heat
int *pClimateSystemMode = &climateSystemMode; //pointer for the above variable.
float ClimateControlDutyCycle = 0.00;         //This is the percentage applied to the PWM calculation.
float *pCCDutyCycle = &ClimateControlDutyCycle;
int climatePWM = 0; //This will be the 0-255 value for the PWM
int loopCounter = 1;

//SECTION 2: LIGHTING CONTROL
//TODO: Setup IR light reader for ambient light.
int photoSensorValue = 0;
int lumenSampleCount = 0;
bool LightsOn = false;
bool interiorLightState = false;
bool HeadLampState = false;
bool HeadLampMode = 1; //HeadLampMode 1 is AUTOMATIC, 0 is MANUAL. Manual is set by using the touch button to On the lights when off. Auto is enabled by default, and re-enabled by turning the lights to Off from On.


int stateOfCharge = 50; //Establish an initial State of Charge for the vehicle. Need to collect this data from somewhere, then round it to an int.

///////////////////////////SETUP///////////////////////////
void setup()
{
  //TCCR5B = TCCR5B & B11111000 | B00000010;  // for PWM frequency of  3921.16 Hz
  //TCCR5B = TCCR5B & B11111000 | B00000100; // for PWM frequency of   122.55 Hz
  //////////////BGIN LCD DISPLAY SETUP//////////////
  digitalWrite(17, LOW);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  InteriorSensors.begin();
  ExteriorSensors.begin();
  Serial.begin(9600);
  widthScalar = PXWIDTH / STATICWIDTH;
  heightScalar = PXHEIGHT / STATCIHEIGHT;

  pinMode(PHOTOSENSOR_PIN, INPUT);

  //////////////END LCD DISPLAY SETUP//////////////
  DrawHomeScreen();
}

///////////////////////////LOOP////////////////////////////
void loop()
{
  //Initialize the home screen with accurate data (only happens on the first loop, because invalid data in the uintCurrentIntTemp value)
  if (uintCurrentIntTemp == 250) //Populate the screen with relevant data, but don't act on it here.
  {
    GetIntTemp();
    UpdateIntTempDisplay();

    GetExtTemp();
    UpdateExtTempDisplay();

    UpdateDesiredTempDisplay(desiredTemp);
  }
  
  //TODO: Sample the lighting values for 5 seconds
  if ((loopCounter % 1000) == 0)
  {
    while (lumenSampleCount < 50)
    {
      int newValueForLumens = analogRead(PHOTOSENSOR_PIN);
      photoSensorValue += newValueForLumens;
      //Serial.println("Photosensor value at: " + newValueForLumens);
      lumenSampleCount++;
    }
  }
  if (lumenSampleCount == 50)
  {
    photoSensorValue = photoSensorValue / lumenSampleCount;

    Serial.print("Photosensor average: ");
    Serial.println(photoSensorValue);
    if ((photoSensorValue <= MINLUMENS) && (!LightsOn) && (HeadLampMode))
    {
      ToggleHeadLamps(1);

    }
    else if (( photoSensorValue > MINLUMENS) && (LightsOn) && (HeadLampMode))
    {
      ToggleHeadLamps(0);
    }
    lumenSampleCount = 0;
    photoSensorValue = 0;
  }



  HandleTouchInput();

  if (climateSystemMode == 1) //If the climate control is in cooling mode output the duty cycle as PWM
  {
    climatePWM = round(ClimateControlDutyCycle * 256); //figure out the PWM to the nearest whole number
    analogWrite(46, climatePWM);                       //And ouput it. ///////NEED TO FIND A DIFFERENT WAY TO DO THIS?
    //Serial.print("\n\t\t\tPWM via pin 46 is ");
    // Serial.println(climatePWM);
  }
  else
  {
    analogWrite(46, LOW);
  }

  ////Determine the Mode the Climate system should be in
  ///////////////////////////////////////////////////////////////////////
  if (loopCounter > 2000)
  {

    GetIntTemp();                                  //Get the interior temperature
    if (uintCurrentIntTemp != lastIntMeasuredTemp) //If the current temp is different from the last reading...
    {
      UpdateIntTempDisplay();
      lastIntMeasuredTemp = uintCurrentIntTemp; //Update the last stored interior temp with new temp.

      if (bCCEnabled) //If it's on, then...
      {
        getACDutyCycle(desiredTemp, uintCurrentIntTemp); //Figure out of the duty cycle output needs to change, or mode changes are needed
        // Serial.print("\nPWM is set to: ");
        // Serial.println(ClimateControlDutyCycle);
      }
    }
    GetExtTemp();
    if (uintCurrentExtTemp != lastExtMeasuredTemp)
    {
      UpdateExtTempDisplay();
      lastExtMeasuredTemp = uintCurrentExtTemp;
    }
    loopCounter = 0;
  }

  loopCounter++;
}

//functions

//LIGHTING

void ToggleHeadLamps(bool HeadLampStateAbs)
{
  if ((HeadLampStateAbs) && (HeadLampMode))
  {
    digitalWrite(HEADLAMP_PIN, HIGH);
    DrawHLCapsuleOn();
    LightsOn = true;
    Serial.println("Turning on headlights");
  }
  else if (!HeadLampStateAbs)
  {
    digitalWrite(HEADLAMP_PIN, LOW);
    DrawHLCapsuleOff();
    LightsOn = false;
    Serial.println("Turning off headlights");
  }
  else if ((HeadLampStateAbs) && (!HeadLampMode))
  {
    digitalWrite(HEADLAMP_PIN, HIGH);
    DrawHLCapsuleOnManual();
    LightsOn = true;
    Serial.println("Turning on headlights");
  }

}

void DrawHLCapsuleOn()
{
  tft.drawRoundRect(320, 70, 125, 50, 25, LIGHTGRAY);
  tft.fillRoundRect(317, 72, 125, 50, 25, YELLOW);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(327, 92);
  tft.print("Headlight");
  
}
void DrawHLCapsuleOnManual()
{
  tft.drawRoundRect(320, 70, 125, 50, 25, LIGHTGRAY);
  tft.fillRoundRect(317, 72, 125, 50, 25, YELLOW);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(327, 92);
  tft.print("Headlight");
  tft.setTextColor(WHITE, DARKGRAY);
  tft.setCursor(356, 125);
  tft.print("     ");
  
}

void DrawHLCapsuleOff()
{
  tft.drawRoundRect(320, 70, 125, 50, 25, LIGHTGRAY);
  tft.fillRoundRect(317, 72, 125, 50, 25, MEDGRAY);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(327, 92);
  tft.print("Headlight");
  tft.setTextColor(WHITE);
  tft.setCursor(356, 125);
  tft.print("AUTO");
}

void InteriorLightsToggle()
{
  if (!interiorLightState)
  {
    digitalWrite(INTERIORLIGHT_PIN, HIGH);
    DrawILCapsuleOn();
    interiorLightState = true;
  }
  else
  {
    digitalWrite(INTERIORLIGHT_PIN, LOW);
    DrawILCapsuleOff();
    interiorLightState = false;
  }
}

void DrawILCapsuleOn()
{
  tft.drawRoundRect(320, 15, 125, 50, 25, LIGHTGRAY);
  tft.fillRoundRect(317, 17, 125, 50, 25, YELLOW);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(329, 37);
  tft.print("Interior");
}

void DrawILCapsuleOff()
{
  tft.drawRoundRect(320, 15, 125, 50, 25, LIGHTGRAY);
  tft.fillRoundRect(317, 17, 125, 50, 25, MEDGRAY);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(329, 37);
  tft.print("Interior");
}

//CLIMATE CONTROL
void getACDutyCycle(int desiredTemp, int currentTemp)
{ // This does a little more than set the AC Duty Cycle.
  //It compares the inside temp to the goal temp to determine which mode the system should be in.
  //It also evaluates how far from the goal the temperature is, and scales the PWM to the A/C compressor (and possibly other motors) based on that relative distance.
  //A delta of 6 degrees will max the AC compressor PWM to .9 (min cooling for tesla PWM-based A/C comprssors) or min of .05 (max cooling for the same compressors)
  //The function will also set the climate system mode to 0, 1, or 2 depending on the delta and direction from goal
  //Lastly, the function will also set the flags for the enable flags for the A/C Compressor and the fan(s)

  if (desiredTemp > currentTemp) //Check for HEATING mode
  {
    *pCCDutyCycle = 1 - (abs(currentTemp - desiredTemp) / 5.5); //Eval the duty cycle for PWM to DEVICE TO BE DETERMINED (Probalby tesla heating coils) Just keeping this here for now

    *pClimateSystemMode = 2; //Sets the climate control system to heating mode
    ClampDutyCycle(currentTemp);

    SetFanPower();

    DrawAutoButton(3); //Draw the Auto button in Heating mode

    *pACCompReq = false; //Don't need a/c while heating typically
  }
  else if (desiredTemp < currentTemp) //check for COOLING mode
  {
    *pCCDutyCycle = 1 - (abs(currentTemp - desiredTemp) / 5.5); //Evaluate the duty cycle for PWM to A/C compressor

    *pClimateSystemMode = 1; //Sets the climate control system to cooling mode
    ClampDutyCycle(currentTemp);

    SetFanPower();

    *pACCompReq = true;

    DrawAutoButton(2); //Draw the Auto button in Cooling mode
  }
  else if (desiredTemp == currentTemp) //SLEEP the CC System
  {
    SleepClimateControl();
    DrawAutoButton(4);
  }
}
void ClampDutyCycle(int currentTemp)
{
  //Clamp values for AC compressor PWM duty cycle to 5% or 90%.
  if ((currentTemp - desiredTemp > 5) && (*pClimateSystemMode == 1))
  {
    //Serial.print("\nSystem clamped to max A/C performance\n");
    *pCCDutyCycle = 0.05;
  }
  else if ((desiredTemp - currentTemp > 5) && (*pClimateSystemMode == 2))
  {
    //Serial.print("\nSystem clamped to max heat performance\n");
    *pCCDutyCycle = 0.05;
  }
}
void DisableClimateControl()
{
  *pCCEnabled = 0; //Set Climate Control to disabled
  DrawAutoButton(4);
  *pFanOn = 0; //Turn off the fan
  DrawFanPower();
  *pACCompReq = 0;        //Let the A/C compressor turn off
  *pCCDutyCycle = 0.9000; //Set the duty cycle to the A/C to outside operational range
}
void SleepClimateControl()
{
  *pCCEnabled = 1; //Set Climate Control to enabled to watch for changes in temp
  DrawAutoButton(4);
  *pFanOn = 0; //Turn off the fan
  DrawFanPower();
  *pACCompReq = 0;        //Let the A/C compressor turn off
  *pCCDutyCycle = 0.9000; //Set the duty cycle to the A/C to outside operational range
}
void WakeClimateControl()
{
  *pCCEnabled = 1; //Set Climate Control to enabled to watch for changes in temp
  DrawAutoButton(1);
  Serial.println("Desired Temp is " + uintCurrentIntTemp);
  getACDutyCycle(desiredTemp, uintCurrentIntTemp);
}

//FAN CONTROL
void ToggleFanPower(int fanModifier)
{
  int fanPower = *pFanOn;
  fanPower = fanPower + fanModifier;
  if (fanPower > 4)
  {
    *pFanOn = 4;
  }
  else if (fanPower < 1)
  {
    *pFanOn = 0;
    DisableClimateControl();
  }
  else
  {
    *pFanOn = fanPower;
  }
  DrawFanPower();
}
void SetFanPower()
{
  *pFanOn = round(4 * (1 - ClimateControlDutyCycle));             // This sets the fan power as an int for display purposes (0-4)
  *pfanDutyCycle = (abs(*pCurrentIntTemp - *pDesiredTemp) / 5.5); //Sets the PWM duty cycle for the actual fan motor

  if (*pfanDutyCycle >= 1.00)
    *pfanDutyCycle = 1;
  else if (*pfanDutyCycle <= 0.00)
    *pfanDutyCycle = 0.00;

  if (fanOn < 1)
  {
    SleepClimateControl();
  }
  DrawFanPower();
  UpdateFanPWM();
}
void DrawFanPower()
{
  //Serial.print("\n\tFan Power is: " + String(*pFanOn));

  tft.setCursor(184, 132);
  tft.setTextColor(WHITE, DARKGRAY2);
  tft.setTextSize(3);
  tft.print(*pFanOn);
}
void UpdateFanPWM()
{
  int fanPWMValue = 0;
  fanPWMValue = round(fanDutyCycle * 256);
  Serial.println("The fan PWM is" + fanPWMValue);
  analogWrite(FANPWMPIN, fanPWMValue);
}

//INTERIOR TEMP AND DISPLAY
void GetIntTemp()
{
  InteriorSensors.requestTemperatures(); //Command to get temps
  *pCurrentIntTemp = round(InteriorSensors.getTempFByIndex(0));
}
void UpdateIntTempDisplay()
{
  tft.setCursor(215, 20);
  tft.setTextColor(WHITE, DARKGRAY);
  tft.setTextSize(2);
  tft.print(*pCurrentIntTemp);
}

//EXTERIOR TEMP AND DISPLAY
void GetExtTemp()
{
  ExteriorSensors.requestTemperatures(); //Command to get temps
  *pCurrentExtTemp = round(ExteriorSensors.getTempFByIndex(0));
}
void UpdateExtTempDisplay()
{
  tft.setCursor(215, 55);
  tft.setTextColor(WHITE, DARKGRAY);
  tft.setTextSize(2);
  tft.print(*pCurrentExtTemp);
}

//DESIRED TEMPERATURE INPUT AND DISPLAY
int ModifyDesiredTemp(int tempChangeDirection, int desiredTemp)
{
  if ((desiredTemp + tempChangeDirection <= *hotMax) && (desiredTemp + tempChangeDirection >= *coldMin))
  {
    desiredTemp = desiredTemp + tempChangeDirection;
    //Serial.print("Desired Temp is now " + desiredTemp);
    UpdateDesiredTempDisplay(desiredTemp);
    return desiredTemp;
  }
  else if ((desiredTemp + tempChangeDirection > *hotMax) || (desiredTemp + tempChangeDirection < *coldMin))
  {
    //Serial.println("I'm here");
    return desiredTemp;
  }
  return 0;
}
void UpdateDesiredTempDisplay(int desiredTemp)
{
  tft.setCursor(20, 105);
  tft.setTextColor(WHITE, DARKGRAY2);
  tft.setTextSize(4);
  tft.print(desiredTemp);
}
void DrawUpTriangle(int x, int y)
{
  //Outline
  x = round(x * widthScalar);
  y = round(y * heightScalar);
  int topX = x ;
  int topY = y - (round(heightScalar * 25));
  int leftX = x - (round(widthScalar * 32));
  int leftY = y + (round(heightScalar * 25));
  int rightX = x + (round(widthScalar * 32));
  int rightY = y + (round(heightScalar * 25));
  tft.drawTriangle(leftX, leftY, topX, topY, rightX, rightY, ORANGE);

  //Outline
  x = round(x * widthScalar);
  y = round(y * heightScalar);
  topX = x ;
  topY = y - (round(heightScalar * 24));
  leftX = x - (round(widthScalar * 31));
  leftY = y + (round(heightScalar * 24));
  rightX = x + (round(widthScalar * 31));
  rightY = y + (round(heightScalar * 24));
  tft.fillTriangle(leftX, leftY, topX, topY, rightX, rightY, RED);
}
void DrawDownTriangle(int x, int y)
{
  //Outline
  x = round(x * widthScalar);
  y = round(y * heightScalar);
  int topX = x ;
  int topY = y + (round(heightScalar * 25));
  int leftX = x - (round(widthScalar * 32));
  int leftY = y - (round(heightScalar * 25));
  int rightX = x + (round(widthScalar * 32));
  int rightY = y - (round(heightScalar * 25));
  tft.drawTriangle(leftX, leftY, topX, topY, rightX, rightY, LIGHTBLUE);

  //Outline
  x = round(x * widthScalar);
  y = round(y * heightScalar);
  topX = x ;
  topY = y + (round(heightScalar * 24));
  leftX = x - (round(widthScalar * 31));
  leftY = y - (round(heightScalar * 24));
  rightX = x + (round(widthScalar * 31));
  rightY = y - (round(heightScalar * 24));
  tft.fillTriangle(leftX, leftY, topX, topY, rightX, rightY, BLUE);
}


//MAIN UI
void DrawHomeScreen()
{
  //BACKGROUND
  tft.fillScreen(DARKGRAY);

  //TEST TRIANGLE

  //DRAW THE HOT UP ARROW

  DrawUpTriangle(40, 56);
  //tft.drawTriangle(40, 30, 8, 80, 72, 80, ORANGE);
  //tft.fillTriangle(40, 32, 10, 78, 70, 78, RED);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.setCursor(30, 50);
  tft.print("+");

  //DRAW THE COLD DOWN ARROW
  //tft.drawTriangle(40, 210, 8, 160, 72, 160, BLUE); //Ys 160 and 210, need to reverse
  //tft.fillTriangle(40, 207, 10, 162, 70, 162, LIGHTBLUE);
  DrawDownTriangle(40, 182);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.setCursor(30, 158);
  tft.print("-");

  //Print "Current" text
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(90, 20);
  tft.print("Current");

  //Auto is handled by  DrawAutoButton()

  //Print "Outside" text
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(90, 55);
  tft.print("Outside");

  //Draw Fan - Button
  tft.drawRoundRect(92, 110, 78, 60, 10, LIGHTGRAY);
  tft.fillRoundRect(90, 112, 78, 58, 10, MEDGRAY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(98, 135);
  tft.print("FAN -");

  //Draw Fan + Button
  tft.drawRoundRect(217, 110, 78, 60, 10, LIGHTGRAY);
  tft.fillRoundRect(215, 112, 78, 58, 10, MEDGRAY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(225, 135);
  tft.print("FAN +");

  //Draw Auto Mode Button
  tft.drawRoundRect(92, 173, 78, 60, 10, LIGHTGRAY);
  tft.fillRoundRect(90, 175, 78, 58, 10, MEDGRAY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(106, 195);
  tft.print("AUTO");

  //Draw Off Mode Button
  tft.drawRoundRect(217, 173, 78, 60, 10, LIGHTGRAY);
  tft.fillRoundRect(215, 175, 78, 58, 10, MEDGRAY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(235, 195);
  tft.print("OFF");

  //Draw Int. Light Button
  DrawILCapsuleOff();

  //Draw Headlight Button
  DrawHLCapsuleOff();
}
void DrawAutoButton(int buttonColor)
{
  tft.drawRoundRect(92, 173, 78, 60, 10, LIGHTGRAY);

  switch (buttonColor)
  {
    case 1:
      tft.fillRoundRect(90, 175, 78, 58, 10, MEDGRAY);
      tft.setTextColor(WHITE, MEDGRAY);
      tft.setTextSize(2);
      tft.setCursor(106, 195);
      tft.print("AUTO");
      break;

    case 2:
      tft.fillRoundRect(90, 175, 78, 58, 10, LIGHTBLUE);
      tft.setTextColor(WHITE, LIGHTBLUE);
      tft.setTextSize(2);
      tft.setCursor(106, 195);
      tft.print("COOL");
      break;

    case 3:
      tft.fillRoundRect(90, 175, 78, 58, 10, RED);
      tft.setTextColor(WHITE, RED);
      tft.setTextSize(2);
      tft.setCursor(106, 195);
      tft.print("HEAT");
      break;

    case 4:
      tft.fillRoundRect(90, 175, 78, 58, 10, DARKGRAY2);
      tft.setTextColor(WHITE, DARKGRAY2);
      tft.setTextSize(2);
      tft.setCursor(102, 195);
      tft.print("SLEEP");
      break;

    default:
      tft.fillRoundRect(90, 175, 78, 58, 10, MEDGRAY);
      tft.setTextColor(WHITE, MEDGRAY);
      tft.setTextSize(2);
      tft.setCursor(106, 195);
      tft.print("AUTO");
      break;
  }
}

/////////////TOUCH CONTROLS//////////////////
//Trigger the read from tp
void ReadTouchInput()
{
  //Serial.println("Getting touchpoint now!");
  tp = ts.getPoint();
  pinMode(YP, OUTPUT); //restore shared pins
  pinMode(XM, OUTPUT);
  //Serial.println("TouchZ is " + String(tp.z));
}
//debounce the read
bool Touching()
{
  int counter = 0;
  int fail = 0;
  while ((counter <= 6) && (fail <= 20))
  {
    ReadTouchInput();
    //Serial.println("\tcounter is " + String(counter));

    //Serial.println("\tFail is " + String(fail));

    if (tp.z > 400)
    {
      counter++;
    }
    else
    {
      fail++;
    }
  }
  if (counter >= 6)
    return true;
  else
    return false;
}
//read the x/y values
void GetTouchScreenPointXY()
{

  *pXpos = map(tp.y, TS_LEFT, TS_RT, 0, 480);
  *pYpos = map(tp.x, TS_TOP, TS_BOT, 0, 320);

  Serial.print("XPos:\t");
  Serial.print(xpos);
  Serial.print("\tYPos:\t");
  Serial.println(ypos);
}
//trigger the appropriate action
void HandleTouchInput()
{
  if (Touching())
  {
    GetTouchScreenPointXY();
    TakeTouchAction();
  }
  else
    return;
}
//and compare them to touch zones on the screen
void TakeTouchAction()
{
  if ((xpos >= 0) && (xpos <= 55) && (ypos >= 225) && (ypos <= 300)) //Temp Increase  touch zone
  {
    desiredTemp = ModifyDesiredTemp(1, desiredTemp);
    //Serial.println("\nMake it warmer");
    getACDutyCycle(desiredTemp, uintCurrentIntTemp);
    delay(400);
  }
  else if ((xpos >= 0) && (xpos <= 55) && (ypos >= 85) && (ypos <= 160)) //Temp Decrease  touch zone
  {
    desiredTemp = ModifyDesiredTemp(-1, desiredTemp);
    //Serial.println("\nMake it colder");
    getACDutyCycle(desiredTemp, uintCurrentIntTemp);
    delay(400);
  }
  else if ((xpos >= 66) && (xpos <= 156) && (ypos >= 146) && (ypos <= 210)) //Fan - button touch zone
  {
    //Serial.println("\nDecrease Fan Speed");
    ToggleFanPower(-1);
    delay(400);
  }
  else if ((xpos >= 205) && (xpos <= 291) && (ypos >= 146) && (ypos <= 210)) //Fan + Button touch zone
  {
    //Serial.println("\nIncrease Fan Speed");
    ToggleFanPower(1);
    delay(400);
  }
  else if ((xpos >= 66) && (xpos <= 156) && (ypos >= 79) && (ypos <= 143)) //Auto Mode  touch zone wakes the climate control system
  {
    //Serial.println("\nWake Climate Control");
    WakeClimateControl();
    delay(400);
  }
  else if ((xpos >= 205) && (xpos <= 291) && (ypos >= 79) && (ypos <= 143)) //System Override Off touch zone
  {
    //Serial.println("\nTurn System Off");
    DisableClimateControl();
    delay(400);
  }  
  else if ((xpos >= 316) && (xpos <= 460) && (ypos >= 262) && (ypos <= 310))  //Interior Lights Toggle
  {
    //Serial.println("\nTurn System Off");
    InteriorLightsToggle();
    delay(400);
  }  
  else if ((xpos >= 316) && (xpos <= 460) && (ypos >= 200) && (ypos <= 257)) //Interior Lights Toggle
  {
    //Serial.println("\nTurn System Off");
    if (!LightsOn)
    {
      HeadLampMode = 0;
      ToggleHeadLamps(1);
    }
    else
    {
      HeadLampMode = 1;
      ToggleHeadLamps(0);
    }
    
    delay(400);
  }
}

//////TASKS//////
//Deactivate A/C  or heat once temp is achieved - Sort of done, system gets set to proper state, but not hooked up to any components.
//Need to store and retrieve the last desiredtemp on start-up
//Shut system down if SOC is less than 10%.
