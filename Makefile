SRC       = src
BUILD     = build
CORE      = $(SRC)/core
API       = $(SRC)/api

MAIN      = $(SRC)/crescent.c

STD       = c99
CC        = gcc
CFLAGS    = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef -Wdouble-promotion -Wfatal-errors -O2 -std=$(STD) -I$(SRC)

TARGET    = $(BUILD)/crescent

CHECKVARS = SRC BUILD CORE API MAIN STD CC CFLAGS TARGET

$(foreach var, $(CHECKVARS), $(if $($(var)),, $(error $(var) not set)))
$(shell mkdir -p $(BUILD))

$(TARGET): $(SRC)/crescent.c $(BUILD)/state.o $(BUILD)/call.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/state.o: $(CORE)/state.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/call.o: $(CORE)/call.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(SRC)/crescent.c:

.PHONY: clean
.PHONY: todo
.PHONY: fixme
.PHONY: notes

clean:
	rm -rf $(BUILD)

todo:
	grep -rnH --color=auto --include "*.c" --include "*.h" "TODO:"

fixme:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME:"

notes:
	grep -rnH --color=auto --include "*.c" --include "*.h" "FIXME:\|TODO:"
