#include <Windows.h>
#include "main.h"

using namespace std;

SAMPFUNCS *SF = new SAMPFUNCS();

struct Controller {
	bool state = false;
	uint8_t value = 0xFF;
};

Controller* Time = new Controller();
Controller* Weather = new Controller();

void CALLBACK CMD_settime(string params)
{
	if (params.empty()) {
		if (Time->state == true) {
			Time->state = false;
			SF->getSAMP()->getInfo()->pSettings->byteWorldTime_Hour = Time->value;
			SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, "[Time] {FFFFFF}Время установлено на серверное");
		}
		return;
	}

	uint8_t time = 0xFF;
	int success = sscanf(params.c_str(), "%hhu", &time);

	if (!success || time < 0 || time > 23) {
		SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, "[Time] {FFFFFF}Используйте: /settime [0-23]");
		return;
	}
	Time->state = true;
	SF->getSAMP()->getInfo()->pSettings->byteWorldTime_Hour = time;
	string message = "[Time] {FFFFFF}Время установлено на {2A78F5}" + to_string(time);
	SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, message.c_str());
}

void CALLBACK CMD_setweather(string params)
{
	if (params.empty()) {
		if (Weather->state == true) {
			Weather->state = false;
			*(DWORD*)(0xC81320) = Weather->value;
			*(DWORD*)(0xC8131C) = Weather->value;
			SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, "[Weather] {FFFFFF}Погода установлена на серверную");
		}
		return;
	}

	uint8_t weather = 0xFF;
	int success = sscanf(params.c_str(), "%hhu", &weather);

	if (!success || weather < 0 || weather > 45) {
		SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, "[Weather] {FFFFFF}Используйте: /setweather [0-45]");
		return;
	}
	Weather->state = true;
	*(DWORD*)(0xC81320) = weather;
	*(DWORD*)(0xC8131C) = weather;
	string message = "[Weather] {FFFFFF}Погода установлена на {2A78F5}" + to_string(weather);
	SF->getSAMP()->getChat()->AddChatMessage(0x2A78F5, message.c_str());
}

bool CALLBACK IncomingRPC(stRakNetHookParams* params)
{
	if (params->packetId == RPC_ScrSetWorldTime || params->packetId == RPC_ScrSetPlayerTime) {
		params->bitStream->ResetReadPointer();
		params->bitStream->Read(Time->value);
		return !Time->state;
	}
	else if (params->packetId == RPC_ScrSetWeather) {
		params->bitStream->ResetReadPointer();
		params->bitStream->Read(Weather->value);
		return !Weather->state;
	}
	return true;
}

void CALLBACK MainLoop()
{
	static bool initialized = false;
	if (!initialized)
	{
		if (GAME && GAME->GetSystemState() == eSystemState::GS_PLAYING_GAME && SF->getSAMP()->IsInitialized())
		{
			initialized = true;
			SF->getSAMP()->registerChatCommand("settime", CMD_settime);
			SF->getSAMP()->registerChatCommand("setweather", CMD_setweather);
			SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_INCOMING_RPC, IncomingRPC);
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
		SF->initPlugin(MainLoop, hModule);
	return TRUE;
}
