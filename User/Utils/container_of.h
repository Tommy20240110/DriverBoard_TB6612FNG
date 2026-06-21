/* container_of.h */
#ifndef USER_CONTAINER_OF_H
#define USER_CONTAINER_OF_H

#include <stddef.h>

#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
