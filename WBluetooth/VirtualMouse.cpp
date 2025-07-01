#include "VirtualMouse.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std::chrono_literals;

std::string VirtualMouse::StatusToString(GattServiceProviderAdvertisementStatus status)
{
    switch (status)
    {
    case GattServiceProviderAdvertisementStatus::Created: return "Created";
    case GattServiceProviderAdvertisementStatus::Stopped: return "Stopped";
    case GattServiceProviderAdvertisementStatus::Started: return "Started";
    case GattServiceProviderAdvertisementStatus::Aborted: return "Aborted";
    default: return "Unknown";
    }
}

std::string VirtualMouse::BufferToString(winrt::Windows::Storage::Streams::IBuffer const& buffer)
{
    std::vector<uint8_t> data(buffer.Length());
    DataReader::FromBuffer(buffer).ReadBytes(data);
    return ByteArrayToString(data);
}

std::string VirtualMouse::ByteArrayToString(const std::vector<uint8_t>& bytes)
{
    std::ostringstream oss;
    for (size_t i = 0; i < bytes.size(); ++i)
    {
        if (i > 0)
            oss << ' ';
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
            << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

void VirtualMouse::InitCharacteristicParameters()
{
    m_hidInputReportParameters.CharacteristicProperties(GattCharacteristicProperties::Read | GattCharacteristicProperties::Notify);
    m_hidInputReportParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);

    std::vector<uint8_t> mouseReportRef{ 
        0x02, // Report ID: 2
        0x01  // Report Type: Input
    };  
    m_hidMouseReportReferenceParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);
    m_hidMouseReportReferenceParameters.StaticValue(CryptographicBuffer::CreateFromByteArray(mouseReportRef));

    std::vector<uint8_t> reportMap = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x02,        //   Report ID (2)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x02,        //     Usage Maximum (0x02)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x75, 0x01,        //     Report Size (1)
            0x95, 0x02,        //     Report Count (2)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x06,        //     Report Count (6)
            0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x09, 0x38,        //     Usage (Wheel)
            0x15, 0x81,        //     Logical Minimum (-127)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x03,        //     Report Count (3)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0xC0,              // End Collection
    };
    m_hidReportMapParameters.CharacteristicProperties(GattCharacteristicProperties::Read);
    m_hidReportMapParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);
    m_hidReportMapParameters.StaticValue(CryptographicBuffer::CreateFromByteArray(reportMap));

    std::vector<uint8_t> hidInfo{ 
        0x11, 0x01, // HID Version: 1101
        0x00,       // Country Code: 0
        0x01        // Not Normally Connectable, Remote Wake supported
    };
    m_hidInformationParameters.CharacteristicProperties(GattCharacteristicProperties::Read);
    m_hidInformationParameters.ReadProtectionLevel(GattProtectionLevel::Plain);
    m_hidInformationParameters.StaticValue(CryptographicBuffer::CreateFromByteArray(hidInfo));

    m_hidControlPointParameters.CharacteristicProperties(GattCharacteristicProperties::WriteWithoutResponse);
    m_hidControlPointParameters.WriteProtectionLevel(GattProtectionLevel::Plain);
}

bool VirtualMouse::Initialize()
{
    InitCharacteristicParameters();
    auto op = CreateHidService();
    op.get();
    return m_initializationFinished;
}

