#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <bluetoothapis.h>
#include <initguid.h> // Required for DEFINE_GUID definition
#include <iomanip>    // For std::setw and std::setfill

// Define the Handsfree Telephony Service GUID
DEFINE_GUID(GUID_HANDSFREE_TELEPHONY_SERVICE, 0x0000111E, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

class BlueToothDevices
{
public:
    struct DeviceInfo
    {
        BLUETOOTH_DEVICE_INFO deviceInfo;
        std::wstring name;
        std::wstring address;
        ULONG deviceClass;
        bool authenticated;
        bool connected;
        bool handsfreeTelephonySupported;
    };

    std::vector<DeviceInfo> getBluetoothDevices()
    {
        BLUETOOTH_FIND_RADIO_PARAMS btfrp = {sizeof(btfrp)};
        HANDLE radio = NULL;
        HBLUETOOTH_RADIO_FIND findRadio = BluetoothFindFirstRadio(&btfrp, &radio);
        std::vector<DeviceInfo> devices;

        if (findRadio == NULL)
        {
            std::cerr << "No Bluetooth radios found." << std::endl;
            return devices;
        }

        do
        {
            BLUETOOTH_RADIO_INFO radioInfo = {sizeof(radioInfo)};
            if (BluetoothGetRadioInfo(radio, &radioInfo) == ERROR_SUCCESS)
            {
                BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
                    sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
                    1, // returnAuthenticated
                    0,
                    0, // returnUnknown
                    1, // returnConnected
                    1, // issueInquiry
                    1, // timeoutMultiplier, adjust based on how much time you want to spend
                    radio};

                BLUETOOTH_DEVICE_INFO deviceInfo = {sizeof(BLUETOOTH_DEVICE_INFO)};
                deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
                HBLUETOOTH_DEVICE_FIND findDevice = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

                if (findDevice != NULL)
                {
                    do
                    {
                        DeviceInfo info;
                        info.deviceInfo = deviceInfo;
                        info.name = deviceInfo.szName;
                        info.address = formatBluetoothAddress(deviceInfo.Address);
                        info.deviceClass = deviceInfo.ulClassofDevice;
                        info.authenticated = deviceInfo.fAuthenticated;
                        info.connected = deviceInfo.fConnected;
                        info.handsfreeTelephonySupported = checkHandsfreeTelephonySupport(deviceInfo);

                        devices.push_back(info);
                    } while (BluetoothFindNextDevice(findDevice, &deviceInfo));
                    BluetoothFindDeviceClose(findDevice);
                }
            }
            CloseHandle(radio);
        } while (BluetoothFindNextRadio(findRadio, &radio));

        BluetoothFindRadioClose(findRadio);
        return devices;
    }

    bool checkHandsfreeTelephonySupport(const BLUETOOTH_DEVICE_INFO &deviceInfo)
    {
        GUID serviceGUIDs[16]; // Adjust size according to expected number of services
        DWORD numServices = 16;
        if (BluetoothEnumerateInstalledServices(NULL, &deviceInfo, &numServices, serviceGUIDs) == ERROR_SUCCESS)
        {
            for (DWORD i = 0; i < numServices; ++i)
            {
                if (IsEqualGUID(serviceGUIDs[i], GUID_HANDSFREE_TELEPHONY_SERVICE))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool setHandsfreeTelephonyServiceState(const BlueToothDevices::DeviceInfo &device, bool enable)
    {
        HANDLE radioHandle = NULL;
        DWORD result;
        bool serviceModified = false;

        // Iterate through all available radios
        BLUETOOTH_FIND_RADIO_PARAMS btfrp = {sizeof(btfrp)};
        HBLUETOOTH_RADIO_FIND radioFind = BluetoothFindFirstRadio(&btfrp, &radioHandle);
        while (radioFind != NULL && !serviceModified)
        {
            // Check if this is the correct radio for the device
            if (isCorrectRadio(radioHandle, device))
            {
                // Try to set the service state
                DWORD serviceState = (enable ? BLUETOOTH_SERVICE_ENABLE : BLUETOOTH_SERVICE_DISABLE);
                result = BluetoothSetServiceState(radioHandle, &device.deviceInfo, &GUID_HANDSFREE_TELEPHONY_SERVICE, serviceState);
                if (result == ERROR_SUCCESS)
                {
                    serviceModified = true;
                }
                CloseHandle(radioHandle); // Close the current radio handle
            }

            if (!BluetoothFindNextRadio(radioFind, &radioHandle))
            {
                break;
            }
        }
        BluetoothFindRadioClose(radioFind); // Close the radio finder

        return serviceModified;
    }

    bool isCorrectRadio(HANDLE radioHandle, const BlueToothDevices::DeviceInfo &device)
    {
        BLUETOOTH_RADIO_INFO radioInfo = {0};
        radioInfo.dwSize = sizeof(radioInfo);
        DWORD result;

        // Get radio info
        result = BluetoothGetRadioInfo(radioHandle, &radioInfo);
        if (result != ERROR_SUCCESS)
        {
            std::wcerr << L"Failed to get radio info." << std::endl;
            return false;
        }

        // Attempt to find the device on this radio
        BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
            sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
            1, // returnAuthenticated
            0,
            0, // returnUnknown
            1, // returnConnected
            1, // issueInquiry
            2, // timeoutMultiplier, adjust based on how much time you want to spend
            radioHandle};

        BLUETOOTH_DEVICE_INFO deviceInfo = device.deviceInfo;
        deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
        HBLUETOOTH_DEVICE_FIND deviceFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
        if (deviceFind == NULL)
        {
            return false; // No devices found on this radio
        }

        bool found = false;
        do
        {
            if (device.address == formatBluetoothAddress(deviceInfo.Address))
            {
                found = true;
                break;
            }
        } while (BluetoothFindNextDevice(deviceFind, &deviceInfo));

        BluetoothFindDeviceClose(deviceFind);
        return found;
    }
    std::wstring formatBluetoothAddress(BLUETOOTH_ADDRESS address)
    { 
        std::wstringstream ws;
        ws << std::hex << std::uppercase << std::setfill(L'0');
        for (int i = 0; i < 6; i++)
        { // Process from byte 0 to 5 for consistency
            ws << std::setw(2) << static_cast<int>(address.rgBytes[i]);
            if (i < 5) // Add colon after each byte except the last one
                ws << L":";
        }
        return ws.str();
    }
};

int main()
{
    BlueToothDevices btObj;
    auto devices = btObj.getBluetoothDevices();
    if (devices.empty())
    {
        std::cout << "No Bluetooth devices found." << std::endl;
    }
    else
    {
        for (const auto &device : devices)
        {
            std::wcout << L"Found Device: " << std::setw(27) << std::left << device.name << L" | "
                       << L"Address: " << std::setw(17) << std::left << device.address << L" | "
                       << L"Handsfree Telephony Supported: " << (device.handsfreeTelephonySupported ? L"Yes" : L"No") << std::endl;
        }
    }

    for (const auto &device : devices)
    {
        if (device.connected && device.handsfreeTelephonySupported)
        {
            std::wcout << L"Attempting to disconnect the Telephony for:" << device.name << std::endl;
            if (btObj.setHandsfreeTelephonyServiceState(device, false))
            {
                std::wcout << L"Handsfree Telephony has been disabled for: " << device.name << std::endl;
            }
            else
            {
                std::wcout << L"Failed to disable Handsfree Telephony for: " << device.name << std::endl;
            }
        }

    }

    return 0;
}
