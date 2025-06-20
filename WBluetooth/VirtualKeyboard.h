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
public:
    VirtualKeyboard() = default;

private:
    GattLocalCharacteristicParameters m_hidInputReportParameters{ GattLocalCharacteristicParameters() };

    static constexpr uint16_t m_hidReportReferenceDescriptorShortUuid = 0x2908;
    GattLocalDescriptorParameters m_hidKeyboardReportReferenceParameters{ GattLocalDescriptorParameters()};

    //HID报告
    GattLocalCharacteristicParameters m_hidReportMapParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidInformationParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidControlPointParameters{ GattLocalCharacteristicParameters()};
    //GattLocalCharacteristicParameters m_batteryLevelParameters
    static constexpr uint32_t m_sizeOfKeyboardReportDataInBytes = 0x8;

    // BLE GATT 服务结构
    GattServiceProvider m_hidServiceProvider{ nullptr };
    GattLocalService m_hidService{ nullptr };

    GattLocalCharacteristic m_hidKeyboardReport{ nullptr };
    GattLocalDescriptor m_hidKeyboardReportReference{ nullptr };
    GattLocalCharacteristic m_hidReportMap{ nullptr };
    GattLocalCharacteristic m_hidInformation{ nullptr };
    GattLocalCharacteristic m_hidControlPoint{ nullptr };

    // 状态控制
    std::mutex m_mutex;
    bool m_initializationFinished = false;

    std::unordered_set<uint8_t> m_currentlyDepressedModifierKeys;
    std::unordered_set<uint8_t> m_currentlyDepressedKeys;
    std::vector<uint8_t> m_lastSentKeyboardReportValue = std::vector<uint8_t>(m_sizeOfKeyboardReportDataInBytes);

    using SubscribedHidClientsChangedHandler = std::function<void(IVectorView<GattSubscribedClient>)>;
    SubscribedHidClientsChangedHandler m_clientChangedHandler{ nullptr };

    // 工具函数
    static std::string StatusToString(GattServiceProviderAdvertisementStatus status);
    static std::string BufferToString(IBuffer const& buffer);
    static std::string ByteArrayToString(const std::vector<uint8_t>& bytes);

public:
    // 初始化与控制
    bool Initialize();
    void Enable();
    void Disable();

    // 操作接口
    void PressKey(uint32_t ps2Set1ScanCode);
    void ReleaseKey(uint32_t ps2Set1ScanCode);
    void DirectSendReport(const std::vector<uint8_t>& reportValue);

    // 回调设置
    void SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler);

private:  
    // 内部异步构建与逻辑
    void InitCharacteristicParameters();
    IAsyncAction CreateHidService();
    void PublishService();
    void UnpublishService();
    
    // GATT 回调事件处理
    void HidKeyboardReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const& args);
    void HidControlPoint_WriteRequested(GattLocalCharacteristic const& sender, GattWriteRequestedEventArgs const& args);
    void HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const& sender, GattServiceProviderAdvertisementStatusChangedEventArgs const& args);

    IAsyncAction ChangeKeyStateAsync(bool isPress, uint8_t hidUsage);
};

#endif // VIRTUAL_KEYBOARD_H