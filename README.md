# SourceMod-TimerEx
SM CreateTimer with extended functionality, include only (`timerex.inc`).

**TimerEx** is a wrapper over SourceMod's native `CreateTimer`, providing a more powerful and flexible timer system for plugin developers.

It adds advanced lifecycle control, duplicate protection (that usually happens in round_start event), randomized intervals, pause/resume support, — all while keeping a familiar API.

## ✨ Features

- Extended timer lifecycle control (run, stop, pause, resume, restart, delay)
- Randomized interval timers
- Duplicate prevention (by callback or callback + data)
- Invoke count limiting
- All usual features, like manual trigger support
- Automatic data handle management
- Map/round lifecycle awareness
- Debugging output in server console, #define controllable
- Optional reference-less timers (lightweight usage)
- Special flags to prevent timer from triggering during map/round loading phase, without kill

## ⚙️ Requirements

1. SourceMod v1.11 or greater
2. You **must** call the following forwards redirection in your plugin:

```
#include <timerex>

public void OnMapStart()
{
    TEX_OnMapStart();
}

public void OnMapEnd()
{
    TEX_OnMapEnd();
}
```

## Basic Usage
```
TimerEx tex;
tex.CreateTimer(3.0, MyCallback, data, TEX_REPEAT);
```
Or without keeping a reference (timer control methods will be **NOT** available):
```
CreateTimerEx(3.0, MyCallback, data, TEX_REPEAT);
```

