#include "win32hooks_plugin.h"
#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include "include/encoding.h"
#include <iostream>
#include <conio.h>
#include <oleacc.h>
#include <stdio.h>
#pragma comment(lib, "Oleacc.lib")

std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>, std::default_delete<flutter::MethodChannel<flutter::EncodableValue>>> channel = nullptr;
#pragma warning(push)
#pragma warning(disable : 4189)

void CALLBACK HandleWinEvent(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
LRESULT CALLBACK HandleMouseHook(int, WPARAM, LPARAM);
HWINEVENTHOOK g_EventHook = NULL;
HHOOK g_MouseHook = NULL;
int mouseWatchButtons[7] = {0, 0, 0, 0, 0, 0, 0};
int mouseControlButtons[7] = {0, 0, 0, 0, 0, 0, 0};
#define EVENTHOOK 1
#define MOUSEHOOK 2

LRESULT CALLBACK HandleMouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode != HC_ACTION) // Nothing to do :(
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    MSLLHOOKSTRUCT *info = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    enum
    {
        BTN_LEFT,
        BTN_RIGHT,
        BTN_MIDDLE,
        BTN_SWUP,
        BTN_SWDOWN,
        BTN_XBUTTON1,
        BTN_XBUTTON2,
        BTN_NONE
    } button = BTN_NONE;

    char const *up_down[] = {"up", "down"};
    bool down = false;

    switch (wParam)
    {

    case WM_LBUTTONDOWN:
        down = true;
    case WM_LBUTTONUP:
        button = BTN_LEFT;
        break;

    case WM_RBUTTONDOWN:
        down = true;
    case WM_RBUTTONUP:
        button = BTN_RIGHT;
        break;

    case WM_MBUTTONDOWN:
        down = true;
    case WM_MBUTTONUP:
        button = BTN_MIDDLE;
        break;

    case WM_XBUTTONDOWN:
        down = true;
    case WM_XBUTTONUP:
        button = BTN_XBUTTON1;
        break;

    case WM_MOUSEWHEEL:
        // the hi order word might be negative, but WORD is unsigned, so
        // we need some signed type of an appropriate size:
        down = static_cast<std::make_signed_t<WORD>>(HIWORD(info->mouseData)) < 0;
        if (!down)
            button = BTN_SWUP;
        else
            button = BTN_SWDOWN;
        break;
    }

    if (button != BTN_NONE)
    {
        if (button == BTN_XBUTTON1)
        {
            if (HIWORD(info->mouseData) == 2)
                button = BTN_XBUTTON2;
        }
        int bID = (int)button;
        if (bID < 7 && (mouseWatchButtons[bID] == 1 || mouseControlButtons[bID] == 1))
        {
            flutter::EncodableMap args = flutter::EncodableMap();
            args[flutter::EncodableValue("hookID")] = flutter::EncodableValue((int)((DWORD_PTR)g_MouseHook)); // DWORD_PTR
            args[flutter::EncodableValue("hookType")] = flutter::EncodableValue(MOUSEHOOK);
            args[flutter::EncodableValue("state")] = flutter::EncodableValue(down);
            args[flutter::EncodableValue("button")] = flutter::EncodableValue(bID);
            if (mouseWatchButtons[bID] == 1)
            {
                args[flutter::EncodableValue("type")] = flutter::EncodableValue("watch");
            }
            if (mouseControlButtons[bID] == 1)
            {
                args[flutter::EncodableValue("type")] = flutter::EncodableValue("control");
                channel->InvokeMethod("onEvent", std::make_unique<flutter::EncodableValue>(args));
                return -1;
            }
            channel->InvokeMethod("onEvent", std::make_unique<flutter::EncodableValue>(args));
        }
    }
    // send output
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    flutter::EncodableMap args = flutter::EncodableMap();

    args[flutter::EncodableValue("hookID")] = flutter::EncodableValue((int)((DWORD_PTR)g_EventHook)); // DWORD_PTR
    args[flutter::EncodableValue("hookType")] = flutter::EncodableValue(EVENTHOOK);
    args[flutter::EncodableValue("event")] = flutter::EncodableValue((int)event);
    args[flutter::EncodableValue("hWnd")] = flutter::EncodableValue((int)((DWORD_PTR)hWnd));
    args[flutter::EncodableValue("idObject")] = flutter::EncodableValue(idObject);
    args[flutter::EncodableValue("idChild")] = flutter::EncodableValue(idChild);
    args[flutter::EncodableValue("dwEventThread")] = flutter::EncodableValue((int)dwEventThread);
    args[flutter::EncodableValue("dwmsEventTime")] = flutter::EncodableValue((int)dwmsEventTime);
    channel->InvokeMethod("onEvent", std::make_unique<flutter::EncodableValue>(args));
    // _EmitEvent(args, (int)((DWORD_PTR)hook), EVENTHOOK);
    // _EmitEvent(args, (int)((DWORD_PTR)g_EventHook), EVENTHOOK);
}

