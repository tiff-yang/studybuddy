#include "Arduino.h"
#include "Button.h"

Button::Button(int pin = 15, int numstates = 0) {
  // Set the pin and total number of states of the button
  _pin = pin;
  _numStates = numstates;

  // Set constants
  READY = 0;
  PUSHDEBOUNCE = 1;
  PUSHED = 2;
  RELEASEDEBOUNCE = 3;

  // Initialize all other state vars
  _cycleState = READY;

  _buttonPressed = false;
  _buttonState = 0;

  tOfButtonChange = 0;
  debounceTime = 10;
}


int Button::getState() {
  update();
  return _buttonState;
}

void Button::read() {
  _buttonPressed = !digitalRead(_pin);
}


void Button::update() {
  read();

  // Ready for button press
  if (_cycleState == READY) {
    if (_buttonPressed) {
      _cycleState = PUSHDEBOUNCE;
      tOfButtonChange = millis(); //0;
    }

    // Press from unpressed
  } else if (_cycleState == PUSHDEBOUNCE) {
    // Button is released - back to state 0
    if (! _buttonPressed) {
      _cycleState = READY;
      tOfButtonChange = millis();
    }
    // Move to actual press if button press time meets debounce threshold
    else if (millis() - tOfButtonChange >= debounceTime) {
      _cycleState = PUSHED;
    }
    // Actual press
  } else if (_cycleState == PUSHED) {
    // Button is released Move to debounce release state (4)
    if (! _buttonPressed) {
      _cycleState = RELEASEDEBOUNCE;
      tOfButtonChange = millis();
    }

    // debounce from a push to back to ready
  } else if (_cycleState == RELEASEDEBOUNCE) {
    if (millis() - tOfButtonChange >= debounceTime) {
      _cycleState = READY;
      // update the button's option state
      _buttonState = (_buttonState + 1 ) % _numStates;
    }
  }
}
