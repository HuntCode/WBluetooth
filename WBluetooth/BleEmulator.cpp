#include "BleEmulator.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

BleEmulator::BleEmulator() {}

BleEmulator::~BleEmulator() {
    m_running = false;
}

void BleEmulator::Initialize() {
    InitializeVirtualDevices();
    m_running = true;
}

void BleEmulator::InitializeVirtualDevices() {
    m_virtualKeyboard = std::make_unique<VirtualKeyboard>();
    m_virtualKeyboard->SetSubscribedHidClientsChangedHandler(
        [this](auto const& clients) { HandleKeyboardSubscribedClientsChanged(clients); });
    m_virtualKeyboard->Initialize();
    m_virtualKeyboard->Enable();

    m_virtualMouse = std::make_unique<VirtualMouse>();
    m_virtualMouse->SetSubscribedHidClientsChangedHandler(
        [this](auto const& clients) { HandleMouseSubscribedClientsChanged(clients); });
    m_virtualMouse->Initialize();
    m_virtualMouse->Enable();
}

void BleEmulator::HandleKeyboardSubscribedClientsChanged(IVectorView<GattSubscribedClient> const& clients) {
    if (clients.Size() > 0) {
        auto device = BluetoothLEDevice::FromIdAsync(clients.GetAt(0).Session().DeviceId().Id()).get();
        std::wcout << L"keyboard-subscribed: " << device.Name().c_str() << std::endl;
        m_deviceName = winrt::to_string(device.Name());
    }
}

void BleEmulator::HandleMouseSubscribedClientsChanged(IVectorView<GattSubscribedClient> const& clients) {
    if (clients.Size() > 0) {
        auto device = BluetoothLEDevice::FromIdAsync(clients.GetAt(0).Session().DeviceId().Id()).get();
        std::wcout << L"mouse-subscribed: " << device.Name().c_str() << std::endl;
        m_deviceName = winrt::to_string(device.Name());
    }

    //Test();
}

VirtualMouse* BleEmulator::Mouse()
{
    return m_virtualMouse.get();
}

VirtualKeyboard* BleEmulator::Keyboard()
{
    return m_virtualKeyboard.get();
}

void BleEmulator::Test()
{
    using namespace std::chrono_literals;

    //std::this_thread::sleep_for(1s);

    //for (int i = 0; i < 10; i++)
    //{
    //    m_virtualKeyboard->PressKey(0x05);  // Example key
    //    std::this_thread::sleep_for(100ms);
    //    m_virtualKeyboard->ReleaseKey(0x05);
    //    std::this_thread::sleep_for(100ms);
    //    m_virtualKeyboard->PressKey(0x1e);  // Example key
    //    std::this_thread::sleep_for(100ms);
    //    m_virtualKeyboard->ReleaseKey(0x1e);
    //    std::this_thread::sleep_for(100ms);
    //}

    //for (int i = 0; i < 10; i++)
    //{
    //    m_virtualMouse->Move(-10, -10, 0);
    //    std::this_thread::sleep_for(300ms);
    //}

    for (int i = 0; i < 10; i++)
    {
        m_virtualMouse->Move(0, 10, 0);
        std::this_thread::sleep_for(300ms);
    }

    for (int i = 0; i < 4; i++)
    {
        m_virtualMouse->Press();
        std::this_thread::sleep_for(300ms);

        m_virtualMouse->Move(100, 0, 0);
        std::this_thread::sleep_for(300ms);

        m_virtualMouse->Release();
        std::this_thread::sleep_for(300ms);
    }

    for (int i = 0; i < 4; i++)
    {
        m_virtualMouse->Press();
        std::this_thread::sleep_for(300ms);

        m_virtualMouse->Move(-100, 0, 0);
        std::this_thread::sleep_for(300ms);

        m_virtualMouse->Release();
        std::this_thread::sleep_for(300ms);
    }
}
