#  Healthy Indoors project (V0.0.2 ALPHA)

![monitor node](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/monitor-01.jpeg)

## version 0.0.2

This version features a TFT monitor node (based on the e-paper version) giving more colorful charts

![monitor node](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/monitor-03.JPEG)
even if TFT displays are not the best choice for battery powered nodes the small 1.14" TFT is nice (Note: The CO2 figures are CO2 equivalents by the VOC-Sensor - they are not really usable in the current setup)

[changes and improvements you find in the changelog](changelog.md)

[general information about the software development environment](./docs/software.md)

<!-- START doctoc -->
<!-- END doctoc -->

## introduction

I started this project in September 2020 as a reaction of the global COVID-19 pandemic.

The need of an **as healthy as possible** indoor environment became necessary as it became obvious that [the virus is likely to be able to spread via aerosols](https://www.who.int/news-room/commentaries/detail/transmission-of-sars-cov-2-implications-for-infection-prevention-precautions) which could stay longer in the air and travel further inside rooms as droplets. 

Reading this study by [Risk assessment of aerosols loaded with virus based on CO2-concentration](https://blogs.tu-berlin.de/hri_sars-cov-2/wp-content/uploads/sites/154/2020/08/hartmann_kriegel_2020_en_v3.pdf) and many other publications and working in an open space office environment I was wondering if it is possible to develop indicators for the employees to be aware of the risk and take action. As humans are not good in sensing CO2 levels (or other gas concentrations affecting the air quality) sensors could be a possible way to help.

[Lately I was pointed to this Video](https://youtu.be/u2rNX2T2Hew?t=364). **This made me aware that other factors (humidity, temperature, particles) are important to factor in.** because they effect the *Virus Health (survivability)* and *Immune System health*. It could be important not only to concentrate on CO2 levels as an indicator to reduce the potential concentration COVID-19 (or other) Viruses but also on factors which maybe reduce the survivability of the virus and (perhaps most important) the immune system health of the occupants of a building. We could perhaps raise the risk of an infection if we do too much ventilation(?) and lower the temperature and weaken our immune system?

As we do not have to fit a [square container in a round canister](https://spacecenter.org/apollo-13-infographic-how-did-they-make-that-co2-scrubber/) when CO2 levels are rising. We can simply open the windows. In the beginning of the colder period and higher risk of infections of any kind the question is when, for how long. Second if the air exchange is sufficient. Mid of September Europe is seeing a second wave rising and as the outside temperatures will fall soon the need of an air exchange perhaps must be indicated due to the discomfort of cold air. Noisy street sounds might also prevent people from opening the windows.

Even without the current covid-19 crises the [health risks factors in indoor environments](https://www.euro.who.int/en/health-topics/environment-and-health/air-quality/publications/2015/the-school-environment-policies-and-current-status) should be addressed. Not all offices and nearly no schools have mechanical air ventilation systems (100% fresh air and no air circulation). 

["A new international day to celebrate clean air – and a sustainable recovery from COVID-19"](https://www.euro.who.int/en/health-topics/environment-and-health/air-quality/news/news/2020/9/a-new-international-day-to-celebrate-clean-air-and-a-sustainable-recovery-from-covid-19)

## disclaimer

This project do not claim to prevent anybody form dangerous diseases or other harm. Stay distant, wear masks, perform any possible hygiene measures, think about possible ways to avoid long stays indoors, like work from home, remote schooling.

![concept diagram](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/graphics/concept-01.svg)

## the Idea

- develop a sensor network to detect accumulation of CO2 or other indicators like Humidity, Tempearture and Particle Concentration
- use multiple sensors to survey larger rooms, find "dead spots"
- Survey the effects of mechanical ventilation, air filters and other methods to improve air quality
- make the project open source that
   - others can participate
   - others can contribute
   - share data and experiences
- raise the awareness of risks in indoor environments
- learn how to improve the air quality and lower the risks with different methods
- test if "self-measured" values can help in communication air quality and perhaps behavior change

## the audience

- everybody who have to stay, study or work in indoor environments
- students who like to build sensor networks, code, analyse the data and perform experiments
- educators who like to teach various aspects form thermo dynamics, electronics, computer science, design, architecture, HVAC systems ... 
- students to get experiences in working on a project remotely using digital techniques like github to exchange ideas and knowlage across borders
- architects and engineers to design environments with better air quality in mind

## the project components

Nothing in this project is really new or innovative. Perhaps the local network idea is a little bit unusual as it does not need an access point as WiFi does. The goal is to establish an ad-hoc network with no setup. Simply place nodes in a room. All nodes in reach can talk to each other and exchange measurements. 

- **sensor nodes** sending environmental sensor readings onto the network
- **monitor nodes** Displaying sensor data on siple (RGB) LEDs, LED arrays, LCD, TFT or e-paper Displays. Monitor nodes can include sensors too.
- **bridge nodes** connecting the local sensor network to other networks like WiFi

### the micro-controller

![wemos-d1](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/wemos-d1.JPEG)

To start the ESP8266 (or ESP32) is used as it is a cheap and powerful micro-controller with 2.4GHz wireless transceiver build in. Other micro-controllers like Arduino (Atmel) or STM32 are possible but will need additional components for RF-Transmissions

### the network

Several networks and protocols can be used:

- **[esp-now](https://www.espressif.com/en/products/software/esp-now/overview)**: Access point / router free peer to peer ad-hoc network protocol for ESP8266/32 without additional components necessary (default protocol for this project)
- **NRF24L01**: 2.4GHz transceiver modules
- **[LoRa-Wan](https://de.wikipedia.org/wiki/Long_Range_Wide_Area_Network#Klasse_A)**: Long Range Wide Area Network - need of a gateway (public gateways are available). The monitor node can take the role of a gateway and create a local wireless network (to be investigated) and act as a bridge node too.
- **Wifi**: access point is necessary. An ESP8266/32 monitor node can take toe role of an software-AP
- **Bluetooth LE**: Simple data broadcasts over BLE should be possible (ESP32 only)

As esp-now runs natively on ESP8266 or ESP32 it seams like the ideal network protocol to start. The bridge node can connect esp-now to wifi or other networks and protocols

### sensor node

![wemos-d1](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/sensor-01.JPEG)

The simplest sensor node reads a connected gas sensor and broadcast its readings onto the network.

A Sensor node can also includes a monitor function to display its own sensor reading or if in a broadcast configuration readings of the other sensors too.

some interesting gas sensors (CO2 and VOC)

- **BME680** is an sensor by [Bosch Sensortec](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors-bme680/) especially for indoor air quality measurements. Instead of measuring CO2 levels it measures volatile organic compounds (VOC). Not only harmful substances like formaldehyde, lacquers and others but also substances exhaled by humans.

  [interesting video about sensing VOC instead of CO2](https://www.youtube.com/watch?time_continue=1831&v=1niOmSsF-tk&feature=emb_logo)

  all tests in the current version of the project are done with three BME680
![BME680](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/BME680.JPEG)
- **Senseair S8** promising infrared CO2 sensor from [Senseair](https://senseair.com/products/size-counts/s8-residential/) 400 to 2.000 ppm. (currently on delivery)

- **MH-Z19B** Chinese CO2 infrared Sensor from [Winsen-Sensors](https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19b.html) 0 to 2.000 ppm.

- **CCS811** VOC sensor from [Sciosense](https://www.sciosense.com/products/environmental-sensors/ccs811-gas-sensor-solution/) seems interesting for battery operation as the sensor has a mcu integrated for VOC and eCO2 calculations providing a wake from deep sleep signal on programmable threshold alarm. **This sensor does not have a temperature and humidity sensor included! For more precise measurements a BME280, BME680 or similar should be included.**

### monitor nodes

![rgb-monitor](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/images/monitor-02.JPEG)

one or more monitor nodes can join the sensor network to display the air quality. Form a simple LED to RGB led strips, LCD, TFT, OLED or e-paper displays up to an alarm horn.

- A e-paper version is shown on top of the file. e-paper displays are attractive as they are easy to read and keep their contend during deep sleep.

- addressable RGB(W) strips or rings can display the sensor readings in a thermometer or gauge style. A RGB LED can be connected to every sensor node. Addressable RGB chips are preferred as they do not need further components like resistors and keep their state even if the mpu goes into deep sleep to save battery power.

- Small TFT or OLED displays are included in many ESP8266/32 boards. The M5Stack modules come with a TFT display and a attractive case and small LiPo battery

- ESP32 powered user programmable smart watches are available.

- other Monitor solutions can be implemented via bridge nodes

### bridge node

To connect the peer to peer network bridge nodes can join the network. A limitation of esp-now currently is that it does not coexists with WiFi at the same time. Currently that problem is solved by a *esp-now to serial* bridge. It could be solved by switching between esp-now and Wifi with the problem of missing data when switched to the other network.

- *serial2http bridge* The mcu provides a web server for displaying the data on mobile phones or desktop browsers

- *serial2mqtt bridge* (serial2homie) The mcu publishes the sensor data to a MQTT broker. As MQTT does not depend on a special data structure the [homie convention](https://homieiot.github.io/) is used to provide a structure how the sensors announce their services and data.

  Live data of my 3 BME680 are published on **mqtt://broker.hivemq.com:1833** topic: **healthy-indoors-project/#**

- other bridge nodes can be implemented like *serial2thingspeak* or *serial2blynk*

### data analyze

Using a **bridge node** and the mqtt protocoll the sensor data can by analyzed or visualized. The default tool should be [Node-RED](https://nodered.org)

![Node-RED dashboard](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/graphics/node-red-dashboard-01.png)

## project costs

All components are selected with a price point in mind that everybody can participate. Depending where (China or local) you buy the cost can be quite different. Local delivery can be 2-3x higher than orders form China including shipping. (detailed information on components and where to buy could be found in the near future)

- sensor node (8€-25€ BME680)
- monitor node (6€-20€ TFT / 18-30€ e-paper)
- bridge node (4-8€ 2x Wemos D1 mini)

## status of the project

The project is in a **proof of concept** stage. The following questions should be answered to progress:

- is the concept feasible and useful?
- how to interprete the the sensor data?
- how can the project implement in real world environments?
- how to communicate about air quality?

The next steps are

- design and 3D-Print cases for the nodes to take them out into the "real world"
- test nodes in office environments
- test nodes in classrooms
- develop battery powered nodes with suitable battery life
- develop a database solution and data analyze
- detect "window open" state by using window sensors or perhaps the build in temperature readings of most sensors could be used
- address security risks like encryption, authentication
- pairing functionality between nodes for multi room use

## participation and contributions

**The project is seeking for any participation as this is it's main goal!**

- test implementations
- school projects
- scientific advice
- help in any kinds of hardware development. Perhaps producing custom PCBs
- help in software development
- help in addressing security concerns
- any Ideas
- support of any kind
- [...]

## conclusion

As mentioned above this project can only be a small part in fighting against the covid-19 pandemic. Other measures might be more effective but I have a strong believe that there isn't a single solution but many small actions can help flattening the second curve as it is already rolling in.

Feel free to contact me (cmeinert@gmx.net) or use a github issue to enable others to participate.