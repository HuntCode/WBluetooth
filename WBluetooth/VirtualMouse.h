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

    // HID����
    GattLocalCharacteristicParameters m_hidReportMapParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidInformationParameters{ GattLocalCharacteristicParameters()};
    GattLocalCharacteristicParameters m_hidControlPointParameters{ GattLocalCharacteristicParameters()};
    //GattLocalCharacteristicParameters m_batteryLevelParameters
    static constexpr uint32_t m_sizeOfMouseReportDataInBytes = 0x4;

    // BLE ����������
    GattServiceProvider m_hidServiceProvider{ nullptr };
    GattLocalService m_hidService{ nullptr };

    GattLocalCharacteristic m_hidMouseReport{ nullptr };
    GattLocalDescriptor m_hidMouseReportReference{ nullptr };
    GattLocalCharacteristic m_hidReportMap{ nullptr };
    GattLocalCharacteristic m_hidInformation{ nullptr };
    GattLocalCharacteristic m_hidControlPoint{ nullptr };

    // ״̬����
    std::mutex m_mutex;
    bool m_initializationFinished = false;
    bool m_lastLeftDown = false;
    bool m_lastRightDown = false;

    using SubscribedHidClientsChangedHandler = std::function<void(IVectorView<GattSubscribedClient>)>;
    SubscribedHidClientsChangedHandler m_clientChangedHandler{ nullptr };

    // ���ߺ���
    static std::string StatusToString(GattServiceProviderAdvertisementStatus status);
    static std::string BufferToString(IBuffer const& buffer);
    static std::string ByteArrayToString(const std::vector<uint8_t>& bytes);

public:
    // ��ʼ�������
    bool Initialize();
    void Enable();
    void Disable();

    // �����ӿ�
    void Move(int dx, int dy, int wheel = 0);
    void Press();
    void Release();
    void Click();

    // �ص�����
    void SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler);

private:
    // �ڲ�����/�߼�����
    void InitCharacteristicParameters();
    IAsyncAction CreateHidService();
    void PublishService();
    void UnpublishService();

    // GATT �ص��¼�����
    void HidMouseReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const& args);
    void HidControlPoint_WriteRequested(GattLocalCharacteristic const& sender, GattWriteRequestedEventArgs const& args);
    void HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const& sender, GattServiceProviderAdvertisementStatusChangedEventArgs const& args);

    IAsyncAction SendMouseState(bool leftDown, bool rightDown, int mx, int my, int wheel);
};


#endif // VIRTUAL_MOUSE_H

