#ifndef BLE_EMULATOR_H
#define BLE_EMULATOR_H

#include "VirtualKeyboard.h"
#include "VirtualMouse.h"
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>

class BleEmulator {
public:
    BleEmulator();
    ~BleEmulator();

    void Initialize();

    VirtualMouse* Mouse();
    VirtualKeyboard* Keyboard();

    void Test();
private:
    void InitializeVirtualDevices();
    void HandleKeyboardSubscribedClientsChanged(winrt::Windows::Foundation::Collections::IVectorView<
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSubscribedClient> const& clients);
    void HandleMouseSubscribedClientsChanged(winrt::Windows::Foundation::Collections::IVectorView<
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSubscribedClient> const& clients);

    std::unique_ptr<VirtualKeyboard> m_virtualKeyboard;
    std::unique_ptr<VirtualMouse> m_virtualMouse;

    std::string m_deviceName;

    std::atomic<bool> m_running{ false };
};

#endif // BLE_EMULATOR_H