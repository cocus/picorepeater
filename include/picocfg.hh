
#include <stddef.h>
#include <stdint.h>

namespace Cfg
{
    static const size_t MaxTimers = 5;
    static const size_t MaxFileSize = 20;

    enum class AudioOutTarget { A, B, BOTH };

    struct ActionPlayBase
    {
        char Filename[MaxFileSize];
        AudioOutTarget AudioOut;
    };

    struct ActionTail
    {
        ActionPlayBase Play;
        bool IsValid;
    };

    struct ActionTimer
    {
        ActionPlayBase Play;
        uint32_t Period;
        bool IsValid;
    };

    struct PicoConfig
    {
        ActionTail TailA;
        ActionTail TailB;
        ActionTimer Timers[MaxTimers];
    };


    bool loadConfiguration(
        const char *filename,
        Cfg::PicoConfig &config
    );
};