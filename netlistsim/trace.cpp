#include "stdio.h"
#include "trace.h"
#include <string>

#define TRACE_ENABLED 0

#if TRACE_ENABLED
static int stackDepth = 0;
#endif

void TRACE_PUSH(std::string label)
{
#if TRACE_ENABLED
	int i;
	for(i=0 ; i<stackDepth ; i++)
	{
		printf(":   ");
	}
	printf("%s", label.c_str());
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
