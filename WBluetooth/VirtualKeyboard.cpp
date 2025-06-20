#include "VirtualKeyboard.h"
#include "HidHelper.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

std::string VirtualKeyboard::StatusToString(GattServiceProviderAdvertisementStatus status)
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

std::string VirtualKeyboard::BufferToString(IBuffer const& buffer)
{
    std::vector<uint8_t> data(buffer.Length());
    DataReader::FromBuffer(buffer).ReadBytes(data);
    return ByteArrayToString(data);
}

std::string VirtualKeyboard::ByteArrayToString(const std::vector<uint8_t>& bytes)
{
    std::ostringstream oss;
    for (size_t i = 0; i < bytes.size(); ++i)
    {
        if (i > 0) oss << ' ';
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

void VirtualKeyboard::InitCharacteristicParameters()
{

    m_hidInputReportParameters.CharacteristicProperties(GattCharacteristicProperties::Read | GattCharacteristicProperties::Notify);
    m_hidInputReportParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);

    std::vector<uint8_t> reportRef = { 
        0x01, // Report ID: 1
        0x01  // Report Type: Input
    };
    m_hidKeyboardReportReferenceParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);
    m_hidKeyboardReportReferenceParameters.StaticValue(CryptographicBuffer::CreateFromByteArray(reportRef));

    std::vector<uint8_t> reportMap = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x06,        // Usage (Keyboard)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x01,        //   Report ID
            0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
            0x19, 0xE0,        //   Usage Minimum (0xE0)
            0x29, 0xE7,        //   Usage Maximum (0xE7)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x95, 0x08,        //   Report Count (8)
            0x75, 0x01,        //   Report Size (1)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x01,        //   Report Count (1)
            0x75, 0x08,        //   Report Size (8)
            0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
            0x19, 0x00,        //   Usage Minimum (0x00)
            0x2a, 0xff, 0x00,  //   Usage Maximum (255)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xff, 0x00,  //   Logical Maximum (255)
            0x95, 0x06,        //   Report Count (6)
            0x75, 0x08,        //   Report Size (8)
            0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              // End Collection
    };
    m_hidReportMapParameters.CharacteristicProperties(GattCharacteristicProperties::Read);
    m_hidReportMapParameters.ReadProtectionLevel(GattProtectionLevel::EncryptionRequired);
    m_hidReportMapParameters.StaticValue(CryptographicBuffer::CreateFromByteArray(reportMap));

    std::vector<uint8_t> hidInfo = { 
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

bool VirtualKeyboard::Initialize()
{
    InitCharacteristicParameters();
    auto op = CreateHidService();
    op.get();
    return m_initializationFinished;
}

void VirtualKeyboard::Enable()
{
    PublishService();
}

void VirtualKeyboard::Disable()
{
    UnpublishService();
}

void VirtualKeyboard::PressKey(uint32_t scanCode)
{
    ChangeKeyStateAsync(true, HidHelper::GetHidUsageFromPs2Set1(scanCode)).get();
}

void VirtualKeyboard::ReleaseKey(uint32_t scanCode)
{
    ChangeKeyStateAsync(false, HidHelper::GetHidUsageFromPs2Set1(scanCode)).get();
}

void VirtualKeyboard::DirectSendReport(const std::vector<uint8_t>& reportValue)
{
    if (reportValue.size() == m_sizeOfKeyboardReportDataInBytes)
        m_hidKeyboardReport.NotifyValueAsync(CryptographicBuffer::CreateFromByteArray(reportValue)).get();
}

IAsyncAction VirtualKeyboard::CreateHidService()
{
    // HID service.
    auto hidServiceProviderCreationResult = co_await GattServiceProvider::CreateAsync(GattServiceUuids::HumanInterfaceDevice());
    if (hidServiceProviderCreationResult.Error() != BluetoothError::Success)
        co_return;

    m_hidServiceProvider = hidServiceProviderCreationResult.ServiceProvider();
    m_hidService = m_hidServiceProvider.Service();

    // HID keyboard Report characteristic.
    auto hidKeyboardReportCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::Report(), m_hidInputReportParameters);
    m_hidKeyboardReport = hidKeyboardReportCharacteristicCreationResult.Characteristic();
    m_hidKeyboardReport.SubscribedClientsChanged({ this, &VirtualKeyboard::HidKeyboardReport_SubscribedClientsChanged });

    // HID keyboard Report Reference descriptor.
    auto hidKeyboardReportReferenceCreationResult = co_await m_hidKeyboardReport.CreateDescriptorAsync(BluetoothUuidHelper::FromShortId(m_hidReportReferenceDescriptorShortUuid), m_hidKeyboardReportReferenceParameters);
    m_hidKeyboardReportReference = hidKeyboardReportReferenceCreationResult.Descriptor();

    // HID Report Map characteristic.
    auto hidReportMapCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::ReportMap(), m_hidReportMapParameters);
    m_hidReportMap = hidReportMapCharacteristicCreationResult.Characteristic();

    // HID Information characteristic.
    auto hidInformationCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::HidInformation(), m_hidInformationParameters);
    m_hidInformation = hidInformationCharacteristicCreationResult.Characteristic();

    // HID Control Point characteristic.
    auto hidControlPointCharacteristicCreationResult = co_await m_hidService.CreateCharacteristicAsync(GattCharacteristicUuids::HidControlPoint(), m_hidControlPointParameters);
    m_hidControlPoint = hidControlPointCharacteristicCreationResult.Characteristic();
    m_hidControlPoint.WriteRequested({ this, &VirtualKeyboard::HidControlPoint_WriteRequested });

    m_hidServiceProvider.AdvertisementStatusChanged({ this, &VirtualKeyboard::HidServiceProvider_AdvertisementStatusChanged });

    std::scoped_lock lock(m_mutex);
    m_initializationFinished = true;
}

