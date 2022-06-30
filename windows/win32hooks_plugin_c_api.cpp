#include "include/win32hooks/win32hooks_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "win32hooks_plugin.h"

void Win32hooksPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
    Win32hooksPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
