# RS Broken Media

Generates new stereo tape speeds, CD skipping patterns, and distortion effects on every pulse of an adjustable clock. Probabilibies and intensities of these effects are controlled by the "Analog FX," "Digital FX," and "Distortion FX" knobs. The "Repeats" knob controls both the number of subdivisions of the buffer, as well as the number of repeats for those subdivisions.

Dropdowns offer bitcrushing/saturation modes for the "Distortion FX" knob, as well as global codec and downsampling options. Codecs currently include "μ-law" nonlinear 8-bit and the [GSM 06.10](https://quut.com/gsm/) cell phone codec.

![Plugin user interface with a row of 3 primary knobs (analog, digital, and distortion FX); a row of 4 secondary knobs (clock rate, buffer length, repeats, and wet/dry); and dropdowns at the bottom for changing distortion type, codec, and sample rate](https://github.com/reillypascal/RSBrokenMedia/assets/94489575/e89a9f13-777b-4a0e-8ec0-9c5e29a5f5d5)

## Build Dependencies:
- JUCE (https://juce.com/download/)
- Projucer (https://docs.juce.com/master/tutorial_new_projucer_project.html) (for creating Xcode/Visual Studio projects or Linux Makefiles in order to build)

<!--## Windows:
- Compiled Windows files are available under "Releases". Unzip the files and place them in 
	- C:\Program Files\Common Files\VST3 (VST3)
	- C:\Program Files\Common Files\Avid\Audio\Plug-Ins (AAX) 
-->
## macOS:
- Compiled macOS files are available under "Releases".
- You will likely need to disable Gatekeeper for the plugins. To do this for AU, open Terminal.app and type...
```sh
spctl --add "/Library/Audio/Plug-Ins/Components/RSBrokenMedia.component"
```

...for VST3...
```sh
spctl --add "/Library/Audio/Plug-Ins/VST3/RSBrokenMedia.vst3"
```

<!--...or for AAX...
```sh
spctl --add "/Library/Application Support/Avid/Audio/Plug-Ins/RSBrokenMedia.aaxplugin"
```
-->
- You can also add the file path by typing...
```sh
spctl --add 
```

...(with a space at the end) and dragging the plugin file into the terminal, which will automatically add the file path.

## Linux/Windows:
Compiler targets are available for Linux and Windows. Set up JUCE on your computer, open the .jucer file in the Projucer, generate the Linux Makefile or Visual Studio project, and then you can compile the plugins.
