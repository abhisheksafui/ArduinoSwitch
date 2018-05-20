#ifndef __ARDUINOSWITCH_H__
#define __ARDUINOSWITCH_H__

/**
     ArduinoSwitch is a library for managing digital switches. 

     Copyright (C) 2018  Abhishek Safui

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <https://www.gnu.org/licenses/>
*/

#include <ArduinoList.h>


#if 1

enum class ActiveMode {

  ACTIVE_HIGH,
  ACTIVE_LOW,

};

enum class ArduinoSwitchFsmState {

  IDLE,
  DEBOUNCE_START,
  PRESSED,

};


class ArduinoSwitch {

    static ArduinoList<ArduinoSwitch *>     _switches;
    static int DEBOUNCE_TIME_MS;
    static int REPEATED_PRESS_INTERVAL_MS; 
    static Stream*  debugPort;
    /**
        ArduinoSwitch initialisation variables
    */
    ActiveMode      _mode;
    int _pin;
    std::function<void(void *)> _callback;
    void *_callbackArg;

    /**
       Runtime variables
    */
    ArduinoSwitchFsmState  _state;
    unsigned int _state_timestamp;

    /**
       ArduinoSwitch private methods
    */

    ArduinoSwitchFsmState state() {

      return _state;
    }

    unsigned int stateUpdateTime() {
      return _state_timestamp;
    }

  public:
    ArduinoSwitch(int pin, ActiveMode mode, std::function<void(void *)> call, void *arg = nullptr): _callback(call), _callbackArg(arg), _pin(pin), _mode(mode) {

      _switches.push_back(this);

      /**
         No need of external pullup resistor for ACTIVE_LOW.
         But external pulldown needed for ACTIVE_HIGH.
      */
      if (_mode == ActiveMode::ACTIVE_LOW)
      {
        pinMode(_pin, INPUT_PULLUP);
      }

      attachInterrupt(_pin, interrupt, (mode == ActiveMode::ACTIVE_HIGH) ? RISING : FALLING);

      if(debugPort != NULL){
        debugPort->print("ArduinoSwitch CONSTRUCTOR called. Count = ");
        debugPort->println(_switches.size());
      }

    }

    ~ArduinoSwitch()
    {
      /**
       * Remove yourself from list of switches
       */
      _switches.remove(this);
      
      if(debugPort != NULL){
        debugPort->print("ArduinoSwitch Destructed. Current count: "); 
        debugPort->println(_switches.size());
      }
      
    }

    /** 
     *  Optional initialization 
     */
    static void init(int debounce_ms, int repeat_ms, Stream *debug_stream=NULL){
       debugPort = debug_stream;
       DEBOUNCE_TIME_MS = debounce_ms;
       REPEATED_PRESS_INTERVAL_MS = repeat_ms;
    }

    static void init(Stream *debug_stream){
      debugPort = debug_stream;
    }

    bool pressed() {
      int state = digitalRead(_pin);
      
      if (_mode == ActiveMode::ACTIVE_HIGH ) {
        return state == HIGH;
      } else {
        return state == LOW;
      }
    }

    static void interrupt() {
      if(debugPort != NULL){
        debugPort->println("Interrupted");
      }
      for (auto it = _switches.begin(); it != _switches.end(); it++) {
        if ((*it)->state() == ArduinoSwitchFsmState::IDLE && (*it)->pressed())
        {
          if(debugPort != NULL){
        debugPort->println("ArduinoSwitch debounce started.. ");
      }
          
          (*it)->_state = ArduinoSwitchFsmState::DEBOUNCE_START;
          (*it)->_state_timestamp = millis();

        }
      }
    }

    static void update() {
      /* Check if any switch needs to be checked */

      for (auto it = _switches.begin(); it != _switches.end(); it++) {
        ArduinoSwitch *sw = (*it);
        if (sw->pressed()) {
          if ( (sw->state() == ArduinoSwitchFsmState::DEBOUNCE_START) &&
               ( millis() - sw->stateUpdateTime() > DEBOUNCE_TIME_MS ) )
          {

            /* ArduinoSwitch pressed so call callback */
            sw->_state = ArduinoSwitchFsmState::PRESSED;
            sw->_state_timestamp = millis();
            sw->_callback(sw->_callbackArg);

          } else if ( (sw->state() == ArduinoSwitchFsmState::PRESSED) &&
                      ( millis() - sw->stateUpdateTime() > REPEATED_PRESS_INTERVAL_MS) ) {

            /* ArduinoSwitch is kept pressed */
            sw->_state_timestamp = millis();
            sw->_callback(sw->_callbackArg);
          }
        } else {
          
          sw->_state = ArduinoSwitchFsmState::IDLE;
          sw->_state_timestamp = millis();
          
        }
      }
    }
};

ArduinoList<ArduinoSwitch*> ArduinoSwitch::_switches;
int ArduinoSwitch::DEBOUNCE_TIME_MS = 100;
int ArduinoSwitch::REPEATED_PRESS_INTERVAL_MS = 700;
Stream* ArduinoSwitch::debugPort = NULL;

#endif
#endif
