/* $Id: tlbflush.h,v 1.3 2006/09/12 13:12:54 tokot Exp $
 * pgalloc.h: the functions and defines necessary to allocate
 * page tables.
 *
 * Copyright 2001 Salavat S. Guiliazov (atic@mcst.ru)
 */
#ifndef _E2K_TLBFLUSH_H
#define _E2K_TLBFLUSH_H

#include <linux/mm.h>


/*
 * TLB flushing:
 *
 *  - flush_tlb_all() flushes all processes TLBs
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr) flushes one page
 *  - flush_tlb_range(mm, start, end) flushes a range of pages
 *
 * "local_" versions flush on the calling CPU only.
 */

/**
 * native_flush_tlb_mm_range - flush specified range from all TLU caches
 *		       (i.e. from L1D, L1I, DTLB, TLU_CACHE.PWC, TLU_CACHE.GWC)
 * @mm: contains hardware context (called ASID on other arch-es)
 * @start: start of the range to flush
 * @end: end of the range to flush
 * @stride: distance between consequential flushes
 * @levels_mask: set if there are intermediate page table levels to flush.
 *
 * Note that @levels_mask should be 0 if we want to flush only the last level.
 * For example, to flush just huge PMDs: levels_mask=0 and stride=PMD_SIZE.
 *
 * Macros are defined for convenience:
 *   FLUSH_TLB_LEVELS_ALL - flush all levels
 *   FLUSH_TLB_LEVELS_LAST - flush last level only (be it huge mapping or not)
 *   FLUSH_TLB_LEVELS_INTERMEDIATE - flush everything but the last level
 */
#define FLUSH_TLB_LEVELS_ALL	(E2K_PAGES_LEVEL_MASK | E2K_PTE_LEVEL_MASK | \
				 E2K_PMD_LEVEL_MASK | E2K_PUD_LEVEL_MASK)
#define FLUSH_TLB_LEVELS_LAST	E2K_PAGES_LEVEL_MASK
#define FLUSH_TLB_LEVELS_INTERMEDIATE	(E2K_PTE_LEVEL_MASK | \
					 E2K_PMD_LEVEL_MASK | E2K_PUD_LEVEL_MASK)
extern void native_flush_tlb_mm_range(struct mm_struct *mm,
		unsigned long start, unsigned long end,
		unsigned long stride, u32 levels_mask);
extern void native_flush_tlb_page(struct mm_struct *mm, unsigned long addr);
extern void native_flush_tlb_mm(struct mm_struct *mm);
extern void native_flush_tlb_all(void);


/* Linux assumes that flush_tlb_range() interface flushes
 * all levels (this may be changed in future versions in
 * which case we can pass FLUSH_TLB_LEVELS_LAST instead).
 *
 * E.g. we may be invalidating table entries as part of
 * collapsing hugepages or moving page tables. */
#define native_flush_tlb_range(mm, start, end) \
	native_flush_tlb_mm_range((mm), (start), (end), \
				  PAGE_SIZE, FLUSH_TLB_LEVELS_ALL)

#define native_flush_tlb_range_and_pgtables(mm, start, end) \
	native_flush_tlb_mm_range((mm), (start), (end), \
				  PAGE_SIZE, FLUSH_TLB_LEVELS_ALL)

#define native_flush_pmd_tlb_range(mm, start, end) \
	native_flush_tlb_mm_range((mm), (start), (end), \
				  PMD_SIZE, FLUSH_TLB_LEVELS_LAST)

static inline void native_flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	native_flush_tlb_all();
}

extern void generic_flush_tlb_mm(struct mm_struct *mm, mm_context_t *context,
		cpumask_t *mm_cpumask, bool trace_enabled);
extern void generic_flush_tlb_page(struct mm_struct *mm, mm_context_t *context,
		cpumask_t *mm_cpumask, unsigned long addr, bool trace_enabled);
extern void generic_flush_tlb_mm_range(struct mm_struct *mm, mm_context_t *context,
		cpumask_t *mm_cpumask, unsigned long start, unsigned long end,
		unsigned long stride, u32 levels_mask, bool trace_enabled);

extern void native_local_flush_tlb_mm_range(struct mm_struct *mm, unsigned long start,
		unsigned long end, unsigned long stride, u32 levels_mask);
extern void native_local_flush_tlb_page(struct mm_struct *mm, unsigned long addr);
extern void native_local_flush_tlb_mm(struct mm_struct *mm);
extern void native_local_flush_tlb_all(void);
static inline void native_local_flush_tlb_page_and_pgtables(
		struct mm_struct *mm, unsigned long uaddr)
{
	native_local_flush_tlb_mm_range(mm, uaddr, uaddr + 1,
			PAGE_SIZE, FLUSH_TLB_LEVELS_ALL);
}

