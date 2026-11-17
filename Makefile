NAME := AsusACPIService
VERSION := 0.1.0
BUILD_DIR ?= build
SDK_VERSION ?= $(shell xcrun --sdk macosx --show-sdk-version)
KERNEL_SDK_DIR ?= upstream/MacKernelSDK
DEPLOYMENT_TARGET ?= 10.15
CLANG ?= clang
CLANGXX ?= clang++
LD ?= ld

KERNEL_HEADERS := $(KERNEL_SDK_DIR)/Headers
KMOD_LIBRARY := $(KERNEL_SDK_DIR)/Library/x86_64/libkmod.a
TARGET := x86_64-apple-macos$(DEPLOYMENT_TARGET)

KERNEL_FLAGS := -target $(TARGET) -mkernel -nostdinc -ffreestanding -fno-builtin \
    -fno-common -mno-red-zone -fno-asynchronous-unwind-tables -O2 \
    -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT \
    -I$(KERNEL_HEADERS) -Iinclude
CPP_FLAGS := $(KERNEL_FLAGS) -std=c++17 -fapple-kext -fno-rtti -fno-exceptions \
    -fno-use-cxa-atexit -mllvm -disable-atexit-based-global-dtor-lowering \
    -Wno-deprecated-declarations

CPP_SOURCES := $(wildcard src/*.cpp src/ACPI/*.cpp src/Controller/*.cpp src/Controller/Forwarding/*.cpp src/Diagnostics/*.cpp)
CPP_OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/obj/%.o,$(CPP_SOURCES))
KMOD_OBJECT := $(BUILD_DIR)/obj/KmodInfo.o
OBJECTS := $(CPP_OBJECTS) $(KMOD_OBJECT)
KEXT_CONTENTS := $(BUILD_DIR)/$(NAME).kext/Contents
EXECUTABLE := $(KEXT_CONTENTS)/MacOS/$(NAME)
PLIST := $(KEXT_CONTENTS)/Info.plist

.PHONY: all kext clean

all: kext

kext: $(EXECUTABLE) $(PLIST)

$(BUILD_DIR)/obj/%.o: src/%.cpp
	@mkdir -p "$(dir $@)"
	$(CLANGXX) $(CPP_FLAGS) -MMD -MP -c "$<" -o "$@"

$(KMOD_OBJECT): src/KmodInfo.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(KERNEL_FLAGS) -MMD -MP -c "$<" -o "$@"

$(EXECUTABLE): $(OBJECTS) $(KMOD_LIBRARY)
	@mkdir -p "$(dir $@)"
	$(LD) -arch x86_64 -kext -platform_version macos $(DEPLOYMENT_TARGET) $(SDK_VERSION) -no_data_const -o "$@" $(OBJECTS) "$(KMOD_LIBRARY)"

$(PLIST): Info.plist
	@mkdir -p "$(dir $@)"
	cp "$<" "$@"

clean:
	rm -rf "$(BUILD_DIR)/obj" "$(BUILD_DIR)/$(NAME).kext"

-include $(OBJECTS:.o=.d)
