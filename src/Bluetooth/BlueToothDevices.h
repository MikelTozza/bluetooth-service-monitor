#ifndef BLUETOOTH_DEVICES_H
#define BLUETOOTH_DEVICES_H
#include <vector>
#include <string>
#include <windows.h>
#include <bluetoothapis.h>
#include <initguid.h>

#define BLUETOOTH_SERVICE_ENABLE 1
#define BLUETOOTH_SERVICE_DISABLE 0


class BlueToothDevices {
public:
    struct DeviceInfo {
        BLUETOOTH_DEVICE_INFO deviceInfo;
        std::wstring name;
        std::wstring address;
        ULONG deviceClass;
        bool authenticated;
        bool connected;
        bool handsfreeTelephonySupported;
    };

    std::vector<DeviceInfo> getBluetoothDevices();
    bool checkHandsfreeTelephonySupport(const BLUETOOTH_DEVICE_INFO& deviceInfo);
    bool setHandsfreeTelephonyServiceState(const DeviceInfo& device, bool enable);
    bool isCorrectRadio(HANDLE radioHandle, const DeviceInfo& device);
    std::wstring formatBluetoothAddress(BLUETOOTH_ADDRESS address);
};

#endif // BLUETOOTH_DEVICES_H
