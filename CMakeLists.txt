# CMakeLists.txt
cmake_minimum_required(VERSION 3.28)
project(bluetooth_service_monitor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

# Set the build type to Debug if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories for project-specific headers
include_directories(src/Audio)
include_directories(src/Bluetooth)

# Add RtAudio as an external subdirectory
add_subdirectory(external/rtaudio)

# Include RtAudio headers
include_directories(external/rtaudio)

# Ensure debugging symbols are added
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Define the main executable target
add_executable(bluetooth_service_monitor
        src/Audio/AudioHandler.cpp
        src/Audio/AudioHandler.h
        src/Bluetooth/BlueToothDevices.cpp
        src/Bluetooth/BlueToothDevices.h
        src/main.cpp
)

# Link the RtAudio library and Bluetooth libraries to the main target
if (WIN32)
    target_link_libraries(bluetooth_service_monitor rtaudio bthprops ws2_32 setupapi)
else()
    target_link_libraries(bluetooth_service_monitor rtaudio)
endif()
