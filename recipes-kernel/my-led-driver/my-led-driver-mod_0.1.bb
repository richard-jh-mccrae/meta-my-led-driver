SUMMARY = "Example of a custom linux driver"
DESCRIPTION = "${SUMMARY}"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

SRC_URI = "file://Makefile \
           file://my_led_driver.c \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

RPROVIDES:${PN} += "kernel-module-my_led_driver"

do_compile() {
    oe_runmake 'MACHINE=${MACHINE}'
}

#do_compile() {
#    export MACHINE
#    oe_runmake
#}
