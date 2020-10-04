# simulation of thermodynamics @ home

Energy2D is a simplified 2D thermodynamic simulation tool which is very easy to use. Visualizing the air and temperature flows.

There are certainly flaws in this simplified data models and the parameters iI used but you get a good impression what (probably) happens. You do not need a supercomputer to get real time simulations on your PC. You can play around with the parameters and get instant feedback.

[Energy2D Website and download Windows/Mac/Linux](https://energy.concord.org/energy2d/)

[A jumpstart tutorial on YouTube](https://www.youtube.com/watch?v=M2kSU06829g)

## classroom with tilted window and significant temperature difference 
![tilted windows classroom enviornment](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/docs/energy2D/slightly_open.gif)

## let`s open the windows 
![open a window](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/docs/energy2D/open_window.gif)
If you open a window there is a lot of turbulence in the room. The warm air escapes trough the windows but still on the inner wall there is less exchange. If you stop the wind there will be only heat exchange near the window but not a lot of airflow.

## cross ventilation
![open a window](https://raw.githubusercontent.com/Christian-Me/healthy-indoors-project/master/docs/energy2D/cross_ventilation.gif)

If you open a window and the door (open to the outside) the maximum air exchange happens. (but still turbulence in the corners causing a part of the air circulating)

## please note

-  The data model is simplified on a 100x100 grid. (There is a 200x200 version available)
- The particle spreader has no parameters that allow to simulate aerosols or droplets
- the color shows temperatures. The airflow can be visualized by arrows (2nd gif)
- all simulations are done with wind! Without everything is much slower
- java applets are blocked by most browsers. So download the desktop app and create your own models. There are a lot of models available through the examples menu
