#ifndef __TRIPLET_ENGINE_RESOURCE_MANAGER_INTERNALS_H__
#define __TRIPLET_ENGINE_RESOURCE_MANAGER_INTERNALS_H__

#include <stdbool.h>

#define TRIPLET_RESOURCE_MANAGER_MAX_NUM_OBJECTS 4096

extern bool t3_init_resource_manager(void);
extern void t3_close_resource_manager(void);

#endif // __TRIPLET_ENGINE_RESOURCE_MANAGER_INTERNALS_H__
