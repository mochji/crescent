ifdef YOWZA
SRC    = src
BUILD  = build
CORE   = $(SRC)/core
API    = $(SRC)/api

MAIN   = $(SRC)/crescent.c

STD    = c99
CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wfatal-errors -std=$(STD) -I$(SRC)

TARGET = $(BUILD)/crescent

ifndef BUILD
	$(error BUILD not set)
endif

$(shell mkdir -p $(BUILD))

$(TARGET): $(MAIN) $(BUILD)/object.o $(BUILD)/state.o $(BUILD)/crescent_api.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/object.o: $(CORE)/object.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/state.o: $(CORE)/state.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/crescent_api.o: $(API)/crescent.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILDDIR):
	mkdir $(BUILD)

.PHONY: clean

clean:
	rm -f $(BUILD)/*
endif
