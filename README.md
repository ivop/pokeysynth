# PokeySynth

An LV2 Virtual Instrument that emulates the Atari Pokey soundchip.

<img src="images/pokeysynth.png" height="256">

## Installation

Head over to the [Releases](https://github.com/ivop/pokeysynth/releases) page and download the latest release for your platform.

Linux users need to unpack the archive in ```$HOME/.lv2```.
The .lv2 directory must be created if it does not already exist.
Windows users need to unpack the archive in ```%HOMEDRIVE%%HOMEPATH%\AppData\Roaming\LV2```.
The LV2 directory must be created if it does not already exist.

## Usage

Once installed, you can load the plugin in your favorite DAW if it supports LV2 plugins, or in a stand-alone LV2 plugin host like ```jalv``` (Linux) or ```carla``` (Linux and Windows).
Each plugin instance emulates a full Pokey chip.
If your use case is creating chiptune-like music in combination with other synths and sampled instruments, it's advised to create as many plugin instances as you need channels and instruments, and play a single instrument on a single Pokey for maximum sound quality and frequency resolution.
If instead you want to create music that can actually be played back on real hardware (stock Atari with single Pokey, Gumby stereo upgrade, or quad Pokey with PokeyMax) you need to limit the amount of plugin instances accordingly and route up to four MIDI tracks to each instance.

#### MIDI Channels

![MIDI Channels](images/midi-channels.png)

Each plugin instance can be configured to listen and respond to four blocks of MIDI channels (1-4, 5-8, 9-12, or 13-16). This distinction is necessary if you want to eventually playback the result on real hardware.

#### Pokey Channels

![Pokey Channels](images/pokey-channels.png)

There are 128 instruments corresponding to the 128 MIDI program numbers.
Internally and in the MIDI protocol they are numbered from 0 to 127.
Some programs think they are clever and number them 1 to 128.
To make it easier in that case to lookup an instrument and set the program change event in your DAW, you can tick the Display 1-128 box.

Each instrument has a name and a type.
The name can be up to 64 characters.
The types are any combination of channel layout and clock frequency.
An ```8-bit channel``` instrument uses a single Pokey channel and has a limited 8-bit frequency range.
A ```2CH Linked``` instrument uses two 8-bit Pokey channels linked together to generate a single tone, and having a 16-bit frequency range.
For convenience, underneath the radio button there's a note displaying which Pokey channel combinations are used when such an instrument is triggered.
A ```2CH Filter``` instrument also utilizes two 8-bit Pokey channels, but filters one with the other, generating a different timbre than the normal square wave.
The frequency resolution is again limited to 8-bits.
Again, underneath it displays which channel combinations are used.
Finally, a ```4CH Filter``` instrument uses all four 8-bit Pokey channels.
That's two pairs creating two ```2CH Linked``` instruments, and then one is filtered by the other, resulting in a single instrument with the same timbre as ```2CH Linked``` instruments, but with 16-bit frequency resolution.

#### Update Speed

![Update Speed](images/update-speed.png)

Update Speed controls the speed at which the incoming MIDI events are processed and played back as Pokey sounds.
It also determines how fast the sound generator(s) go through the volume envelope and distortion list (see below).
Each tick is one step.
50Hz is the most common setting and all instruments in the default sound bank are designed for it.

### Instrument Editor

#### Name and Type

![Instrument Editor Name and Type](images/instrument-editor-head.png)

#### Volume Envelope and Distortion

![Volume Envelope and Distortion](images/instrument-editor-vol-dist.png)

#### Note Table

![Note Table](images/instrument-editor-notes.png)

#### Miscellaneous Settings

![Misc Settings](images/misc-settings.png)

#### Loading and Saving

![Loading and Saving](images/load-save.png)

#### SAP-R Recording

![SAP-R Recording](images/sapr.png)

#### Overdrive and Panic!

![Overdrive and Panic](images/overdrive-panic.png)

### Channel Priorities

Channel priorities are handled according to the following table:

<table>
  <tr>
    <th colspan="1" align="center"> Priority </th>
    <th colspan="4" align="center"> Channel Configuration </th>
  </tr>
  <tr>
    <th colspan="1" align="center"> 1 </th>
    <td colspan="4" align="center"> 1+2+3+4 Linked Filtered </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 2 </th>
    <td colspan="2" align="center"> 1+3 Filtered </td>
    <td colspan="2" align="center"> 2+4 Filtered </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 3 </th>
    <td colspan="2" align="center"> 1+3 Filtered </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 4 </th>
    <td colspan="2" align="center"> 1+2 Linked </td>
    <td colspan="2" align="center"> 3+4 Linked </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 5 </th>
    <td colspan="2" align="center"> 1+2 Linked </td>
    <td colspan="1" align="center"> 3 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 6 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="2" align="center"> 2+4 Filtered </td>
    <td colspan="1" align="center"> 3 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 7 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="2" align="center"> 3+4 Linked </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 8 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="1" align="center"> 3 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
</table> 

This means that, for example, a single channel instrument on channel 1, 2, and 3,
and a filtered instrument on channel 4 is handled according to priority rule 6,
which means that channel 1, 3 and 2+4 filtered are audible and the single instrument
on channel 2 is muted.
