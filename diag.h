#ifndef DIAG_H
#define DIAG_H

#if DEBUG_OUTPUT
#define DPRINT(msg, ...)       DIAG_print(msg, ##__VA_ARGS__)
#else
#define DPRINT(msg, ...)
#endif

#ifdef LOG_OUTPUT
#define DLOG(file, msg, ...)   DIAG_log(file, msg, ##__VA_ARGS__)
#else
#define DLOG(file, msg, ...)
#endif

void DIAG_print(char *msg, ...);
void DIAG_log(char *filename, char *msg, ...);

#endif
