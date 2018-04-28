# CfgBus-GUI
Attempt to make a "Generic" interface + gui for my projects. Call it ConfigBus (CfgBus)

More details will follow, but for now:

Cfgbus is a wrapper around ModBus RTU. It allows easy definition of configurable settings/variables
on a slave (this would be you microcontroller project), which can then be accessed and written over
ModBus. 

On the master side, it is possible to access the slave over modbus directly, or to use CfgBus. For the
first it mmust be known at what addresses the variables are placed, and in what byte order.
For the second (CfgBus), CfgBus will automatically detect the configurable settings on the slave, and
retrieve discriptions, addresses and variables types. 

CfgBus-Gui implement a CfgBus master for a COM-Port on a host PC. See screenshot.jpg

Open an issue if you find one or have questions. I will trry to answer asap and get you going.
