#pragma semicolon 1
#pragma newdecls required

#define TIMEREX_DEBUG 1 // remove this line in production code
#include <timerex>

public Plugin myinfo =
{
	name = "TimerEx Tester",
	author = "Dragokas",
	description = "Platform to test various methods of TimerEx include",
	version = "1.0",
	url = "https://github.com/dragokas"
};

// ================================================
//					REQUIREMENTS
// ================================================

public void OnMapStart()
{	
	TEX_OnMapStart(); // required
}
public void OnMapEnd()
{
	TEX_OnMapEnd(); // required
}

// =================================================

TimerEx g_Timer;

public void OnPluginStart()
{
	PrintToServer(" ====================================== ");
	PrintToServer(" =========== OnPluginStart ============ ");
	PrintToServer(" ====================================== ");
	
	DataPack dp = new DataPack();
	g_Timer.CreateTimer(4.0, OnTrigger_I, dp, TEX_REPEAT | TEX_NO_ROUNDCHANGE | TEX_ROUNDSTART_REQ | TEX_PAUSE);
	
	// You can limit the number of callback executions right here:
	//g_Timer.SetInvokeMaxCount(2);
	
	RegConsoleCmd("sm_t.run", 		CmdRun, 		"Runs the timer if it was paused or stopped");
	RegConsoleCmd("sm_t.stop", 		CmdStop, 		"Stops the timer");
	RegConsoleCmd("sm_t.restart", 	CmdRestart, 	"Restarts the timer, thus Stop + Run been called");
	RegConsoleCmd("sm_t.pause", 	CmdPause, 		"Pauses the timer");
	RegConsoleCmd("sm_t.resume", 	CmdResume, 		"Resumes the timer, that was paused");
	RegConsoleCmd("sm_t.kill", 		CmdKill, 		"Stops and kills the timer");
	RegConsoleCmd("sm_t.delay", 	CmdDelay, 		"Delays trigger execution by specified number of seconds");
	RegConsoleCmd("sm_t.maxinvoke", CmdMaxInvoke,	"Sets maximum number of callback invokes");
	RegConsoleCmd("sm_t.trigger", 	CmdTrigger, 	"Triggers timer callback instantly (timer must be at running state)");
	RegConsoleCmd("sm_t.info", 		CmdInfo, 		"Prints all internal timer info");
	RegConsoleCmd("sm_t.mapend", 	CmdMapEnd, 		"Imitate Map End event");
	RegConsoleCmd("sm_t.roundend", 	CmdRoundEnd, 	"Imitate Round End event");
	RegConsoleCmd("sm_t.mapstart", 	CmdMapStart, 	"Imitate Map Start event");
	RegConsoleCmd("sm_t.roundstart",CmdRoundStart, 	"Imitate Round Start event");
	
	// Example of reference-less timer
	//CreateTimerEx(2.0, OnTrigger_NoRef, dp, TEX_DATA_HNDL_CLOSE);
	
	if (!HookEventEx("round_start", 		Event_RoundStart, 		EventHookMode_PostNoCopy))
	{
		HookEventEx("round_freeze_end", 	Event_RoundStart, 		EventHookMode_PostNoCopy);
	}
}

// ===================================================
//		ROUND AND MAP START DUPLICATE PREVENTION
// ===================================================

void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
	// Prevents creating multiple timers if Event_RoundStart called multiple times and the previously created timer is still running
	CreateTimerEx(3.0, OnTrigger_NoRef, 0, TEX_NO_CALLBACK_DUPLICATES);
	
	// You can imitate such an issue
	// The result would be: the only first (above) timer is created
	CreateTimerEx(3.0, OnTrigger_NoRef, 0, TEX_NO_CALLBACK_DUPLICATES);
	CreateTimerEx(3.0, OnTrigger_NoRef, 0, TEX_NO_CALLBACK_DUPLICATES);
	CreateTimerEx(3.0, OnTrigger_NoRef, 0, TEX_NO_CALLBACK_DUPLICATES);
	
	// If you wanna pass different data to the same callback,
	// use TEX_NO_CALLBACK_AND_DATA_DUPLICATES flag instead
	// Example:
	DataPack dp1 = new DataPack();
	CreateTimerEx(1.0, OnTrigger_NoRef, dp1, TEX_NO_CALLBACK_AND_DATA_DUPLICATES | TEX_DATA_HNDL_CLOSE);
	DataPack dp2 = new DataPack();
	CreateTimerEx(2.0, OnTrigger_NoRef, dp2, TEX_NO_CALLBACK_AND_DATA_DUPLICATES | TEX_DATA_HNDL_CLOSE);
}

