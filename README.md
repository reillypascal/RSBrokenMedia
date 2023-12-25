# RS Broken Media

Generates new stereo tape speeds, CD skipping patterns, and distortion effects on every pulse of an adjustable clock. Probabilibies and intensities of these effects are controlled by the "Analog FX," "Digital FX," and "Distortion FX" knobs. The "Repeats" knob controls both the number of subdivisions of the buffer, as well as the number of repeats for those subdivisions.

Dropdowns offer downsampling and codec options. Codecs currently include nonlinear 8-bit "μ-law" and the [GSM 06.10](https://quut.com/gsm/) cell phone codec.

![Screenshot of the user interface, showing a row of 3 primary knobs, a row of 4 secondary knobs, and dropdowns at the bottom for changing codec and sample rate](https://github.com/reillypascal/RSBrokenMedia/assets/94489575/614e155d-10ee-44f1-826a-9dbfd1671fdb)

## Build Dependencies:
- JUCE (https://juce.com/download/)
- Projucer (https://docs.juce.com/master/tutorial_new_projucer_project.html) (for creating Xcode/Visual Studio projects in order to build)

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