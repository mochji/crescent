# =============================================================================
# Crescent build configuration
# =============================================================================

STD          = c99
OPTIMIZATION = 2
CFLAGS       =         \
	-Wall              \
	-Wextra            \
	-Wpedantic         \
	-Werror            \
	-Wshadow           \
	-Wundef            \
	-Wdouble-promotion \
	-Wnull-dereference \
	-Wfatal-errors

CC       = gcc
AR       = ar rcs
VALGRIND = valgrind

# =============================================================================
# End of configurable options
# =============================================================================

SRC    = src
BUILD  = build
TYPES  = $(SRC)/types
CORE   = $(SRC)/core
VM     = $(SRC)/vm
API    = $(SRC)/api

MAIN   = $(SRC)/crescent.c
TARGET = $(BUILD)/crescent

TYPESSRC = $(wildcard $(TYPES)/*.c)
CORESRC  = $(wildcard $(CORE)/*.c)
VMSRC    = $(wildcard $(VM)/*.c)
APISRC   = $(wildcard $(API)/*.c)
OBJECTS  = $(foreach source,$(TYPESSRC) $(CORESRC) $(VMSRC) $(APISRC),$(BUILD)/$(subst .c,.o,$(notdir $(source))))

CFLAGS := $(CFLAGS) -I$(SRC)

ifdef STD
	CFLAGS := $(CFLAGS) -std=$(STD)
endif

ifdef OPTIMIZATION
	CFLAGS := $(CFLAGS) -O$(OPTIMIZATION)
endif

ifdef DEBUG
	CFLAGS := $(CFLAGS) -g
endif

.DEFAULT_GOAL = build

.PHONY: build run valgrind clean rmobj todo fixme notes echo

build:
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/string.o $(TYPES)/string.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/array.o $(TYPES)/array.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/object.o $(CORE)/object.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/memory.o $(CORE)/memory.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/gc.o $(CORE)/gc.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/format.o $(CORE)/format.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/state.o $(CORE)/state.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/call.o $(CORE)/call.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/opcodes.o $(VM)/opcodes.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/vm.o $(VM)/vm.c
	$(CC) $(CFLAGS) -fPIC -c -o $(BUILD)/api.o $(API)/api.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(BUILD)/libcrescent.so $(OBJECTS)
	$(AR) $(BUILD)/libcrescent.a $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN) $(OBJECTS)

run: build
	./$(TARGET)

valgrind: build
	valgrind --tool=massif ./$(TARGET)
	valgrind --tool=callgrind --dump-instr=yes ./$(TARGET)
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
	@echo "STD          = $(STD)"
	@echo "OPTIMIZATION = $(OPTIMIZATION)"
	@echo "CFLAGS       = $(CFLAGS)"
	@echo "CC           = $(CC)"
	@echo "VALGRIND     = $(VALGRIND)"
	@echo "SRC          = $(SRC)"
	@echo "BUILD        = $(BUILD)"
	@echo "TYPES        = $(TYPES)"
	@echo "CORE         = $(CORE)"
	@echo "VM           = $(VM)"
	@echo "API          = $(API)"
	@echo "MAIN         = $(MAIN)"
	@echo "TARGET       = $(TARGET)"
