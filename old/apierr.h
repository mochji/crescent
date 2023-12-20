#ifndef APIERR_H
#define APIERR_H

#define crescent_apiError(msg) fprintf(stderr, "internal error!\n\n%s\n\tin internal function %s\n", msg, __func__); exit(EXIT_FAILURE)

#endif
