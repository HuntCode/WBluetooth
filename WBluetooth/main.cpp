#include "BleEmulator.h"
#include <iostream>
#include <thread>

int main()
{
    std::cout << "BLE Emulator starting..." << std::endl;

    // 创建 BLE 模拟器实例
    BleEmulator emulator;

    // 初始化虚拟鼠标和键盘设备
    emulator.Initialize();


    // 启动模拟器服务（包含 NamedPipe 通信）
    //emulator.RunServerLoop();

    // 进入主循环，防止程序退出（你也可以用更优雅的机制，如线程 join 或条件变量）
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
