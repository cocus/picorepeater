
#include "picocfg.hh"
// We make heavy use of the Serial object on this file
#include <Arduino.h>

#include <IniFile.h>

#if PICO_DEBUG
static void printErrorMessage(uint8_t e, bool eol = true)
{
    Serial.print(F(" ini error = ")); Serial.print(e); Serial.print(F(" = "));
    switch (e)
    {
        case IniFile::errorNoError:
            Serial.print("no error");
            break;
        case IniFile::errorFileNotFound:
            Serial.print("file not found");
            break;
        case IniFile::errorFileNotOpen:
            Serial.print("file not open");
            break;
        case IniFile::errorBufferTooSmall:
            Serial.print("buffer too small");
            break;
        case IniFile::errorSeekError:
            Serial.print("seek error");
            break;
        case IniFile::errorSectionNotFound:
            Serial.print("section not found");
            break;
        case IniFile::errorKeyNotFound:
            Serial.print("key not found");
            break;
        case IniFile::errorEndOfFile:
            Serial.print("end of file");
            break;
        case IniFile::errorUnknownError:
            Serial.print("unknown error");
            break;
        default:
            Serial.print("unknown error value");
            break;
    }
    if (eol)
    {
        Serial.println();
    }
}
#endif // #if PICO_DEBUG

static bool loadPlayBase(
    const IniFile &ini,
    const char *section,
    Cfg::ActionPlayBase &playBase
)
{
    constexpr size_t len = sizeof(playBase.Filename) + 60;
    char buffer[len];

    bool status;

    // Read the "play" value
    status = ini.getValue(
        section,
        "play",
        buffer,
        sizeof(buffer)
    );

    // Not found?
    if (!status)
    {
#if PICO_DEBUG
        Serial.print(F("'play' not found on section ")); Serial.println(section);
        printErrorMessage(ini.getError());
#endif // #if PICO_DEBUG
        // Nope
        return status;
    }

    // Copy the string, taking not of the
    // struct string size.
    strncpy(
        playBase.Filename,
        buffer,
        sizeof(playBase.Filename)
    );

    // Read the "out" value
    status = ini.getValue(
        section,
        "out",
        buffer,
        sizeof(buffer)
    );

    // Not found?
    if (!status)
    {
#if PICO_DEBUG
        Serial.print(F("'out' not found on section ")); Serial.println(section);
        printErrorMessage(ini.getError());
#endif // #if PICO_DEBUG
        // Nope
        return status;
    }

    // There should be just 1 char
    if (strnlen(buffer, len) != 1)
    {
        // Seems like not
        return false;
    }

    switch (buffer[0])
    {
        case 'A':
        case 'a':
        {
            playBase.AudioOut = Cfg::AudioOutTarget::A;
            break;
        }
        case 'B':
        case 'b':
        {
            playBase.AudioOut = Cfg::AudioOutTarget::B;
            break;
        }
        case '*':
        {
            playBase.AudioOut = Cfg::AudioOutTarget::BOTH;
            break;
        }
        default:
        {
#if PICO_DEBUG
            Serial.print(F("Unknown 'out' value ")); Serial.print(buffer[0]); Serial.print(F(" on section ")); Serial.println(section);
#endif // #if PICO_DEBUG
            // Unknown value
            return false;
        }
    }
#if PICO_DEBUG
    Serial.print(F("section ")); Serial.print(section); Serial.print(F(", play=")); Serial.print(playBase.Filename); Serial.print(F(", out = ")); Serial.println(buffer[0]);
#endif // #if PICO_DEBUG

    return status;
}

static bool loadTail(
    const IniFile &ini,
    const char *section,
    Cfg::ActionTail &tail
)
{
    tail.IsValid = loadPlayBase(ini, section, tail.Play);
    return tail.IsValid;
}

static bool loadTimer(
    const IniFile &ini,
    const char *section,
    Cfg::ActionTimer &timer
)
{
    timer.IsValid = loadPlayBase(ini, section, timer.Play);
    if (!timer.IsValid)
    {
        return timer.IsValid;
    }

    constexpr size_t len = 80;
    char buffer[len];

    // Read the "play" value
    timer.IsValid = ini.getValue(
        section,
        "period",
        buffer,
        sizeof(buffer),
        timer.Period
    );

    // Not found?
    if (!timer.IsValid)
    {
#if PICO_DEBUG
        Serial.print(F("'period' not found on section ")); Serial.println(section);
        printErrorMessage(ini.getError());
#endif // #if PICO_DEBUG
        // Nope
    }

    return timer.IsValid;
}

bool Cfg::loadConfiguration(
    const char *filename,
    Cfg::PicoConfig &config
)
{
    IniFile ini(filename);
    ini.open();

    if (!ini.isOpen())
    {
#if PICO_DEBUG
        Serial.print(F("Can't open file ")); Serial.println(ini.getFilename());
#endif // #if PICO_DEBUG
        return false;
    }

    loadTail(ini, "tail_a", config.TailA);
    loadTail(ini, "tail_b", config.TailB);

    constexpr size_t len = 20;
    char section[len];
    for (size_t i = 0; i < Cfg::MaxTimers; i++)
    {
        snprintf(section, len, "timer%d", i);
        loadTimer(ini, section, config.Timers[i]);
    }

    return true;
}