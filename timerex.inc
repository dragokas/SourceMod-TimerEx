/**
 * timerex.inc | TimerEx (Кукушка) | Wrapper over SourceMod's CreateTimer with extended functionality
 *
 * Copyright (C) 2026 Alex Dragokas
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#if defined _timerex_included
 #endinput
#endif
#define _timerex_included

#include <core>

#define TIMEREX_VERSION "1.0"

// The maximum number of TimerEx instances allowed to be created in parallel by a single plugin.
// This constant can be re-defined before including timerex.inc.
#if !defined TIMEREX_MAX_TIMERS
	#define TIMEREX_MAX_TIMERS 256
#endif

// Set 1 to print essential TimerEx actions to the server console.
#if !defined TIMEREX_DEBUG
	#define TIMEREX_DEBUG 0
#endif

#if TIMEREX_DEBUG
	#pragma tabsize 0
	#if SOURCEMOD_PLUGINAPI_VERSION >= 7
		#define _TEX_DBG(%0, %1) 	_TEX_DBG_Print(%0, %1);
		#define _TEX_DBG_WS(%0, %1) _TEX_DBG_Print("(%s) " ... %0, this.GetStateString(), %1);
	#else
		#define _TEX_DBG(%0) 		_TEX_DBG_Print(%0);
		#define _TEX_DBG_WS(%0) 	_TEX_DBG_Print(%0);
	#endif
#else
	#if SOURCEMOD_PLUGINAPI_VERSION >= 7
		#define _TEX_DBG(%0, %1)
		#define _TEX_DBG_WS(%0, %1)
	#else
		#define _TEX_DBG(%0)
		#define _TEX_DBG_WS(%0)
	#endif
#endif

#define _TEX_USED_INDICES_KEY_SIZE 8
#define _TEX(%0) 	_TEX_Storage[view_as<int>(%0)]
#define _TEX_P 		_TEX(this._index)
#define private

enum TEX_Flags
{
	TEX_FLAGS_NONE = 0,								 /* None flags provided, no effect */
	TEX_REPEAT = TIMER_REPEAT, 						 /* (1<<0) Timer will repeat until it returns Plugin_Stop */
	TEX_NO_MAPCHANGE = TIMER_FLAG_NO_MAPCHANGE, 	 /* (1<<1) Timer will not carry over map changes */
	TEX_NO_ROUNDCHANGE = (1<<2),				 	 /* (1<<2) Timer will not carry over round changes */
	TEX_NO_CALLBACK_DUPLICATES = (1<<3),			 /* (1<<3) On multiple calls, single-only timer is created per unique callback, unless timer stopped */
	TEX_NO_CALLBACK_AND_DATA_DUPLICATES = (1<<4),	 /* (1<<4) On multiple calls, single-only timer is created per unique callback and data handle, unless timer stopped */
	TEX_NO_REF = (1<<5),							 /* (1<<5) Disposes TimerEx reference as soon as timer gets stopped, no deferred access to reference allowed */
	TEX_PAUSE = (1<<6),								 /* (1<<6) Creates timer in paused state */
	TEX_MAPSTART_REQ = (1<<7),						 /* (1<<7) Requires map start to trigger the timer, trigger will be skipped after map ends */
	TEX_ROUNDSTART_REQ = (1<<8),					 /* (1<<8) Requires round start to trigger the timer, trigger will be skipped after round ends */
	TEX_DATA_HNDL_CLOSE = TIMER_DATA_HNDL_CLOSE,   	 /* (1<<9) Timer will automatically call CloseHandle() on its data when finished */
}

enum TEX_State
{
	TEX_Disposed,		/* timer index is disposed, see details in .Dispose() method note */
	TEX_Running,		/* timer is started, callback triggering is expected when the interval elapses */
	TEX_Triggering,		/* intermediate state when TimerEx is executing the callback function at the moment */
	TEX_Paused,			/* timer is paused via .Pause() method */
	TEX_Stopped			/* timer is stopped via .Stop() method, Map/Round end events, or when all required cycles are finished */
}

/**
 * Any of the following prototypes will work for a TimerEx callback function.
 */
typeset TimerExCallback
{
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param hTimerEx      Pseudo-handle to the TimerEx object. Call GetTimerEx(hTimerEx) to access TimerEx methods.
	 * @param data          Handle or value passed to TimerEx.CreateTimer() when timer was created.
	 * @return              Plugin_Stop to stop a repeating timer, any other value for default behavior.
	 *                      Ignored for non-repeating timers.
	 */
	function Action(Handle hTimerEx, any data);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @note				Use this prototype if you don't need to supply data
	 *
	 * @param hTimerEx      Pseudo-handle to the TimerEx object. Call GetTimerEx(hTimerEx) to access TimerEx methods.
	 * @return              Plugin_Stop to stop a repeating timer, any other value for default behavior.
	 *                      Ignored for non-repeating timers.
	 */
	function Action(Handle hTimerEx);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param hTimerEx      Pseudo-handle to the TimerEx object. Call GetTimerEx(hTimerEx) to access TimerEx methods.
	 * @param data          Handle or value passed to TimerEx.CreateTimer() when timer was created.
	 *
	 * @noreturn
	 * @note				This prototype automatically returns Plugin_Continue, so it doesn't stop repeatable timer.
	 *						Use Action-based prototype to control repeatable timer behaviour.
	 */
	function void(Handle hTimerEx, any data);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param hTimerEx      Pseudo-handle to the TimerEx object. Call GetTimerEx(hTimerEx) to access TimerEx methods.
	 *
	 * @noreturn
	 * @note				Use this prototype if you don't need to supply data.
	 * 						This prototype automatically returns Plugin_Continue, so it doesn't stop repeatable timer.
	 *						Use Action-based prototype to control repeatable timer behaviour.
	 */
	function void(Handle hTimerEx);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param timerEx       TimerEx pseudo-interface object. Can be used directly to access TimerEx methods.
	 * @param data          Handle or value passed to TimerEx.CreateTimer() when timer was created.
	 * @return              Plugin_Stop to stop a repeating timer, any other value for default behavior.
	 *                      Ignored for non-repeating timers.
	 */
	function Action(ITimerEx timerEx, any data);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @note				Use this prototype if you don't need to supply data
	 *
	 * @param timerEx       TimerEx pseudo-interface object. Can be used directly to access TimerEx methods.
	 * @return              Plugin_Stop to stop a repeating timer, any other value for default behavior.
	 *                      Ignored for non-repeating timers.
	 */
	function Action(ITimerEx timerEx);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param timerEx       TimerEx pseudo-interface object. Can be used directly to access TimerEx methods.
	 * @param data          Handle or value passed to TimerEx.CreateTimer() when timer was created.
	 *
	 * @noreturn
	 * @note				This prototype automatically returns Plugin_Continue, so it doesn't stop repeatable timer.
	 *						Use Action-based prototype to control repeatable timer behaviour.
	 */
	function void(ITimerEx timerEx, any data);
	
	/**
	 * Called when the timer interval has elapsed.
	 *
	 * @param timerEx       TimerEx pseudo-interface object. Can be used directly to access TimerEx methods.
	 *
	 * @noreturn
	 * @note				Use this prototype if you don't need to supply data.
	 * 						This prototype automatically returns Plugin_Continue, so it doesn't stop repeatable timer.
	 *						Use Action-based prototype to control repeatable timer behaviour.
	 */
	function void(ITimerEx timerEx);
};

