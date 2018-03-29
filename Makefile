# Target specific macros
TARGET = part1_OSEK
TARGET_SOURCES = \
	skeleton.c 
TOPPERS_OSEK_OIL_SOURCE = ./helloworld.oil 
 
# USER_LIB = ../../../lib/
# USER_INC_PATH = ../../../usr/


# Don't modify below part
O_PATH ?= build
include ../../ecrobot/ecrobot.mak
