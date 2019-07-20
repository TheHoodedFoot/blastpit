#!/bin/sh

SERVER=webserver

mosquitto_pub -h ${SERVER} -t lmos -m '<command id="1"><qpset name="bp_73d216" current="99.0" speed="234" frequency="23456"/><qpset name="bp_myqpset" current="99.0" speed="234" frequency="23456"/><qpset name="bp_myqpset2" current="99.0" speed="234" frequency="23456"/>2</command>'

# mosquitto_sub -C 3 --host ${SERVER} --topic broadcast

mosquitto_pub -h ${SERVER} -t lmos -m '<command id="4">6<DRAWING UNIT="MM"><ROOT HEIGHT="120.0" ID="Blastpit" WIDTH="120.0"><LAYER NAME="bp_73d216" COLOR="115,210,22" HEIGHT_Z_AXIS="120.0" /><GROUP HATCH="Y" HP="bp_0_01" ID="gpath820" LAYER="bp_73d216" LP="bp_73d216" REF_POINT="CC" USE_BOX="Y"><POLYLINE HATCH="Y" HP="Standard" ID="path820" LAYER="bp_73d216" LP="bp_73d216"><POLYPOINT TYPE="LINE">39.119869 83.990284</POLYPOINT><POLYPOINT TYPE="BEZIER">36.123108 79.994602</POLYPOINT><POLYPOINT TYPE="BEZIER">33.12635 75.99892</POLYPOINT><POLYPOINT TYPE="LINE">30.129591 72.003239</POLYPOINT><POLYPOINT TYPE="BEZIER">35.934128 70.869331</POLYPOINT><POLYPOINT TYPE="BEZIER">41.738663 69.73542</POLYPOINT><POLYPOINT TYPE="LINE">47.543198 68.601513</POLYPOINT><POLYPOINT TYPE="BEZIER">44.73542 73.731102</POLYPOINT><POLYPOINT TYPE="BEZIER">41.927647 78.860695</POLYPOINT><POLYPOINT TYPE="LINE">39.119869 83.990284</POLYPOINT><POLYPOINT TYPE="LINE">39.119869 83.990284</POLYPOINT></POLYLINE></GROUP></ROOT></DRAWING></command>'

# mosquitto_sub -C 1 --host ${SERVER} --topic broadcast

mosquitto_pub -h ${SERVER} -t lmos -m '<command id="5" filename="C:\Rofin\VisualLaserMarker\MarkingFiles\test_vlm_save.VLM">18</command>'
