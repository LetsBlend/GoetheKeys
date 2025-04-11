//
// Created by Let'sBlend on 09/04/2025.
//

#include <iostream>
#include <windows.h>
#include <strsafe.h>
#include <unordered_map>

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
        HKL keyLayout = GetKeyboardLayout(0);
        BYTE keyboardState[256] = {0};
        keyboardState[VK_SHIFT] = GetKeyState(VK_SHIFT);
        keyboardState[VK_CAPITAL] = GetKeyState(VK_CAPITAL);

        KBDLLHOOKSTRUCT* kbHookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        wchar_t UTFChar = L'\0';
        int result = ToUnicodeEx(kbHookStruct->vkCode, kbHookStruct->scanCode, keyboardState, &UTFChar, 1, 0, keyLayout);

        if (result > 0 && UTFChar == '^') {
            stickyKey = true;
            return 1;
        }

        if (!stickyKey)
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        stickyKey = false;

        if (!gTranslateKeys.contains(UTFChar)) {
            SendCharacter('^');
            if (kbHookStruct->vkCode == VK_SPACE)
                return 1;
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }

        SendCharacter(gTranslateKeys[UTFChar]);
        return 1;
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main()
{
    HINSTANCE hInst = GetModuleHandle(nullptr);
    keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
}