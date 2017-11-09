lib.name = keyboard

uname := $(shell uname -s)


keyboard.class.sources := src/keyboard.c 
knob.class.sources := src/knob.c 

include pd-lib-builder/Makefile.pdlibbuilder
