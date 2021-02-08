# Plug-in Sample - Wwise Up On Air Hands On

This repository contains sample code used as reference in the [Wwise Up On Air
series](https://www.youtube.com/watch?v=abMtq9nGj8Y). It requires the Wwise SDK, which can be acquired from the [Wwise
Launcher](https://www.audiokinetic.com/download/): select SDK and the appropriate SDK platforms, e.g., Windows.

Visual Studio Code workspace settings are provided and rely on the environment variables `%WWISEROOT`,
which should point to the Wwise installation you wish to target. You may also use the command-line to directly use
`wp.py`, the development tool provided at `%WWISEROOT%/Scripts/Build/Plugins/wp.py`.

For documentation on creating plug-ins, refer to the Wwise SDK section [Creating New Plug-ins - Audio
Plug-ins](https://www.audiokinetic.com/library/edge/?source=SDK&id=effectplugin.html).

## Summary of Content
The following is a summary of the content covered so far in the series.

## Part 1: Creating a Sound Engine plug-in

### Creating a new Plug-in
We started by creating a plug-in using `wp.py`, the Wwise Plug-in development tool. We created a plug-in by using the
`new` command and answered the questions from the wizard:

```sh
> py -3 "%WWISEROOT%/Scripts/Build/Plugins/wp.py" new

Plug-in type: {source, sink, effect, mixer}: effect
Do you need out-of-place processing? (no)
Project name: WoaGain
Display name: Woa Gain
Author: Samuel Longchamps
Description: It is a super gain!
Is this OK? (yes)
Generating project structure for WoaGain
```

This provided us with a directory `WoaGain` located under our current working directory that contained the following
files:

```
WoaGain/
│
├── SoundEnginePlugin/        # Wwise Sound Engine plug-in code
│   ├── WoaGainFX.cpp         # DSP plug-in implementation
│   ├── WoaGainFX.h
│   ├── WoaGainFXFactory.h
│   ├── WoaGainFXParams.cpp   # Parameters definition and loading/updating
│   ├── WoaGainFXParams.h
│   └── WoaGainFXShared.cpp   # Export for the shared library format
├── WwisePlugin/              # Wwise Authoring plug-in code
│   ├── WoaGain.cpp           # Registration and factory
│   ├── WoaGain.def
│   ├── WoaGain.h
│   ├── WoaGain.xml           # Definition of properties
│   ├── WoaGainPlugin.cpp     # GUI implementation
│   └── WoaGainPlugin.h
├── FactoryAssets/
│   └── Manifest.xml          # Info to match assets and plug-ins
├── Help/                     # Property Help documentation
├── additional_artifacts.json # Additional files to package
├── bundle_template.json      # Template for bundle.json (for Launcher)
├── PremakePlugin.lua         # Premake file for generating platform solutions
└── WoaGainConfig.h           # Common definitions for Sound Engine and Authoring
```

### Building a Plug-in

We changed the current directory to be that of the generated plug-in using the `cd` command:

```sh
> cd WoaGain
```

From the plug-in's directory, we then used the `premake` command to generate the Sound Engine part's solution for the
target platform `Windows_vc160` (Windows using Visual Studio 2019):

```sh
> py -3 "%WWISEROOT%/Scripts/Build/Plugins/wp.py" premake Windows_vc160
```

We tested that the generated solution could be used to build the generated code in the `Debug` configuration using the
`build` command:

```sh
> py -3 "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Windows_vc160 -c Debug
```

By specifying no architecture (flag `-x`), `wp.py` built all of the defined architectures for the target platform.
For Windows, those architectures correspond to the `x86` family: `Win32` (aka `i386`) and `x64` (aka `x86-64` or `amd64`).

We proceeded to do the same for the Authoring part of the plug-in. We used the `Release` configuration as that is the
configuration of Wwise Authoring that we have access to, and chose the `x64_vc160` architecture (`x64` using Visual Studio 2019):

```sh
> py -3 "%WWISEROOT%/Scripts/Build/Plugins/wp.py" premake Authoring
> py -3 "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Authoring -c Release -x x64_vc160
```

### Modifying the Plug-in Defaults

To have a parameter to control the gain amount, we modified the placeholder property that was generated in the
`WoaGain.xml` as follow:

* We changed the `DisplayName` to `Gain`
* We added the `DataMeaning` attribute and set its value to `Decibels`
* We changed the minimum and maximum value restriction to a range of -96.0 to 10.0 dB

We also changed the `ID` of the plug-in to avoid clashes with other plug-ins by setting the `PluginID` attribute of the
`EffectPlugin` element in the XML and the corresponding `PluginID` variable in `WoaGainConfig.h` file to `123`. If you
plan to provide Factory Assets, you will also need to change the ID in the `Manifest.xml` file.

### Adding DSP code

We added some DSP code that applied the Gain value we acquire from the plug-in's parameter `struct`.

We started by observing the content of the generated code, which had a for-loop going over each channels of the audio
buffer and acquiring a pointer to the beginning of the channel array of samples. Then, a while-loop uses a counter
variable `uFrameProcessed` that is incremented (1 is added to it) at each iteration (at each execution of its body, located between braces
`{}`) until it reaches the maximum number of samples we are allowed to process per channel.
This amount is provided by the `uValidFrames`, which we acquire from `io_pBuffer`. The `uFrameProcessed` counter
variable is reset to 0 before starting a new while-loop.


Inside the while-loop, a single line was added for our DSP behavior:

```c++
pBuf[uFramesProcessed] = pBuf[uFramesProcessed] * AK_DBTOLIN(m_pParams->RTPC.fDummy);
```

This code accesses the buffer `pBuf` at the position of the frame being processed `uFramesProcessed` to retrieve the
value of a sample in the channel being currently processed. This value is then multiplied by our Gain property
`m_pParams->RTPC.fDummy` (accessed through the parameters structure), which we convert to a linear format. This
conversion is needed because earlier we set the parameter to use decibel units (range -96.0 to 10 dB) for convenience to
the user. The converted value decreases (value below 1) or increases (value above 1) the value of the sample, leading to
a softer or louder output gain. We finally assign the new value at the position of the sample in the buffer, and the
process is repeated for all samples of all channels.

We rebuilt the plug-in and demonstrated the result by playing a sound of constant loudness and changing the value of the
Gain: we could observe the result in Wwise Authoring Meter view.