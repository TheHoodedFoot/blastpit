# Blastpit

## Description

![Tentacle](/res/img/blastpit.png)

**blastpit** adds additional functionality to the Rofin Easyjewel marking
system. Features will **(hopefully)** include:

* Network control of laser marking
* Enhanced positioning help
* Controller support
* Python language support
* An [Inkscape](https://inkscape.org) plugin

**blastpit** is composed of:

1. The **blastpit** library, containing the core class and geometry functions,
   along with bindings for the Python language
2. **lmos-tray**, the interface to the Rofin LMOS ActiveX control

## Notes

### Obtaining the code

The development git repository can be found
[here](https://github.com/TheHoodedFoot/blastpit.git)

Blastpit has some submodule dependencies, so you will need to clone the
repository recursively:

~~~{.sh}
git clone --recursive https://github.com/TheHoodedFoot/blastpit.git
~~~

### Prerequisites for building

A standard POSIX development environment (shell, compiler, linker).

[Git](https://git-scm.com)

[SWIG](https://www.swig.org) is needed to create the Python language bindings.

[Zig](https://ziglang.org) is used for cross-compiling.

[Mongoose](https://github.com/cesanta/mongoose) is used for the WebSockets
server and client code.

[Qt](https://www.qt.io) with ActiveQt is required for the Rofin LMOS ActiveX
control, either running natively on Windows or under emulation using Wine or a
virtual machine.

### Building on Linux

---
**NOTE**

The build instructions have only been tested on Funtoo Linux, and are likely
to fail on other distributions, which may not be shipped with certain
dependencies used here.

---

#### Blastpit

The core files and WebSockets server, which are required
by the Inkscape plugin, can be compiled with:

~~~{.sh}
make release
~~~

in the root of the project directory.

#### Python libraries

Once blastpit has been compiled, you can install the Python bindings to a local directory with:

~~~{.sh}
make python
~~~

#### lmos-tray (using Wine)

lmos-tray needs to be built using Qt for Windows. This can be done on a Windows
PC, a virtual machine, or using Wine.

The instructions below are for using Wine, although the initial setup detailed
here is slightly laborious.

First, obtain the Windows version of Qt (an offline installer can be downloaded
[here](https://download.qt.io/archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe)).

You will need to install the Windows version of Qt into a 32-bit Wine
installation. For some Linux distributions, this may require running the
following installation commands from within a 32-bit chroot environment, the
setting up of which is not covered here. Other Linux distributions may use a
multilib version of Wine, and the chroot may not be needed.

These steps use a specific WINEPREFIX, which creates an isolated 32-bit
Wine directory (modify the WINEPREFIX to a location of your choice):

~~~{.sh}
export WINEPREFIX="${HOME}/.wine_32bit"
mkdir -p ${WINEPREFIX}
WINEARCH="win32" wine qt-opensource-windows-x86-5.14.2.exe
~~~

From the Qt installer components tree, choose 'MinGW 7.3.0 32-bit' and continue
the installation.

If you have access to the Rofin VLM installation media then you can install VLM
to the Wine directory, since the lmos ActiveX control will run under Wine. This
can be useful for testing and development.

Once installed, you can compile the Win32 blastpit code from your normal
development environment (not the 32-bit chroot, if you used one) with the
following command, which uses zig to cross-compile:

~~~{.sh}
make clean cross
~~~

Then (from within the 32-bit chroot, if necessary):

~~~{.sh}
make lmosrelease
~~~

This will create a zip file in the /tmp directory, which will contain a
portable version of lmos-tray, plus the Qt .dll files. This can be unzipped and
executed on the target laser controller Windows PC.

#### Inkscape

The Inkscape plugins can be installed by copying them from the src/inkscape
directory to your Inkscape plugins directory. For Linux, this is likely to be
${HOME}/.config/inkscape/extensions.

Please note that the plugins are developed using Inkscape version 1.0, so they
may not work with earlier versions of Inkscape.

## Usage

Currently, Blastpit is used solely to quickly convert Inkscape drawings into
VLM files that can be opened and engraved from within VLM.

Firstly, start the WebSockets server running with:

~~~{.sh}
wscli -s 8000
~~~

Then start lmos-tray on the Windows machine. Right-click on the tray icon,
and choose 'Settings'. In the dialog box, enter the WebSockets address of
the server started above, in the form ws://SERVER_IPV4_ADDRESS:PORT

If lmos-tray connects to the server correctly, then the tray icon should change
and a balloon should indicate a successful connection.

Open the template 'laser.svg' from the src/inkscape directory. There are
several text objects containing parameters for blastpit. The most important is
the parameter 'server', which needs to be modified to the address of the
WebSockets server above (the same address that is used by the lmos-tray
program).

Once the Inkscape plugins are installed, there should be a new 'Laser' submenu
in the Extensions menu. Selecting the 'Laser' option will attempt to send the
current Inkscape drawing to the server. The default option is to save the
drawing in the C:\Rofin\VisualLaserMarker\MarkingFiles folder.

## Bugs

Yes. It doesn't work very well yet.

In addition, **blastpit** relies on the Rofin LMOS ActiveX control for its
functionality, so any problems that affect it will also affect **blastpit**.

## Copyright and Licencing

All original source code is released under the [BSD Licence](https://opensource.org/licenses/BSD-3-Clause).
All original text and graphic files are released under the [Creative Commons
CC0 public domain license](https://creativecommons.org/publicdomain/zero/1.0).

Other non-original code is released under licenses stated within the files.

The LMOS ActiveX control is copyright Rofin Sinar Laser GmbH.

Sample video capture code is copyright IDS Imaging GmbH and is used with permission.

Boilerplate Qt application code is copyright Digia Plc and is released under
the terms of the BSD license.

Boilerplate TeX code was obtained from https://www.latextemplates.com and
released under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0
unported licence](https://creativecommons.org/licenses/by-nc-sa/3.0/).
