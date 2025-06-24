#include "BleEmulator.h"
#include "VirtualKeyboard.h"
#include "VirtualMouse.h"
#include <string>
#include <memory>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>

class BleEmulatorImpl {
public:
    std::unique_ptr<VirtualKeyboard> m_virtualKeyboard;
    std::unique_ptr<VirtualMouse> m_virtualMouse;
    std::string m_deviceName;
    std::atomic<bool> m_running{ false };


    void InitializeVirtualDevices() {
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

    void HandleKeyboardSubscribedClientsChanged(IVectorView<GattSubscribedClient> const& clients) {
        if (clients.Size() > 0) {
            auto device = BluetoothLEDevice::FromIdAsync(clients.GetAt(0).Session().DeviceId().Id()).get();
            std::wcout << L"keyboard-subscribed: " << device.Name().c_str() << std::endl;
            m_deviceName = winrt::to_string(device.Name());
        }
    }

    void HandleMouseSubscribedClientsChanged(IVectorView<GattSubscribedClient> const& clients) {
        if (clients.Size() > 0) {
            auto device = BluetoothLEDevice::FromIdAsync(clients.GetAt(0).Session().DeviceId().Id()).get();
            std::wcout << L"mouse-subscribed: " << device.Name().c_str() << std::endl;
            m_deviceName = winrt::to_string(device.Name());
        }
    }
};

BleEmulator::BleEmulator()
    : pImpl(new BleEmulatorImpl()) {
}

BleEmulator::~BleEmulator() {
    delete pImpl;
}

void BleEmulator::Initialize() {
    pImpl->InitializeVirtualDevices();
    pImpl->m_running = true;
}

void BleEmulator::Test() {
    using namespace std::chrono_literals;

    for (int i = 0; i < 10; i++) {
        pImpl->m_virtualMouse->Move(0, 10, 0);
        std::this_thread::sleep_for(300ms);
    }

    for (int i = 0; i < 4; i++) {
        pImpl->m_virtualMouse->Press();
        std::this_thread::sleep_for(300ms);

        pImpl->m_virtualMouse->Move(100, 0, 0);
        std::this_thread::sleep_for(300ms);

        pImpl->m_virtualMouse->Release();
        std::this_thread::sleep_for(300ms);
    }

    for (int i = 0; i < 4; i++) {
        pImpl->m_virtualMouse->Press();
        std::this_thread::sleep_for(300ms);

        pImpl->m_virtualMouse->Move(-100, 0, 0);
        std::this_thread::sleep_for(300ms);

        pImpl->m_virtualMouse->Release();
        std::this_thread::sleep_for(300ms);
    }
}

void BleEmulator::VirtualMouseMove(int dx, int dy, int wheel)
{
	pImpl->m_virtualMouse->Move(dx, dy, wheel);
}

void BleEmulator::VirtualMousePress()
{
	pImpl->m_virtualMouse->Press();
}

void BleEmulator::VirtualMouseRelease()
{
	pImpl->m_virtualMouse->Release();
}

void BleEmulator::VirtualMouseClick()
{
	pImpl->m_virtualMouse->Click();
}

void BleEmulator::VirtualKeyboardPress(int ps2Set1ScanCode)
{
	pImpl->m_virtualKeyboard->PressKey(ps2Set1ScanCode);
}

void BleEmulator::VirtualKeyboardRelease(int ps2Set1ScanCode)
{
    pImpl->m_virtualKeyboard->ReleaseKey(ps2Set1ScanCode);
}
