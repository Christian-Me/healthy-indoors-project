# how to build your own firmware

## project organization 

The project currently knows tree different nodes

- **sensor nodes** containing one or many sensors
- **monitor nodes** nodes to process and display the data
- **bridges** nodes to act as bridges between network protocol and typologies

The functionality of a node is not limited. So for example a monitor node can carry a sensor too or a bridge a display. Even an all in one node is possible. Every node project is a own folder in this project. Ideally it only carries the code necessary for this special node and uses the provided libraries for all the other stuff. 
The build process is controlled by build environments and build flags so functions can be switched on or off by simply commenting or un commenting lines.

## Integrated Development Environment IDE

This project use VSCode together with platform.io as development environment. Instead of using the arduino IDE vs code brings a lot of advantages:

- **One editor for many languages:** The Editor can be used not only for programming micro controller in C/C++ the same environment can be used for Python, JavaScript, PHP, HTML, CSS and many other languages
- **Code Completion** VSCode makes suggestions when you type. This is based not only on dictionaries but also on language structures
- **Linter:** VSCode runs syntax checks and even a compiler in background as you type. So may mistakes or typos are quickly marked before compiling.
- **git and github integration:** Seamless integration of many git and github tools for code versioning & collaboration via github
- **Professional organization:** VSCode brings a professional code organization with standardized file types and structures.
- and many many more features through thousands of extensions.

As a compile / make / build / upload and debug extension platform.io is used. It integrates many hundred of different micro controller, frameworks and boards. This awesome project helps to setup and maintain your build environment together with a in circuit debugger.

Many project like this are using the arduino IDE. This IDE is great as it tricks the newcomer into coding for micro controller in C/C++ by providing a simple to use IDE and simply not telling the user that he is about to learn C/C++. But the Arduino IDE has it's limitations. The code can be interchanged between vscode and Arduino IDE but many adaptations and reorganization has to be done.

The IDE should not be mixed with the framework. A framework organize all the features of different micro controllers in an standard way to do the heavy hardware related lifting. The arduino framework is beside espressive IDF the most popular framework. Many tools and libraries run with the arduiono framework.

## the libraries

The project contains three libraries types providing APIs for certain tasks

- **sensors** these Libraries start with an "s_" like **s_bme680** and interface between the sensor hardware or library and the project
- **communication** these libraries start with "c_" like **c_espnow** interface between the network and the project
- **monitor** these libraries start with "m_" like **m_epaper** to display sensor data

Beside that some other libs exists

- **chart.h** displaying charts on matrix displays
- **color.h** color conversion and calculations
- **ui.h** simple user interface
- **untils.h** misc utilities functions

The following standard libraries are currently used and can be installed by the platform.io library manager (if not automatically installed)

- **Button2** handle button presses, de-bouncing and long- or multiple button presses.
- **GxEPD2** including **Adafruit GFX** and **Adafruit BusIO** to interface with e-paper displays (could perhaps streamlined together with TFT_eSPI)
- **TFT_eSPI** to interface with TFT displays, touch screens and perhaps e-paper displays
- **Adafruit NoPixel** driving addressable RGB and RGBW LEDs