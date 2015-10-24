#ifndef DEBUG_H
#define DEBUG_H

#include <assert.h>
#include <stdio.h>


// #define DEBUG

// #define DETAILED_EVAL_DEBUG


#ifdef DEBUG

#define ASSERT(X) assert(X)
#define WARN(X)  {if(X) {printf("WARNING: in %s at line %d\n", __FILE__, __LINE__);}}
#define TRACE    {printf("TRACE: In %s, function %s at line %d\n", __FILE__, __FUNCTION__, __LINE__);} 
#define DEBUG_PRINT(args...) printf(args)

#else

#define ASSERT(X)
#define DEBUG_PRINT(args...) 
#define WARN(X)
#define TRACE

#endif


#endif

