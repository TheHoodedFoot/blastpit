TEMPLATE = subdirs
#SUBDIRS = libbp
SUBDIRS = libbp lmos-tray
CONFIG += ordered

lmos-tray.depends = libbp
testchamber.depends = libbp
bpgui.depends = libbp

!win32 {
        SUBDIRS += bpgui
}
