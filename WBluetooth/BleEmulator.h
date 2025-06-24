#ifndef BLE_EMULATOR_H
#define BLE_EMULATOR_H

#ifdef BLEEMULATOR_EXPORTS
#define BLEEMULATOR_API __declspec(dllexport)
#else
#define BLEEMULATOR_API __declspec(dllimport)
#endif

class BleEmulatorImpl;
class VirtualMouse;
class VirtualKeyboard;

class BLEEMULATOR_API BleEmulator {
public:
    BleEmulator();
    ~BleEmulator();

    void Initialize();
    void Test();

    void VirtualMouseMove(int dx, int dy, int wheel = 0);
    void VirtualMousePress();
    void VirtualMouseRelease();
    void VirtualMouseClick();

    void VirtualKeyboardPress(int ps2Set1ScanCode);
    void VirtualKeyboardRelease(int ps2Set1ScanCode);

private:
    BleEmulatorImpl* pImpl;
};

#endif // BLE_EMULATOR_H
