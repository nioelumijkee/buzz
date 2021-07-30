################################################################################
lib.name = buzz~
cflags = 
class.sources = buzz~.c
sources = 
datafiles = \
buzz~-help.pd \
buzz~-help.conf \
README.md \
LICENSE.txt

################################################################################
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
