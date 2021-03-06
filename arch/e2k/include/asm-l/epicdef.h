#ifndef _ASM_L_EPICDEF_H
#define _ASM_L_EPICDEF_H

/*
 * Constants for EPICs (CEPIC, IOEPIC)
 */

#define	MAX_EPICS_ORDER	10

#define	EPIC_REGS_SIZE			0x2000
#define	IO_EPIC_REGS_SIZE		0x100000


/* CEPIC registers */
#define	CEPIC_CTRL	0x0
#define		CEPIC_CTRL_BSP_CORE	0x100
#define	CEPIC_ID	0x10
#define		CEPIC_ID_BIT_MASK	0x3ff
#define		CEPIC_ID_SHORT_VALID_BITS	4
#define		CEPIC_ID_SHORT_VALID_MASK	0xf
#define	CEPIC_CPR	0x70
#define		CEPIC_CPR_CORE_PRIORITY_SHIFT	8
#define	CEPIC_ESR	0x80
#define		CEPIC_ESR_BIT_MASK	0x7e0
#define	CEPIC_ESR2	0x90
#define	CEPIC_EOI	0xa0
#define	CEPIC_CIR	0xb0

#define	CEPIC_PMIRR 0x100
#define		CEPIC_PMIRR_NR_BITS	0x400
#define		CEPIC_PMIRR_NR_REGS	0x20
#define		CEPIC_PMIRR_NR_DREGS	0x10
#define	CEPIC_PNMIRR	0x1e0
#define		CEPIC_PNMIRR_BIT_MASK	0x33e00
#define		CEPIC_PNMIRR_NMI	0x400
#define		CEPIC_PNMIRR_STARTUP	0x1000
#define		CEPIC_PNMIRR_STARTUP_ENTRY	0xff
#define	CEPIC_ESR_NEW	0x1f0

#define	CEPIC_ICR	0x200
#define		CEPIC_ICR_DST_FULL	0
#define		CEPIC_ICR_DST_SELF	1
#define		CEPIC_ICR_DST_ALLBUT	2
#define		CEPIC_ICR_DST_ALLINC	3
#define		CEPIC_ICR_DLVM_FIXED_EXT	0
#define		CEPIC_ICR_DLVM_FIXED_IPI	1
#define		CEPIC_ICR_DLVM_SMI	2
#define		CEPIC_ICR_DLVM_NM_SPECIAL	3
#define		CEPIC_ICR_DLVM_NMI	4
#define		CEPIC_ICR_DLVM_INIT	5
#define		CEPIC_ICR_DLVM_STARTUP	6
#define	CEPIC_ICR2	0x204
#define	CEPIC_TIMER_LVTT	0x220
#define	CEPIC_TIMER_INIT	0x230
#define	CEPIC_TIMER_CUR	0x240
#define	CEPIC_TIMER_DIV	0x250
#define		CEPIC_TIMER_DIV_1	0xb
#define	CEPIC_NM_TIMER_LVTT	0x260
#define	CEPIC_NM_TIMER_INIT	0x270
#define	CEPIC_NM_TIMER_CUR	0x280
#define	CEPIC_NM_TIMER_DIV	0x290
#define	CEPIC_SVR	0x2a0
#define	CEPIC_PNMIRR_MASK	0x2d0
#define	CEPIC_VECT_INTA	0x2f0
#define		CEPIC_VECT_INTA_VMASK		0x3ff
#define		CEPIC_VECT_INTA_PRI_SHIFT	16

/* CEPIC (HP) registers */
#define	CEPIC_GUEST	0x1000

#define	CEPIC_CTRL2	0x1820
#define	CEPIC_DAT	0x1830
#define		CEPIC_DAT_READ	0
#define		CEPIC_DAT_INVALIDATE	2
#define		CEPIC_DAT_WRITE	3
#define	CEPIC_DAT2	0x1834
#define	CEPIC_EPIC_INT	0x1850
#define	CEPIC_EPIC_INT2	0x1860
#define	CEPIC_EPIC_INT3	0x1864
#define	CEPIC_GSTBASE_LO	0x18c0
#define	CEPIC_GSTBASE_HI	0x18c4
#define	CEPIC_GSTID	0x18d0

#define	CEPIC_PMIRR_OR	0x1900
#define	CEPIC_PNMIRR_OR	0x19e0
#define	CEPIC_ESR_NEW_OR	0x19f0

#define CEPIC_PNMIRR_INT_VIOLAT_BIT	13

#define	BAD_EPICID	0xffff

#endif /* _ASM_L_EPICDEF_H */
