gb-test
=======
The objective of this project is to aim to create a Game Boy game with the help of modern tools and generators. It is written using the GBZ80 Assembly flavor implemented by RGBDS.

Building
========
This project consists in a toolkit written in C++14 to generate assembly files for the resources (tilesets, maps and sprites as of writing), as well as the assembly files for the source code of the game. The included *Makefile* makes sure all the files are properly built. However, it depends on a working *g++** that supports C++14 and that the path for the RGBDS binaries be properly included in PATH, as well as on Tiled to export the maps in *.tmx* format to a more readable *.json* format.

To properly generate the game's ROM:

1. Modify the *Makefile* to make the variable `TILED` point to the correct location of your Tiled executable. Replace the line that reads like this:

```
TILED := <<change me>>
```

2. Run `make` (or `make all`). The ROM generated will be stored in the file *bin/game.gb*

3. [OPTIONAL] You can automate the build-and-run step by configuring an emulator in the variable `GBEMU` and typing `make run`, that will build the game *and* run it for you.

Editing the map files
---------------------
The Tiled map files included in the _tiled-maps_ folder can be edited; however, the Remex-style autotiles used in the editing must be previously generated by running
```
./gen-autotiles.sh
```
Otherwise, you risk failing to open the map files _and_ corrupting them.

We are running in a limited plataform, so unfortunately there are some expectations in the map generation. Currently, you can only use a single tileset and at max two autotiles, provided the combined tiles generated by all of them (47 for each autotile plus the tileset count) do not exceed 127. More documentation in the limitations of the maps is in the way.

Contributing
============
All contribution is welcome. If you have interesting things to add, please submit a pull request!

Credits
=======
All files in this repository, with exception of *nlohmann/json.hpp*, all LodePNG files and the files in the *modified-remex* folder are Copyright (c) 2019 João Baptista de Paula e Silva and are under the MIT license

The autotile generation scripts are a modified version of the original Remex scripts that can be found [here](https://app.assembla.com/spaces/rpg-maker-to-tiled-suite/subversion/source), and are under the MIT license by Rastagong Librato.

Niels Lohmann's "JSON for Modern C++" library can be found [here](https://github.com/nlohmann/json) and is also under the MIT license.

The LodePNG files belong to Lode Vandevenne and can be found [here](https://lodev.org/lodepng/) under the Zlib license.
