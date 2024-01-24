#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

#include "api/crescent.h"

static char*
crescent_readLine(FILE* stream, size_t* bytesRead, int* endOfFile) {
	size_t bufferSize  = 64;
	size_t bufferIndex = 0;
	char*  buffer      = malloc(bufferSize);

	if (buffer == NULL) {
		if (bytesRead) {
			*bytesRead = 0;
		}

		return NULL;
	}

	char c;

	while ((c = getc(stream)) != '\n' && c != '\r' && c != '\0') {
		if (c == EOF && bufferIndex == 0) {
			if (bytesRead) {
				*bytesRead = 0;
			}

			free(buffer);

			if (endOfFile) {
				*endOfFile = 1;
			}

			return NULL;
		}

		if (c == '\b' && bufferIndex > 0) {
			bufferIndex--;

			if (bufferSize - bufferIndex > 64) {
				bufferSize /= 2;
				char* newBuffer = realloc(buffer, bufferSize);

				if (newBuffer == NULL) {
					free(buffer);

					if (bytesRead) {
						*bytesRead = 0;
					}

					return NULL;
				}

				buffer = newBuffer;
			}

			continue;
		}

		if (bufferIndex == bufferSize) {
			bufferSize *= 2;
			char* newBuffer = realloc(buffer, bufferSize);

			if (newBuffer == NULL) {
				free(buffer);

				if (bytesRead) {
					*bytesRead = 0;
				}

				return NULL;
			}

			buffer = newBuffer;
		}

		buffer[bufferIndex++] = c;
	}

	if (endOfFile) {
		*endOfFile = 0;
	}

	return buffer;
}

int
main(int argc, char** argv) {
	((void)argc);

	char*           input;
	int             endOfInput;
	crescent_State* state;

	state = crescent_openState();

	if (state == NULL) {
		fprintf(stderr, "%s: not enough memory, cannot create state!\n", argv[0]);

		return EXIT_FAILURE;
	}

	printf(CRESCENT_INTERACTIVE_STR);
	printf("type 'help' for a list of all operations\n");

	for (;;) {
		printf("> ");
		input = crescent_readLine(stdin, NULL, &endOfInput);

		if (endOfInput) {
			printf("\n");

			return 0;
		}

		if (input == NULL) {
			printf("error: out of memory or input too large\n");

			return EXIT_FAILURE;
		}
	}

	crescent_closeState(state);

	return 0;
}
