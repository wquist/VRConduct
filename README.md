# VRConduct
Simulate conducting a virtual orchestra, with a VR headset, spatialized sound, and instrument visualization. This program allows users to load arbitrary MIDI files and listen, view, and interact with them in VR, adjusting the speed of the players by using a controller like a conductor's baton.

Developed with [nicholascioli](https://github.com/nicholascioli)

![Preview](/img/preview.png)

## Features
* MIDI file support, with sounds generated through SoundFont files.
* Orchestra visualization, with 3D models for most common instruments.
* Conductor's baton simulation in VR.
* Adjust positions of different instruments with spatialized audio.
* Custom OpenGL rendering engine with skybox, shadows, etc.

## Building
Building the program will take a little effort; the program was developed mainly for a demo. Some data has been excluded from the repository due to its size.

Before building, you will need:
* Appropriate orchestral SoundFont (`.sf2`) files.
* MIDI files; some examples are included in the `data/midi` directory.
* Corresponding sheet music for the target MIDI (if desired).

You will also need to expand some of the instrument models, saved as `.zip` files in `data/model`. Further, the open source libraries used have also been excluded. The following must be installed globally or in a project `lib` folder:

`assimp, glew, glfw, glm, openal-soft, openvr, stb, tinysoundfont`

Once these are present, it should be possible to build the project with the included CMake file.
