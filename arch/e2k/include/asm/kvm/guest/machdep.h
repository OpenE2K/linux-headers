#ifndef _E2K_KVM_GUEST_MACHDEP_H_
#define _E2K_KVM_GUEST_MACHDEP_H_

#include <linux/init.h>
#include <linux/types.h>

#ifdef __KERNEL__

#ifdef	CONFIG_VIRTUALIZATION

typedef struct guest_machdep {
	/* only for guest kernel and machines */
	int		id;		/* guest machine Id */
	int		rev;		/* guest VCPU revision */
	unsigned char	iset_ver;	/* Instruction set version */

	/* guest interface functions */
} guest_machdep_t;

#endif	/* CONFIG_VIRTUALIZATION */
#endif /* __KERNEL__ */

#endif /* _E2K_KVM_GUEST_MACHDEP_H_ */
