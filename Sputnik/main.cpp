
#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    // ОПИСАТЬ КАЖДУЮ API !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //Запуск приложения
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t cmdLine[] = L"F:/Application/Lab1/build/Desktop_Qt_6_8_2_MSVC2022_64bit-Debug/Lab1.exe";

    if(CreateProcess(cmdLine, NULL, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
        std::cout << "Process id created !" << std::endl;
        std::cout << "Pid = " << std::dec << pi.dwProcessId << std::endl;
    }
    else {
        std::cout << "Process is not created! unlucky :( "<< std::endl;
    }

    //Подключение к приложению как отладчик

    bool isAttached = DebugActiveProcess(pi.dwProcessId);
    if(!isAttached) {
        DWORD lastError = GetLastError();
        std::cout << std::hex << "DebugActiveProcess is FAILED, LastError is " << lastError;
    }
    else {
        std::cout << "DebugActiveProcess is success" << std::endl;
    }

    // Пропускать поступающие сигналы отладки

    DEBUG_EVENT debugEvent;
    while(true) {
        bool result1 = WaitForDebugEvent(&debugEvent, INFINITE);
        bool result2 = ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
    }
    return 0;

}
