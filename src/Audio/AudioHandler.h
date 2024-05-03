#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include "RtAudio.h"
#include <string>

class AudioHandler {
public:
    int recordCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                       double streamTime, RtAudioStreamStatus status, void *userData);

    std::string getRtAudioErrorMessage(RtAudioErrorType errorType);
        
    static int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                             double streamTime, RtAudioStreamStatus status, void *userData);

    void runAudioStream();  // Method to encapsulate audio stream handling
};

#endif
 