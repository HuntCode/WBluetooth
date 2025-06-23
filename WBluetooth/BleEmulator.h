#ifndef BLE_EMULATOR_H
#define BLE_EMULATOR_H

#ifdef BLEEMULATOR_EXPORTS
#define BLEEMULATOR_API __declspec(dllexport)  // ������ͺ���
#else
#define BLEEMULATOR_API __declspec(dllimport)  // ������ͺ���
#endif

// ǰ������ʵ����
class BleEmulatorImpl;
class VirtualMouse;
class VirtualKeyboard;

class BLEEMULATOR_API BleEmulator {
public:
    BleEmulator();  // ���캯��
    ~BleEmulator(); // ��������

    void Initialize();
    void Test();

    VirtualMouse* Mouse();
    VirtualKeyboard* Keyboard();

private:
    BleEmulatorImpl* pImpl;  // ʹ����ָ�������⵼�� STL ����
};

#endif // BLE_EMULATOR_H
