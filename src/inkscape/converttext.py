#!/usr/bin/env python3

import sys

sys.path.append("/usr/share/inkscape/extensions")

import inkex.command


svg = ' <?xml version="1.0" encoding="UTF-8" standalone="no"?> <svg width="120mm" height="120mm" viewBox="0 0 120 120" id="svg4149" version="1.1" inkscape:version="1.1-alpha (ac89b25f3, 2021-02-20, custom)" sodipodi:docname="arse.svg" xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg"> <defs id="defs4151"> <inkscape:perspective sodipodi:type="inkscape:persp3d" inkscape:vp_x="0 : 60 : 1" inkscape:vp_y="0 : 1000 : 0" inkscape:vp_z="120 : 60 : 1" inkscape:persp3d-origin="60 : 40 : 1" id="perspective214" /> </defs> <sodipodi:namedview id="base" pagecolor="#ffffff" bordercolor="#666666" borderopacity="1.0" inkscape:pageopacity="0.0" inkscape:pageshadow="2" inkscape:zoom="1.6333611" inkscape:cx="192.24163" inkscape:cy="191.6294" inkscape:document-units="mm" inkscape:current-layer="svg4149" showgrid="false" inkscape:showpageshadow="false" showguides="true" inkscape:guide-bbox="true" inkscape:document-rotation="0" objecttolerance="10.0" gridtolerance="10.0" guidetolerance="10.0" inkscape:pagecheckerboard="0" /> <defs id="defs92" /> <sodipodi:namedview id="namedview94" pagecolor="#ffffff" bordercolor="#666666" borderopacity="1.0" inkscape:pageopacity="0.0" inkscape:pageshadow="2" inkscape:zoom="1.0" inkscape:cx="212.59843" inkscape:cy="212.59843" inkscape:document-units="mm" inkscape:current-layer="layer1" showgrid="false" inkscape:showpageshadow="false" showguides="true" inkscape:guide-bbox="true" inkscape:snap-global="false" inkscape:document-rotation="0" /> <text xml:space="preserve" style="font-size:10.5833px;line-height:1.25;font-family:\'CodeNewRoman NF\';-inkscape-font-specification:\'CodeNewRoman NF\';word-spacing:0px;stroke-width:0.264583" x="41.765091" y="65.834946" id="text15235"><tspan sodipodi:role="line" id="tspan15233" style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:Beeb;-inkscape-font-specification:Beeb;stroke-width:0.264583" x="41.765091" y="65.834946">ARSE</tspan></text> </svg>'

result = inkex.command.inkscape_command(
    svg, ("EditSelectAll"), ("ObjectToPath")
)
print(result)
