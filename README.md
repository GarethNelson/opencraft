# OpenCraft

This is an attempt to rewrite minecraft as free software, currently a work in progress so don't expect it to work properly yet.

It is split into 2 components (and some random old legacy stuff) named after similar components from Second Life because i'm nostalgic as hell:

+ newsim: the server component, needed even for single player games

+ newview: the client component, this is what you run if you want to actually play the game

All game logic is handled by the server whenever possible, so both components are required to run the game.

## Build instructions - server (newsim)

First ensure you have the relevant dependencies installed, newsim requires the following C++ development packages that should be available on any modern GNU/Linux distro:

* CMake version 2.8.3 or later
* Python development headers - version 2.7 or later
* Boost version 1.55 or later
* A working C++ compiler with C++11 support - developed & tested on gcc 4.9.2-10 from Debian Jessie

Additionally the python code needs the following at present:

* pip (for installing the other packages)
* twisted version 13.0.0 or later
* python-cryptography version 0.9 or later
* PyOpenSSL version 0.15.1 or later
* Python service_identity version 14.0.0 or later
* quarry version 0.6 or later: see https://github.com/barneygale/quarry

You should use pip to install all python packages except quarry.

## Build instructions - client (newview)

Note: heavy work in progress, the below may be out of date

The client requires the following packages in addition to what the server requires:

* SDL2
* OpenGL development headers
* PhysFS

## Running the server

The server daemon by default binds to all interfaces on port 25565 and must be available (or a compatible minecraft server) before starting the game.

To run newsim the normal way as a daemon (will be the usual case once the project hits a stable release):
1. cd newsim/build/
2. ./newsim -r..

Log files will be stored in newsim/build/ by default so you can see what the daemon is doing by monitoring that directory.

It is recommended however to run in debug mode and not detach from the console - this allows you to see better what's going on.

To run newsim in debug mode without detaching:
1. cd newsim/build/
2. ./newsim -r.. -fd

Further usage options can be seen by passing the -h parameter to newsim.

You must use the -r param to inform newsim where to find the install root (which is currently simply the source checkout location, eventually once ready for packaging it will be somewhere in /usr/local/share/opencraft or equivalent).

## Running the client

First you must make sure you have an appropriate texture pack. Due to copyright law it is NOT legal to distribute the textures and sprites from the original minecraft so instead a freely available texture pack named OSReP is included in newview/texture_packs/osrep/. Please see the bottom of this document for information on the license for this texture pack.

If you are happy to use the default texture pack you can simply start the server as indicated above and then start newview in a similar manner:

1. cd newview/build/
2. ./newview -r..

As with newsim, the -r option simply tells the client where the install root is and should point to the source checkout location for newview.

If you would like to use an alternate texture pack you must specify it on the command line using the -t parameter after installing it into newview/texture_packs/, for example to use a texture pack named "mypack" you would do this:

./newview -r.. -tmypack

## Format of texture packs

Texture packs mostly share the same format as minecraft texture packs with a few key differences:

1. Inside assets/ the subdirectory may be named either opencraft or minecraft
2. The texture pack may be a .pk3 file instead of a directory

pack.mcmeta files may be included, but if not found everything will still work.

## Thankyous, license notices, attributions and such

OSReP is included in this repo as the default texture pack.
The license for OSReP is Creative Commons Attribution Sharealike for the block textures, "no stated restrictions" for those classed as "doku-stuff", creative commons attribution sharealike for item sprites and an informal "reuse so long as you credit DataErase" for the music tracks.

Please see the images in texture_packs/osrep/credits for full details on OSReP.

Additionally, i'd like to thank some of the guys on freenode #mcdevs and especially those listed in the CONTRIBUTORS file for helping with specific issues.

## FAQ

Q: Why?

A: Because minecraft is an awesome game and is increasingly being used for non-game purposes and I believe it requires a free software implementation.

Q: Is bacon delicious?

A: Yes