void VirtualKeyboard::PublishService()
{
    GattServiceProviderAdvertisingParameters advParams;
    advParams.IsConnectable(true);
    advParams.IsDiscoverable(true);
    m_hidServiceProvider.StartAdvertising(advParams);
}

void VirtualKeyboard::UnpublishService()
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
    catch (...) {
        std::cerr << "Failed to stop advertising" << std::endl;
    }
}

IAsyncAction VirtualKeyboard::ChangeKeyStateAsync(bool isPress, uint8_t usage)
{
    if (!m_initializationFinished || m_hidKeyboardReport.SubscribedClients().Size() == 0)
        co_return;

    if (isPress)
    {
        if (HidHelper::IsModifierKey(usage)) 
            m_currentlyDepressedModifierKeys.insert(usage);
        else 
            m_currentlyDepressedKeys.insert(usage);
    }
    else
    {
        if (HidHelper::IsModifierKey(usage)) 
            m_currentlyDepressedModifierKeys.erase(usage);
        else 
            m_currentlyDepressedKeys.erase(usage);
    }

    std::vector<uint8_t> report(m_sizeOfKeyboardReportDataInBytes, 0);
    for (auto mod : m_currentlyDepressedModifierKeys)
        report[0] |= HidHelper::GetFlagOfModifierKey(mod);

    size_t idx = 2;
    for (auto key : m_currentlyDepressedKeys)
    {
        if (idx >= report.size()) 
            break;
        report[idx++] = key;
    }

    m_lastSentKeyboardReportValue = report;
    co_await m_hidKeyboardReport.NotifyValueAsync(CryptographicBuffer::CreateFromByteArray(report));
}

void VirtualKeyboard::SetSubscribedHidClientsChangedHandler(SubscribedHidClientsChangedHandler handler)
{
    m_clientChangedHandler = std::move(handler);
}

void VirtualKeyboard::HidKeyboardReport_SubscribedClientsChanged(GattLocalCharacteristic const& sender, IInspectable const&)
{
    if (m_clientChangedHandler) 
        m_clientChangedHandler(sender.SubscribedClients());
}

void VirtualKeyboard::HidControlPoint_WriteRequested(GattLocalCharacteristic const&, GattWriteRequestedEventArgs const& args)
{
    auto deferral = args.GetDeferral();
    auto request = args.GetRequestAsync().get();
    std::cout << "VirtualKeyboard ControlPoint Write: " << BufferToString(request.Value()) << std::endl;
    deferral.Complete();
}

void VirtualKeyboard::HidServiceProvider_AdvertisementStatusChanged(GattServiceProvider const&, GattServiceProviderAdvertisementStatusChangedEventArgs const& args)
{
    std::cout << "VirtualKeyboard Advertisement status: " << StatusToString(args.Status()) << std::endl;
}