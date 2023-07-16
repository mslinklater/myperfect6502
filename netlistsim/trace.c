#include "stdio.h"
#include "trace.h"

#define TRACE_ENABLED 0

#if TRACE_ENABLED
static int stackDepth = 0;
#endif

void TRACE_PUSH(char* label)
{
#if TRACE_ENABLED
	int i;
	for(i=0 ; i<stackDepth ; i++)
	{
		printf(":   ");
	}
	printf("%s", label);
	printf("\n");
	stackDepth += 1;
#endif // TRACE_ENABLED
}

void TRACE_POP()
{
#if TRACE_ENABLED
	stackDepth -= 1;
#endif // TRACE_ENABLED
}
