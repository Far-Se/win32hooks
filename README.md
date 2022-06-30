# win32hooks

A Windows Flutter package that tracks mouse buttons and Window Events

#### Example output:
```sh
flutter: WinEventInfo: EventStruct(event: 10, hWnd: 262318, idObject: 0, idChild: 0, dwEventThread: 11296, dwmsEventTime: 183474890)
flutter: WinEventInfo: EventStruct(event: 11, hWnd: 262318, idObject: 0, idChild: 0, dwEventThread: 11296, dwmsEventTime: 183474968)
flutter: MouseInfo: MouseStruct(button: MouseButtons.Button4, down: true, up: false, type: MouseEvent.control)
flutter: MouseInfo: MouseStruct(button: MouseButtons.Button4, down: false, up: true, type: MouseEvent.control)
```
## Getting Started

This is not fully finished, but it is good if any of you don't want to debug Windows API.


### Simple example:
```dart
//outside of any class:
final winHooks = WinHooks(); //global.
```

```dart
class _MyAppState extends State<MyApp> with WinHookEventListener {

  @override
  void initState() {
    super.initState();
    initialState();
  }

  Future<void> initialState() async {
    if (!mounted) return;
    // For onMouseInfoReceived and onWinEventInfoReceived
    winHooks.addListener(this);

    /// Cleans cpp memory for mouse buttons and dart event variables
    /// It's for when you reloads/hotreload your app and change buttons in meantime.
    winHooks.cleanHooks();

    /// parameters are self-explanatory. 
    winHooks.setWinEventParameters(minEvent: WinHookEvent.SYSTEM_MOVESIZESTART, maxEvent: WinHookEvent.SYSTEM_MOVESIZEEND, reinstallHooks: false);

    ///  [MouseEvent.control] is for blocking the propagation of the button
    ///  [MouseEvent.watch] is for just receiving callback when the button is pressed.
    winHooks.addMouseHook(button: MouseButtons.Button4, mouseEvent: MouseEvent.control, reinstallHooks: false);

    /// Initiate cpp call.
    winHooks.installWinHook();
    setState(() {});
  }

    /**
    MouseStruct {
        MouseButtons button;
        bool down = false;
        bool up = false;
        MouseEvent type;
    }
    MouseButtons {
        Left,
        Right,
        Middle,
        ScrollUp,
        ScrollDown,
        Button3,
        Button4,
    }
    enum MouseEvent { control, watch }
    **/
  @override
  void onMouseInfoReceived(MouseStruct mouse) {
    print("MouseInfo: ${mouse.toString()}");
  }

    /**
        int event = 0;
        int hWnd = 0;
        int idObject = 0;
        int idChild = 0;
        int dwEventThread = 0;
        int dwmsEventTime = 0;
    **/

  @override
  void onWinEventInfoReceived(WinEventStruct event) {
    print("WinEventInfo: ${event.toString()}");
  }
}
```