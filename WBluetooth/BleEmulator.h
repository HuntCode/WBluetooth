#ifndef BLE_EMULATOR_H
#define BLE_EMULATOR_H

#ifdef BLEEMULATOR_EXPORTS
#define BLEEMULATOR_API __declspec(dllexport)  // 导出类和函数
#else
#define BLEEMULATOR_API __declspec(dllimport)  // 导入类和函数
#endif

// 前向声明实现类
class BleEmulatorImpl;
class VirtualMouse;
class VirtualKeyboard;

class BLEEMULATOR_API BleEmulator {
public:
    BleEmulator();  // 构造函数
    ~BleEmulator(); // 析构函数

    void Initialize();
    void Test();

    VirtualMouse* Mouse();
    VirtualKeyboard* Keyboard();

private:
    BleEmulatorImpl* pImpl;  // 使用裸指针来避免导出 STL 类型
};

#endif // BLE_EMULATOR_H
