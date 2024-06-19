# =============================================================================
# Crescent build configuration
# =============================================================================

SRC       = src
BUILD     = build
TYPES     = $(SRC)/types
CORE      = $(SRC)/core
VM        = $(SRC)/vm
API       = $(SRC)/api

MAIN      = $(SRC)/crescent.c

STD       = c99
CC        = gcc
CFLAGS    = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef -Wdouble-promotion -Wnull-dereference -Wfatal-errors -O2 -I$(SRC)

VALGRIND  = valgrind

TARGET    = $(BUILD)/crescent

# =============================================================================
# End of configurable options
# =============================================================================

CHECKVARS = SRC BUILD CORE API MAIN CC CFLAGS TARGET

$(foreach var, $(CHECKVARS), $(if $($(var)),, $(error $(var) not set)))

ifdef STD
	CFLAGS := $(CFLAGS) -std=$(STD)
endif

ifdef DEBUG
	CFLAGS := $(CFLAGS) -g
endif

TYPESFILES = $(wildcard $(TYPES)/*.c)
COREFILES  = $(wildcard $(CORE)/*.c)
VMFILES    = $(wildcard $(VM)/*.c)
APIFILES   = $(wildcard $(API)/*.c)
OBJECTS    = $(foreach source,$(TYPESFILES) $(COREFILES) $(VMFILES) $(APIFILES),$(BUILD)/$(subst .c,.o,$(notdir $(source))))

.DEFAULT_GOAL = build

.PHONY: build run valgrind clean rmobj todo fixme notes echo

build:
	echo $(OBJECTS)
	mkdir -p $(BUILD)
	$(foreach source,$(TYPESFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(foreach source,$(COREFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(foreach source,$(VMFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(foreach source,$(APIFILES),$(CC) $(CFLAGS) -fPIC -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(CC) $(CFLAGS) -fPIC -shared -o $(BUILD)/crescent.so $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN) $(OBJECTS)

run: build
	./$(TARGET)

valgrind: build
	valgrind --tool=massif ./$(TARGET)
	valgrind --tool=callgrind ./$(TARGET)
	valgrind --tool=memcheck ./$(TARGET)

clean:
	rm -rf $(BUILD)

rmobj:
	rm -f $(BUILD)/*.o

todo:
	grep -rnH --color=auto --include "*.c" --include "*.h" "TODO:"

fixme:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME:"

notes:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME:\|TODO:"

echo:
	@echo "SRC      = $(SRC)"
	@echo "BUILD    = $(BUILD)"
	@echo "CORE     = $(CORE)"
	@echo "VM       = $(VM)"
	@echo "API      = $(API)"
	@echo "MAIN     = $(MAIN)"
	@echo "STD      = $(STD)"
	@echo "CC       = $(CC)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "VALGRIND = $(VALGRIND)"
	@echo "TARGET   = $(TARGET)"
