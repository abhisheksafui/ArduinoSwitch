/**
 * ArduinoSwitch is a library for managing digital switches.
 *
 * Copyright (c) 2018 abhisheksafui
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ARDUINOSWITCH_H__
#define __ARDUINOSWITCH_H__

#include <ArduinoList.h>

#if 1

enum class ActiveMode
{

	ACTIVE_HIGH,
	ACTIVE_LOW,

};

enum class ArduinoSwitchFsmState
{

	IDLE,
	DEBOUNCE_START,
	PRESSED,

};

class ArduinoSwitch
{

	static ArduinoList<ArduinoSwitch *> _switches;
	static int DEBOUNCE_TIME_MS;
	static int REPEATED_PRESS_INTERVAL_MS;
	static Stream *debugPort;
	/**
	    ArduinoSwitch initialisation variables
	*/
	ActiveMode _mode;
	int _pin;
	std::function<void(void *)> _callback;
	void *_callbackArg;

	/**
	   Runtime variables
	*/
	ArduinoSwitchFsmState _state;
	unsigned int _state_timestamp;

	/**
	   ArduinoSwitch private methods
	*/

	ArduinoSwitchFsmState state()
	{

		return _state;
	}

	unsigned int stateUpdateTime()
	{
		return _state_timestamp;
	}

public:
	ArduinoSwitch(int pin, ActiveMode mode, std::function<void(void *)> call, void *arg = nullptr) : _callback(call), _callbackArg(arg), _pin(pin), _mode(mode)
	{

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

		if (debugPort != NULL)
		{
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

		if (debugPort != NULL)
		{
			debugPort->print("ArduinoSwitch Destructed. Current count: ");
			debugPort->println(_switches.size());
		}
	}

	/**
	 *  Optional initialization
	 */
	static void init(int debounce_ms, int repeat_ms, Stream *debug_stream = NULL)
	{
		debugPort = debug_stream;
		DEBOUNCE_TIME_MS = debounce_ms;
		REPEATED_PRESS_INTERVAL_MS = repeat_ms;
	}

	static void init(Stream *debug_stream)
	{
		debugPort = debug_stream;
	}

	bool pressed()
	{
		int state = digitalRead(_pin);

		if (_mode == ActiveMode::ACTIVE_HIGH)
		{
			return state == HIGH;
		}
		else
		{
			return state == LOW;
		}
	}

	static void interrupt()
	{
		if (debugPort != NULL)
		{
			debugPort->println("Interrupted");
		}
		for (auto it = _switches.begin(); it != _switches.end(); it++)
		{
			if ((*it)->state() == ArduinoSwitchFsmState::IDLE && (*it)->pressed())
			{
				if (debugPort != NULL)
				{
					debugPort->println("ArduinoSwitch debounce started.. ");
				}

				(*it)->_state = ArduinoSwitchFsmState::DEBOUNCE_START;
				(*it)->_state_timestamp = millis();
			}
		}
	}

	static void update()
	{
		/* Check if any switch needs to be checked */
		for (auto it = _switches.begin(); it != _switches.end(); it++)
		{
			(*it)->updateInstance();
		}
	}

	void updateInstance();
};

void ArduinoSwitch::updateInstance()
{
	if (pressed())
	{
		if ((state() == ArduinoSwitchFsmState::DEBOUNCE_START) &&
		    (millis() - stateUpdateTime() > DEBOUNCE_TIME_MS))
		{

			/* ArduinoSwitch pressed so call callback */
			_state = ArduinoSwitchFsmState::PRESSED;
			_state_timestamp = millis();
			_callback(_callbackArg);
		}
		else if ((state() == ArduinoSwitchFsmState::PRESSED) &&
			 (millis() - stateUpdateTime() > REPEATED_PRESS_INTERVAL_MS))
		{

			/* ArduinoSwitch is kept pressed */
			_state_timestamp = millis();
			_callback(_callbackArg);
		}
	}
	else
	{

		_state = ArduinoSwitchFsmState::IDLE;
		_state_timestamp = millis();
	}
}

ArduinoList<ArduinoSwitch *> ArduinoSwitch::_switches;
int ArduinoSwitch::DEBOUNCE_TIME_MS = 100;
int ArduinoSwitch::REPEATED_PRESS_INTERVAL_MS = 700;
Stream *ArduinoSwitch::debugPort = NULL;

#endif
#endif
