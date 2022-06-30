import 'package:flutter/material.dart';
import 'package:win32hooks/const.win.dart';
import 'dart:async';

import 'package:win32hooks/win32hooks.dart';

final winHooks = WinHooks();
void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> with WinHookEventListener {
  // final _win32hooksPlugin = WinHooks();

  @override
  void initState() {
    super.initState();
    initialState();
  }

  Future<void> initialState() async {
    if (!mounted) return;
    //winHooks.uninstallWinHook();
    winHooks.addListener(this);
    winHooks.cleanHooks();
    winHooks.setWinEventParameters(minEvent: WinHookEvent.SYSTEM_MOVESIZESTART, maxEvent: WinHookEvent.SYSTEM_MOVESIZEEND, reinstallHooks: false);
    winHooks.addMouseHook(button: MouseButtons.Button4, mouseEvent: MouseEvent.control, reinstallHooks: false);
    winHooks.installWinHook();
    setState(() {});
  }

  @override
  void onMouseInfoReceived(MouseStruct mouse) {
    print("MouseInfo: ${mouse.toString()}");
  }

  @override
  void onWinEventInfoReceived(WinEventStruct event) {
    print("WinEventInfo: ${event.toString()}");
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: Center(
          child: Column(
            children: [
              const Divider(height: 10, thickness: 10, color: Colors.transparent),
              OutlinedButton(
                child: const Text('Install WinHooks'),
                onPressed: () async {
                  await winHooks.installWinHook();
                  setState(() {});
                },
              ),
              const Divider(
                height: 10,
                thickness: 10,
                color: Colors.transparent,
              ),
              OutlinedButton(
                child: const Text('Uninstall Hooks'),
                onPressed: () {
                  winHooks.uninstallWinHook();
                },
              ),
              Text("${winHooks.hookMouseID} : ${winHooks.hookEventID}"),
              const SizedBox(height: 10),
              //insert rich text area
            ],
          ),
        ),
      ),
    );
  }
}
