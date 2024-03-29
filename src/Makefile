# ROOT_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
MAKEFILE := $(firstword $(MAKEFILE_LIST))
SHELL := /bin/bash

TARGET := pavol-dunst

LIBS     := pulse
PKG_LIBS := libnotify xcb-xrm librsvg-2.0 gdk-3.0     # pkg-config --list-all

CC        := gcc
CFLAGS    := $(shell pkg-config --cflags $(PKG_LIBS)) -std=c11
LIB_FLAGS := $(shell pkg-config --libs $(PKG_LIBS))
LIB_FLAGS += $(addprefix -l, $(LIBS))

PREFIX    ?= /usr/local
BINPREFIX := $(PREFIX)/bin

SOURCES   := $(wildcard *.c)
HEADERS   := $(wildcard *.h)
OBJECTS   := $(SOURCES:.c=.o)

ICON_SVGS  := $(wildcard svg/*.svg)
ICON_HEADERS  := $(patsubst svg/%.svg, icon/%.h, $(ICON_SVGS))

FEATURES :=
ifneq ($(ENABLE_TRANSIENT_HINT),)
	FEATURES += -DENABLE_TRANSIENT_HINT
endif
ifneq ($(FORMAT_VOLUME_IN_NOTIFICATION_BODY),)
	FEATURES += -DFORMAT_VOLUME_IN_NOTIFICATION_BODY
endif

all:	# Multi-threaded make by default
	$(MAKE) -j $(shell nproc) $(TARGET)

debug: CFLAGS += -D DEBUG
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) $^ -o $@ -O3

$(OBJECTS): $(SOURCES) $(HEADERS) $(ICON_HEADERS)

%.o: %.c
	$(CC) $(FEATURES) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -c $< -o $@

$(SOURCES): $(MAKEFILE) # If Makefile changes, recompile
	@touch $(SOURCES)

icons: $(ICON_HEADERS)

$(ICON_HEADERS): $(ICON_SVGS)

icon/%.h: svg/%.svg $(MAKEFILE)
	@mkdir -p ./icon
	<$< \
		sed -e 's/"/\\"/g' \
			-e 's@<style>.*</style>@@g' \
			-e 's/^/const char $(patsubst svg/%.svg,%_svg_raw,$<)[] = "/' \
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
