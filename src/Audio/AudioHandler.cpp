#include "AudioHandler.h"
#include <iostream>
#include <cstdlib>

int AudioHandler::recordCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                  double streamTime, RtAudioStreamStatus status, void *userData) {
    if (status)
        std::cout << "Stream overflow detected!" << std::endl;
    std::cout << "Frames: " << nBufferFrames << std::endl;
    return 0; // Continue recording
}

std::string AudioHandler::getRtAudioErrorMessage(RtAudioErrorType errorType) {
    switch (errorType) {
    case RTAUDIO_NO_ERROR:
        return "No error";
    case RTAUDIO_INVALID_DEVICE:
        return "Invalid device";
    case RTAUDIO_MEMORY_ERROR:
        return "Memory error";
    case RTAUDIO_INVALID_PARAMETER:
        return "Invalid parameter";
    default:
        return "Unknown error";
    }
}

void AudioHandler::runAudioStream() {
    RtAudio adc;
    if (adc.getDeviceCount() == 0) {
        std::cerr << "No audio devices found!\n";
        exit(1);
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = adc.getDefaultInputDevice();
    parameters.nChannels = 2;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 512;

RtAudioErrorType result = adc.openStream(NULL, &parameters, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, this);
    if (result != RTAUDIO_NO_ERROR) {
        std::cerr << "Failed to open stream: " << getRtAudioErrorMessage(result) << std::endl;
        exit(1);
    }

    result = adc.startStream();
    if (result != RTAUDIO_NO_ERROR) {
        std::cerr << "Failed to start stream: " << getRtAudioErrorMessage(result) << std::endl;
        adc.closeStream();
        exit(1);
    }

    std::cout << "Recording... Press Enter to quit.\n";
    std::cin.get();

    if (adc.isStreamOpen()) {
        adc.stopStream();
    }
    if (adc.isStreamOpen()) {
        adc.closeStream();
    }
}

int AudioHandler::audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                double streamTime, RtAudioStreamStatus status, void *userData) {
    return static_cast<AudioHandler *>(userData)->recordCallback(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, userData);
}

