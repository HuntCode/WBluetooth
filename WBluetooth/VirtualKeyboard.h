#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <functional>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <string>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Cryptography;

class VirtualKeyboard
{
    enum class FunctionKey : uint8_t {
        F1 = 0x3A,
        F2 = 0x3B,
        F3 = 0x3C,
        F4 = 0x3D,
        F5 = 0x3E,
        F6 = 0x3F,
        F7 = 0x40,
        F8 = 0x41,
        F9 = 0x42,
        F10 = 0x43,
        F11 = 0x44, // not use
        F12 = 0x45  // not use
    };

    struct FunctionKeyMapping {
        FunctionKey key;
        uint16_t consumerCode;
    };

public:
    VirtualKeyboard() = default;

private:
    GattLocalCharacteristicParameters m_hidInputReportParameters{ GattLocalCharacteristicParameters() };
    
	// for consumer control keys
    GattLocalCharacteristicParameters m_hidConsumerReportParameters{ GattLocalCharacteristicParameters() };

    static constexpr uint16_t m_hidReportReferenceDescriptorShortUuid = 0x2908;
    GattLocalDescriptorParameters m_hidKeyboardReportReferenceParameters{ GattLocalDescriptorParameters()};

    // for consumer control keys
    GattLocalDescriptorParameters m_hidConsumerReportReferenceParameters{ GattLocalDescriptorParameters() };

    //HID Report
    GattLocalCharacteristicParameters m_hidReportMapParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidInformationParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidControlPointParameters{ GattLocalCharacteristicParameters()};
    //GattLocalCharacteristicParameters m_batteryLevelParameters
    static constexpr uint32_t m_sizeOfKeyboardReportDataInBytes = 0x8;

	// BLE GATT Service Structure
    GattServiceProvider m_hidServiceProvider{ nullptr };
    GattLocalService m_hidService{ nullptr };

    GattLocalCharacteristic m_hidKeyboardReport{ nullptr };
    GattLocalDescriptor m_hidKeyboardReportReference{ nullptr };

    // for consumer control keys
    GattLocalCharacteristic m_hidConsumerReport{ nullptr };
    GattLocalDescriptor m_hidConsumerReportReference{ nullptr };
    GattLocalCharacteristic m_hidReportMap{ nullptr };
    GattLocalCharacteristic m_hidInformation{ nullptr };
    GattLocalCharacteristic m_hidControlPoint{ nullptr };

	// State Variables
    std::mutex m_mutex;
    bool m_initializationFinished = false;

    std::unordered_set<uint8_t> m_currentlyDepressedModifierKeys;
    std::unordered_set<uint8_t> m_currentlyDepressedKeys;
    std::vector<uint8_t> m_lastSentKeyboardReportValue = std::vector<uint8_t>(m_sizeOfKeyboardReportDataInBytes);

    using SubscribedHidClientsChangedHandler = std::function<void(IVectorView<GattSubscribedClient>)>;
    SubscribedHidClientsChangedHandler m_clientChangedHandler{ nullptr };

	// Utility Functions
    static std::string StatusToString(GattServiceProviderAdvertisementStatus status);
    static std::string BufferToString(IBuffer const& buffer);
    static std::string ByteArrayToString(const std::vector<uint8_t>& bytes);

public:
    bool Initialize();
    void Enable();
    void Disable();

    void PressKey(uint32_t ps2Set1ScanCode);
    void ReleaseKey(uint32_t ps2Set1ScanCode);
    void DirectSendReport(const std::vector<uint8_t>& reportValue);

    void SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler);

	// for function keys
    void SetFunctionKeyBinding(FunctionKey key, uint16_t consumerUsage);
    void ClearFunctionKeyBinding(FunctionKey key);
    void ClearAllFunctionKeyBindings();

private:  
    void InitCharacteristicParameters();
    IAsyncAction CreateHidService();
    void PublishService();
    void UnpublishService();
    
    void HidKeyboardReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const& args);
    void HidControlPoint_WriteRequested(GattLocalCharacteristic const& sender, GattWriteRequestedEventArgs const& args);
    void HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const& sender, GattServiceProviderAdvertisementStatusChangedEventArgs const& args);

    IAsyncAction ChangeKeyStateAsync(bool isPress, uint8_t hidUsage);
    IAsyncAction SendConsumerControlKeyAsync(bool isPress, uint16_t usage);

    void InitFunctionKeyBindings();
    std::vector<FunctionKeyMapping> m_functionKeyBindings;
};

#endif // VIRTUAL_KEYBOARD_H