# UDSim

UDS Simulator and Fuzzer

## Unified Diagnostic Services Simulator

The UDSim is a graphical simulator that can emulate different modules
in a vehicle and respond to UDS request.  It was designed as a training
tool to run alongside of ICSim.  It also has some unqiue learning
features and can even be used to security test dealership or other
tools that plug into the OBD dongle

NOTE: This is a complete rewrite of [uds-server](https://github.com/zombieCraig/uds-server).

![alt text](https://github.com/zombieCraig/UDSim/blob/master/data/ss.png "UDSim Screenshot")

## Capabilities

UDSim takes the following command line options:

```
  Usage: udsim [options] <can-if>
     -c <config file>    Configuration file for simulator
     -l <logfile>        Parse candump log to generate learning DB
     -f                  Fullscreen
     -v                  Increase verbosity
```

The only required argument is the CAN interface.  There are some example CAN initialization scripts
in the src folder for setting up canX and vcanX interfaces if you need them.

UDSim as three modes:  Learning, Simulation and Attack

### Config File Syntax & Parameters

Each file is a plain text file divided into sections, with configuration entries in the style
key=value. Whitespace immediately before or after the "=" is ignored. Empty lines and lines
starting with ";" are ignored, which may be used for commenting.

[UDS_ID] - Section delimeter; UDS_ID is a hex CAN ID; each section defines a UDS server or client.

pos - x,y coordinates in GUI to anchor this ECU graphic element

responder - set to '1' if UDS server or '0' if UDS client

positiveID - the arbitration ID to be monitored for positive UDS responses

negativeID - the arbitration ID to be monitored for negative UDS responses

{Packets} - Sub-section delimeter; Packets is a constant string; following lines contain observed packets formatted in the [can-utils way (see `parse_canframe`)](https://github.com/linux-can/can-utils/blob/master/lib.h)

#### Example Config File

Useful if you just want to get up and running on a virtual can bus without access to a car 
or existing config file. Only supports SID 0x10 DiagnosticSessionControl.

```
[7e0]
pos = 300,131
responder = 0
positiveID = 7e8
negativeID = 7e8
{Packets}
7e0#0210030000000000

[7e8]
pos = 350,181
responder = 1
{Packets}
7e8#0650030096177000
```

## Learning

By default UDSim listens for activity on the specified CAN interface and will automatically
learn UDS capable modules in the vehicle based on live network traffic.  To gather data you should
attach a diagnostic device of some sort and run it against your vehicle while UDSim is monitoring
the CANbus.

Press the disk icon to Save the learned information. When you press Save a config file called, 
config_data.cfg is created.  This is a fairly easy to read the 
summary of UDS Can packets layed out in an INI format.  You may want to copy this file and give it
a specific name once you saved it so you don't accidently overwrite it.  This file can be used
later by passing the -c option to UDSim.

You can get UDSim to learn from recorded network traffic as well.  If you feed UDSim a logfile
via the -l option, it will parse all of the logging information and then automatically switch
to simulation mode once it is done.  The logfiles are in candump format.

Because UDSim uses network communication to learn about modules in the vehicle that can communicate
to UDS it does not need any previous knowledge of the vehicle to work.  It can work on any vehicle
that uses ISO-TP.  It has some base knowledge of common UDS but does not rely on it and should
work even on new vehicles or with vehicles with non-standard UDS commands.

## Simulation

Once you are satisfied with that UDSim has learned enough information you can press the MODE button
to toggle the state to Simulation.  At this point UDSim will simulate the vehicle.  You can detach
UDSim from the actual car and your diagnostic tools should still function and pull informaion as if
you were attached to a real car.

Note for presenters:  UDSim works very well for demonstrating attacks without showing the vehicle
or device directly.  You can record your attack session with UDSim (or any CAN Logger) an once
UDSim has monitored the traffic you can save and modify the config file to protect the guilty.
Then all you need to bring with you on stage is UDSim and the config file and you can recreate
the attack in a live interactive manner.

Simulation is also good for testing out CAN related tools or providing classes.  Please note that
simulation mode (in its current revision) won't make up traffic it has never seen.  So don't expect
to query information in simulation mode that it hasn't seen traffic for.

### Simulation Options
There are two options available on a modules info card: Fake Responses and Ignore.  Ignore is
fairly self explanatory.  If you set a module to ignore then it will not respond to incoming
requests.

Fake Responses uses behavior similar to uds-server in that it will try to create a response
to any request even if it hasn't learned how that request works (ie: never seen it before).
It does this by using the standard UDS syntax for responses and it will just plug in some
default or random values for a response.  This is mainly only useful to see what a tool is
trying to talk to or if you need a module to use a mix of learned behavior and simulate
behavior for any missing UDS requests.


## Attack

UDSim has a builtin fuzzer.  This is a very limited fuzzing system compared to a full blown
fuzzing framework.  However it can be useful for quick and easy fuzzing of a device and it
understands and respects the ISO-TP protocol (unless you don't want it to).

### Attack Options
The module info card has two options: Fuzz VIN and a Fuzz Level.

When you are doing fuzzing of a module, it is often good to keep the VIN from changing.  
Many automototive tools use just the VIN to decide that they are indeed talking to the
expected car.  So unless you are testing the VIN inputs of the diagnostic tools, it is
usually best to leave this unchecked.

Fuzz Level.  When the bar is to the far left that disables fuzzing and makes the module work
in simulation mode.  This can be useful if you want some modules to fuzz while others to work
properly.  This bar was originally designed to work with the fuzzing levels of uds-server but
we are currently re-evaluating the fuzzing plan.  As it stands here are the levels:

* 0 - No Fuzzing, simulation on
* 1 - Fuzz the data portion of a responses
* 2 - TBD - Current plan, variable length fuzzing
* 3 - TBD
* 4 - TBD - Current plan, fuzz even ISO-TP, complete random

For now just use fuzz level 1 or 0.  Most likely we will implement level 2 as planned and
if there are better ideas for 3 and 4, let us know.  For more complex fuzzing you should
use Peach Fuzzer.

### Peach Fuzzer
During the original design of UDSim it was going to use a built-in fuzzing system much like
the uds-server predecessor.  However, it was later decided that it would be better to offload
fuzzing to a more complete engine.  For this reason we have decided to export the learned
information to another open source utility, Peach Fuzzer.  There are still widgets in the UI
for fuzzing that currently do not do anything.  This is because we are still deciding if
this is the correct approach.  We may use the slider to change how Peach is fuzzing or we
may scrap Peach altogether.

The purpose of UDSim attack mode is to fuzz responses from the (simulated) vehicle.  This
is very useful in testing for vulnerabilities in dealership tools and in security auditing
tools.  We want to make UDSim easy to use but also allow the flexibility a research will
need to dive in deeper in their research.

Peach XML files are saved to fuzz_can.xml and can be further modified for your own testing
purposes.

For now consider the Attack mode very alpha.

NOTE:  There is currently a bug with Mono for the CAN publisher.  In the mono source for
UnixSocket, Mono has a *feature* that can check if the socket is writable.  It does this
by writing a 0 byte frame to the socket.  This causes an error on a CAN socket which in
turn makes Mono mark the socket as unwritable by setting a flag.  Even if you later send
a valid frame length, Mono will not even try to write the packet because of this flag.

Issue is here: https://github.com/mono/mono/blob/master/mcs/class/Mono.Posix/Mono.Unix/UnixStream.cs#L63
```c#
			long write = Native.Syscall.write (fileDescriptor, IntPtr.Zero, 0);
			if (write != -1)
				canWrite = true; 
```

There are a few ways to work around this.  We can write a different publisher that uses [socketcand](https://github.com/dschanoeh/socketcand) or a even more hacked up version that calls out to can-utils.

I'm looking for some peach fuzzing experts to pitch in some feedback on the best direction to go with this.
Until this the fuzz_can.xml is in a hold state until we know what type of data would be easiest for the
publisher to digest.

Compiling
---------
You need libsdl2-dev, libsdl2-image and libsdl2-ttf

For Attack Fuzzing
------------------
The goal of UDSim is to offload the fuzzing to an actual fuzzing engine.
The Peach Fuzzer was selected as a good open source engine for fuzzing.

The Community version can be obtained here: http://community.peachfuzzer.com/

You will need to copy the folders in UDSim to the source of Peach

```
  cp Peach.Core.OS.Linux/Publishers/* <your peach source folder>/Peach.Core.OS.Linux/Publishers/
```

In the peach folder you will then want to compile a new version of peach:

```
  ./waf install --variant=linux_x86_64_debug
```

Note at the time of this writing there was a bug where you also needed to delete the file: Peach.Core.Analysis.Pin.BasicBlocks/wscript_build to get peach to compile on 64 bit linux.

Once you have compiled then put a peach script in your path.  UDSim assumes peach is in the search path.

