#ifndef _DEBUG_
#define _DEBUG_

// #define _DPRINT

#ifdef _DPRINT
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif