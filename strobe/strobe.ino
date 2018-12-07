
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include "Button.h"
//#include "MPU6050.h"



#define SERVOPIN 12                   // SERVO WRITE PIN
#define SHADE_MAXDISP 30              // SHADE DISPLACEMENT
#define SHADE_EPSILON 3               // Within 3 degrees
#define SAMPLE_TIME 1000              // 1 sample / update per second

#define LIGHTSENSORPIN 0              // AMBIENT LIGHT SENSOR reading pin
#define LIGHTSENSOR_MAXVAL 150.0      //    light sensor max value to consider. 1023 is too high

#define STRIP_PIN 8                   // LIGHT STRIP IN pin
#define DEFAULT_BRIGHTNESS 100         // LIGHT STRIP default brightness
#define STRIP_MAXBRIGHTNESS 255       // max brightness
#define BRIGHTNESS_INCREMENT 10

#define BUTTON_PIN 7                  // Button pin

// Servo
Servo servo;

// RGB LED light
// 24 lights, pin 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, STRIP_PIN, NEO_GRBW + NEO_KHZ800);

// Button
// 3 states for off, default, and study
Button button(BUTTON_PIN, 3);


void setup() {

    // AMBIENT LIGHT SENSOR
    pinMode(LIGHTSENSORPIN,  INPUT); 
    
    // SERVO 
    servo.attach(SERVOPIN); // Connect the Servo
    
    // LIGHTS 
    strip.begin();
    _turnLightsOff();

    // BLUTOOTH motion sensor - https://howtomechatronics.com/tutorials/arduino/how-to-configure-pair-two-hc-05-bluetooth-module-master-slave-commands/
    Serial.begin(9600); // Default communication rate of the Bluetooth module
    Serial.println("BEGIN");
}

//////////////////////////////////// MAIN LOOP ///////////////////////////////////
/////////// SETUP SOME COLORS /////////////////////////

// blue-ish light values
const int studyr = 170;
const int studyg = 228;
const int studyb = 225;

// yellow-ish light values
const int defaultr = 244;
const int defaultg = 182;
const int defaultb = 66;

////////////////// SETUP MODE STATES //////////////////
const int DEFAULT_MODE   = 0;
const int STUDY_MODE     = 1;
const int OFF_MODE       = 2;
int mode                 = OFF_MODE;

////////////////// SETUP STUDY MODE STATES ///////////
const int AWAKE = 0;
const int SLEEPY = 1;
const int ASLEEP = 2;
const int WAKING = 3;
int userState = AWAKE;

const int MOVEMENT_THRESHOLD = 7000;        // 30 seconds
unsigned long lastMovementTime = millis();
int currentBrightness = DEFAULT_BRIGHTNESS;

///// STROBING state info
unsigned long strobeColorDuration = 300;          // on for 300 ms
unsigned long strobeColorStart = millis();        // When we start the flashing on portion

unsigned long strobeOffDuration = 80;             // off for 80 ms
unsigned long strobeOffStart = millis();          // when we start the flashing off portion
bool isStrobeOff = true;

// red light for the strobing
const int strober = 255;
const int strobeg = 0;
const int strobeb = 0;

///////////////// BUTTON STATE //////////////////////
int lastButtonPress = button.getState();
unsigned long lastAdjustTime = millis();

