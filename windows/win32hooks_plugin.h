#ifndef FLUTTER_PLUGIN_WIN32HOOKS_PLUGIN_H_
#define FLUTTER_PLUGIN_WIN32HOOKS_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

#include <memory>
#include <sstream>

extern HHOOK g_MouseHook;
extern HWINEVENTHOOK g_EventHook;
class Win32hooksPlugin : public flutter::Plugin
{
public:
    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    Win32hooksPlugin();

    virtual ~Win32hooksPlugin();

    // Disallow copy and assign.
    Win32hooksPlugin(const Win32hooksPlugin &) = delete;
    Win32hooksPlugin &operator=(const Win32hooksPlugin &) = delete;

private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

#endif // FLUTTER_PLUGIN_WIN32HOOKS_PLUGIN_H_
