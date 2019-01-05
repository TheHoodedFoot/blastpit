import FreeCAD
import Part
from FreeCAD import Gui


class bpCalibrate_Class():
    """My new command"""

    def GetResources(self):
        return {"Pixmap": "blastpit",  # the name of a svg file available in the resources
                "MenuText": "Calibrate",
                "ToolTip": "Mark ring calibration lines"}

    def Activated(self):
        "Do something here"
        print("Calibrate")
        return

    def IsActive(self):
        """Here you can define if the command must be active or not (greyed) if certain conditions
        are met or not. This function is optional."""
        return True


Gui.addCommand('bpCalibrate', bpCalibrate_Class())
