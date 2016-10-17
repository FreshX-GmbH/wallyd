# Overview

The wally daemon is a small daemon used to control wally devices right after start. Its functionality mainly is

* display startup informations in a very early stage of a device startup
* control peripherie of the wally device (i.e. touch panel, inputs, rfid, gpio, sensors)
* display generic data locally acquired by the device
* render remote data (such as webpages) to the display
* render remote data from a middleware to the display

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
* Linux devices running DRM/KMS (ATI,Nvidia, Intel, VMWare)
* Mac OS X
* (Windows, not tested)

# Prerequisites

Mac : 
```
brew install libtool libuv sdl2 sdl2_gfx sdl2_image sdl2_ttf cmake ffmpeg libcurl
```
Linux:
```
# Fedora : build SDL2 from source (for openGL/raspi support) and add the follwoing dependecies
yum -y install cmake libcurl-devel ffmpeg-devel
# Debian : (not tested)
apt-get sdl2 sdl2_gfx sdl2_image sdl2_ttf curl ffmpeg
```

# To build wallyd run

```
sh autogen.sh
./configure
make && make install
```

# To build in Linux or the firmware either
   
* Install packages : SDL2 SDL2-devel SDL2_image SDL2_ttf SDL2_gfx
* Required but included jsmn (Json parser from https://bitbucket.org/zserge/jsmn/wiki/Home)
* Required but included jsmn-example from https://github.com/alisdair/jsmn-example
* Required but included map_lib from https://github.com/jimlawless/map_lib
* Required but included hashtable from https://github.com/larsendt/hashtable
* Required but included duktape from https://github.com/svaarala/duktape
* Required but included dukluv from https://github.com/creationix/dukluv
* the sdl video plugin requires ffmpeg-devel

# Commands

Built-In system commands : 

```   sys::quit
   sys::setDebug
   sys::debug
   sys::sleep
   sys::loadPlugins
   sys::callback
   sys::info
```
Besides this, each of the plugins exports various commands into the wallyd namespace as well as into the ecmascript space
See the plugins for more info

