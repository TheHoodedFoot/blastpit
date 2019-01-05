#!/usr/bin/env python

import blastpit

bp = Blastpit()

# Create a simple path consisting of a hatched circle
path = bp.Path()
path.Circle( 60, 60, 10 )
path.SetHatchType( "Argent" )

# Create a layer and put the path on to it
layer = bp.Layer( "Test Layer", 10, 0xFF7700 )
path.SetLayer( layer )

# Pick the laser parameters from the library
path.SetMarkingType( "Silver_With_Dark_Cleaning_Pass" )

# Check if we have network access and burn the layout
if bp.Connect( "laser.rfbevan.co.uk", 1234 ):
    bp.Burn()
    bp.OpenDoor()
