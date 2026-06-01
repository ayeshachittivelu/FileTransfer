#ifndef _TAURUS_MEMORY_BARRIER_H_
#define _TAURUS_MEMORY_BARRIER_H_

#include "rcar-xos/osal/r_osal.h"

static inline void taurus_memory_cache_invalidate(void *addr, uint32_t size)
{
    R_OSAL_CacheInvalidate((size_t)addr, (size_t)size, R_OSAL_CACHE_D);
}

static inline void taurus_memory_cache_flush(void *addr, uint32_t size)
{
    R_OSAL_CacheFlush((size_t)addr, (size_t)size, R_OSAL_CACHE_D);
}

#endif /* _TAURUS_MEMORY_BARRIER_H_ */

