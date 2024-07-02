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

ifeq ($(OS),Windows_NT)
	RM     = del /S
	RMR    = /Q
	MKDIR  = mkdir
	OBJEXT = .obj
	ARCEXT = .lib
	LIBEXT = .dll
	EXEEXT = .exe
else
	RM     = rm -f
	RMR    = -r
	MKDIR  = mkdir -p
	OBJEXT = .o
	ARCEXT = .a
	LIBEXT = .so
	EXEEXT =
endif

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
OBJECTS  = $(foreach source,$(TYPESSRC) $(CORESRC) $(VMSRC) $(APISRC),$(BUILD)/$(subst .c,$(OBJEXT),$(notdir $(source))))

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
	-$(MKDIR) build
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/string$(OBJEXT) $(TYPES)/string.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/array$(OBJEXT) $(TYPES)/array.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/object$(OBJEXT) $(CORE)/object.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/state$(OBJEXT) $(CORE)/state.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/call$(OBJEXT) $(CORE)/call.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/opcodes$(OBJEXT) $(VM)/opcodes.c
	$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/vm$(OBJEXT) $(VM)/vm.c
	$(CC) $(CFLAGS) -fPIC -c -o $(BUILD)/api$(OBJEXT) $(API)/api.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(BUILD)/libcrescent$(LIBEXT) $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN) $(OBJECTS)
	$(AR) $(BUILD)/libcrescent$(ARCEXT) $(OBJECTS)

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
