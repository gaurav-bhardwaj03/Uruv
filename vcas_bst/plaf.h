/**
 * Preliminary C++ implementation of binary search tree using LLX/SCX and DEBRA(+).
 * 
 * Copyright (C) 2015 Trevor Brown
 * 
 */

#ifndef MACHINECONSTANTS_H
#define	MACHINECONSTANTS_H

#ifndef MAX_TID_POW2
    #define MAX_TID_POW2 512 // MUST BE A POWER OF TWO, since this is used for some bitwise operations
#endif
#ifndef LOGICAL_PROCESSORS
    #define LOGICAL_PROCESSORS 160
#endif

// the following definition is only used to pad data to avoid false sharing.
// although the number of words per cache line is actually 8, we inflate this
// figure to counteract the effects of prefetching multiple adjacent cache lines.
#define PREFETCH_SIZE_WORDS 24
#define PREFETCH_SIZE_BYTES 192
#define BYTES_IN_CACHE_LINE 64

#endif	/* MACHINECONSTANTS_H */

