========================================
About uCOS-III-ExN
========================================
This project contains six Micrium uC/OS-III projects for the Renesas RX62N 
Renesas Development Kit (RDK) system.  Every project includes cloud connectivity
that does the following things:<br>
1) Runs a provisioning routine at startup to ensure the kit is authenticated<br>
2) Writes a "ping" value to the cloud<br>
3) Reads a "ledctrl" value from the cloud to control on-board LEDs<br>
4) Outputs cloud status on the LCD screen when any button held down for >2s<br>
5) Writes other values to the cloud (project specific)<br>

License of all cloud-specific components is BSD, Copyright 2011, Exosite LLC 
(see LICENSE file)<br>

Tested with HEW version 4.09.00.007, including:<br>
Compiler KPIT GNURX [ELF] Toolchain v11.02  (7-20-2011)<br>
Renesas Demonstration Kit (RDK) RX62N v3.00 Release 00 (6-15-2011 00:08:55)

========================================
Project Descriptions
========================================
----------------------------------------
uCOS-III-Ex1
----------------------------------------
* Blinks the LEDs in different patterns<br>

----------------------------------------
uCOS-III-Ex2
----------------------------------------
* Reads from the temperature sensor and accelerometer<br>
* Sends data to the cloud as "temp" and "angle"<br>

----------------------------------------
uCOS-III-Ex3
----------------------------------------
* Runs MuTex, Semaphore and Message Queue tests (TX from one task, RX in other)<br>

----------------------------------------
uCOS-III-Ex4
----------------------------------------
* Implements a VFD (variable freqency drive) motor controller simulation<br>
* Uses a PWM signal as the motor drive and uses the on-board ADC to sample the 
resulting sine wave and calculate the frequency.<br>
* Sends the actual frequency and desired frequency to the cloud as "afreq" and
"dfreq"<br>

----------------------------------------
uCOS-III-Ex5
----------------------------------------
* Micro "HTTPs" webserver<br>

----------------------------------------
uCOS-III-Ex6
----------------------------------------
* Audio playback demo - plays .wav files from SD card /Music folder<br>


========================================
Quick Start
========================================
1) Install the Micrium exectuable from the RDK DVD <br>
* HINT: updates may be avaiable from www.micrium.com

2) Install the KPIT GNURX Toolchain v.11.02 (if not already)<br>
* HINT: download from http://www.kpitgnutools.com/index.php

3) Copy all files (if not already) into the path:<br>
* C:\Micrium\Software\EvalBoards\Renesas\YRDKRX62N\GNURX\<br>

4) Open workspace "GNURX.hws" and select your desired project as the "Current 
Project"<br>
* Default directory is: C:\Micrium\Software\EvalBoards\Renesas\YRDKRX62N\GNURX<br>

5) Compile and download -> check https://renesas.exosite.com to see your data
in the cloud!<br>
* HINT: Your RDK must be connected to the Internet via the RJ-45 ethernet jack<br>
* HINT: If your network does not support DHCP, you will need to set static 
        values<br>

For more information on this project and other examples, reference the online 
documentation at http://exosite.com/renesas<br>

========================================
Release Info
========================================
----------------------------------------
Next...
----------------------------------------
--) Made vendor a parameter for Exosite_Init<br>

----------------------------------------
Release 2011-12-13
----------------------------------------
--) Added vendor information to the provision function<br>

----------------------------------------
Release 2011-11-09
----------------------------------------
--) Move all .Mot files to Download page<br>

----------------------------------------
Release 2011-11-08
----------------------------------------
--) Added Ex1~Ex6 .Mot file for user OOBE download<br>   
--) Micrium added support for an Audio Demonstration kit on Ex6<br>

----------------------------------------
Release 2011-10-27
----------------------------------------
--) minor update to show MAC address on Ex1~Ex5<br>
--) minor update to hide cloud status on Ex6<br>
--) minor update to add key press "SW1+SW2+SW3" for cloud status on Ex6<br>

----------------------------------------
Release 2011-09-19
----------------------------------------
--) added all 6 Micrium Example projects<br>

----------------------------------------
Release 2011-09-14
----------------------------------------
--) minor updates to support latest Micrium RX release<br>

----------------------------------------
Release 2011-09-11
----------------------------------------
--) must push switch buttons to display cloud status<br>

----------------------------------------
Release 2011-09-05
----------------------------------------
--) added auto device provisioning<br>
--) abstracted library interface<br>

----------------------------------------
Release 2011-08-31
----------------------------------------
--) initial cloud-enabled version<br>
