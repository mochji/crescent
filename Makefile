# ============================
#  build configuration
# ============================

STD          = c99
OPTIMIZATION = 2
CFLAGS       =         \
	-Wall              \
	-Wextra            \
	-Wpedantic         \
	-Wshadow           \
	-Wundef            \
	-Wdouble-promotion \
	-Wconversion       \
	-fvisibility=hidden
LDFLAGS = -lm

CC       = gcc
AR       = ar rcs
VALGRIND = valgrind
DEBUG    = oksurewhynot

# ============================
#  end of user configuration
# ============================

SRC      = src
BUILD    = build
TYPES    = $(SRC)/types
CORE     = $(SRC)/core
COMPILER = $(SRC)/compiler
VM       = $(SRC)/vm
API      = $(SRC)/api

MAIN    = $(SRC)/crescent.c
TARGET  = $(BUILD)/crescent
ARCHIVE = $(BUILD)/libcrescent.a
SHARED  = $(BUILD)/libcrescent.so

TYPESSRC    = $(wildcard $(TYPES)/*.c)
CORESRC     = $(wildcard $(CORE)/*.c)
COMPILERSRC = $(wildcard $(COMPILER)/*.c)
VMSRC       = $(wildcard $(VM)/*.c)
APISRC      = $(wildcard $(API)/*.c)
OBJECTS     = $(foreach source,$(TYPESSRC) $(CORESRC) $(COMPILERSRC) $(VMSRC) $(APISRC),$(BUILD)/$(subst .c,.o,$(notdir $(source))))

CFLAGS := $(CFLAGS) -I$(SRC)

ifdef STD
	CFLAGS := $(CFLAGS) -std=$(STD)
endif

ifdef OPTIMIZATION
	CFLAGS := $(CFLAGS) -O$(OPTIMIZATION)
endif

ifdef DEBUG
	CFLAGS := $(CFLAGS) -g -DCRS_DEBUG
endif

.DEFAULT_GOAL = build
.PHONY: build clean rmobj todo fixme notes echo

build:
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $(BUILD)/string.o $(TYPES)/string.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/table.o $(TYPES)/table.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/function.o $(TYPES)/function.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/object.o $(CORE)/object.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/memory.o $(CORE)/memory.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/gc.o $(CORE)/gc.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/buffer.o $(CORE)/buffer.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/format.o $(CORE)/format.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/state.o $(CORE)/state.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/call.o $(CORE)/call.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/debug.o $(CORE)/debug.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/chunk.o $(COMPILER)/chunk.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/lexer.o $(COMPILER)/lexer.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/parser.o $(COMPILER)/parser.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/opcodes.o $(VM)/opcodes.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/vm.o $(VM)/vm.c
	$(CC) $(CFLAGS) -c -o $(BUILD)/api.o $(API)/api.c
	$(AR) $(ARCHIVE) $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(MAIN) $(ARCHIVE)

clean:
	rm -rf $(BUILD)

rmobj:
	rm -f $(BUILD)/*.o

todo:
	grep -rnH --color=auto --include "*.c" --include "*.h" "TODO"

fixme:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME"

notes:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME\|TODO"

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
	@echo "COMPILER     = $(COMPILER)"
	@echo "VM           = $(VM)"
	@echo "API          = $(API)"
	@echo "MAIN         = $(MAIN)"
	@echo "TARGET       = $(TARGET)"
