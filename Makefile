# =============================================================================
# Crescent build configuration
# =============================================================================

SRC       = src
BUILD     = build
CORE      = $(SRC)/core
API       = $(SRC)/api

MAIN      = $(SRC)/crescent.c

STD       = c99
CC        = gcc
CFLAGS    = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef -Wdouble-promotion -Wfatal-errors -O2 -std=$(STD) -I$(SRC)

TARGET    = $(BUILD)/crescent

# =============================================================================
# End of configurable options
# =============================================================================

CHECKVARS = SRC BUILD CORE API MAIN STD CC CFLAGS TARGET

$(foreach var, $(CHECKVARS), $(if $($(var)),, $(error $(var) not set)))
$(shell mkdir -p $(BUILD))

$(TARGET): $(MAIN) $(BUILD)/crescent.so
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/crescent.so: $(API)/crescent.c $(BUILD)/state.o $(BUILD)/call.o
	$(CC) $(CFLAGS) -fPIC -shared -Wl,--version-script=shared.lds -o $@ $^

$(BUILD)/state.o: $(CORE)/state.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/call.o: $(CORE)/call.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(MAIN):

.PHONY: clean
.PHONY: rmobj
.PHONY: todo
.PHONY: fixme
.PHONY: notes
.PHONY: echo

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
	@echo "API    = $(API)"
	@echo "MAIN   = $(MAIN)"
	@echo "STD    = $(STD)"
	@echo "CC     = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "TARGET = $(TARGET)"