#ifdef	CONFIG_COPY_USER_PGD_TO_KERNEL_ROOT_PT
extern void __flush_cpu_root_pt_mm(struct mm_struct *mm);
extern void __flush_cpu_root_pt(void);
#else	/* ! CONFIG_COPY_USER_PGD_TO_KERNEL_ROOT_PT */
#define	__flush_cpu_root_pt_mm(mm)
#define	__flush_cpu_root_pt()
#endif	/* CONFIG_COPY_USER_PGD_TO_KERNEL_ROOT_PT */


#if	defined(CONFIG_PARAVIRT_GUEST)
/* it is paravirtualized host and guest kernel */
# include <asm/paravirt/tlbflush.h>
#elif	defined(CONFIG_KVM_GUEST_KERNEL)
/* it is native guest kernel */
#include <asm/kvm/guest/tlbflush.h>
# define flush_tlb_all			kvm_flush_tlb_all
# define flush_tlb_kernel_range		kvm_flush_tlb_kernel_range
# define flush_tlb_mm			kvm_flush_tlb_mm
# define flush_tlb_page(vma, addr)	kvm_flush_tlb_page((vma)->vm_mm, (addr))
# define flush_tlb_mm_range		kvm_flush_tlb_mm_range
# define flush_tlb_range_and_pgtables	kvm_flush_tlb_range_and_pgtables
# define flush_tlb_range(vma, start, end) \
		kvm_flush_tlb_range((vma)->vm_mm, (start), (end))
# define flush_pmd_tlb_range(vma, start, end) \
		kvm_flush_pmd_tlb_range((vma)->vm_mm, (start), (end))
/* local_*() versions are for flushing current CPU only.
 * A bit of an overkill: flushing whole mm instead of just current CPU... */
# define local_flush_tlb_mm_range	kvm_flush_tlb_mm_range
# define local_flush_tlb_page		kvm_flush_tlb_page
# define local_flush_tlb_mm		kvm_flush_tlb_mm
# define local_flush_tlb_page_and_pgtables kvm_flush_tlb_page_and_pgtables
/* TODO FIXME write flush_tlb_kernel_range_nmi() function and use it
 * in set_memory_attr() */
# define local_flush_tlb_all() \
		pr_warn_once("%s(): try to flush whole local TLB : op 0x%lx\n", \
			     __func__, flush_op_tlb_all)
#else	/* ! CONFIG_PARAVIRT_GUEST && ! CONFIG_KVM_GUEST_KERNEL */
/* it is native kernel without virtualization support */
/* or native kernel with virtualization support */
# define flush_tlb_all			native_flush_tlb_all
# define flush_tlb_kernel_range		native_flush_tlb_kernel_range
# define flush_tlb_mm			native_flush_tlb_mm
# define flush_tlb_page(vma, addr)	native_flush_tlb_page((vma)->vm_mm, (addr))
# define flush_tlb_mm_range		native_flush_tlb_mm_range
# define flush_tlb_range_and_pgtables	native_flush_tlb_range_and_pgtables
# define flush_tlb_range(vma, start, end) \
		native_flush_tlb_range((vma)->vm_mm, (start), (end))
# define flush_pmd_tlb_range(vma, start, end) \
		native_flush_pmd_tlb_range((vma)->vm_mm, (start), (end))
/* local_*() versions are for flushing current CPU only */
# define local_flush_tlb_mm_range	native_local_flush_tlb_mm_range
# define local_flush_tlb_page		native_local_flush_tlb_page
# define local_flush_tlb_mm		native_local_flush_tlb_mm
# define local_flush_tlb_all		native_local_flush_tlb_all
# define local_flush_tlb_page_and_pgtables \
		native_local_flush_tlb_page_and_pgtables
#endif	/* ! CONFIG_PARAVIRT_GUEST && ! CONFIG_KVM_GUEST_KERNEL */

static inline void update_mmu_cache(struct vm_area_struct *vma,
		unsigned long address, pte_t *pte)
{
}

static inline void update_mmu_cache_pmd(struct vm_area_struct *vma,
		unsigned long address, pmd_t *pmd)
{
}

#ifdef	CONFIG_KVM_HOST_MODE
#include <asm/kvm/tlbflush.h>
#endif	/* CONFIG_KVM_HOST_MODE */

#endif /* _E2K_TLBFLUSH_H */
