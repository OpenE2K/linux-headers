#ifndef _ASM_PTRACE_ABI_H
#define _ASM_PTRACE_ABI_H

#define PTRACE_OLDSETOPTIONS      21

/* only useful for access 32bit programs / kernels */
#define PTRACE_GET_THREAD_AREA    25
#define PTRACE_SET_THREAD_AREA    26

#ifdef __x86_64__
# define PTRACE_ARCH_PRCTL	  30
#endif

#define PTRACE_SYSEMU		  31
#define PTRACE_SYSEMU_SINGLESTEP  32

#define PTRACE_SINGLEBLOCK	33	/* resume execution until next branch */

#ifndef __ASSEMBLY__
#include <linux/types.h>

/* configuration/status structure used in PTRACE_BTS_CONFIG and
   PTRACE_BTS_STATUS commands.
*/
struct ptrace_bts_config {
	/* requested or actual size of BTS buffer in bytes */
	__u32 size;
	/* bitmask of below flags */
	__u32 flags;
	/* buffer overflow signal */
	__u32 signal;
	/* actual size of bts_struct in bytes */
	__u32 bts_size;
};
#endif /* __ASSEMBLY__ */

#define PTRACE_BTS_O_TRACE	0x1 /* branch trace */
#define PTRACE_BTS_O_SCHED	0x2 /* scheduling events w/ jiffies */
#define PTRACE_BTS_O_SIGNAL     0x4 /* send SIG<signal> on buffer overflow
				       instead of wrapping around */
#define PTRACE_BTS_O_ALLOC	0x8 /* (re)allocate buffer */

#define PTRACE_BTS_CONFIG	40
/* Configure branch trace recording.
   ADDR points to a struct ptrace_bts_config.
   DATA gives the size of that buffer.
   A new buffer is allocated, if requested in the flags.
   An overflow signal may only be requested for new buffers.
   Returns the number of bytes read.
*/
#define PTRACE_BTS_STATUS	41
/* Return the current configuration in a struct ptrace_bts_config
   pointed to by ADDR; DATA gives the size of that buffer.
   Returns the number of bytes written.
*/
#define PTRACE_BTS_SIZE		42
/* Return the number of available BTS records for draining.
   DATA and ADDR are ignored.
*/
#define PTRACE_BTS_GET		43
/* Get a single BTS record.
   DATA defines the index into the BTS array, where 0 is the newest
   entry, and higher indices refer to older entries.
   ADDR is pointing to struct bts_struct (see asm/ds.h).
*/
#define PTRACE_BTS_CLEAR	44
/* Clear the BTS buffer.
   DATA and ADDR are ignored.
*/
#define PTRACE_BTS_DRAIN	45
/* Read all available BTS records and clear the buffer.
   ADDR points to an array of struct bts_struct.
   DATA gives the size of that buffer.
   BTS records are read from oldest to newest.
   Returns number of BTS records drained.
*/

#endif /* _ASM_PTRACE_ABI_H */
