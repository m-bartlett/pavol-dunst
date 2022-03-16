# ROOT_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
MAKEFILE := $(firstword $(MAKEFILE_LIST))
SHELL := /bin/bash

TARGET := pavol-dunst

ICON_SIZE ?= 64
ICON_DARK_PRIMARY    ?= \#282828
ICON_DARK_SECONDARY  ?= \#888
ICON_LIGHT_PRIMARY   ?= \#e8e8e8
ICON_LIGHT_SECONDARY ?= \#808080

LIBS     := pulse
PKG_LIBS := libnotify xcb-xrm librsvg-2.0    # pkg-config --list-all

CC        :=  g++
CFLAGS    := $(shell pkg-config --cflags $(PKG_LIBS))
LIB_FLAGS := $(shell pkg-config --libs $(PKG_LIBS))
LIB_FLAGS += $(addprefix -l, $(LIBS))

PREFIX    ?= /usr/local
BINPREFIX := $(PREFIX)/bin

SOURCES   := $(wildcard *.cpp)
HEADERS   := $(wildcard *.h)
OBJECTS   := $(SOURCES:.cpp=.o)

ICON_SVGS  := $(wildcard svg/*.svg)
ICON_HEADERS  := $(patsubst svg/%.svg, icon/%.h, $(ICON_SVGS))
# ICON_PNGS_LIGHT := $(patsubst svg/%.svg, png/light_%.png, $(ICON_SVGS))
# ICON_PNGS_DARK  := $(patsubst svg/%.svg, png/dark_%.png,  $(ICON_SVGS))
# ICON_PNGS  := $(ICON_PNGS_DARK) $(ICON_PNGS_LIGHT)
# ICON_BLOBS := $(patsubst png/%.png, icon/%.h, $(ICON_PNGS))


all:	# Multi-threaded make by default
	$(MAKE) -j $(shell nproc) $(TARGET)

debug: CFLAGS += -D DEBUG
debug: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) $^ -o $@ -O3

$(OBJECTS): $(SOURCES) $(HEADERS) $(ICON_HEADERS)

%.o: %.cpp		# Recompile objects only if their respective source changes
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -c $< -o $@

$(SOURCES): $(MAKEFILE) # If Makefile changes, recompile
	@touch $(SOURCES)

icons: $(ICON_HEADERS)

$(ICON_HEADERS): $(ICON_SVGS)

icon/%.h: svg/%.svg $(MAKEFILE)
	@mkdir -p ./icon
	<$< \
		sed -e 's/"/\\"/g' \
			-e 's/128px/$(ICON_SIZE)px/g' \
			-e 's@<style>.*</style>@@g' \
			-e 's/^/const char* $(patsubst svg/%.svg,%_svg_raw,$<) = "/' \
			-e 's/$$/"\;\x0aconst size_t $(patsubst svg/%.svg,%_svg_raw_size,$<) = strlen($(patsubst svg/%.svg,%_svg_raw,$<));/' \
		> $@

install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm -f "$(BINPREFIX)/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJECTS) $(ICON_HEADERS)
	rm -rf icon/

.PHONY: debug default uninstall clean icons