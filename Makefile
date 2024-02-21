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
CFLAGS    = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef -Wdouble-promotion -Wnull-dereference -Wfatal-errors -O2 -std=$(STD) -I$(SRC)

TARGET    = $(BUILD)/crescent

# =============================================================================
# End of configurable options
# =============================================================================

CHECKVARS = SRC BUILD CORE API MAIN STD CC CFLAGS TARGET

$(foreach var, $(CHECKVARS), $(if $($(var)),, $(error $(var) not set)))

COREFILES  = $(wildcard $(CORE)/*.c)
TYPESFILES = $(wildcard $(TYPES)/*.c)
VMFILES    = $(wildcard $(VM)/*.c)
APIFILES   = $(wildcard $(API)/*.c)

.DEFAULT_GOAL = build

.PHONY: build
.PHONY: clean
.PHONY: rmobj
.PHONY: todo
.PHONY: fixme
.PHONY: notes
.PHONY: echo

build:
	mkdir -p $(BUILD)
	$(foreach source,$(COREFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(foreach source,$(TYPESFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(foreach source,$(VMFILES),$(CC) $(CFLAGS) -fvisibility=hidden -c -o $(BUILD)/$(notdir $(subst .c,.o, $(source))) $(source);)
	$(CC) $(CFLAGS) -fPIC -shared -o $(BUILD)/crescent.so $(API)/crescent.c $(BUILD)/*.o
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN) $(BUILD)/crescent.so

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
	@echo "SRC    = $(SRC)"
	@echo "BUILD  = $(BUILD)"
	@echo "CORE   = $(CORE)"
	@echo "VM     = $(VM)"
	@echo "API    = $(API)"
	@echo "MAIN   = $(MAIN)"
	@echo "STD    = $(STD)"
	@echo "CC     = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
