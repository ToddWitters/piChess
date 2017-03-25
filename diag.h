#ifndef DIAG_H
#define DIAG_H

#if DEBUG_OUTPUT
#define DPRINT(msg, ...)       DIAG_print(msg, ##__VA_ARGS__)
#else
#define DPRINT(msg, ...)
#endif

void DIAG_print(char *msg, ...);

#endif