enum struct TEX_Properties
{
	#define _TEX_FIELDS_COUNT 14

	TimerExCallback callback;
	Handle timer;
	float interval;
	float minInterval;
	float maxInterval;
	bool resetInterval;
	any data;
	int flags;
	float creationTime;
	float startTime;
	float stopTime;
	int invokeCount;
	int invokeMaxCount;
	TEX_State state;
}

#pragma dynamic 16984 /*default size*/ / 4 + _TEX_FIELDS_COUNT * TIMEREX_MAX_TIMERS

TEX_Properties 	_TEX_Storage[TIMEREX_MAX_TIMERS];
TEX_Properties  _TEX_Prop_Empty;
ArrayStack 		_TEX_FreeIndeces;	/* O(1) for sequental insert/remove operation */
StringMap  		_TEX_UsedIndeces;	/* O(1) for random insert/remove operation */

bool _TEX_IsInit = false;
bool _TEX_IsRoundStarted = true;
bool _TEX_IsMapStarted = true;

enum struct TimerEx
{
	#define _TEX_ValidateIndex if (this._index == 0) { ThrowError("TimerEx cannot call timer control methods when disposed"); }

	int _index;

	//														//
	// ================== PUBLIC METHODS ================== //
	//														//
	
	/**
	 * Creates a basic TimerEx timer.
	 *
	 * @param interval      Interval from the current game time to execute the given function.
	 * @param func          Function to execute once the given interval has elapsed.
	 * @param data          Handle or value to pass through to the timer callback function.
	 * @param flags         Flags to set (such as repeatability or auto-Handle closing).
	 * @return              true - if timer created successfully,
	 *                      false - if timer already created and running, according to callback pointer 
	 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
	 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
	 *
	 * @note				Calling .Kill() on a timer will end the timer.
	 *
	 * @error				thrown if interval set incorrectly (less than zero).
	 */
	bool CreateTimer(float interval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
	{
		return this.CreateRandomTimer(interval, interval, func, data, flags);
	}
	
	/**
	 * Creates a TimerEx timer associated with a new datapack, and returns the datapack.
	 * @note The datapack is automatically freed when the timer ends.
	 * @note The position of the datapack is not reset or changed for the timer function.
	 *
	 * @param interval      Interval from the current game time to execute the given function.
	 * @param func          Function to execute once the given interval has elapsed.
	 * @param datapack      The newly created datapack is passed though this by-reference
	 *                      parameter to the timer callback function.
	 * @param flags         Timer flags.
	 * @return              true - if timer created successfully,
	 *                      false - if timer already created and running, according to callback pointer 
	 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
	 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
	 *
	 * @error				thrown if interval set incorrectly (less than zero).
	 */
	bool CreateDataTimer(float interval, TimerExCallback func, Handle& datapack, TEX_Flags flags = TEX_FLAGS_NONE)
	{
		datapack = new DataPack();
		return this.CreateTimer(interval, func, datapack, flags | TEX_DATA_HNDL_CLOSE);
	}
	
	/**
	 * Creates a basic TimerEx timer using the randomized interval.
	 * The actual interval is calculated at each cycle.
	 *
	 * @param minInterval   Minimum interval from the current game time to execute the given function.
	 * @param maxInterval   Maximum interval from the current game time to execute the given function.
	 * @param func          Function to execute once the given interval has elapsed.
	 * @param data          Handle or value to pass through to the timer callback function.
	 * @param flags         Flags to set (such as repeatability or auto-Handle closing).
	 * @return              true - if timer created successfully,
	 *                      false - if timer already created and running, according to callback pointer 
	 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
	 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
	 *
	 * @note				Calling .Kill() on a timer will end the timer.
	 *
	 * @error				thrown if interval set incorrectly (less than zero, or minInterval > maxInterval).
	 */
	bool CreateRandomTimer(float minInterval, float maxInterval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
	{
		if (minInterval > maxInterval)
		{
			ThrowError("TimerEx invalid call: minInterval (%f) should be < maxInterval (%f)", minInterval, maxInterval);
		}
		if (minInterval < 0)
		{
			ThrowError("TimerEx invalid call: minInterval (%f) should be >= 0", minInterval);
		}
		if (maxInterval < 0)
		{
			ThrowError("TimerEx invalid call: maxInterval (%f) should be >= 0", maxInterval);
		}
		if (!_TEX_IsInit)
		{
			_TEX_Init();
		}
		
		int out_timerIndex;
		int out_dataHandle = 0;
		bool found = false;
		
		// There is a nasty OnMapStart / round_start events duplication that happening in some games or modes.
		// When CreateTimer often declared in such events, the double timer creation is likely.
		// The TimerEx's TEX_NO_CALLBACK_DUPLICATES and TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags
		// are helping in such situation by filtering any subsequent calls matching the same
		// callback id and interval settings, and optionally data handle to check for its uniqueness among already running timers cache.
		if (flags & (TEX_NO_CALLBACK_DUPLICATES | TEX_NO_CALLBACK_AND_DATA_DUPLICATES))
		{
			bool checkDataHandle = view_as<int>(flags & TEX_NO_CALLBACK_AND_DATA_DUPLICATES) != 0;
			found = _TEX_GetTimerExByCallback(minInterval, maxInterval, func, data, checkDataHandle, out_timerIndex, out_dataHandle);
		}
		if (found)
		{
																					_TEX_DBG_WS("Prevented timer duplication", "")
			if (data != 0 && (view_as<int>(flags & TEX_DATA_HNDL_CLOSE) != 0))
			{
				// ensure user didn't already pass the same data handle in the current TimerEx reference
				if (data != out_dataHandle)
				{
																					_TEX_DBG_WS("HANDLE data disposed: 0x%x", data)
					CloseHandle(data);
				}
			}
		}
		else {
			if (this._index != 0)
			{
				_TEX_Storage[this._index] = _TEX_Prop_Empty;
			}
			return this._init(minInterval, maxInterval, func, data, view_as<int>(flags), false, GetEngineTime());
		}
		return false;
	}
	
	/**
	 * Starts the timer with a full interval if it was stopped or paused.
	 * The method will not re-start timer if it is already running.
	 * Use this method if only you need manual control.
	 *
	 * @return 		true - if timer successfully started,
	 *				false - if not started, because timer is already running.
	 *
	 * @error		thrown if TimerEx disposed, or data handle invalid considering the TEX_DATA_HNDL_CLOSE flag is passed.
	 */
	bool Run()
	{
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Run", "")
		if (_TEX_P.state != TEX_Running)
		{
			return this._init(_TEX_P.minInterval, _TEX_P.maxInterval, _TEX_P.callback, _TEX_P.data, _TEX_P.flags, false);
		}
		return false;
	}
	
	/**
	 * Stops the timer, closing its handle (data handle will not be closed).
	 *
	 * @param resetInvokeCount	Resets the invokes counter to zero, so the timer callback calls number
	 * 							will be counted from zero to SetInvokeMaxCount() value.
	 *
	 * @return 					true - if timer successfully stopped,
	 *							false - if timer already stopped.
	 *
	 * @error					thrown if TimerEx disposed
	 *
	 * @note 					The timer can be re-started again with Run() call.
	 */
	bool Stop(bool resetInvokeCount = true)
	{
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Stop", "")
		if (resetInvokeCount)
		{
			_TEX_P.invokeCount = 0;
		}
		if (_TEX_P.state == TEX_Running || _TEX_P.state == TEX_Triggering)
		{
			this._kill(
				.closeDataHandle = false);
			return true;
		}
		else if (_TEX_P.state == TEX_Paused)
		{
			_TEX_P.state = TEX_Stopped;
			return true;
		}
		return false;
	}
	
	/**
	 * Restarts the timer.
	 *
	 * @return 		true - if timer successfully started,
	 *				false - on unexpected error.
	 *
	 * @error		thrown if TimerEx disposed, or data handle invalid considering the TEX_DATA_HNDL_CLOSE flag is passed.
	 */
	bool Restart()
	{
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Restart", "")
		this.Stop();
		return this.Run();
	}
	
	/**
	 * Pauses the timer.
	 *
	 * @return 		true - if timer successfully paused,
	 *				false - if timer state is already paused or stopped.
	 *
	 * @error		thrown if TimerEx disposed
	 */
    bool Pause()
    {
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Pause", "")
        if (_TEX_P.state == TEX_Running || _TEX_P.state == TEX_Triggering)
		{
			this._kill(
				.closeDataHandle = false);
			_TEX_P.state = TEX_Paused;
			return true;
		}
		return false;
    }
	
	/**
	 * Resumes the timer.
	 *
	 * @return 		true - if timer successfully resumed,
	 *				false - if timer stopped or already resumed.
	 *
	 * @noerror		thrown if TimerEx disposed, or data handle invalid considering the TEX_DATA_HNDL_CLOSE flag is passed.
	 */
    bool Resume()
    {
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Resume", "")
        if (_TEX_P.state == TEX_Paused)
		{
			float time_remaining = this.GetTimeRemaining();
			return this._init(time_remaining, time_remaining, _TEX_P.callback, _TEX_P.data, _TEX_P.flags, true);
		}
		return false;
    }
	
	/**
	 * Delays triggering the callback of current cycle by the specified number of seconds.
	 *
	 * @param seconds			Interval to append to current timer's cycle. Value can be positive or negative.
	 *							If the remaining time becomes negative, the callback will be triggered at the next frame.
	 * @param applyToAllCycles	true - to apply delay to current and all future timer cycles (permanently).
	 *							false - to apply delay to currect cycle only.
	 *
	 * @return 		true - if timer execution successfully delayed,
	 *				false - if timer is in stopped state.
	 *
	 * @error		thrown if TimerEx disposed, or data handle invalid considering the TEX_DATA_HNDL_CLOSE flag is passed.
	 */
	bool Delay(float seconds, bool applyToAllCycles = false)
	{
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Delay | seconds:%f", seconds)
		if (applyToAllCycles)
		{
			_TEX_P.minInterval += seconds;
			_TEX_P.maxInterval += seconds;
			if (_TEX_P.minInterval < 0.0) {
				_TEX_P.minInterval = 0.0;
			}
			if (_TEX_P.maxInterval < 0.0) {
				_TEX_P.maxInterval = 0.0;
			}
		}
		switch (_TEX_P.state)
		{
			case TEX_Running:
			{
				this.Pause();
				float time_remaining = this.GetTimeRemaining() + seconds;
				if (time_remaining < 0.0) {
					time_remaining = 0.0;
				}
				return this._init(time_remaining, time_remaining, _TEX_P.callback, _TEX_P.data, _TEX_P.flags, !applyToAllCycles);
			}
			case TEX_Triggering:
			{
				if (_TEX_P.invokeMaxCount == 0 || (_TEX_P.invokeCount < _TEX_P.invokeMaxCount))
				{
					this.Stop(false);
					float interval = _TEX_P.interval + seconds;
					if (interval < 0.0) {
						interval = 0.0;
					}
					return this._init(interval, interval, _TEX_P.callback, _TEX_P.data, _TEX_P.flags, !applyToAllCycles);
				}
			}
			case TEX_Paused:
			{
				_TEX_P.interval += seconds;
			}
		}
		return false;
	}
	
	/**
	 * Returns the time (in seconds) elapsed since the timer was started or re-started.
	 * Repeatable timers are measured since each new cycle.
	 *
	 * @noerror
	 */
    float GetTimeElapsed()
    {
		if (_TEX_P.state == TEX_Running || _TEX_P.state == TEX_Triggering)
		{
			return GetEngineTime() - _TEX_P.startTime;
		}
		return _TEX_P.stopTime - _TEX_P.startTime;
    }
	
	/**
	 * Returns the time (in seconds) remaining until timer triggers.
	 *
	 * @noerror
	 */
    float GetTimeRemaining()
    {
		float time_remaining;
		if (_TEX_P.state == TEX_Paused)
		{
			time_remaining = this.GetEndTime() - _TEX_P.stopTime;
		}
		else if (_TEX_P.state != TEX_Running)
		{
			return 0.0;
		}
		else {
			time_remaining = this.GetEndTime() - GetEngineTime();
		}
		if (time_remaining < 0.0)
		{
			time_remaining = 0.0;
		}
		return time_remaining;
    }
	
	/**
	 * Manually triggers a timer so its function will be called.
	 * If timer is not repeatable, the callback is called and the timer will be destroyed.
	 *
	 * @param reset         If reset is true, the elapsed time counter is reset
	 *                      so the full interval must pass again (for REPEAT-able timer only).
	 *
	 * @return 				true - if timer callback successfully triggered,
	 *						false - if timer is not in running state.
	 *
	 * @error				thrown if TimerEx disposed, or data handle invalid considering the TEX_DATA_HNDL_CLOSE flag is passed.
	 */
	bool Trigger(bool reset)
	{
		_TEX_ValidateIndex
																					_TEX_DBG_WS("Trigger | reset:%b", reset)
		if (_TEX_P.state == TEX_Running)
		{
			_TEX_P.resetInterval = reset;
			TriggerTimer(_TEX_P.timer, reset);
			return true;
		}
		return false;
	}
	
	/**
	 * Stops the timer and closes its handle safely.
	 * Multiple calls are allowed.
	 *
	 * @param closeDataHandle   If true, the data that was passed to CreateTimerEx() will
	 *                      	be closed even if TEX_DATA_HNDL_CLOSE flag was not specified.
	 * 							If false, data handle closed if only TEX_DATA_HNDL_CLOSE flag specified.
	 *
	 * @return 					true - if timer successfully disposed,
	 *							false - if timer already disposed.
	 * @noerror
	 */
	bool Kill(bool closeDataHandle = false)
	{
																					_TEX_DBG_WS("Kill | closeDataHandle:%b", closeDataHandle)
		_TEX_P.invokeCount = 0;
																					
		if ((_TEX_P.flags & TIMER_DATA_HNDL_CLOSE) != 0)
		{
			closeDataHandle = true;
		}
		return this._kill(
			.closeDataHandle = closeDataHandle);
	}
	
	/**
	 * Disposes the TimerEx reference and free up the index (pseudo-handle).
	 * Any further attempts to call TimerEx methods by this reference will throw en error.
	 * Calling methods of property-wrappers is still allowed, but they will return the garbage (null) values.
	 *
	 * @note			Use this method in rare cases, when you really need to throw an error
	 * 					if TimerEx object is disposed. TimeEx is automatically get disposed 
	 *					only if TEX_NO_REF flag is passed, it happens when timer is stopped
	 *					(when last cycle is triggered or on Map/Round end events with appropriate flags passed).
	 *
	 * @return 			true - if reference successfully disposed,
	 *					false - if reference already disposed.
	 * @noerror
	 */
	bool Dispose()
	{
		if (this._index == 0)
		{
			return false;
		}
		_TEX_DisposeIndex(this._index);
		this._index = 0;
		return true;
	}
	
	/**
	 * Returns the maximum number of callback invokes allowed in repeatable timer.
	 *
	 * @noerror
	 */
	int GetInvokeMaxCount() { return _TEX_P.invokeMaxCount; }
	
	/**
	 * Limits the maximum number of callback invokes allowed in repeatable timer.
	 * When the number of callback invokes reaches this limit, the timer is been killed.
	 *
	 * @param count		Invokes count to limit (zero to disable).
	 *
	 * @noerror
	 */
	void SetInvokeMaxCount(int count)
	{
		_TEX_P.invokeMaxCount = count;
	}
	
	/**
	 * Returns the number of times the timer was triggered.
	 *
	 * @noerror
	 */
	int GetInvokeCount() { return _TEX_P.invokeCount; }
	
	/**
	 * Returns string representation of the current state of the timer (TEX_State).
	 *
	 * @noerror
	 */
	char[] GetStateString()
	{
		char stateStr[16];
		TEX_StateToString(_TEX_P.state, stateStr, sizeof(stateStr));
		return stateStr;
	}
	
	/**
	 * Returns the current state of the timer.
	 *
	 * @noerror
	 */
	TEX_State GetState() 	{ return _TEX_P.state; }

	/**
	 * Returns true if the timer is in 'Running' or 'Triggering' state, false otherwise.
	 *
	 * @noerror
	 */
	bool IsStarted() 	 	{ return _TEX_P.state == TEX_Running || _TEX_P.state == TEX_Triggering; }
	
	/**
	 * Returns true if the timer is in 'Paused' state, false otherwise.
	 *
	 * @noerror
	 */
	bool IsPaused() 	 	{ return _TEX_P.state == TEX_Paused;  }
	
	/**
	 * Returns true if the timer is in 'Stopped' or 'Disposed' state, false otherwise.
	 *
	 * @noerror
	 */
	bool IsFinished() 	 	{ return (_TEX_P.state == TEX_Stopped || _TEX_P.state == TEX_Disposed); }
	
	/**
	 * Returns the callback pointer to execute once the given interval has elapsed.
	 *
	 * @noerror
	 */
	TimerExCallback GetCallback() 	{ return _TEX_P.callback; }
	
	/**
	 * Returns inner CreateTimer's handle.
	 *
	 * @note		Do not use this handle, unless you're really know what you're doing.
	 *
	 * @noerror
	 */
	Handle GetTimer() 		{ return _TEX_P.timer; }
	
	/**
	 * Returns the interval (in seconds) used in the current cycle.
	 *
	 * @noerror
	 */
	float GetInterval()	 	{ return _TEX_P.interval; }
	
	/**
	 * Sets the interval (in seconds) to use with the new timer cycle.
	 * 
	 * @note		The new interval will be applied after the next timer trigger.
	 * 				To modify the current interval, use .Delay() method.
	 *
	 * @noerror
	 */
	void SetInterval(float interval)
	{
		this.SetMinInterval(interval);
		this.SetMaxInterval(interval);
	}
	
	/**
	 * Returns the minimum interval to execute the given function.
	 *
	 * @noerror
	 */
	float GetMinInterval()	 	{ return _TEX_P.minInterval; }
	
	/**
	 * Sets the minimum interval (in seconds) to execute the given function.
	 * 
	 * @note		The new interval will be applied after the next timer trigger.
	 * 				To modify the current interval, use .Delay() method.
	 *
	 * @noerror
	 */
	void SetMinInterval(float interval)
	{
		if (interval != _TEX_P.minInterval)
		{
			_TEX_P.minInterval = interval;
			_TEX_P.resetInterval = true;
		}
	}
	
	/**
	 * Returns the maximum interval to execute the given function.
	 *
	 * @noerror
	 */
	float GetMaxInterval()	 	{ return _TEX_P.maxInterval; }
	
	/**
	 * Sets the maximum interval (in seconds) to execute the given function.
	 * 
	 * @note		The new interval will be applied after the next timer trigger.
	 * 				To modify the current interval, use .Delay() method.
	 *
	 * @noerror
	 */
	void SetMaxInterval(float interval)
	{
		if (interval != _TEX_P.maxInterval)
		{
			_TEX_P.maxInterval = interval;
			_TEX_P.resetInterval = true;
		}
	}
	
	/**
	 * Returns the handle or value to be passed through to the timer callback function.
	 *
	 * @noerror
	 */
	any GetData()	 			{ return _TEX_P.data; }
	
	/**
	 * Returns the flags (such as repeatability or auto-Handle closing).
	 *
	 * @noerror
	 */
	int GetFlags()	 			{ return _TEX_P.flags; }
	
	/**
	 * Returns time when the timer was initially created.
	 *
	 * @noerror
	 */
	float GetCreationTime()		{ return _TEX_P.creationTime; }
	
	/**
	 * Returns time when the most recent timer cycle started.
	 *
	 * @noerror
	 */
	float GetStartTime()	 	{ return _TEX_P.startTime; }
	
	/**
	 * Returns time when the timer expected to be triggered.
	 *
	 * @noerror
	 */
	float GetEndTime()	 		{ return _TEX_P.startTime + _TEX_P.interval; }
	
	/**
	 * Returns time when the timer was stopped or paused or when the last cycle was triggered.
	 *
	 * @noerror
	 */
	float GetStopTime()	 		{ return _TEX_P.stopTime; }
	
	/**
	 * Combines the current timer's state information to print it by caller, for debug purposes.
	 *
	 * @param buffer		String buffer to return the information.
	 * @param maxlength		Maximum length of the buffer passed.
	 *
	 * @noerror
	 */
	void GetInfo(char[] buffer, int maxlength)
	{
		int len = 0;
		char stateStr[16];
		char flagsStr[256];
		TEX_StateToString(this.GetState(), stateStr, sizeof(stateStr));
		TEX_FlagsToString(view_as<TEX_Flags>(this.GetFlags()), flagsStr, sizeof(flagsStr));
		len += FormatEx(buffer[len], maxlength - len, "Data:           0x%x\n", 	this.GetData());
		len += FormatEx(buffer[len], maxlength - len, "Time Elapsed:   %f\n", 		this.GetTimeElapsed());
		len += FormatEx(buffer[len], maxlength - len, "Time Remaining: %f\n", 		this.GetTimeRemaining());
		len += FormatEx(buffer[len], maxlength - len, "Curr. Interval: %f\n", 		this.GetInterval());
		len += FormatEx(buffer[len], maxlength - len, "Min Interval:   %f\n", 		this.GetMinInterval());
		len += FormatEx(buffer[len], maxlength - len, "Max Interval:   %f\n", 		this.GetMaxInterval());
		len += FormatEx(buffer[len], maxlength - len, "Should reset?   %b\n", 		_TEX_P.resetInterval);
		len += FormatEx(buffer[len], maxlength - len, "Flags:          %i: %s\n", 	this.GetFlags(), flagsStr);
		len += FormatEx(buffer[len], maxlength - len, "Creation time:  %f\n", 		this.GetCreationTime());
		len += FormatEx(buffer[len], maxlength - len, "Start time:     %f\n", 		this.GetStartTime());
		len += FormatEx(buffer[len], maxlength - len, "End time:       %f\n", 		this.GetEndTime());
		len += FormatEx(buffer[len], maxlength - len, "Stopped time:   %f\n", 		this.GetStopTime());
		len += FormatEx(buffer[len], maxlength - len, "Invokes Count:  %i\n", 		this.GetInvokeCount());
		len += FormatEx(buffer[len], maxlength - len, "Invokes Max:    %i\n", 		this.GetInvokeMaxCount());
		len += FormatEx(buffer[len], maxlength - len, "State:          %i: %s\n", 	this.GetState(), stateStr);
	}
	
	
	//														 //
	// ================== PRIVATE METHODS ================== //
	//														 //
	
	/**
	 * Initializes the TimerEx
	 */
    private bool _init(float minInterval, float maxInterval, TimerExCallback func, any data, int flags, bool reset, float creationTime = 0.0)
    {
		#if TIMEREX_DEBUG
			_TEX_DBG_Print("(%s) _init | min:%f | max:%f | data:%i | flags:%i | reset:%b", this.GetStateString(), minInterval, maxInterval, data, flags, reset);
		#endif
		// Do not use deprecated function as it will trigger compile time warning (see SP: #1040), also removed in SM 1.13.7304.
		//if (((flags & TIMER_DATA_HNDL_CLOSE) != 0) && (data == 0 || !IsValidHandle(data)))
		//{
		//	ThrowError("TimerEx invalid call: TIMER_DATA_HNDL_CLOSE flag supplied, but the handle is not valid: %i", data);
		//}
		if (this._index != 0 && _TEX_P.state == TEX_Running)
		{
			this._kill(
				.closeDataHandle = false);
		}
		if (this._index == 0)
		{
			if (!_TEX_GetFreeIndex(this._index))
			{
				ThrowError("TimerEx free indices is depleted: please check for missing TEX_NO_REF flag");
			}
			_TEX_Storage[this._index] = _TEX_Prop_Empty;
		}
		if (!reset || _TEX_P.interval == 0.0)
		{
			_TEX_P.minInterval = minInterval;
			_TEX_P.maxInterval = maxInterval;
		}
		if (minInterval == maxInterval)
		{
			_TEX_P.interval = minInterval;
		}
		else {
			_TEX_P.interval = minInterval + (GetURandomFloat() * (maxInterval - minInterval));
			reset = true;
		}
		
		_TEX_P.resetInterval = reset;
        _TEX_P.callback = func;
        _TEX_P.data = data;
		_TEX_P.startTime = GetEngineTime();
		if (creationTime != 0.0)
		{
			_TEX_P.creationTime = creationTime;
		}
		
		if (flags & view_as<int>(TEX_PAUSE))
		{
			_TEX_P.stopTime = GetEngineTime();
			_TEX_P.state = TEX_Paused;
			_TEX_P.flags = flags &~ view_as<int>(TEX_PAUSE);
		}
		else {
			_TEX_P.flags = flags;
			_TEX_P.state = TEX_Running;
			_TEX_P.timer = 
				CreateTimer(_TEX_P.interval, _TEX_OnTrigger, this._index, flags & TIMER_REPEAT); // TEX_REPEAT is only allowed flag here
			
																						_TEX_DBG_WS("HANDLE timer created: 0x%x", _TEX_P.timer)
			if (_TEX_P.timer == INVALID_HANDLE)
			{
				LogError("[TimerEx] SourceMod failed to create the timer. CreateTimer returned INVALID_HANDLE");
				_TEX_P.state = TEX_Stopped;
				return false;
			}
		}
		return true;
    }
	
	/**
	 * Stops the timer and closes its handles safely.
	 *
	 * @param closeDataHandle - to close data handle as well
	 *
	 * @return 					true - if timer successfully disposed,
	 *							false - if timer already disposed.
	 */
	private bool _kill(bool closeDataHandle)
	{
																					_TEX_DBG_WS("_kill | closeDataHandle:%b", closeDataHandle)
		bool isDisposed = false;
		if (this._index == 0)
		{
			return false;
		}
		if (_TEX_P.timer != INVALID_HANDLE && _TEX_P.state != TEX_Triggering)
		{
																					_TEX_DBG_WS("HANDLE timer disposed: 0x%x", _TEX_P.timer)
			CloseHandle(_TEX_P.timer);
			_TEX_P.timer = INVALID_HANDLE;
			isDisposed = true;
		}
		if (closeDataHandle && _TEX_P.data != 0)
		{
																					_TEX_DBG_WS("HANDLE data disposed: 0x%x", _TEX_P.data)
			CloseHandle(_TEX_P.data);
			_TEX_P.data = 0;
		}
		if (_TEX_P.state == TEX_Running || _TEX_P.state == TEX_Triggering)
		{
			_TEX_P.stopTime = GetEngineTime();
			_TEX_P.state = TEX_Stopped;
		}
		if (_TEX_P.flags & view_as<int>(TEX_NO_REF))
		{
			_TEX_DisposeIndex(this._index);
			this._index = 0;
		}
		return isDisposed;
	}
}

/*
	Pseudo-interface, call redirector.
	Allows to substiture TimerEx pseudo-handle and call TimerEx methods 
	directly through interface. 
	Used in alternate version of TimerEx callback prototype.
*/
methodmap ITimerEx __nullable__
{
	#define _TEX_V TimerEx t;\
		t._index = view_as<int>(this);\
		t
	#define _TEX_R TimerEx t;\
		t._index = view_as<int>(this);\
		return t

	public ITimerEx(Handle index)
	{
		return view_as<ITimerEx>(index);
	}
	
	public bool Run() 								{ _TEX_R.Run(); }
	public bool Stop(bool resetInvokeCount = true) 	{ _TEX_R.Stop(resetInvokeCount); }
	public bool Restart() 							{ _TEX_R.Restart(); }
    public bool Pause() 							{ _TEX_R.Pause(); }
    public bool Resume() 							{ _TEX_R.Resume(); }
	public bool Delay(float seconds, 
		bool applyToAllCycles = false) 				{ _TEX_R.Delay(seconds, applyToAllCycles); }
    public float GetTimeElapsed() 					{ _TEX_R.GetTimeElapsed(); }
    public float GetTimeRemaining() 				{ _TEX_R.GetTimeRemaining(); }
	public bool Trigger(bool reset) 				{ _TEX_R.Trigger(reset); }
	public bool Kill(bool closeDataHandle = false) 	{ _TEX_R.Kill(closeDataHandle); }
	public bool Dispose()							{ _TEX_R.Dispose(); }
	public int GetInvokeMaxCount() 					{ _TEX_R.GetInvokeMaxCount(); }
	public void SetInvokeMaxCount(int count) 		{ _TEX_V.SetInvokeMaxCount(count); }
	public int GetInvokeCount() 					{ _TEX_R.GetInvokeCount(); }
	public char[] GetStateString()					{ _TEX_R.GetStateString(); }
	public TEX_State GetState() 					{ _TEX_R.GetState(); }
	public bool IsStarted() 						{ _TEX_R.IsStarted(); }
	public bool IsPaused() 							{ _TEX_R.IsPaused(); }
	public bool IsFinished() 						{ _TEX_R.IsFinished(); }
	public TimerExCallback GetCallback() 			{ _TEX_R.GetCallback(); }
	public Handle GetTimer() 						{ _TEX_R.GetTimer(); }
	public float GetInterval() 						{ _TEX_R.GetInterval(); }
	public void SetInterval(float interval) 		{ _TEX_V.SetInterval(interval); }
	public float GetMinInterval() 					{ _TEX_R.GetMinInterval(); }
	public void SetMinInterval(float interval) 		{ _TEX_V.SetMinInterval(interval); }
	public float GetMaxInterval() 					{ _TEX_R.GetMaxInterval(); }
	public void SetMaxInterval(float interval) 		{ _TEX_V.SetMaxInterval(interval); }
	public any GetData()							{ _TEX_R.GetData(); }
	public int GetFlags() 							{ _TEX_R.GetFlags(); }
	public float GetCreationTime()					{ _TEX_R.GetCreationTime(); }
	public float GetStartTime() 					{ _TEX_R.GetStartTime(); }
	public float GetEndTime() 						{ _TEX_R.GetEndTime(); }
	public float GetStopTime() 						{ _TEX_R.GetStopTime(); }
	public void GetInfo(char[] buffer, int maxlength)
	{
		_TEX_V.GetInfo(buffer, maxlength);
	}
}

/**
 * Gets the free index for storing the timer's properties
 *
 * @param index 	variable to return free index to
 * @return          true - on success
 *					false - when free indeces depleted, usually means invalid API usage such as TEX_NO_REF flag missing
 */
private bool _TEX_GetFreeIndex(int& index)
{
	if (_TEX_FreeIndeces.Empty)
	{
		return false;
	}
	index = _TEX_FreeIndeces.Pop();
	char key[_TEX_USED_INDICES_KEY_SIZE];
	IntToString(index, key, _TEX_USED_INDICES_KEY_SIZE);
	_TEX_UsedIndeces.SetValue(key, index, true);
																					_TEX_DBG("Free index allocated: %i", index)
	return true;
}

/**
 * Free up the given timer's index making the reference to be disposed, so the index could be re-usable again
 *
 * @param index 	timer's index to dispose
 */
private void _TEX_DisposeIndex(int index)
{
	char key[_TEX_USED_INDICES_KEY_SIZE];
	_TEX(index).state = TEX_Disposed;
	IntToString(index, key, _TEX_USED_INDICES_KEY_SIZE);
	_TEX_UsedIndeces.Remove(key);
	_TEX_FreeIndeces.Push(index);
																					_TEX_DBG("Index disposed: %i", index)
}

/*	Using static callback for our CreateTimer's wrapper, because SM doesn't support calling 
	methodmap's non-static methods by pointer or passing them as a callback.
	Well, it makes sense since SM doesn't have a mechanism for extending such object's lifetime.
*/

/**
 * Intermediate callback to trigger the target callback, manage the inner TimerEx state and handles, and restarts the timer if required.
 *
 * @param hTimer 	Handle to the timer object.
 * @param hProp 	Properties index associated with TimerEx.
 * @return          Action value returned from callback or Plugin_Stop when resetInterval required.
 */
private Action _TEX_OnTrigger(Handle hTimer, int hProp)
{
																					_TEX_DBG("OnTrigger | reset:%b", _TEX(hProp).resetInterval)
	Action result;
	bool isRepeatable = (_TEX(hProp).flags & TIMER_REPEAT) != 0;
	_TEX(hProp).state = TEX_Triggering;
	
	if (((_TEX(hProp).flags & view_as<int>(TEX_MAPSTART_REQ)) == 0) || _TEX_IsMapStarted)
	{
		if (((_TEX(hProp).flags & view_as<int>(TEX_ROUNDSTART_REQ)) == 0) || _TEX_IsRoundStarted)
		{
			if (_TEX(hProp).invokeMaxCount == 0 || (_TEX(hProp).invokeCount < _TEX(hProp).invokeMaxCount))
			{
				++ _TEX(hProp).invokeCount;
				Call_StartFunction(INVALID_HANDLE, _TEX(hProp).callback);
				Call_PushCell(view_as<Handle>(hProp));
				Call_PushCell(_TEX(hProp).data);
				Call_Finish(result);
			}
		}
	}
	
	// if state is changed because user called a TimerEx method in mid-callback
	if (_TEX(hProp).state != TEX_Triggering)
	{
		// we whould dispose the current timer handle
		result = Plugin_Stop;
																					_TEX_DBG("Mid-callback timer state changed to: %s", (new ITimerEx(view_as<Handle>(hProp))).GetStateString())
	}
	
	if (_TEX(hProp).invokeMaxCount > 0 && (_TEX(hProp).invokeCount >= _TEX(hProp).invokeMaxCount))
	{
		result = Plugin_Stop;
	}
	
	if (!isRepeatable || result == Plugin_Stop)
	{
		// ensure the state not modified by mid-callback user method call
		if (_TEX(hProp).state == TEX_Triggering)
		{
			if ((_TEX(hProp).flags & TIMER_DATA_HNDL_CLOSE) && (_TEX(hProp).data != 0))
			{
																					_TEX_DBG("HANDLE data disposed: 0x%x", _TEX(hProp).data)
				CloseHandle(_TEX(hProp).data);
				_TEX(hProp).data = 0;
			}
		
			if (_TEX(hProp).flags & view_as<int>(TEX_NO_REF))
			{
				_TEX_DisposeIndex(hProp);
			}
			else {
				_TEX(hProp).timer = INVALID_HANDLE;
				_TEX(hProp).stopTime = GetEngineTime();
				_TEX(hProp).state = TEX_Stopped;
			}
		}
	}
	else // repeatable, not Plugin_Stop, TEX_Triggering
	{
		if (_TEX(hProp).resetInterval)
		{
			_TEX(hProp).resetInterval = false;
			(new ITimerEx(view_as<Handle>(hProp))).Run();
			result = Plugin_Stop; // to auto-close previous timer handle
		}
		else {
			_TEX(hProp).startTime = GetEngineTime();
			_TEX(hProp).state = TEX_Running;
		}
	}
	
	#if TIMEREX_DEBUG
		if (result == Plugin_Stop || !isRepeatable)
		{
			_TEX_DBG_Print("HANDLE timer auto-closed: 0x%x", hTimer);
		}
	#endif
	return result;
}

/**
 * Returns TimerEx from internal storage matching the Function id, interval, and optionally Handle id speficied.
 * 
 * @param minInterval       Min interval to match.
 * @param maxInterval       Max interval to match.
 * @param func          	Function id to match.
 * @param data          	Handle id to match.
 * @param checkDataHandle	if true, check both Function id and Data handle to match, otherwise Function id only.
 * @param tResult			TimerEx to return.
 * 
 * @return 					true on success, false if not found.
 */
private bool _TEX_GetTimerExByCallback(float minInterval, float maxInterval, TimerExCallback func, any data, bool checkDataHandle, int& hTimerEx, int& cachedDataHandle)
{
	char key[_TEX_USED_INDICES_KEY_SIZE];
	StringMapSnapshot hSnap = _TEX_UsedIndeces.Snapshot();
	int _index;
	for (int i = 0; i < hSnap.Length; i++)
	{
		hSnap.GetKey(i, key, _TEX_USED_INDICES_KEY_SIZE);
		_index = StringToInt(key);
		if (_TEX(_index).state == TEX_Running)
		{
			if (_TEX(_index).callback == func && _TEX(_index).minInterval == minInterval && _TEX(_index).maxInterval == maxInterval)
			{
				if (!checkDataHandle || (checkDataHandle && _TEX(_index).data == data))
				{
					hTimerEx = _index;
					cachedDataHandle = _TEX(_index).data;
					CloseHandle(hSnap);
					return true;
				}
			}
		}
	}
	CloseHandle(hSnap);
	return false;
}

/**
 * Kills all timers matching the flags specified.
 */
private void _TEX_KillTimers(int requiredFlag)
{
	TimerEx t;
	char key[_TEX_USED_INDICES_KEY_SIZE];
	StringMapSnapshot hSnap = _TEX_UsedIndeces.Snapshot();
	int _index;
	for (int i = 0; i < hSnap.Length; i++)
	{
		hSnap.GetKey(i, key, _TEX_USED_INDICES_KEY_SIZE);
		_index = StringToInt(key);
		
		if (_TEX(_index).flags & requiredFlag)
		{
																						_TEX_DBG("_TEX_KillTimers | handle:0x%x", _TEX(_index).timer)
			bool autoClose = (_TEX(_index).flags & TIMER_DATA_HNDL_CLOSE) != 0;
			t._index = _index;
			t.Kill(autoClose);
		}
	}
	CloseHandle(hSnap);
}

/**
 * Initializes internal objects and events.
 *
 * @param trackRoundEvents		true - to enable round start/end events tracking for using them by TEX_NO_ROUNDCHANGE flag,
 *								false - do not track round start/end events, TEX_NO_ROUNDCHANGE flag will not work.
 *
 * @note						The plugin may manually call this function with .trackRoundEvents=false in OnPluginStart()
 *								to optimize the resources usage a little bit in case plugin never uses TEX_NO_ROUNDCHANGE flag.
 *								This is undocumented behavior, and not recommended.
 */
private void _TEX_Init(bool trackRoundEvents = true)
{
	_TEX_FreeIndeces = new ArrayStack(ByteCountToCells(4));
	_TEX_UsedIndeces = new StringMap();
	
	for (int i = sizeof(_TEX_Storage) - 1; i > 0 /* zero index used for Disposed state fallback prop */; i--) 
	{
		_TEX_FreeIndeces.Push(i);
	}
	
	if (trackRoundEvents)
	{
		if (!HookEventEx("round_start", 		_TEX_Event_RoundStart, 		EventHookMode_PostNoCopy))
		{
			HookEventEx("round_freeze_end", 	_TEX_Event_RoundStart, 		EventHookMode_PostNoCopy);
		}
		HookEventEx("map_transition", 	_TEX_Event_RoundEnd, 		EventHookMode_PostNoCopy);
		HookEventEx("finale_win", 		_TEX_Event_RoundEnd, 		EventHookMode_PostNoCopy);
		HookEventEx("round_end", 		_TEX_Event_RoundEnd, 		EventHookMode_PostNoCopy);
		HookEventEx("mission_lost", 	_TEX_Event_RoundEnd, 		EventHookMode_PostNoCopy);
	}
	
	_TEX_IsInit = true;
}

/**
 * Helper to print debug information to the server console.
 */
private stock void _TEX_DBG_Print(const char[] format, any ...)
{
	static char buffer[192];
	VFormat(buffer, sizeof(buffer), format, 2);
	PrintToServer("[TEX] %s", buffer);
}

/** Private events, Do not use!
*/
private void _TEX_Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
																						_TEX_DBG("Event 'RoundStart'", "")
	_TEX_IsRoundStarted = true;
}
private void _TEX_Event_RoundEnd(Event event, const char[] name, bool dontBroadcast)
{
																						_TEX_DBG("Event 'RoundEnd'", "")
	if (_TEX_IsRoundStarted)
	{
		_TEX_KillTimers(view_as<int>(TEX_NO_ROUNDCHANGE));
		_TEX_IsRoundStarted = false;
	}
}


//												//
// =============== REQUIREMENTS =============== //
//												//

/** This method must be called from base plugin OnMapStart() forward to initialize TimerEx properly.
 */
void TEX_OnMapStart()
{
																						_TEX_DBG("OnMapStart", "")
	_TEX_IsMapStarted = true;
}

/** This method must be called from base plugin OnMapEnd() forward to initialize TimerEx properly.
 */
void TEX_OnMapEnd()
{
																						_TEX_DBG("TEX_OnMapEnd", "")
	if (_TEX_IsMapStarted)
	{
		_TEX_KillTimers(view_as<int>(TEX_NO_ROUNDCHANGE) | view_as<int>(TEX_NO_MAPCHANGE));
		_TEX_IsMapStarted = false;
	}
}


//												//
// ================== STOCKS ================== //
//												//

/**
 * Gets the TimerEx from the internal storage associated with a given handle value.
 * Can be used inside the Callback.
 *
 * @param hTimer	timer handle available through callback.
 * @param timerEx	TimerEx object to return.
 * 
 * @noerror
 */
stock void GetTimerEx(Handle hTimerEx, TimerEx timerEx)
{
	timerEx._index = view_as<int>(hTimerEx);
}

/**
 * Converts the state (TEX_State enum) into String, for debug purposes.
 *
 * @param state			state to convert.
 * @param buffer		String buffer to return the result.
 * @param maxlength		Maximum length of the buffer passed.
 *
 * @noerror
 */
stock void TEX_StateToString(TEX_State state, char[] buffer, int maxlength)
{
	if (state == TEX_Running)  		{ strcopy(buffer, maxlength, "Running");  	}
	if (state == TEX_Triggering)  	{ strcopy(buffer, maxlength, "Triggering"); }
	if (state == TEX_Paused)   		{ strcopy(buffer, maxlength, "Paused");   	}
	if (state == TEX_Stopped)  		{ strcopy(buffer, maxlength, "Stopped");  	}
	if (state == TEX_Disposed) 		{ strcopy(buffer, maxlength, "Disposed"); 	}
}

/**
 * Converts flags (TEX_Flags enum) into String, for debug purposes.
 *
 * @param flags			flags to convert.
 * @param buffer		String buffer to return the result.
 * @param maxlength		Maximum length of the buffer passed.
 *
 * @noerror
 */
stock void TEX_FlagsToString(TEX_Flags flags, char[] buffer, int maxlength)
{
	#define _TEX_FL_DELIM " | "
	buffer[0] = 0;
	if (flags & TEX_REPEAT) 						{ StrCat(buffer, maxlength, "TEX_REPEAT" 							... _TEX_FL_DELIM); }
	if (flags & TEX_NO_MAPCHANGE) 					{ StrCat(buffer, maxlength, "TEX_NO_MAPCHANGE" 						... _TEX_FL_DELIM); }
	if (flags & TEX_NO_ROUNDCHANGE) 				{ StrCat(buffer, maxlength, "TEX_NO_ROUNDCHANGE" 					... _TEX_FL_DELIM); }
	if (flags & TEX_NO_CALLBACK_DUPLICATES) 		{ StrCat(buffer, maxlength, "TEX_NO_CALLBACK_DUPLICATES" 			... _TEX_FL_DELIM); }
	if (flags & TEX_NO_CALLBACK_AND_DATA_DUPLICATES){ StrCat(buffer, maxlength, "TEX_NO_CALLBACK_AND_DATA_DUPLICATES" 	... _TEX_FL_DELIM); }
	if (flags & TEX_NO_REF)							{ StrCat(buffer, maxlength, "TEX_NO_REF" 							... _TEX_FL_DELIM); }
	if (flags & TEX_PAUSE)							{ StrCat(buffer, maxlength, "TEX_PAUSE" 							... _TEX_FL_DELIM); }
	if (flags & TEX_MAPSTART_REQ)					{ StrCat(buffer, maxlength, "TEX_MAPSTART_REQ" 						... _TEX_FL_DELIM); }
	if (flags & TEX_ROUNDSTART_REQ)					{ StrCat(buffer, maxlength, "TEX_ROUNDSTART_REQ" 					... _TEX_FL_DELIM); }
	if (flags & TEX_DATA_HNDL_CLOSE) 				{ StrCat(buffer, maxlength, "TEX_DATA_HNDL_CLOSE" 					... _TEX_FL_DELIM); }
	if (buffer[0] != 0)
	{
		buffer[(strlen(buffer) - 3 /*sizeof(_TEX_FL_DELIM)*/ + 1)] = 0;
	}
}

/**
 * Creates a basic TimerEx timer (without a reference).
 * To create timer with a reference, call the appropriate method via TimerEx instance.
 *
 * @param interval      Interval from the current game time to execute the given function.
 * @param func          Function to execute once the given interval has elapsed.
 * @param data          Handle or value to pass through to the timer callback function.
 * @param flags         Flags to set (such as repeatability or auto-Handle closing).
 * @return              true - if timer created successfully,
 *                      false - if timer already created and running, according to callback pointer 
 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
 *
 * @note				Calling .Kill() on a timer will end the timer.
 *
 * @error				thrown if interval set incorrectly (less than zero).
 */
stock void CreateTimerEx(float interval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
{
	TimerEx t;
	t.CreateRandomTimer(interval, interval, func, data, flags | TEX_NO_REF);
}

/**
 * Creates a TimerEx timer (without a reference) associated with a new datapack, and returns the datapack.
 * To create timer with a reference, call the appropriate method via TimerEx instance.
 *
 * @note The datapack is automatically freed when the timer ends.
 * @note The position of the datapack is not reset or changed for the timer function.
 *
 * @param interval      Interval from the current game time to execute the given function.
 * @param func          Function to execute once the given interval has elapsed.
 * @param datapack      The newly created datapack is passed though this by-reference
 *                      parameter to the timer callback function.
 * @param flags         Timer flags.
 * @return              true - if timer created successfully,
 *                      false - if timer already created and running, according to callback pointer 
 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
 *
 * @error				thrown if interval set incorrectly (less than zero).
 */
stock void CreateDataTimerEx(float interval, TimerExCallback func, Handle& datapack, TEX_Flags flags = TEX_FLAGS_NONE)
{
	TimerEx t;
	t.CreateDataTimer(interval, func, datapack, flags | TEX_NO_REF);
}

/**
 * Creates a basic TimerEx timer (without a reference) using the randomized interval.
 * The actual interval is calculated at each cycle.
 * To create timer with a reference, call the appropriate method via TimerEx instance.
 *
 * @param minInterval   Minimum interval from the current game time to execute the given function.
 * @param maxInterval   Maximum interval from the current game time to execute the given function.
 * @param func          Function to execute once the given interval has elapsed.
 * @param data          Handle or value to pass through to the timer callback function.
 * @param flags         Flags to set (such as repeatability or auto-Handle closing).
 * @return              true - if timer created successfully,
 *                      false - if timer already created and running, according to callback pointer 
 *						and/or data duplicate verification when TEX_NO_CALLBACK_DUPLICATES 
 *						or TEX_NO_CALLBACK_AND_DATA_DUPLICATES flags passed.
 *
 * @note				Calling .Kill() on a timer will end the timer.
 *
 * @error				thrown if interval set incorrectly (less than zero, or minInterval > maxInterval).
 */
stock void CreateRandomTimerEx(float minInterval, float maxInterval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
{
	TimerEx t;
	t.CreateRandomTimer(minInterval, maxInterval, func, data, flags | TEX_NO_REF);
}
