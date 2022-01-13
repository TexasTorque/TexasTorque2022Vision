# Makefile for cross-compiling a C++ vision project to a
# Raspberry-Pi from a Unix system.
#
# Relies on the Rasbian10 compiler toolchain being added
# to the PATH. 
#
# All headers and linkable libraris are already present 
# in the ./include and ./lib directories respectively.
#
# This file was initially part of the WPILib project, and
# modified by Texas Torque to fit specific needs.
#
# Copyright (c) 2009-2021 FIRST and other WPILib contributors
# Copyright (c) 2021-2022 Justus Languell and Texas Torque
# 
# Note: at some point, I will contribute this to WPILib
# because this is *by far* a supperior Makefile.

# Constants
BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.cc -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
CC = arm-raspbian10-linux-gnueabihf-gcc
CXX = arm-raspbian10-linux-gnueabihf-g++
DEPS_CFLAGS = -Iinclude -Iinclude/opencv -Iinclude
DEPS_LIBS = -Llib -lwpilibc -lwpiHal -lcameraserver -lntcore -lcscore -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_videoio -lopencv_imgcodecs -lopencv_features2d -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core -lwpiutil -latomic
EXE = camera-binary
DESTDIR ?= /home/pi/

# Main rule
.PHONY: clean build install

# Rule to build binary
build: $(BUILD_DIR)/${EXE}

# Rule to build and export the binary to be used by the cameraserver (unncessary on cross-compilation)
install: build
	cp $(BUILD_DIR)/${EXE} runCamera ${DESTDIR}

# Rule to clean all the .o files generated by the build
clean:
	$(RM) -r $(BUILD_DIR)

# Link all .o files into a single binary
$(BUILD_DIR)/$(EXE): $(OBJS)
	${CXX} -pthread -g -o $@ $^ ${DEPS_LIBS} -Wl,--unresolved-symbols=ignore-in-shared-libs

# C++ source files with .cpp extension (non-perfered extension)
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	${CXX} -pthread -g -Og -c -o $@ -std=c++17 ${CXXFLAGS} ${DEPS_CFLAGS} $<

# C++ source files with .cc extension (perfered extension)
$(BUILD_DIR)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	${CXX} -pthread -g -Og -c -o $@ -std=c++17 ${CXXFLAGS} ${DEPS_CFLAGS} $<

# C source files with .c extension (obviously)
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	${CC} -pthread -g -Og -c -o $@ -std=c17 ${CFLAGS} ${DEPS_CFLAGS} $<
	
MKDIR_P ?= mkdir -p