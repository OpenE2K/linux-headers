/*
 * KVM guest stacks support
 * Copyright 2017 Salavat S. Guiliazov (atic@mcst.ru)
 */

#ifndef _E2K_KVM_STACKS_H
#define _E2K_KVM_STACKS_H

#include <asm/kvm/guest/stacks.h>

#ifdef	CONFIG_VIRTUALIZATION
#ifdef	CONFIG_KVM_GUEST
/*
 * Guest kernel thread stacks descriptions
 */
#define	VIRT_KERNEL_C_STACK_SIZE	\
		(KVM_GUEST_KERNEL_C_STACK_SIZE + KVM_GUEST_KERNEL_C_STACK_OFFSET)
#define	VIRT_KERNEL_PS_SIZE		KVM_GUEST_KERNEL_PS_SIZE
#define	VIRT_KERNEL_PS_INIT_SIZE	KVM_GUEST_KERNEL_PS_INIT_SIZE
#define	VIRT_KERNEL_PCS_SIZE		KVM_GUEST_KERNEL_PCS_SIZE
#define	VIRT_KERNEL_PCS_INIT_SIZE	KVM_GUEST_KERNEL_PCS_INIT_SIZE

#define VIRT_KERNEL_P_STACK_PAGES	(VIRT_KERNEL_PS_SIZE / PAGE_SIZE)
#define VIRT_KERNEL_PC_STACK_PAGES	(VIRT_KERNEL_PCS_SIZE / PAGE_SIZE)

/*
 * Guest user task stacks descriptions
 */
#define	VIRT_USER_C_STACK_SIZE		KVM_GUEST_USER_C_STACK_SIZE
#define	VIRT_USER_PS_SIZE		KVM_GUEST_USER_PS_SIZE
#define	VIRT_USER_PS_INIT_SIZE		KVM_GUEST_USER_PS_INIT_SIZE
#define	VIRT_USER_PCS_SIZE		KVM_GUEST_USER_PCS_SIZE
#define	VIRT_USER_PCS_INIT_SIZE		KVM_GUEST_USER_PCS_INIT_SIZE

#else	/* ! CONFIG_KVM_GUEST */
 #error	"Unknown virtualization type:
#endif	/* CONFIG_KVM_GUEST*/

#else	/* ! CONFIG_VIRTUALIZATION */
#define VIRT_KERNEL_P_STACK_PAGES	0
#define VIRT_KERNEL_PC_STACK_PAGES	0
#endif	/* CONFIG_VIRTUALIZATION */

#endif /* ! _E2K_KVM_STACKS_H */
