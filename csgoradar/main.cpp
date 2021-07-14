#include <iostream>
#include <string>
#include <stdlib.h>
#include "SerialPort.h"
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <cmath>   
#include <algorithm>
#include "math.h"
char output[MAX_DATA_LENGTH];
char incomingData[MAX_DATA_LENGTH];

// change the name of the port with the port name of your computer
// must remember that the backslashes are essential so do not remove them
char *port = "\\\\.\\COM3";



/*
* you will probably have to update these adresses for it to work.
* which you can find here: 
* https://github.com/frk1/hazedumper/blob/master/csgo.cs
*/
#define dwEntityList 0x4DA215C
#define dwViewMatrix 0x4D93A74
#define dwLocalPlayer 0xD892CC
#define m_bSpotted 0x93D
#define m_vecOrigin 0x138
#define m_vecViewOffset 0x108
#define dwClientState_ViewAngles 0x4D90
#define dwClientState 0x588FEC

#define m_angEyeAnglesX 0xB380
//#define m_angEyeAnglesY 0xB384
#define m_iTeamNum 0xF4
#define m_iHealth 0x100
#define m_bDormant 0xED


struct vector
{
    float x, y, z;
};

struct vector2
{
    float x, y;
};



DWORD dwPid;
HANDLE hProcess;
DWORD client;
DWORD engine;

uintptr_t GetModule(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

template<typename T> T RPM(SIZE_T address) {
    T buffer; ReadProcessMemory(hProcess, (void*)address, &buffer, sizeof(T), nullptr);
    return buffer;
}




template<typename T> void WPM(SIZE_T address, T buffer) {
    WriteProcessMemory(hProcess, (void*)address, &buffer, sizeof(T), nullptr);
}





int main(){
	SerialPort arduino(port);
	if(arduino.isConnected()){
		std::cout << "Connection made" << std::endl<< std::endl;
	}
	else{
		std::cout << "Error in port name" << std::endl << std::endl;
	}

    GetWindowThreadProcessId(FindWindowA(0, "Counter-Strike: Global Offensive"), &dwPid);
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);
    client = GetModule(dwPid, L"client.dll");
    engine = GetModule(dwPid, L"engine.dll");
    DWORD LocalPlayer = RPM<DWORD>(client + dwLocalPlayer);
    DWORD ClientState = RPM<DWORD>(engine + dwClientState);


    std::string data;



    while (arduino.isConnected()) {
        data = "00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000";
        data[4 * 9 + 4] = '1';

        for (int i = 1; i < 64; i++) {

            DWORD dwCurrentEntity = RPM<DWORD>(client + dwEntityList + i * 0x10);

            if (dwCurrentEntity) {
                int ehealth = RPM<int>(dwCurrentEntity + m_iHealth);
                int eteam = RPM<int>(dwCurrentEntity + m_iTeamNum);
                int team = RPM<int>(LocalPlayer + m_iTeamNum);
                bool isdormant = RPM<int>(dwCurrentEntity + m_bDormant);
                
                
                WPM<bool>(dwCurrentEntity + m_bSpotted, true);

                if (ehealth > 0 && eteam != team && isdormant == false) {
                    vector Loc = RPM<vector>(LocalPlayer + m_vecOrigin);
                    vector Enemy = RPM<vector>(dwCurrentEntity + m_vecOrigin);
                    Vector angle = RPM<Vector>(ClientState + dwClientState_ViewAngles);

                   // Vector angle = RPM<Vector>(LocalPlayer + m_angEyeAnglesX);
                    Math::NormalizeAngles(angle);

                    float x_diff = Loc.x - Enemy.x;
                    float y_diff = Loc.y - Enemy.y;

                    float flOffset = atanf(y_diff / x_diff);
                    flOffset *= M_PI;
                    flOffset /= 180;

                    if ((x_diff < 0) && (y_diff >= 0))
                        flOffset = 180 + flOffset;
                    else if ((x_diff < 0) && (y_diff < 0))
                        flOffset = 180 + flOffset;
                    else if ((x_diff >= 0) && (y_diff < 0))
                        flOffset = 360 + flOffset;

                    y_diff = -1 * (sqrtf((x_diff * x_diff) + (y_diff * y_diff)));
                    x_diff = 0;

                    flOffset = angle.y - flOffset;

                    flOffset *= M_PI;
                    flOffset /= 180;


                    float xnew_diff = x_diff * cosf(flOffset) + y_diff * sinf(flOffset);
                    float ynew_diff = x_diff * sinf(flOffset) + y_diff * cosf(flOffset);


                    if ((xnew_diff < 0) && (ynew_diff >= 0))
                        xnew_diff *= .8;
                    else if ((xnew_diff < 0) && (ynew_diff < 0))
                        ynew_diff *= .8;

                   float distance = sqrt(pow(Loc.x - Enemy.x, 2) + pow(Loc.y - Enemy.y, 2) + pow(Loc.z - Enemy.z, 2));
                   int scale = 100;


                    int dx = 4+(xnew_diff / scale);
                    int dy = 4+(ynew_diff / scale);

                    if (dx <= 0)dx = 1;
                    if (dy <= 0)dx = 1;
                    if (dx > 7)dx = 7;
                    if (dy > 7)dx = 7;

                   



                    if (dx < 8 && dx > 0 && dy < 8 && dy > 0 && (dx * 8 + dy) <= 71) data[dx * 9 + dy] = '1';
                    if(dx * 9 + dy == 9)data[4 * 9 + 1] = '1';

                    std::cout << "\n" << std::endl;
                    std::cout << xnew_diff << "   " << ynew_diff << std::endl;
                   // std::cout << dx * 9 + dy << std::endl;
                    //std::cout << data << std::endl;
               }
           }
        }
        //data = "111111111,00000000,00000000,00010000,00000000,00000000,00000000";
   

        char* charArray = new char[data.size() + 1];
        std::copy(data.begin(), data.end(), charArray);
        charArray[data.size()] = '\n';

        arduino.writeSerialPort(charArray, MAX_DATA_LENGTH);
        //arduino.readSerialPort(output, MAX_DATA_LENGTH);

        delete[] charArray;

       // std::cout << data << std::endl;

        Sleep(10);
    }
    //std::cout << "\x1B[2J\x1B[H";

	return 0;
}
