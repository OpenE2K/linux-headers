#ifndef MMU_PTE_H
#define MMU_PTE_H

/*			puwx	(u - user mode, w - writable, x executable) */
/*				(p - user privileged access to hardware stacks) */
#define ACC_EXEC_MASK	0x1
#define ACC_WRITE_MASK	0x2
#define	ACC_USER_MASK	0x4
#define	ACC_PRIV_MASK	0x8	/* only for user privileged access: hw stacks */
#define ACC_ALL		(ACC_EXEC_MASK | ACC_WRITE_MASK | ACC_USER_MASK)
/* guest user acces can include special case: privileged hardware stacks */
#define	ACC_USER_ALL	(ACC_ALL | ACC_PRIV_MASK)
/* page tables directories always privileged & not executable */
#define	ACC_PT_DIR	(ACC_WRITE_MASK)

#define PFERR_PRESENT_BIT	0
#define PFERR_WRITE_BIT		1
#define PFERR_USER_BIT		2
#define PFERR_RSVD_BIT		3
#define PFERR_FETCH_BIT		4
#define	PFERR_NOT_PRESENT_BIT	5
#define	PFERR_PT_FAULT_BIT	6
#define	PFERR_INSTR_FAULT_BIT	7
#define	PFERR_INSTR_PROT_BIT	8
#define	PFERR_FORCED_BIT	9
#define	PFERR_WAIT_LOCK_BIT	10
#define	PFERR_GPTE_CHANGED_BIT	11
#define	PFERR_MMIO_BIT		12
#define	PFERR_ONLY_VALID_BIT	13
#define	PFERR_READ_PROT_BIT	14
#define	PFERR_IS_UNMAPPED_BIT	15
#define	PFERR_FAPB_BIT		16
#define	PFERR_HW_ACCESS_BIT	17
#define	PFERR_USER_ADDR_BIT	18
#define	PFERR_ILLEGAL_PAGE_BIT	19

#define	PFERR_ACCESS_SIZE_BIT	24

#define PFERR_PRESENT_MASK	(1U << PFERR_PRESENT_BIT)
#define PFERR_WRITE_MASK	(1U << PFERR_WRITE_BIT)
#define PFERR_USER_MASK		(1U << PFERR_USER_BIT)
#define PFERR_RSVD_MASK		(1U << PFERR_RSVD_BIT)
#define PFERR_FETCH_MASK	(1U << PFERR_FETCH_BIT)
#define	PFERR_NOT_PRESENT_MASK	(1U << PFERR_NOT_PRESENT_BIT)
#define	PFERR_PT_FAULT_MASK	(1U << PFERR_PT_FAULT_BIT)
#define	PFERR_INSTR_FAULT_MASK	(1U << PFERR_INSTR_FAULT_BIT)
#define	PFERR_INSTR_PROT_MASK	(1U << PFERR_INSTR_PROT_BIT)
#define	PFERR_FORCED_MASK	(1U << PFERR_FORCED_BIT)
#define	PFERR_WAIT_LOCK_MASK	(1U << PFERR_WAIT_LOCK_BIT)
#define	PFERR_GPTE_CHANGED_MASK	(1U << PFERR_GPTE_CHANGED_BIT)
#define	PFERR_MMIO_MASK		(1U << PFERR_MMIO_BIT)
#define	PFERR_ONLY_VALID_MASK	(1U << PFERR_ONLY_VALID_BIT)
#define	PFERR_READ_PROT_MASK	(1U << PFERR_READ_PROT_BIT)
#define	PFERR_IS_UNMAPPED_MASK	(1U << PFERR_IS_UNMAPPED_BIT)
#define	PFERR_FAPB_MASK		(1U << PFERR_FAPB_BIT)
#define	PFERR_HW_ACCESS_MASK	(1U << PFERR_HW_ACCESS_BIT)
#define	PFERR_USER_ADDR_MASK	(1U << PFERR_USER_ADDR_BIT)
#define	PFERR_ILLEGAL_PAGE_MASK	(1U << PFERR_ILLEGAL_PAGE_BIT)

#define	PFERR_ACCESS_SIZE_MASK	(~0U << PFERR_ACCESS_SIZE_BIT)

#define	PFRES_GET_ACCESS_SIZE(pfres)	\
		(((pfres) & PFERR_ACCESS_SIZE_MASK) >> PFERR_ACCESS_SIZE_BIT)
#define	PFRES_SET_ACCESS_SIZE(pfres, size)	\
		(((pfres) & ~PFERR_ACCESS_SIZE_MASK) | \
			((size) << PFERR_ACCESS_SIZE_BIT))

typedef struct gw_attr {
	int level;
	u32 access;
} gw_attr_t;

#endif /* MMU_PTE_H */
