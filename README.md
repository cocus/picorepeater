![PlatformIO](https://github.com/cocus/picorepeater/workflows/PlatformIO/badge.svg?branch=master)

# The "brain" for a Small RF repeater/relay
This code is the foundation of a simple UHF/VHF or any kind of RF repeater. It doesn't transmit or receive anything by itself, requiring two RF modules for that.

For instance, when the module 'A' receives a signal on its assigned channel, it will assert a pin, signaling the code to put the other module ('B') into transmission mode. The audio output of module 'A' is routed via a relay (NC) to the audio input of module 'B'.
When the module 'A' finishes receiving the signal, the code will energize the relay, routing the audio input of the module 'B' into the filtered PWM output of the Arduino. A "tail" sound will be played. After this sound finishes, the code stops module 'B' transmission.
The opposite happens when module 'B' receives a signal on its assigned channel.

There are some timers slots that can be assigned with intervals, where the inverval is expired, a sound will play to either the A or B module, or both.

All of these functions can be configured via a simple .ini file on an SD card. The sounds are stored there as well, on a FAT32 formatted card as WAV files, 8 bits/16kHz/mono.

# Automagic builds
Go to https://github.com/cocus/picorepeater/actions and select PlatformIO to download the hex or elf for the latest PRs and pushes to the repo.

# Requirements
* Any Arduino with an AVR M328
* An SD card adapter (with appropriate 3v3 level shifter if your Arduino is 5V)
* Visual Studio Code + PlatformIO for vscode
* Arduino Framework for AVR installed on PlatformIO

# Hardware Setup
SD Card module connections:
| Arduino | M328 Arduino Pin | SD Card |
|---------|------------------|---------|
| MISO    | D12              | MISO    |
| MOSI    | D11              | MOSI    |
| SCK     | D13              | CLK     |
| D4      | D4               | /CS     |

RF Modules connections:
| Arduino | Usage              |
|---------|--------------------|
| D5      | Module A PTT (TX)  |
| D2      | Module A Busy (RX) |
| D6      | Module B PTT (TX)  |
| D3      | Module B Busy (RX) |

Miscelaneous:
| Arduino | Usage                                  |
|---------|----------------------------------------|
| D7      | Blinky LED                             |
| D8      | Relay control output for audio route   |
| D9      | PWM audio output, needs to be filtered |

Note about D7: You'll need to remove the LED that's usually connected to the D13 pin to avoid some issues with the loading of the pin and the high speed mode of the SPI peripheral. That's why I've chosen the D7 pin for the blinky LED instead of the default D13.

Note about D8: It's encouraged to add a transistor to turn on the relay. Use a simple BJT to achieve this.

Note about D9: The PWM present on the pin D9 needs to be filtered out, I didn't get a really good performance using a RC filter, but neither I did with the RLC filter. I recommend the classic 1k and 100nF for it. But you can chose a better one. I strongly suggest you to check the amazing [Filter Design and Analysis](http://sim.okawa-denshi.jp/en/Fkeisan.htm) tools of OKAWA Electric Design.


For the RF modules, you'll need to connect the audio input and output to a relay as follows:
| Relay Contact           | RF Module connection     |
|-------------------------|--------------------------|
| COM1 (common 1)         | Module A audio input     |
| NC1 (normally closed 1) | Module B audio output    |
| NO1 (normally open 1)   | Output of the PWM filter |
| COM2 (common 2)         | Module B audio input     |
| NC2 (normally closed 2) | Module A audio output    |
| NO2 (normally open 2)   | Output of the PWM filter |

You might want to add potentiometers on each NC and NO poles of each throw of the relay so the volume can be tweaked independently for all the combinations. Use the wiper of the potentiometer as the output, one end to ground and the other ground to the NC or NO.

# SD Card setup
An SD card of 1, 2, 4, 8GB should be used. It needs to be formatted in either FAT16 (FAT) or FAT32.
A file called `pico.ini` should be placed on the SD card to make usage of the fancy features of this project. If not, this will act as a dumb relay.
The structure of it should follow the standard structure of an ini file.
## pico.ini Sections
All of the supported sub-sections should contain the `play` and `out` keys.
* `play` sets the file name to be played for the trigger of the sub-section.
* `out` sets the output module, can be `A`, `B` or `*` (both).

### tail_a and tail_b Section
These sections specify the audio to play after reception ends on the `A` or `B` module. For instance:
```ini
[tail_a]
play = tail_a.wav
out = B
```
Will play the file `tail_a.wav` on module `B` after reception of the module `A` finishes.

### timerX (X = 0 thru 4) Section
These sections specify the audio to play after a selected time passes. It will not play it if the modules are receiving/transmitting.
The key `period` specifies the time to wait before playing the audio file. It's expressed in milliseconds.
For instance:
```ini
[timer0]
play = play10s.wav
out = A
# 10s
period = 10000
```
Will play the file `play10s.wav` on module `A` every 10 seconds, if neither of the RF modules are busy.

# Contributions
Please feel free to open an [Issue](https://github.com/cocus/picorepeater/issues) to discuss features to be added, or bugs to be addressed. Incoming PRs are encouraged!