#ifndef VIRTUAL_MOUSE_H
#define VIRTUAL_MOUSE_H

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <functional>
#include <mutex>
#include <vector>
#include <string>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Cryptography;

class VirtualMouse
{
public:
    VirtualMouse() = default;

private:
    GattLocalCharacteristicParameters m_hidInputReportParameters{ GattLocalCharacteristicParameters()};

    static constexpr uint16_t m_hidReportReferenceDescriptorShortUuid = 0x2908;
    GattLocalDescriptorParameters m_hidMouseReportReferenceParameters{ GattLocalDescriptorParameters()};

    // HID报告
    GattLocalCharacteristicParameters m_hidReportMapParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidInformationParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidControlPointParameters{ GattLocalCharacteristicParameters()};
    //GattLocalCharacteristicParameters m_batteryLevelParameters
    static constexpr uint32_t m_sizeOfMouseReportDataInBytes = 0x4;

    // BLE 服务与特征
    GattServiceProvider m_hidServiceProvider{ nullptr };
    GattLocalService m_hidService{ nullptr };

    GattLocalCharacteristic m_hidMouseReport{ nullptr };
    GattLocalDescriptor m_hidMouseReportReference{ nullptr };
    GattLocalCharacteristic m_hidReportMap{ nullptr };
    GattLocalCharacteristic m_hidInformation{ nullptr };
    GattLocalCharacteristic m_hidControlPoint{ nullptr };

    // 状态变量
    std::mutex m_mutex;
    bool m_initializationFinished = false;
    bool m_lastLeftDown = false;
    bool m_lastRightDown = false;

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
    void Move(int dx, int dy, int wheel = 0);
    void Press();
    void Release();
    void Click();

    // 回调设置
    void SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler);

private:
    // 内部构建/逻辑流程
    void InitCharacteristicParameters();
    IAsyncAction CreateHidService();
    void PublishService();
    void UnpublishService();

    // GATT 回调事件处理
    void HidMouseReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const& args);
    void HidControlPoint_WriteRequested(GattLocalCharacteristic const& sender, GattWriteRequestedEventArgs const& args);
    void HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const& sender, GattServiceProviderAdvertisementStatusChangedEventArgs const& args);

    IAsyncAction SendMouseState(bool leftDown, bool rightDown, int mx, int my, int wheel);
};


#endif // VIRTUAL_MOUSE_H

