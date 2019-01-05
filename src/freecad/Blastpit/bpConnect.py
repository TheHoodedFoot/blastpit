import FreeCAD
import Part
from FreeCAD import Gui


class bpConnect_Class():
    """My new command"""

    def GetResources(self):
        return {"Pixmap": "blastpit",  # the name of a svg file available in the resources
                "MenuText": "My New Command",
                "ToolTip": "What my new command does"}

    def Activated(self):
        "Do something here"
        print("Cock")
        return

    def IsActive(self):
        """Here you can define if the command must be active or not (greyed) if certain conditions
        are met or not. This function is optional."""
        return True


Gui.addCommand('bpConnect', bpConnect_Class())