## Security considerations

 - TimerEx designed to be silent, e.g. double Kill() call is not considered as an error. This may hide an essential error in your plugin algorithm. Such a way it is unacceptable to have TimerEx as a direct CreateTimer() replacement. For the general purposes, you should stick with standard SM CreateTimer() and following the [Typical templates](https://forums.alliedmods.net/showpost.php?p=2688863&postcount=32) unless you really understand and accept the consequences.
 - You'll never ever need to call CloseHandle() manually.
 - Never ever use the same **tex** reference when you need to create multiple different timers; you need array of references, temporarily reference, or reference-less stock call to do that.
 - TimerEx is not AI, you still need a brain to pass TEX_DATA_HNDL_CLOSE where appropriate, or not pass it if you wanna re-use same data on the next round/map.
 - With a lot of additional TimerEx flags, you can change your approach to the Timer coding phylosophy a little bit, e.g. by utilizing `TEX_ROUNDSTART_REQ` flag instead of killing the timer each time on round end. See later about flags details.


# 🧠 TimerEx Methods

## Creation
```
bool CreateTimer(float interval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
bool CreateDataTimer(float interval, TimerExCallback func, Handle& datapack, TEX_Flags flags = TEX_FLAGS_NONE)
bool CreateRandomTimer(float minInterval, float maxInterval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
```

## Stocks for reference-less timer
```
stock void CreateTimerEx(float interval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
stock void CreateDataTimerEx(float interval, TimerExCallback func, Handle& datapack, TEX_Flags flags = TEX_FLAGS_NONE)
stock void CreateRandomTimerEx(float minInterval, float maxInterval, TimerExCallback func, any data = 0, TEX_Flags flags = TEX_FLAGS_NONE)
```

## Control
```
bool Run()
bool Stop(bool resetInvokeCount = true)
bool Restart()
bool Pause()
bool Resume()
bool Delay(float seconds, bool applyToAllCycles = false)
bool Trigger(bool reset)
bool Kill(bool closeDataHandle = false)
bool Dispose()
```

## State & Info
```
bool IsStarted()
bool IsPaused()
bool IsFinished()
TEX_State GetState()
char[] GetStateString()
void GetInfo(char[] buffer, int maxlength)
```

## Timing
```
float GetTimeElapsed()
float GetTimeRemaining()
float GetCreationTime()
float GetStartTime()
float GetEndTime()
float GetStopTime()
float GetInterval()
void SetInterval(float interval)
float GetMinInterval()
void SetMinInterval(float interval)
float GetMaxInterval()
void SetMaxInterval(float interval)
```

## Invoke Control
```
void SetInvokeMaxCount(int count)
int GetInvokeMaxCount()
int GetInvokeCount()
``` 

## Properties Access
```
TimerExCallback GetCallback()
Handle GetTimer()
any GetData()
int GetFlags()
```

## 📦 Stock Functions
```
stock void GetTimerEx(Handle hTimerEx, TimerEx timerEx)
stock void TEX_StateToString(TEX_State state, char[] buffer, int maxlength)
stock void TEX_FlagsToString(TEX_Flags flags, char[] buffer, int maxlength)
```

## Callbacks (TimerExCallback)
```
function Action(Handle hTimerEx, any data);
function Action(Handle hTimerEx);
function void(Handle hTimerEx, any data);
function void(Handle hTimerEx);
function Action(ITimerEx timerEx, any data);
function Action(ITimerEx timerEx);
function void(ITimerEx timerEx, any data);
function void(ITimerEx timerEx);
```

Note: callback is fully compatible with SM ["Timer" typeset](https://sm.alliedmods.net/new-api/timers/Timer) , it is additionally supports ITimerEx pseudo-interface, so you may call TimerEx methods directly by ITimerEx reference.

## Flags (TEX_Flags)

| Flag | Description |
|------|-------------|
| `TEX_REPEAT` | Timer will repeat until it returns `Plugin_Stop` |
| `TEX_NO_MAPCHANGE` | Timer will not carry over map changes |
| `TEX_NO_ROUNDCHANGE` | Timer will not carry over round changes |
| `TEX_NO_CALLBACK_DUPLICATES` | On multiple calls, single-only timer is created per unique callback, unless timer stopped |
| `TEX_NO_CALLBACK_AND_DATA_DUPLICATES` | On multiple calls, single-only timer is created per unique callback and data handle, unless timer stopped |
| `TEX_NO_REF` | Disposes TimerEx reference as soon as timer gets stopped, no deferred access to reference allowed; this flag is used by CreateTimerEx stock |
| `TEX_PAUSE` | Creates timer in paused state |
| `TEX_MAPSTART_REQ` | Requires map start to trigger the timer; trigger will be skipped after map ends |
| `TEX_ROUNDSTART_REQ` | Requires round start to trigger the timer; trigger will be skipped after round ends |
| `TEX_DATA_HNDL_CLOSE` | Timer will automatically call `CloseHandle()` on its data when finished |

## States (TEX_State)

| State | Description |
|------|-------------|
| `TEX_Disposed` | Timer index is disposed (see `.Dispose()` method note) |
| `TEX_Running` | Timer is started; callback triggering is expected when the interval elapses |
| `TEX_Triggering` | Intermediate state when TimerEx is executing the callback |
| `TEX_Paused` | Timer is paused via `.Pause()` method |
| `TEX_Stopped` | Timer is stopped via `.Stop()`, map/round end events, or when all cycles are finished |

## Stop() vs Kill() vs Dispose() difference

```
bool Stop(bool resetInvokeCount = true)
bool Kill(bool closeDataHandle = false)
bool Dispose()
```

 - `Kill()` is analogue of SM KillTimer(), however unlike SM, it is allowed to `Run()` timer again using same **tex** reference.
 - Unlike Kill, the `Stop()` method does **NOT** closes the data handle when TEX_DATA_HNDL_CLOSE flag passed, so `Stop()` - `Run()` methods are usually working in pair.
 - Prefer `Stop()` method if you plan to `Run()` timer again without re-creating **data handle** (or where you not using data handle at all). In this case, you can **tex.CreateTimer()** only once ever in **OnPluginStart()**, but it's not a strict requirement, you can freely call tex.CreateTimer() multiple times using same "tex" reference without risk of handles leaking.
 - Prefer `Kill()` if you start the timer each time with **tex.CreateTimer()** paired with creating new data handle each time.
 - `Kill()` automatically resets invokation counter property, used with **SetInvokeMaxCount()** in further execution check.
 - `Dispose()` kills the timer and releases the TimerEx pseudo-handle (index), so the next calls to **tex** reference considered as invalid or undefined behavior, usually such call should throw an error: "TimerEx cannot call timer control methods when disposed". Use `Dispose()` only if you really want to see runtime errors when user side code try to call the disposed reference. `Dispose()` is get automatically called for reference-less timers (they created with TEX_NO_REF flag).

## Performance & Technical details

 - TimerEx doing a little overhead due to calling your callback via Call_StartFunction, so it's not recommended for extremely frequent repeatable timers and very slow servers.
 - TimerEx creates 1 handle per timer, and 2 permanent handles per plugin.
 - Methods like Delay(), Pause() / Resume() re-creates the handle.

## Limitations

There are maximum 256 timers per single plugin allowed. To decrease (for less memory allocation) or increase (for whatever reason?) this limit, please override the define in your .sp **before** including timerex.inc:
```
#define TIMEREX_MAX_TIMERS 128
#include <timerex>
```

## Documentation & Usage

See `timerex.inc` for the full arguments description.

See `TimerEx_test.sp` for usage examples.
