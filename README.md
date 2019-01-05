# Blastpit

## Description

**blastpit** adds additional functionality to the Rofin Easyjewel marking system. Features will **(hopefully)** include:

* Network control of laser marking
* Enhanced positioning help
* Controller support
* Python language support
* An [Inkscape](https://inkscape.org) plugin
* A [FreeCAD](http://www.freecadweb.org) plugin

**blastpit** is composed of three parts:

1. The **blastpit** library contains the core class and geometry functions, along with bindings for the Python language
2. **blastpit-gui** is a tray application providing notifications, controller support and gui adjustment of laser parameters
3. **lmos-tray** is the server interface to the Rofin LMOS ActiveX control

## Notes

### Obtaining the code

The development git repository can be found at http://git.47or.com/thf/Blastpit.git

### Prerequisites for building

**blastpit** requires [Qt](http://www.qt.io), but can be statically compiled for ease of installation.
[SWIG](http://www.swig.org) is needed to create the Python language bindings. **lmos-tray** will only compile and run on Microsoft Windows platforms, as it requires the Rofin LMOS ActiveX control.

### Building

The software can be compiled by typing:

~~~{.sh}
	make
~~~

in the root of the project directory. Alternatively, the software can be compiled from Qt Creator. A project file can be found in the src/ directory.

## Bugs

Yes. It doesn't work yet. The **blastpit** bug tracker can be found at https://git.47or.com/thf/Blastpit/issues

In addition, **blastpit** relies on the Rofin LMOS ActiveX control for its functionality, so any problems that affect it will also affect **blastpit**.

## Example

### Mark a circle in Python

~~~{.py}
import Blastpit

bp = Blastpit()
layer = bp.Layer( "Test Layer", 10, 0xFF7700 )
path = bp.Path()
path.Circle( 60, 60, 10 )
path.SetLayer( layer )
path.SetHatchType( "Argent" )
path.SetMarkingType( "Silver_With_Dark_Cleaning_Pass" )
if bp.Connect( "laser.rfbevan.co.uk", 1234 ):
    bp.Burn()
    bp.OpenDoor()
~~~

## Copyright and Licencing

All original source code is released under the [GNU GPLv3](http://en.wikipedia.org/wiki/GNU_General_Public_License).
All original text and graphic files are released under the [Creative Commons CC0 public domain license](https://creativecommons.org/publicdomain/zero/1.0).

[pugixml](http://pugixml.org) is released under the MIT licence.

The LMOS ActiveX control is copyright Rofin Sinar Laser GmbH.

Sample video capture code is copyright IDS Imaging GmbH and is used with permission.

Boilerplate Qt application code is copyright Digia Plc and is released under the terms of the BSD license.

Boilerplate TeX code was obtained from http://www.latextemplates.com and released under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 unported licence](http://creativecommons.org/licenses/by-nc-sa/3.0/).