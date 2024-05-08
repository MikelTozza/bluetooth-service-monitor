#include "BlueToothDevices.h"
#include <sstream>
#include <iomanip>
#include <iostream>

DEFINE_GUID(GUID_HANDSFREE_TELEPHONY_SERVICE, 0x0000111E, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

std::vector<BlueToothDevices::DeviceInfo> BlueToothDevices::getBluetoothDevices() {
    BLUETOOTH_FIND_RADIO_PARAMS btfrp = {sizeof(btfrp)};
    HANDLE radio = NULL;
    HBLUETOOTH_RADIO_FIND findRadio = BluetoothFindFirstRadio(&btfrp, &radio);
    std::vector<DeviceInfo> devices;

    if (findRadio == NULL) {
        std::cerr << "No Bluetooth radios found." << std::endl;
        return devices;
    }

    do {
        BLUETOOTH_RADIO_INFO radioInfo = {sizeof(radioInfo)};
        if (BluetoothGetRadioInfo(radio, &radioInfo) == ERROR_SUCCESS) {
            BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
                    sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
                    1, // returnAuthenticated
                    0,
                    0, // returnUnknown
                    1, // returnConnected
                    1, // issueInquiry
                    1, // timeoutMultiplier, adjust based on how much time you want to spend
                    radio
            };

            BLUETOOTH_DEVICE_INFO deviceInfo = {sizeof(BLUETOOTH_DEVICE_INFO)};
            deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
            HBLUETOOTH_DEVICE_FIND findDevice = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

            if (findDevice != NULL) {
                do {
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

bool BlueToothDevices::checkHandsfreeTelephonySupport(const BLUETOOTH_DEVICE_INFO& deviceInfo) {
    BLUETOOTH_DEVICE_INFO mutableDeviceInfo = deviceInfo;
    GUID serviceGUIDs[16];
    DWORD numServices = 16;
    if (BluetoothEnumerateInstalledServices(NULL, &mutableDeviceInfo, &numServices, serviceGUIDs) == ERROR_SUCCESS) {
        for (DWORD i = 0; i < numServices; ++i) {
            if (IsEqualGUID(serviceGUIDs[i], GUID_HANDSFREE_TELEPHONY_SERVICE)) {
                return true;
            }
        }
    }
    return false;
}

bool BlueToothDevices::setHandsfreeTelephonyServiceState(const DeviceInfo& device, bool enable) {
    HANDLE radioHandle = NULL;
    DWORD result;
    bool serviceModified = false;

    BLUETOOTH_FIND_RADIO_PARAMS btfrp = {sizeof(btfrp)};
    HBLUETOOTH_RADIO_FIND radioFind = BluetoothFindFirstRadio(&btfrp, &radioHandle);
    if (radioFind == NULL) {
        std::cerr << "No Bluetooth radios found." << std::endl;
        return false;
    }

    while (radioFind != NULL && !serviceModified) {
        if (isCorrectRadio(radioHandle, device)) {
            DWORD serviceState = (enable ? BLUETOOTH_SERVICE_ENABLE : BLUETOOTH_SERVICE_DISABLE);
            GUID mutableGUID = GUID_HANDSFREE_TELEPHONY_SERVICE;

            result = BluetoothSetServiceState(radioHandle, const_cast<BLUETOOTH_DEVICE_INFO*>(&device.deviceInfo), &mutableGUID, serviceState);
            if (result == ERROR_SUCCESS) {
                serviceModified = true;
            }
        }
        CloseHandle(radioHandle);

        if (!BluetoothFindNextRadio(radioFind, &radioHandle)) {
            break;
        }
    }

    BluetoothFindRadioClose(radioFind);
    return serviceModified;
}

bool BlueToothDevices::isCorrectRadio(HANDLE radioHandle, const DeviceInfo& device) {
    BLUETOOTH_RADIO_INFO radioInfo = {0};
    radioInfo.dwSize = sizeof(radioInfo);
    DWORD result = BluetoothGetRadioInfo(radioHandle, &radioInfo);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to get radio info." << std::endl;
        return false;
    }

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
            sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
            1, // returnAuthenticated
            0,
            0, // returnUnknown
            1, // returnConnected
            1, // issueInquiry
            2, // timeoutMultiplier, adjust based on how much time you want to spend
            radioHandle
    };

    BLUETOOTH_DEVICE_INFO deviceInfo = device.deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
    HBLUETOOTH_DEVICE_FIND deviceFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

    if (deviceFind == NULL) {
        return false;
    }

    bool found = false;
    do {
        if (device.address == formatBluetoothAddress(deviceInfo.Address)) {
            found = true;
            break;
        }
    } while (BluetoothFindNextDevice(deviceFind, &deviceInfo));

    BluetoothFindDeviceClose(deviceFind);
    return found;
}

std::wstring BlueToothDevices::formatBluetoothAddress(BLUETOOTH_ADDRESS address) {
    std::wstringstream ws;
    ws << std::hex << std::uppercase << std::setfill(L'0');
    for (int i = 0; i < 6; i++) {
        ws << std::setw(2) << static_cast<int>(address.rgBytes[i]);
        if (i < 5) {
            ws << L":";
        }
    }
    return ws.str();
}
