#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
#include <stdint.h>

#include "picocfg.hh"

#include "rfmodule.hh"


static void check_for_blinky();
static void check_for_timer_events();
static void check_for_rx_events();
static void module_b_finished_receiving();
static void module_b_started_receiving();
static void module_a_finished_receiving();
static void module_a_started_receiving();
static void play_audio_in_module(Cfg::ActionPlayBase &target);


TMRpcm tmrpcm;

static RFModule module_a(2, 5);
static RFModule module_b(3, 6);

static constexpr uint8_t PinSDCardChipSelect = 4;

static constexpr uint8_t PinToEnableAudioToModules = 8;

static constexpr uint8_t PinBlinky = 7;

static constexpr unsigned long BlinkyIntervalInMs = 500;

static constexpr unsigned long DebounceIntervalForRxBusyInMs = 75;

static constexpr unsigned long RelayDelayInMs = 50;

static Cfg::PicoConfig _main_config = { 0 };


static void inline audio_route_arduino()
{
    // Route the arduino audio into the modules
    digitalWrite(PinToEnableAudioToModules, 1);
}

static void inline audio_route_module2module()
{
    // Route the module's audio signals
    digitalWrite(PinToEnableAudioToModules, 0);
}

static void inline audio_route_init()
{
    pinMode(PinToEnableAudioToModules, OUTPUT);
}







static void reboot()
{
    while (1) {}
}


static void play_audio_in_module(Cfg::ActionPlayBase &target)
{
    audio_route_arduino();

    delay(RelayDelayInMs);

    module_a.SetPtt(
        target.AudioOut == Cfg::AudioOutTarget::A ||
        target.AudioOut == Cfg::AudioOutTarget::BOTH
    );

    module_b.SetPtt(
        target.AudioOut == Cfg::AudioOutTarget::B ||
        target.AudioOut == Cfg::AudioOutTarget::BOTH
    );

    Serial.print(F(" * playing ")); Serial.print(target.Filename);
    Serial.print(F(" on A = ")); Serial.print(module_a.IsTx() ? F("true") : F("false"));
    Serial.print(F(" on B = ")); Serial.println(module_b.IsTx() ? F("true") : F("false"));
    tmrpcm.play(target.Filename);

    while(tmrpcm.isPlaying())
    {
        // Do nothing :D
        check_for_blinky();
    }

    Serial.println(F(" * finished playing"));
    Serial.println(F(" * ModA finished TX"));
    Serial.println(F(" * ModB finished TX"));

    module_a.SetPtt(false);
    module_b.SetPtt(false);

    audio_route_module2module();

    delay(RelayDelayInMs);
}

static void module_a_started_receiving()
{
    Serial.println(F(" * ModB started TX"));
    module_b.SetPtt(true);
}

static void module_a_finished_receiving()
{
    if (_main_config.TailB.IsValid)
    {
        play_audio_in_module(_main_config.TailB.Play);
    }
    else
    {
        Serial.println(F(" * ModA finished TX"));
        Serial.println(F(" * ModB finished TX"));
        module_a.SetPtt(false);
        module_b.SetPtt(false);
    }
}

static void module_b_started_receiving()
{
    Serial.println(F(" * ModA started TX"));
    module_a.SetPtt(true);
}

static void module_b_finished_receiving()
{
    if (_main_config.TailA.IsValid)
    {
        play_audio_in_module(_main_config.TailA.Play);
    }
    else
    {
        Serial.println(F(" * ModA finished TX"));
        Serial.println(F(" * ModB finished TX"));
        module_a.SetPtt(false);
        module_b.SetPtt(false);
    }
}

static void check_for_rx_events()
{
    static bool mod_a_prev_busy = false;
    static bool mod_b_prev_busy = false;
    static unsigned long debounce_last_sample = 0;

    if (millis() - debounce_last_sample > DebounceIntervalForRxBusyInMs)
    {
        if (module_a.IsRxBusy() != mod_a_prev_busy)
        {
            mod_a_prev_busy = module_a.IsRxBusy();

            if (mod_a_prev_busy)
            {
                // Module A is receiving
                Serial.println(F("ModA started RX"));
                module_a_started_receiving();
            }
            else
            {
                // Module A finished receiving
                Serial.println(F("ModA finished RX"));
                module_a_finished_receiving();
            }
        }

        if (module_b.IsRxBusy() != mod_b_prev_busy)
        {
            mod_b_prev_busy = module_b.IsRxBusy();

            if (mod_b_prev_busy)
            {
                // Module B is receiving
                Serial.println(F("ModB started RX"));
                module_b_started_receiving();
            }
            else
            {
                // Module B finished receiving
                Serial.println(F("ModB finished RX"));
                module_b_finished_receiving();
            }
        }

        debounce_last_sample = millis();
    }
}

static void check_for_timer_events()
{
    static unsigned long last_checked_timers[Cfg::MaxTimers] = { 0 };

    for (size_t timer_idx = 0; timer_idx < Cfg::MaxTimers; timer_idx++)
    {
        if (!_main_config.Timers[timer_idx].IsValid)
        {
            continue;
        }

        if (millis() - last_checked_timers[timer_idx] > _main_config.Timers[timer_idx].Period)
        {
            if (module_a.IsRxBusy() || module_b.IsRxBusy())
            {
                Serial.print(F("Timer ")); Serial.print(timer_idx); Serial.println(F(" elapsed, but channel was busy. Waiting a moment!"));
                last_checked_timers[timer_idx] += 5000;
                continue;
            }

            Serial.print(F("Timer ")); Serial.print(timer_idx); Serial.println(F(" elapsed, playing!"));
            play_audio_in_module(_main_config.Timers[timer_idx].Play);
            last_checked_timers[timer_idx] = millis();
        }
    }
}

static void check_for_blinky()
{
    static unsigned long blinky_last_sample = 0;

    if (millis() - blinky_last_sample > BlinkyIntervalInMs)
    {
        digitalWrite(PinBlinky, !digitalRead(PinBlinky));
        blinky_last_sample = millis();
    }
}

void setup()
{
    // Initialize serial port
    Serial.begin(9600);
    while (!Serial)
    {
        continue;
    }

    // Init audio route pin
    audio_route_init();
    audio_route_module2module();

    // Init blinky
    pinMode(PinBlinky, OUTPUT);

    // Init tmrPCM player
    tmrpcm.speakerPin = 9;
    tmrpcm.setVolume(4);
    tmrpcm.quality(true);

    // Init SD card
    if (!SD.begin(PinSDCardChipSelect))
    { // see if the card is present and can be initialized:
        Serial.println(F("Failure to mount the SD card"));
    }
    else
    {
        Serial.println(F("SD OK!"));

        // Read the config file
        Serial.println(F("Reading config"));
        Cfg::loadConfiguration("pico.ini", _main_config);
        Serial.println(F("Done!"));
    }

    delay(200);
}

void loop()
{
    check_for_rx_events();
    check_for_timer_events();
    check_for_blinky();
}