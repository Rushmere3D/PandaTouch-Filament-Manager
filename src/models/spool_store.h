#pragma once

#include "spool_info.h"

constexpr size_t MAX_SPOOLS = 100;

static SpoolInfo spool_store[MAX_SPOOLS];
static size_t spool_count = 0;

inline void spool_store_clear()
{
    spool_count = 0;
}

inline bool spool_store_add(const SpoolInfo &spool)
{
    if (spool_count >= MAX_SPOOLS)
    {
        return false;
    }

    spool_store[spool_count] = spool;
    spool_count++;

    return true;
}