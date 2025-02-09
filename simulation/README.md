# Simulator project for LittlevGL embedded GUI Library

This a fork by copy directory from https://github.com/lvgl/lv_port_pc_eclipse.git based on tag 7.4
The idea is this modified version work with the rest of project
Here is english as not motivation to write something explicit

## How to use this one

* Run the [setup](setup_build.sh) script as it will
  * Clone dependant repository branch tag
  * Make link to source code of display
  * Run the Makefile to build the simulator
  * Launch the demo executable at end

The the VSCode can use directly the build and debugger as per debug setup.
This is useful to catch internal issue with ui components

The simulator is a small part of real feature on the embedded board
The source code is made using define tag to run on board or on the simulator.

<hr/>
## Original README content


The [LittlevGL](https://github.com/littlevgl/lvgl) is written mainly for microcontrollers and embedded systems however you can run the library **on your PC** as well without any embedded hardware. The code written on PC can be simply copied when your are using an embedded system.

Using a PC simulator instead of an embedded hardware has several advantages:
* **Costs $0** because you you don't have to buy design PCB
* **Fast** because you don't have to design an manufacture PCB
* **Collaborative** because any number of developers can work without hardware requirements
* **Developer friendly** because much easier and faster to debug on PC

## Requirements
* The PC simulator is cross platform.  **Windows, Linux and OSX** are supported, however on Windows it's easier to get started with a [another simulator](https://github.com/littlevgl/lvgl/#quick-start-in-a-simulator) project. 
* **SDL** a low level driver library to use graphics, handle mouse, keyboard etc.
* This project (configured for **Eclipse CDT IDE**)

## Usage

### Get the PC project

Clone the PC project and the related sub modules:

```
git clone --recursive https://github.com/littlevgl/pc_simulator_sdl_eclipse.git
```

or download it as zip [here](https://github.com/littlevgl/pc_simulator_sdl_eclipse/archive/dev-6.0.zip).

### Install SDL
You can download SDL from https://www.libsdl.org/

On Linux you can install it via terminal:
```
sudo apt-get update && sudo apt-get install -y build-essential libsdl2-dev
```

### Install Eclipse CDT
Download and install Eclipse CDT from  http://www.eclipse.org/cdt/

### Import the PC simulator project
1. Open Eclipse CDT
2. Click **File->Import** and choose **General->Existing project into Workspace**
3. Browse the root directory of the project and click Finish
4. Build your project and run it

## Docker
1. Build the docker container
```
docker build -t lvgl_simulator .
```
2. Run the docker container
```
docker run lvgl_simulator
```
GUI with docker is platform dependent. For example, on macOS you can follow 
[this tutorial](https://cntnr.io/running-guis-with-docker-on-mac-os-x-a14df6a76efc) 
and run a command similar to:
```
docker run -e DISPLAY=10.103.56.101:0 lvgl_simulator
```

For Linux environments with X Server, the following will the `docker run` command. Note that the first command, `xhost +` grants access to X server to everyone.

```
xhost +
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix:ro -t lvgl_simulator
```

## Contributing
1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request!

If you find an issue, please report it via GitHub!

## Donate
If you are pleased with the graphics library and found it useful pleas support its further development:

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LWHHAQYZMRQJS)
