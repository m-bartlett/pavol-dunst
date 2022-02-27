CC      =  g++
CFLAGS  += -std=c++17 -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L
CFLAGS  += $(shell pkg-config --cflags libnotify)
LDFLAGS += -L$(PREFIX)/lib

LIBS     = -lm -lpulse
LIBS		 += $(shell pkg-config --libs libnotify)
TARGET   := pavol-dunst

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES))

all: clean $(TARGET)

debug: CFLAGS += -O0 -g
debug: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.cpp $(LIBS) -O3

install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm -f "$(BINPREFIX)/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: debug default uninstall clean
# .PRECIOUS: $(TARGET) $(OBJECTS)
