CC      =  g++
CFLAGS  += -std=c++17 -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L
CFLAGS  += $(shell pkg-config --cflags libnotify)
LDFLAGS += -L$(PREFIX)/lib

LIBS     = -lm -lpulse -lX11
LIBS		 += $(shell pkg-config --libs libnotify)
TARGET   := pavol-dunst

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)


all: $(TARGET)

debug: CFLAGS += -O0 -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJECTS) -o $@ -O3

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -c $?

install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm -f "$(BINPREFIX)/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: debug default uninstall clean
# .PRECIOUS: $(TARGET) $(OBJECTS)
