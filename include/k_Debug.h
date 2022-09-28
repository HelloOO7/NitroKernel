#ifndef __K_DEBUG_H
#define __K_DEBUG_H

#include "kPrint.h"

#ifdef DEBUG
#define K_DEBUG_PRINTF(...) k::Printf(__VA_ARGS__)
#else
#define K_DEBUG_PRINTF(...)
#endif

#endif