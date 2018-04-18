# Target specific macros
TARGET = LineFollower
TARGET_SOURCES := \
	line.c
TOPPERS_OSEK_OIL_SOURCE := ./line.oil

O_PATH ?= build

include ../../ecrobot/ecrobot.mak