IAsyncAction VirtualMouse::CreateHidService()
{
    // HID service.
    auto hidServiceProviderCreationResult = co_await GattServiceProvider::CreateAsync(GattServiceUuids::HumanInterfaceDevice());
    if (hidServiceProviderCreationResult.Error() != BluetoothError::Success)
    {
        co_return;
    }

    m_hidServiceProvider = hidServiceProviderCreationResult.ServiceProvider();
    m_hidService = m_hidServiceProvider.Service();

    // HID mouse Report characteristic.
    auto hidMouseReportCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::Report(), m_hidInputReportParameters);
    m_hidMouseReport = hidMouseReportCharacteristicCreationResult.Characteristic();
    m_hidMouseReport.SubscribedClientsChanged({ this, &VirtualMouse::HidMouseReport_SubscribedClientsChanged });

    // HID mouse Report Reference descriptor.
    auto hidMouseReportReferenceCreationResult = co_await m_hidMouseReport.CreateDescriptorAsync(
        BluetoothUuidHelper::FromShortId(m_hidReportReferenceDescriptorShortUuid), m_hidMouseReportReferenceParameters);
    m_hidMouseReportReference = hidMouseReportReferenceCreationResult.Descriptor();

    // HID Report Map characteristic.
    auto hidReportMapCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::ReportMap(), m_hidReportMapParameters);
    m_hidReportMap = hidReportMapCharacteristicCreationResult.Characteristic();

    // HID Information characteristic.
    auto hidInformationCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::HidInformation(), m_hidInformationParameters);
    m_hidInformation = hidInformationCharacteristicCreationResult.Characteristic();

    // HID Control Point characteristic.
    auto hidControlPointCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::HidControlPoint(), m_hidControlPointParameters);
    m_hidControlPoint = hidControlPointCharacteristicCreationResult.Characteristic();
    m_hidControlPoint.WriteRequested({ this, &VirtualMouse::HidControlPoint_WriteRequested });

    m_hidServiceProvider.AdvertisementStatusChanged({ this, &VirtualMouse::HidServiceProvider_AdvertisementStatusChanged });

    std::scoped_lock lock(m_mutex);
    m_initializationFinished = true;
}

void VirtualMouse::Enable()
{
    PublishService();
}

void VirtualMouse::Disable()
{
    UnpublishService();
}

void VirtualMouse::PublishService()
{
    GattServiceProviderAdvertisingParameters advParams;
    advParams.IsConnectable(true);
    advParams.IsDiscoverable(true);
    m_hidServiceProvider.StartAdvertising(advParams);
}

void VirtualMouse::UnpublishService()
{
    try
    {
        auto status = m_hidServiceProvider.AdvertisementStatus();
        if (status == GattServiceProviderAdvertisementStatus::Started ||
            status == GattServiceProviderAdvertisementStatus::Aborted)
        {
            m_hidServiceProvider.StopAdvertising();
            if (m_clientChangedHandler)
                m_clientChangedHandler(nullptr);
        }
    }
    catch (...)
    {
        std::cerr << "Failed to stop advertising" << std::endl;
    }
}

void VirtualMouse::Move(int dx, int dy, int wheel)
{
    SendMouseState(m_lastLeftDown, m_lastRightDown, dx, dy, wheel).get();
    std::this_thread::sleep_for(10ms);
}

void VirtualMouse::Press()
{
    SendMouseState(true, false, 0, 0, 0).get();
}

void VirtualMouse::Release()
{
    SendMouseState(false, false, 0, 0, 0).get();
}

void VirtualMouse::Click()
{
    SendMouseState(true, false, 0, 0, 0).get();
    std::this_thread::sleep_for(40ms);
    SendMouseState(false, false, 0, 0, 0).get();
}

void VirtualMouse::SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler)
{
    m_clientChangedHandler = std::move(handler);
}

void VirtualMouse::HidMouseReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const&)
{
    if (m_clientChangedHandler)
        m_clientChangedHandler(sender.SubscribedClients());
}

void VirtualMouse::HidControlPoint_WriteRequested(GattLocalCharacteristic const&, GattWriteRequestedEventArgs const& args)
{
    auto deferral = args.GetDeferral();
    auto request = args.GetRequestAsync().get();
    std::cout << "VirtualMouse Control Point Write: " << BufferToString(request.Value()) << std::endl;
    deferral.Complete();
}

void VirtualMouse::HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const&, GattServiceProviderAdvertisementStatusChangedEventArgs const& args)
{
    std::cout << "VirtualMouse Advertisement status: " << StatusToString(args.Status()) << std::endl;
}

IAsyncAction VirtualMouse::SendMouseState(bool leftDown, bool rightDown, int mx, int my, int wheel)
{
    if (!m_initializationFinished || m_hidMouseReport.SubscribedClients().Size() == 0)
        co_return;

    std::vector<uint8_t> report(4);
    report[0] = (leftDown ? 0x01 : 0x00) | (rightDown ? 0x02 : 0x00);
    report[1] = static_cast<uint8_t>(static_cast<int8_t>(mx));
    report[2] = static_cast<uint8_t>(static_cast<int8_t>(my));
    report[3] = static_cast<uint8_t>(static_cast<int8_t>(wheel));

    m_lastLeftDown = leftDown;
    m_lastRightDown = rightDown;

    co_await m_hidMouseReport.NotifyValueAsync(CryptographicBuffer::CreateFromByteArray(report));
}
