//
// Created by Let'sBlend on 09/04/2025.
//

#include <iostream>
#include <windows.h>
#include <strsafe.h>
#include <unordered_map>
#include <bitset>

#ifdef DEBUG
template<typename... Args>
void print(Args... args) {
    (std::wcout << ... << args) << std::endl;
}
#define PRINT(...) print(__VA_ARGS__)
#else
#define PRINT(...)
#endif
static HHOOK keyboardHook = nullptr;
static std::unordered_map<wchar_t, wchar_t> gTranslateKeys = {
    {L'\'', L'ä'},
    {L'[', L'ü'},
    {L';', L'ö'},
    {L'"', L'Ä'},
    {L'{', L'Ü'},
    {L':', L'Ö'},
    {L'-', L'ß'}
};
static bool stickyKey = false;

void SendCharacter(const wchar_t& character) {
    INPUT inputs[2] = {0};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = character;
    inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wScan = character;
    inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}
LRESULT CALLBACK KeyboardProc(const int nCode, const WPARAM wParam, const LPARAM lParam)
{
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        HWND foreGroundWindow = GetForegroundWindow();
        DWORD threadID = GetWindowThreadProcessId(foreGroundWindow, NULL);
        HKL keyLayout = GetKeyboardLayout(threadID);
        PRINT(keyLayout);

        BYTE keyboardState[256] = {0};
        keyboardState[VK_SHIFT] = GetKeyState(VK_SHIFT);
        keyboardState[VK_CAPITAL] = GetKeyState(VK_CAPITAL);

        KBDLLHOOKSTRUCT* kbHookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        wchar_t UTFChar = L'\0';
        int result = ToUnicodeEx(kbHookStruct->vkCode, kbHookStruct->scanCode, keyboardState, &UTFChar, 1, 0, keyLayout);
        if (result > 0) {
            PRINT("Captured input: ", std::bitset<16>(UTFChar));
        }
        else
            PRINT("ERROR while converting keyCode to Unicode");

        if (UTFChar == '^') {
            PRINT("Input was ^ -> We Block the event!");
            stickyKey = true;
            return 1;
        }

        if (!stickyKey) {
            PRINT("Pressed insignificant key -> Letting through the event!");
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }
        stickyKey = false;

        if (!gTranslateKeys.contains(UTFChar)) {
            PRINT("Previous key was ^, current key is insignificant -> Sending fake ^ input and letting through current key event");
            SendCharacter('^');
            if (kbHookStruct->vkCode == VK_SPACE)
                return 1;
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }

        PRINT("Previous key was ^, current key is significant -> Sending special german character and blocking current key event");
        SendCharacter(gTranslateKeys[UTFChar]);
        return 1;
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main()
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
}