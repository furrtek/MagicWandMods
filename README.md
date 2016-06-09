# EMWAnalog
Analog-in (CV) firmware mod for the "Europe Magic Wand".

## Teardown
Be sure to unplug the EMW and wait a few minutes before taking it apart.

The EMW is held together by 3 big phillips screws:
* On the back, close to the power cable, hidden by a plastic plug (hard to pull).
* Under the buttons silicone strip (just glued, can be lifted with a flat screwdriver).
* On the metal ring (visible).

Inside, the PCB is held in place by a central screw and two other ones used to fasten the power cable.

The PCB needs to be pulled out with a bit of effort because of the silicone joints.

Be careful about the flex cable going to the buttons on the back of the PCB, gently lift the black part of the connector to release it.

The motor should come loose out of the casing.

## Adding the input
TODO

## Programming
Locate the 4 programming points on the underside of the PCB:

![EMW](/pcb.jpg)

Connect a SWIM programmer (they're around $4 on eBay) to the annotated points.

Start STVP, select STLINK as the programmer and STM8S003F3 as the device.

Go in the OPTION BYTE tab. Make sure ROP is set to OFF and AFR0 to "Port C6 Alternate Function". Hit program, this will reset the protection and wipe the original firmware.

Back in the PROGRAM MEMORY tab, load main.ihx and hit program.

## Usage
Individually, the "+" and "-" buttons work the same way.

The speed steps should also be the same as the original ones.

Keeping both the "+" and "-" buttons pushed at the same time for more than a second will toggle between the regular mode, and the analog-in mode.

In the analog-in mode, the motor speed is multiplied by the amplitude of the input signal (so the speed adjustment still works).

Enjoy :3

## Disclaimer
I'm not responsible if you fuck everything up. Be careful, this isn't your mom's dishwasher.

This wasn't tested on animals (yet).
