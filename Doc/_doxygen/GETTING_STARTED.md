# Getting started

These instructions will get you a copy of the project up and running on your local machine.

## Tested on

* Microsoft Visual Studio version 2019.
* Windows Driver Kit (WDK) 10.0.19041.685.
* Windows Software Development Kit (SDK) 10.0.19041.685.

## Building

* Clone the project.
```bash
git clone https://github.com/PI-Defender/pi-defender.git
```
* Load PI-Defender.sln with Visual Studio.
* Make sure the configuration manager is set to x64 (Release).
* Build the solution (Crtl + Maj + B).

## Installation
1. The driver uses test signing, so before using it Windows should be set to install and run test signed drivers. 
	Enable test signed drivers can be done with the following steps:
	* *Reset this PC* (search bar)  
    * *Troubleshoot*
    * *Advanced Options*
    * *Startup repair*
    * Select the number **7** to disable the driver signature enforcement.

	Windows will restart for changes to take effect.  
2. Install the driver with the user-mode service.
   ```bash
   PI-Defender_UM.exe install
   ```
   To ensure a smooth installation, *PI-Defender_UM.exe*, *PI-Defender.sys* and *PI-Defender_MsgFile.dll* must be in the same folder before running the previous command.
If everything went well, you should have a success message.

## Start / Usage

```
$ PI-Defender_UM.exe [install,start,query,stop,delete]

Protect your system from process injection.

Actions:
	install		Install the service User-Mode and the Driver kernel.
	start		Start the service User-Mode and the driver kernel.
	query		Query informations about the service and the driver.
	stop		Stop the service user-mode and the driver.
	delete		Delete the service user-mode and the driver.

```
