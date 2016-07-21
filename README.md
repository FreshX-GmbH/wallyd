# Overview

The wally daemon is a small daemon used to control wally devices right after start. Its functionality mainly is

* display startup informations in a very early stage of a device startup
* control peripherie of the wally device (i.e. touch panel, inputs, rfid, gpio, sensors)
* display generic data locally acquired by the device
* render remote data (such as webpages) to the display
* render remote data from a wallaby middleware

# Features

* a plugin system (see ./plugin/myPlugin.c) which enables you to easily extend the daemon
* embedded JavaScript engine to easily control the daemon / display
* SDL2 driven (runs on macOS, Linux, Windows)
* can be run as a part of your system (i.e. seperate app, ScreenSaver etc) or as
* standalone wally device (running from flash, SDCard, USB, HDD, network)
* a dedicated wally device can be any linux ready hardware (see https://github.com/FreshXOpenSource/Firmware-builder)
* the standalone wally device firmware is highly robust to power failures / power off as in TV's etc

Projects Wally (TV,Cam,ID,Photobooth) are used in

* customers info panels driven by the wallaby backend
* static info panels
* Wally Photobooth
* CO2/Humidity sensor+display
* Identity devices 

# Hardware supported

* Raspberry Pi 1/2/3
* Linux devices running DRM/KMS
* Mac OS X
* (Windows, not tested)

# Prerequisites

Mac : 
```
brew install libtool libuv sdl2 sdl2_gfx sdl2_image sdl2_ttf autoconf automake ffmpeg
```
Linux:
```
# Fedora : build SDL2 from source (for openGL/raspi support) and add the follwoing dependecies
yum -y install autoconf automake ffmpeg-devel
# Debian : (not tested)
apt-get sdl2 sdl2_gfx sdl2_image sdl2_ttf autoconf automake ffmpeg
```

# To build wallyd run

```
sh autogen.sh
./configure
make && make install
```

# To build in Linux or the firmware either
   
  Install packages : SDL2 SDL2-devel SDL2_image SDL2_ttf SDL2_gfx
  Required but included jsmn (Json parser from https://bitbucket.org/zserge/jsmn/wiki/Home)
  Required but included jsmn-example from https://github.com/alisdair/jsmn-example
  Required but included map_lib from https://github.com/jimlawless/map_lib
  Required but included hashtable from https://github.com/larsendt/hashtable
  Required but included duktape from https://github.com/svaarala/duktape
  Required but included dukluv from https://github.com/creationix/dukluv
- the sdl video plugin requires ffmpeg-devel

# Commands

```
  setpng <file>             - set and display png on mainTexture
  setpngscaled <file>       - set and display png scaled on mainTexture
  settext <x> <y> <text>    - set and display text on mainTexture
  settext2 <x> <y> <text>   - set and display text on mainTexture using 2nd font
  settextrot <x> <y> <rotate> <text> 
                            - set, rotate and display text on mainTexture using 2nd font
  video <filename | url>    - play video from file or url
  log <text>                - copy mainTexture, add logText and display 

  discover                  - start an ssdp discovery
  register [IP] [port]      - (re-)register at discovered server (or for use of IP/Port)
  config                    - get system config from server
  quit                      - quit the server
```