void loop(){
  
  // Read in movement from the bluetooth serial
  char movement = 0;
  if(Serial.available()){
    movement = Serial.read();
    Serial.println(String(millis()) + " " + movement);
  }
  
  switch(mode){
 /////// MODE - OFF //////////
    case OFF_MODE:
      if(button.getState() != lastButtonPress){
     //   Serial.println("CHANGED TO DEFAULT MODE");
        _setStripColorRGB(defaultr, defaultg, defaultb, DEFAULT_BRIGHTNESS);
        mode = DEFAULT_MODE;
        lastButtonPress = button.getState();
      }
      break;
 //////// MODE - DEFAULT ////////
 //////// Assumes the light is already set to yellow
    case DEFAULT_MODE:
      // adjust the servo every second
      if(millis() - lastAdjustTime >= SAMPLE_TIME){
     //     Serial.println("Adjusting servo in deafult mode");
          _adjustServo();
          lastAdjustTime = millis();
      }
      // Check for mode change
      if (button.getState() != lastButtonPress){
     //   Serial.println("CHANGED TO STUDY MODE");
        _setStripColorRGB(studyr, studyg, studyb, DEFAULT_BRIGHTNESS);
        mode = STUDY_MODE;
        userState = AWAKE;    // bug here. make sure you reset the user state as well
        lastAdjustTime = millis();
        lastMovementTime  = millis();
        lastButtonPress = button.getState();
      }
      break;
 ///////// MODE - STUDY /////////
 ///////// Assumes the light is already set to blue
    case STUDY_MODE:
      // check for mode change
      if(button.getState() != lastButtonPress){
   //     Serial.println("CHANGED TO OFF");
        _turnLightsOff();
        mode = OFF_MODE;
        lastButtonPress = button.getState();
      }

      // Check the motion value we read from the bluetooth serial
      if (movement == '1'){
        lastMovementTime = millis();
    //    Serial.println("READ MOVEMENT");
      }
  
      //////////// INTO THE USER WAKE-SLEEP STATE CYCLE /////////////////
      switch(userState){

        case AWAKE:
            // adjust the servo every second
            if(millis() - lastAdjustTime >= SAMPLE_TIME){
                  _adjustServo();
                  lastAdjustTime = millis();
            }

            // Change state. User hasn't moved for a while and is SLEEPY 
            if(millis() - lastMovementTime >= MOVEMENT_THRESHOLD){
                userState = SLEEPY;
           //    Serial.println("CHANGED TO SLEEPY");
            }
            break;
            
        case SLEEPY:
        
            // Do sleepy adjustments once a second 

            // Hasn't been a second, so skip this iteration
            if(millis() - lastAdjustTime < SAMPLE_TIME){
              break;
            }
            
            lastAdjustTime = millis();
            
            // Shade is already open all the way
            // so incrementally increase the brightness
            if(abs(servo.read() - SHADE_MAXDISP) <= SHADE_EPSILON){
                if(currentBrightness < STRIP_MAXBRIGHTNESS){
                    currentBrightness = min(STRIP_MAXBRIGHTNESS, currentBrightness + BRIGHTNESS_INCREMENT); 
                    _increaseBrightness(currentBrightness);
                } else {
                    // STUDY MODE - SLEEPY --> everything is maxed out, start strobing
                    userState = ASLEEP;
                    _turnLightsOff();
                  //  Serial.println("CHANGED TO ASLEEP");
                    break;
                }
            // Otherwise open the shade all the way
            } else {
                servo.write(SHADE_MAXDISP);
            }
            // Change state. User woke up, is now AWAKE
            if(millis() - lastMovementTime < MOVEMENT_THRESHOLD){
          //      Serial.println("CHANGED TO AWAKE");
                currentBrightness = DEFAULT_BRIGHTNESS;
                userState = AWAKE;
                _setStripColorRGB(studyr, studyg, studyb, DEFAULT_BRIGHTNESS);    // go back to default brightness
                break;
            }
            break;
        case ASLEEP:
          if(millis() - lastMovementTime < MOVEMENT_THRESHOLD){
            userState = AWAKE;
            currentBrightness = DEFAULT_BRIGHTNESS;
            _setStripColorRGB(studyr, studyg, studyb, DEFAULT_BRIGHTNESS);
            break;
          }
          
          // Show the current color
          if(! isStrobeOff) {
            // Turn the light on for strobeColorDuration duration
            if(millis() - strobeColorStart < strobeColorDuration){
              break;
            } else {
            // Light on duration has ended, switch the light off for a bit
            //  Serial.println("Turned the lights off");
              _turnLightsOff();
              isStrobeOff = true;
              strobeOffStart = millis();
            }
          } else {
            // Keep the light off for strobeOffDuration
            if(millis() - strobeOffStart < strobeOffDuration){
              break; 
            } else {
            // Time to turn the light  back on
            //  Serial.println("Turned lights on");
              _setStripColorRGB(strober, strobeg, strobeb, STRIP_MAXBRIGHTNESS);
              isStrobeOff = false;
              strobeColorStart = millis();
            }
          }
   }
  
  }
}


void _increaseBrightness(int brightness){
  strip.setBrightness(brightness);
  strip.show();
}

// Servo adjusts based on light reading
void _adjustServo() {
  
  float reading = min(analogRead(LIGHTSENSORPIN), LIGHTSENSOR_MAXVAL); //Read light level
  float square_ratio = reading / LIGHTSENSOR_MAXVAL;      //Get percent of maximum value (1023)
  square_ratio = pow(square_ratio, 2.0);      //Square to make response more obvious

  // Positions range from 0 degrees (OPEN) to 45 degrees (CLOSED)
  int absolute_location = SHADE_MAXDISP * ((LIGHTSENSOR_MAXVAL - reading) / LIGHTSENSOR_MAXVAL);
  servo.write(absolute_location);
}

void _turnLightsOff(){
  for(int light = 0; light < strip.numPixels(); light++){
    strip.setPixelColor(light, 0,0,0);
  }
  strip.show();
}

void _setStripColorRGB(int r, int g, int b, int brightness){
  for(int i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, r, g, b);
  }
  strip.setBrightness(brightness);
  strip.show();
}

void _setStripColor(uint32_t color){
  for(int i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, color);
  }
  strip.setBrightness(STRIP_MAXBRIGHTNESS);
  strip.show();
}


