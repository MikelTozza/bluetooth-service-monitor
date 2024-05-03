#include <iostream>
#include <iomanip>
#include "Bluetooth/BlueToothDevices.h"
#include "Audio/AudioHandler.h"

int main()
{
    // AudioHandler handler;
    // handler.runAudioStream();
    // return 0;
    
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