// ================================================
//					COMMANDS
// ================================================

Action CmdRun(int client, int argc)
{
	g_Timer.Run();
	return Plugin_Handled;
}

Action CmdStop(int client, int argc)
{
	g_Timer.Stop();
	return Plugin_Handled;
}

Action CmdRestart(int client, int argc)
{
	g_Timer.Restart();
	return Plugin_Handled;
}

Action CmdPause(int client, int argc)
{
	g_Timer.Pause();
	return Plugin_Handled;
}

Action CmdResume(int client, int argc)
{
	g_Timer.Resume();
	return Plugin_Handled;
}

Action CmdKill(int client, int argc)
{
	g_Timer.Kill();
	return Plugin_Handled;
}

Action CmdDelay(int client, int argc)
{
	if (argc == 0) {
		ReplyToCommand(client, "sm_t.delay [seconds]");
		return Plugin_Handled;
	}
	float seconds;
	if (GetCmdArgFloatEx(1, seconds))
	{
		g_Timer.Delay(seconds);
	}
	return Plugin_Handled;
}

Action CmdMaxInvoke(int client, int argc)
{
	if (argc == 0) {
		ReplyToCommand(client, "sm_t.maxinvoke [count]");
		return Plugin_Handled;
	}
	int count;
	if (GetCmdArgIntEx(1, count))
	{
		g_Timer.SetInvokeMaxCount(count);
	}
	return Plugin_Handled;
}

Action CmdTrigger(int client, int argc)
{
	g_Timer.Trigger(true);
	return Plugin_Handled;
}

Action CmdInfo(int client, int argc)
{
	char info[512];
	g_Timer.GetInfo(info, sizeof(info));
	PrintToServer(info);
	return Plugin_Handled;
}

Action CmdMapEnd(int client, int argc)
{
	OnMapEnd();
	return Plugin_Handled;
}

Action CmdMapStart(int client, int argc)
{
	OnMapStart();
	return Plugin_Handled;
}

Action CmdRoundEnd(int client, int argc)
{
	//_TEX_Event_RoundEnd(null, NULL_STRING, false);
	CreateEvent("round_end", false).Fire(false);
	return Plugin_Handled;
}

Action CmdRoundStart(int client, int argc)
{
	//_TEX_Event_RoundStart(null, NULL_STRING, false);
	CreateEvent("round_start", false).Fire(false);
	return Plugin_Handled;
}

// ================================================
//					CALLBACKS
// ================================================

// Example callback of reference-less timer, e.g. via CreateTimerEx() or TEX_NO_REF
public Action OnTrigger_NoRef(ITimerEx timerEx, any data)
{
	char info[512];
	timerEx.GetInfo(info, sizeof(info));
	PrintToServer("OnTrigger_NoRef is called:\n%s", info);
	
	//This call is not valid in reference-less timer!
	//timerEx.Delay(2.0, true);
	
	// You can only retrieve the information here.
	// Any attempt to control the timer is UD, and may throw an error.
	return Plugin_Continue;
}

// "Timer" TypeSet compatible callback, see: https://sm.alliedmods.net/new-api/timers/Timer
public Action OnTrigger_TimerCB(Handle hTimerEx /* pseudo-handle */, any data)
{
	// you can use g_Timer directly, or retrieve TimerEx object from pseudo-handle:
	TimerEx tex;
	GetTimerEx(hTimerEx, tex);
	char info[512];
	tex.GetInfo(info, sizeof(info));
	PrintToServer("OnTrigger_TimerCB is called:\n%s", info);
	
	// alternatively, you can call via interface:
	ITimerEx ITex = new ITimerEx(hTimerEx);
	// example, how to delay each next cycle by 2 seconds
	ITex.Delay(2.0, true);
	
	return Plugin_Continue;
}

// or you can pass pseudo-interface ITimerEx as callback argument to call TimerEx methods directly:
public Action OnTrigger_I(ITimerEx timerEx, any data)
{
	char info[512];
	timerEx.GetInfo(info, sizeof(info));
	PrintToServer("OnTrigger_I is called:\n%s", info);
	
	// example, how to delay each next cycle by 2 seconds
	timerEx.Delay(2.0);
	
	return Plugin_Continue;
}
