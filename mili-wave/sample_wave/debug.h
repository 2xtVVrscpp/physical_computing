#ifndef _DEBUG_
#define _DEBUG_

#define _DEBUG

#ifdef _DEBUG
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) ((void)0)
#endif

#endif