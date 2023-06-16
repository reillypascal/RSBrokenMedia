# RS Broken Media

Generates new stereo tape speeds, CD skipping patterns, and distortion effects on every pulse of an adjustable clock. Probabilibies and intensities of these effects are controlled by the "Analog FX," "Digital FX," and "Distortion FX" knobs.

Dropdowns offer downsampling and codec options. Codecs currently include [Émilie Gillet's](https://github.com/pichenettes) open-source "μ-law" 8-bit implementation, with the GSM 06.10 cell phone codec to come.

## Build Dependencies:
- JUCE (https://juce.com/download/)
- Projucer (https://docs.juce.com/master/tutorial_new_projucer_project.html) (for creating Xcode/Visual Studio projects in order to build)

<!--## Windows:
- Compiled Windows files are available under "Releases". Unzip the files and place them in 
	- C:\Program Files\Common Files\VST3 (VST3)
	- C:\Program Files\Common Files\Avid\Audio\Plug-Ins (AAX) 
-->
## macOS:
- Compiled macOS files are available under "Releases". Unzip the files and place them in 
	- Macintosh HD/Library/Audio/Plug-Ins/Components/ (AU)
	- Macintosh HD/Library/Audio/Plug-Ins/VST3/ (VST3)
<!--	- Macintosh HD/Library/Application Support/Avid/Audio/Plug-Ins (AAX) -->
- You will likely need to disable Gatekeeper for the plugins. To do this for AU, type...
```sh
spctl --add "/Library/Audio/Plug-Ins/Components/RSAlgorithmicVerb.component"
```

...for VST3...
```sh
spctl --add "/Library/Audio/Plug-Ins/VST3/RSAlgorithmicVerb.vst3"
```

<!--...or for AAX...
```sh
spctl --add "/Library/Application Support/Avid/Audio/Plug-Ins/RSAlgorithmicVerb.aaxplugin"
```
-->
- You can also add the file path by typing...
```sh
spctl --add 
```

...(with a space at the end) and dragging the plugin file into the terminal, which will automatically add the file path.