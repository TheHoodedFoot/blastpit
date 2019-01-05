#!/usr/bin/env python3

from PIL import Image

im = Image.open("/tmp/bitmap.bmp")
pixelMap = im.load()

for x in range( im.size[0] ):
    # Reset row
    lastYPixel = -1 
    for y in range( im.size[1] ):
        #print("Pixel: " + str(x) + "," + str(y))
        if pixelMap[x,y] == 1:
            if lastYPixel == -1:
                lastYPixel = y
            if y == im.size[1]-1:
                # We have hit the bottom, so the line ends here
                print( "Line from " + str(x) + "," + str(lastYPixel) + " to " + str(x) + "," + str(y))
        else:
            if lastYPixel != -1:
                print( "Line from " + str(x) + "," + str(lastYPixel) + " to " + str(x) + "," + str(y-1))
                lastYPixel = -1
