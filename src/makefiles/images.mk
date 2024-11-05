# ░▀█▀░█▄█░█▀█░█▀▀░█▀▀░█▀▀
# ░░█░░█░█░█▀█░█░█░█▀▀░▀▀█
# ░▀▀▀░▀░▀░▀░▀░▀▀▀░▀▀▀░▀▀▀

$(PROJECT_ROOT)/src/lmos/lmos-tray.ico:	$(PROJECT_ROOT)/res/img/blastpit.svg
	magick -background none -define icon:auto-resize=256,128,64,48,32,16 $^ $@

%.ico:	%.svg
	magick -background none -define icon:auto-resize=256,128,64,48,32,16 $^ $@

%.png:	%.svg
	rsvg-convert --width 256 --height 256 --format png $^ > $@

images:	pngs $(PROJECT_ROOT)/src/lmos/lmos-tray.ico

pngs:	$(PROJECT_ROOT)/res/img/laseractive.png $(PROJECT_ROOT)/res/img/noconnection.png $(PROJECT_ROOT)/res/img/nolaser.png $(PROJECT_ROOT)/res/img/blastpit.png
