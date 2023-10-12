FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:cyclone5 = " file://0001-led-device-tree.patch "

# prepend to patch file
# arch/arm/boot/dts/
