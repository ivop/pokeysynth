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
Some knowledge about how the Pokey chip operates is recommended, but not strictly necessary.

#### MIDI Channels

![MIDI Channels](images/midi-channels.png)

Each plugin instance can be configured to listen and respond to four blocks of MIDI channels (1-4, 5-8, 9-12, or 13-16). This distinction is necessary if you want to eventually playback the result on real hardware.

#### Pokey Channels

![Pokey Channels](images/pokey-channels.png)

There are 128 instruments corresponding to the 128 MIDI program numbers.
Internally and in the MIDI protocol they are numbered from 0 to 127.
Some programs think they are clever and number them 1 to 128.
To make it easier in that case to lookup an instrument and set the program change event in your DAW, you can tick the Display 1-128 box.

#### Update Speed

![Update Speed](images/update-speed.png)

Update Speed controls the speed at which the incoming MIDI events are processed and played back as Pokey sounds.
It also determines how fast the sound generator(s) go through the volume envelope and distortion list (see below).
Each tick is one step.
50Hz is the most common setting and all instruments in the default sound bank are designed for it.

### Instrument Editor

#### Name and Type

![Instrument Editor Name and Type](images/instrument-editor-head.png)

Each instrument has a name and a type.
The name can be up to 64 characters.
The types are any combination of channel layout and clock frequency.

##### Pokey Channel combinations

* An ```8-bit channel``` instrument uses a single Pokey channel and has a limited 8-bit frequency range.
* A ```2CH Linked``` instrument uses two 8-bit Pokey channels linked together to generate a single tone, and having a 16-bit frequency range.
For convenience, underneath the radio button there's a note displaying which Pokey channel combinations are used when such an instrument is triggered.
This type is most useful with an 1.8Mhz clock to have the widest frequency range.
With lower clocks it only extends the resolution, not the range.
* A ```2CH Filter``` instrument also utilizes two 8-bit Pokey channels, but filters one with the other, generating a different timbre than the normal square wave.
The frequency resolution is again limited to 8-bits.
Again, underneath it displays which channel combinations are used.
* Finally, a ```4CH Filter``` instrument uses all four 8-bit Pokey channels.
That's two pairs creating two ```2CH Linked``` instruments, and then one is filtered by the other, resulting in a single instrument with the same timbre as ```2CH Linked``` instruments, but with 16-bit frequency resolution.

When playing multiple notes at once on a single plugin instance, there's a possibility of channel conflicts.
See **Channel Priorities** below how these are resolved.
In short, the instrument with the highest priority wins, and the lowest priority is muted.

##### Pokey Channel Clocks

Each Pokey channel generates its sound frequency relative to a base clock.
The 15kHz and 64kHz are mutually exclusive and influence all four Pokey channels.
1.8MHz overrides the 15 or 64kHz base clock, but can only be set for channel 1 or 3 (or channel 1+2 or 3+4 when the channels are linked).
Contrary to the channel layout conflicts mentioned earlier, frequency conflicts _do not_ mute the offending channel.
If two instruments are set to play at the same time (both are in the MIDI Note On phase) and there is a clock frequency mismatch, the 15kHz instrument wins (sounds in tune) and the 64kHz instrument will sound out of tune.
1.8MHz instruments never conflict, except when played on a channel that has no 1.8MHz support, i.e. playing an 8-bit 1.8MHz instrument on channel 2.
This is by design as to have audible feedback when combing 15kHz and 64kHz instruments on a single Pokey.
It is possible to mix 15kHz and 64kHz instruments on a single Pokey, but one has to take great care to avoid two mismatching instruments being played at the same time.

* 15kHz, low frequencies in the bass range, useful for bass lines and kick drums
* 64kHz, mid-range frequencies, used for chords, melodies, and percussive sounds
* 1.8MHz, overrides 15/64kHz base clock, full frequency range from C0-C9

#### Volume Envelope and Distortion

![Volume Envelope and Distortion](images/instrument-editor-vol-dist.png)

##### Volume

The volume envelope describes how the volume of the instrument changes throught time.
Each tick has a specific volume assigned.
You can either draw them as a bar graph, edit them manually in hexadecimal below it, or use the ADSR helper on the left.
Note that the ADSR values are _not_ real-time.
You need to set the values you want and then click one of the AD, ADR or ADSR buttons to generate the envelope.

* Attack sets the amount of ticks to rise from 0 to 15 (F)
* Decay sets the amount of ticks to fall to sustain level
* Sustain sets the sustain level, i.e. the volume of the instrument while you keep pressing the key
* Release sets the amount of ticks to fall from sustain level to 0

Every MIDI Note On event starts at the beginning of the envelope.
When it reaches the marker of the ```Sustain End``` slider, it will loop back to the ```Sustain Start``` slider.
Most of the time these are the same, sustaining on a constant volume when a note is held.
You can use a small window between Start and End to create a volume tremelo.
Once a MIDI Note Off event arrives, the Release period starts, which usually fades out the volume to 0.
If the ```Sustain End``` marker is equal to or beyond the ```Release End``` marker, there will be no sustain and it will progress linearly from start to end and then stop.
This is useful for percussion or pizzicato instruments which have no sustain.

##### Distortion

The distortion list denotes which Pokey distortion is used while playing back the note.

* 0 - Pure, this is a pure square wave, used for instruments, and chords, or bass notes at 15kHz or with 2CH Linked instruments
* 1 - Noise, white noise generator, useful for percussion
* 2 - Buzzy Bass, the typical Pokey bass sound, with a soft edge
* 3 - Gritty Bass, the typical Pokey bass sound, but more harsh. Note that 15kHz and 1.8MHz 2CH Linked instruments have no Gritty bass. Setting this type of distortion will fallback to buzzy bass.
* 4 - Poly5 Square, sort of a sqaure wave, but sounds more like a hobo or clarinet. Only useful for 1.8MHz 8-bit channel instruments.

On the right there are handy buttons to set the whole envelope to one of the specified distortions.

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
