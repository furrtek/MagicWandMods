# EMWAnalog
Analog-in (CV) firmware mod for the "Europe Magic Wand"

## Teardown
Be sure to unplug the EMW and wait a few minutes before taking it apart.

The EMW is held together by 3 big phillips screws:
* On the back, close to the power cable, hidden by a plastic plug (hard to pull).
* Under the buttons silicone strip (just glued, can be lifted with a flat screwdriver).
* On the metal ring (visible).

Inside, the PCB is held in place by a central screw and two other ones pinching the power cable.
The PCB needs to be pull out with a bit of effort because of the silicone joints. The motor should come loose out of the casing.

## Adding the input
TODO

## Programming
Locate the 4 programming points on the underside of the PCB:
![EMW](/pcb.jpg)

Connect a SWIM programmer (they're around $4 on eBay) to the annotated points.
Start STVP, select STLINK as the programmer, STM8S003F3 as the devicex.
Go in the OPTION BYTE tab, make sure ROP is set to OFF and hit program, this will reset the protection and wipe the original firmware.
Back in the PROGRAM MEMORY tab, load main.ihx and hit program.

Enjoy :3
