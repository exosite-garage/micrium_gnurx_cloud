========================================
About uCOS-III-Ex4
========================================
This project is a Micrium uC/OS-III project that implements a VFD (variable 
freqency drive) motor controller simulation for the Renesas RX62N RDK.  It uses 
a PWM signal as the motor drive and uses the on-board ADC to sample the 
resulting sine wave and calculate the frequency.  The project also implements 
cloud connectivity to send and receive data to/from the cloud by using 
Exosite's Cloud Data Platform.  Communication with the cloud is accomplished 
over HTTP.  The project functionality includes writing the frequency value of 
the RDK's on-board PWM motor control to the cloud and reading from a data 
source from the cloud to control the on-board LEDs.  Cloud status can be 
displayed on the RDK's LCD screen by pushing SW1, SW2 or SW3.  This project can 
be re-used, extended and deployed as desired.

Tested with HEW version 4.09.00.007, including:
Compiler KPIT GNURX [ELF] Toolchain v11.02  (7-20-2011)
Renesas Demonstration Kit (RDK) RX62N v3.00 Release 00 (6-15-2011 00:08:55)

========================================
Quick Start
========================================
1) Install the Micrium exectuable from the RDK DVD
* HINT: updates may be avaiable from www.micrium.com

2) Install the KPIT GNURX Toolchain v.11.02 (if not already)
* HINT: download from http://www.kpitgnutools.com/index.php

2) Open workspace "GNURX.hws" and select uCOS-III-Ex4 as the Current Project
* Default directory is: C:\Micrium\Software\EvalBoards\Renesas\YRDKRX62N\GNURX
* NOTE: uC/Probe is disabled by default in this project

4) Compile and download -> check https://renesas.exosite.com to see your data
in the cloud!
* HINT: Your RDK must be connected to the Internet via the RJ-45 ethernet jack
* HINT: If your network does not support DHCP, you will need to set static 
        values

For more information on this project and other examples, reference the online 
documentation at http://tinyurl.com/28cphut

========================================
Release Info
========================================
----------------------------------------
Release 2011-09-1
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
