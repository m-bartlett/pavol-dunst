# ROOT_DIR := $(notdir $(patsubst %/,%,$(CURDIR)))
MAKEFILE := $(firstword $(MAKEFILE_LIST))
SHELL := /bin/bash

TARGET := pavol-dunst
ICON_SIZE ?= 256

LIBS     := pulse
PKG_LIBS := libnotify xcb-xrm

CC        :=  g++
CFLAGS    := $(shell pkg-config --cflags $(PKG_LIBS))
LIB_FLAGS := $(shell pkg-config --libs $(PKG_LIBS))
LIB_FLAGS += $(addprefix -l, $(LIBS))

PREFIX    ?= /usr/local
BINPREFIX := $(PREFIX)/bin

ICON_SVGS  := $(wildcard svg/*.svg)
ICON_PNGS_LIGHT := $(patsubst svg/%.svg, png/light_%.png, $(ICON_SVGS))
ICON_PNGS_DARK  := $(patsubst svg/%.svg, png/dark_%.png,  $(ICON_SVGS))
ICON_PNGS  := $(ICON_PNGS_DARK) $(ICON_PNGS_LIGHT)
ICON_BLOBS := $(patsubst png/%.png, icon/%.h, $(ICON_PNGS))

SOURCES   := $(wildcard *.cpp)
OBJECTS   := $(SOURCES:.cpp=.o)

all:	# Multi-threaded make by default
	$(MAKE) -j $(shell nproc) $(TARGET)

debug: CFLAGS += -D DEBUG
debug: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) $^ -o $@ -O3

$(OBJECTS): $(SOURCES) $(ICON_BLOBS)

%.o: %.cpp		# Recompile objects only if their respective source changes
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -c $< -o $@

$(SOURCES): $(MAKEFILE) # If Makefile changes, recompile
	@touch $(SOURCES)

icons: $(ICON_BLOBS)

$(ICON_BLOBS): $(ICON_PNGS)

icon/%.h: png/%.png
	gdk-pixbuf-csource --raw --name=$(basename $(notdir $<)) $< > $@

$(ICON_PNGS): $(ICON_SVGS) $(MAKEFILE)
	mkdir -p ./png ./icon
	$(shell \
		cd svg; \
		for svg in $$(ls -1 *.svg); do \
			rsvg-convert \
				-w $(ICON_SIZE) \
				$$svg > ../png/dark_$${svg//.svg/.png}; \
			rsvg-convert \
				-w $(ICON_SIZE) \
					<(<$$svg sed 's/#000/#fff/g') > ../png/light_$${svg//.svg/.png}; \
		done)


install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm -f "$(BINPREFIX)/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJECTS) $(ICON_PNGS) $(ICON_BLOBS)
	rmdir png/ icon/

.PHONY: debug default uninstall clean