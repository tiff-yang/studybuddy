#ifndef Button_h
#define Button_h
#include "Arduino.h"


/*
 * Button class is intended to be used with a button that cycles between specified number of states
 */
class Button{
  
  private:
  int READY, PUSHDEBOUNCE, PUSHED, RELEASEDEBOUNCE, _cycleState;
  bool _buttonPressed;
  int _pin;
  int _buttonState;
  int _numStates;

  int tOfButtonChange;
  int debounceTime;
  
  void read();

  public:
  
  /**
   * int p: value of the input pin for this button
   * int numStates: total number of states this button cycles through.
   *                the button's state will vary from [0, numStates)
   */
  Button(int pin, int numstates);
  
  /**
   * Updates the button and
   * Returns the current state of the button from [0, user specified numOptions)
   */
  int getState();

  /**
   * Updates the button's state based on its current state
   * and the values read at the specified input pin
   * Presses and releases must surpass the debounce period in order to be
   * counted as a true press or release
   */
  void update();
};
#endif

