#ifndef API_CRESCENT_H
#define API_CRESCENT_H

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

crescent_State*
crescent_openState();

void
crescent_closeState(crescent_State* state);

#endif
