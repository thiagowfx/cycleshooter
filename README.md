cycleshooter
============

Teaser: [https://vimeo.com/135421333](https://vimeo.com/135421333)

Dependencies
------------

This project uses the following C++ libraries:

- Ogre3d (for rendering)
- SFML (audio, clock, input, threads and other utilities)

If you want to build Ogre or SFML, you must have some dependencies already installed on your system:

On Debian/Ubuntu:
- `sudo apt-get build-dep libogre-1.9-dev libsfml-dev; sudo apt-get install cmake libglew-dev libudev-dev`

On Arch Linux:
- `sudo pacman -S cmake $(expac -S "%E" ogre sfml)`

Building
--------

To build the project:

- `cd <project_root>`
- `mkdir build; cd build`
- `cmake ..`
- `make -j`
- `cmake -DDEPENDENCIES_READY=TRUE ..`
- `make -j`

**Rationale**: the first `make` will check if you have the dependencies already set up.
If not, then they will be automatically downloaded, compiled and installed (by default, to `$HOME/.lib`).
After setting up the dependencies, set the `DEPENDENCIES_READY` variable to `TRUE` then finally build the project.

If you're experiencing slow compile times, try using either `ccache` or `clang`.

Notes
-----

- Binaries are placed on `<project_build_root>/dest/bin` assets are placed on `<project_build_root>/dest`. There is an additional `launch` target to launch the main executable of the project. This target depends on `cycleshooter-launcher`.
- To build the `cycleshooter-launcher` target, you will need the qt5 libraries.
- To add `qmake` to your PATH, do a `export PATH="<gcc_64/bin>:$PATH"`

Rolling release
---------------

    git submodule init; git submodule sync; git submodule update; git submodule foreach --recursive git pull origin master
