lib.name = keyboard

uname := $(shell uname -s)


keyboard.class.sources := src/keyboard.c


include pd-lib-builder/Makefile.pdlibbuilder
