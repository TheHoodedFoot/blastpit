class Blastpit (Workbench):

    MenuText = "Blastpit"
    ToolTip = "Rofin EasyJewel Controller"
    Icon = """
            /* XPM */
            static char * blastpit_xpm[] = {
            "16 16 17 1",
            " 	c None",
            ".	c #203A1E",
            "+	c #234222",
            "@	c #27451D",
            "#	c #264A23",
            "$	c #2E5323",
            "%	c #2F5A22",
            "&	c #5A5A02",
            "*	c #306626",
            "=	c #37722A",
            "-	c #7E7F00",
            ";	c #8F8F00",
            ">	c #A1A200",
            ",	c #BABB00",
            "'	c #C7C700",
            ")	c #D4D300",
            "!	c #DEE201",
            "    .           ",
            "   ##@+.        ",
            "  $@ @@+ .      ",
            "  %   #+..      ",
            "  *      ..     ",
            "  *       .     ",
            "  =             ",
            "  *             ",
            "  =             ",
            "  *       -;    ",
            "  =*     ;);    ",
            "  =      ;)!;   ",
            "  =     ;,&-;   ",
            "  =     >)&-);  ",
            "  =    -''>,';; ",
            "                "};
            """

    def Initialize(self):
        "This function is executed when FreeCAD starts"
        import sys
        from os.path import expanduser
        sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
        import bpConnect
        import bpDisconnect
        import bpUpload
        import bpReference
        import bpPosHelpOn
        import bpPosHelpOff
        import bpDoorToggle
        import bpLightToggle
        import bpRing
        import bpCalibrate

        # import MyModuleA, MyModuleB # import here all the needed files that
        # create your FreeCAD commands
        self.list = [
            "bpConnect",
            "bpDisconnect",
            "bpUpload",
            "bpReference",
            "bpPosHelpOn",
            "bpPosHelpOff",
            "bpDoorToggle",
            "bpLightToggle",
            "bpRing",
            "bpCalibrate"]  # A list of command names created in the line above
        # creates a new toolbar with your commands
        self.appendToolbar("Blastpit", self.list)
        self.appendMenu("Blastpit", self.list)  # creates a new menu
        # appends a submenu to an existing menu
        self.appendMenu(["Blastpit", "My submenu"], self.list)

    def Activated(self):
        "This function is executed when the workbench is activated"
        return

    def Deactivated(self):
        "This function is executed when the workbench is deactivated"
        return

    def ContextMenu(self, recipient):
        "This is executed whenever the user right-clicks on screen"
        # "recipient" will be either "view" or "tree"
        # add commands to the context menu
        self.appendContextMenu("My commands", self.list)

    def GetClassName(self):
        # this function is mandatory if this is a full python workbench
        return "Gui::PythonWorkbench"


Gui.addWorkbench(Blastpit())