#pragma warning(pop)

void Win32hooksPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar)
{
    channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
        registrar->messenger(), "win32hooks", &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<Win32hooksPlugin>();

    channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto &call, auto result)
                                  { plugin_pointer->HandleMethodCall(call, std::move(result)); });

    registrar->AddPlugin(std::move(plugin));
}

Win32hooksPlugin::Win32hooksPlugin()
{
    // g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, HandleMouseHook, GetModuleHandle(NULL), 0);
    // g_EventHook = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZEEND, NULL, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

Win32hooksPlugin::~Win32hooksPlugin()
{
    UnhookWinEvent(g_EventHook);
    UnhookWindowsHookEx(g_MouseHook);
}

void Win32hooksPlugin::HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
    if (method_call.method_name().compare("installHooks") == 0)
    {

        const flutter::EncodableMap &getArgs = std::get<flutter::EncodableMap>(*method_call.arguments());
        int eventMin = std::get<int>(getArgs.at(flutter::EncodableValue("eventMin")));
        int eventMax = std::get<int>(getArgs.at(flutter::EncodableValue("eventMax")));
        int eventFilters = std::get<int>(getArgs.at(flutter::EncodableValue("eventFilters")));

        g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, HandleMouseHook, GetModuleHandle(NULL), 0);

        if (eventMin > 0)
            g_EventHook = SetWinEventHook(eventMin, eventMax, NULL, HandleWinEvent, 0, 0, eventFilters);
        else
            g_EventHook = NULL;

        flutter::EncodableMap args = flutter::EncodableMap();
        args[flutter::EncodableValue("mouseHookID")] = flutter::EncodableValue((int)((LONG_PTR)g_MouseHook)); // DWORD_PTR
        args[flutter::EncodableValue("eventHookID")] = flutter::EncodableValue((int)((LONG_PTR)g_EventHook)); // DWORD_PTR
        result->Success(flutter::EncodableValue(args));
    }
    else if (method_call.method_name().compare("uninstallHooks") == 0)
    {
        if (g_EventHook != NULL)
            UnhookWinEvent(g_EventHook);
        if (g_MouseHook != NULL)
            UnhookWindowsHookEx(g_MouseHook);
        g_EventHook = NULL;
        g_MouseHook = NULL;
        result->Success(flutter::EncodableValue("Hooks uninstalled"));
    }
    else if (method_call.method_name().compare("cleanHooks") == 0)
    {
        for (int i = 0; i < 7; i++)
        {
            mouseWatchButtons[i] = 0;
            mouseControlButtons[i] = 0;
        }
        result->Success(flutter::EncodableValue(true));
    }
    else if (method_call.method_name().compare("uninstallSpecificHookID") == 0)
    {
        const flutter::EncodableMap &args = std::get<flutter::EncodableMap>(*method_call.arguments());
        int hookID = std::get<int>(args.at(flutter::EncodableValue("hookID")));
        int hookType = std::get<int>(args.at(flutter::EncodableValue("hookType")));
        if (hookType == 1)
        {
            UnhookWinEvent((HWINEVENTHOOK)((DWORD_PTR)hookID));
            result->Success(flutter::EncodableValue("Hook WinEvent Uninstalled."));
        }
        else if (hookType == 2)
        {
            UnhookWindowsHookEx((HHOOK)((DWORD_PTR)hookID));
            result->Success(flutter::EncodableValue("Hook Mouse Uninstalled."));
        }
    }
    else if (method_call.method_name().compare("manageMouseHook") == 0)
    {
        const flutter::EncodableMap &args = std::get<flutter::EncodableMap>(*method_call.arguments());
        int button = std::get<int>(args.at(flutter::EncodableValue("button")));
        std::string method = std::get<std::string>(args.at(flutter::EncodableValue("method")));
        std::string mouseEvent = std::get<std::string>(args.at(flutter::EncodableValue("mouseEvent")));

        if (mouseEvent == "hold")
            mouseControlButtons[button] = method == "add" ? 1 : 0;
        else
            mouseWatchButtons[button] = method == "add" ? 1 : 0;
        result->Success(flutter::EncodableValue(true));
    }
    else
    {
        result->NotImplemented();
    }
}
