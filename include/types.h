#ifndef _TYPES_H_
#define _TYPES_H_


#define DEBUG_CODE      0x00000
//1. thread
#define DEBUG_THREAD    0x00001
//2. fork
#define DEBUG_FORK      0x00010
//3. exit
#define DEBUG_EXIT      0x00100
//4. waitpid
#define DEBUG_WP        0x01000
//5. execv
#define DEBUG_EXECV     0x10000
/*
 * The model for the include files in the kernel is as follows:
 *
 *     1. Every source file includes this file, <types.h>, first.
 *
 *     2. Every other header file may assume this file has been
 *        included, but should explicitly include any other headers
 *        it requires to compile.
 */


/* Get machine-dependent types visible to userland */
#include <machine/types.h>

/* Get machine-independent types visible to userland */
#include <kern/types.h>

/* Get machine-dependent types not visible to userland */
#include <machine/ktypes.h>

/*
 * Define userptr_t as a pointer to a one-byte struct, so it won't mix
 * with other pointers.
 */

struct __userptr { char _dummy; };
typedef struct __userptr *userptr_t;
typedef const struct __userptr *const_userptr_t;

#endif /* _TYPES_H_ */
