/*
 * Guest MMU caches flushing on KVM host
 *
 * Copyright 2016 Salavat S. Gilyazov (atic@mcst.ru)
 */
#ifndef _E2K_KVM_TLBFLUSH_H
#define _E2K_KVM_TLBFLUSH_H

#include <linux/mm.h>
#include <linux/kvm_host.h>

#include <asm/kvm/hypercall.h>

/*
 * Guest VM support on host
 * TLB flushing:
 *
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr) flushes one page
 *  - flush_tlb_range(mm, start, end) flushes a range of pages
 *  - flush_tlb_pgtables(mm, start, end) flushes a range of page tables
 */

extern void mmu_pv_flush_tlb_address(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t addr);
extern void mmu_pv_flush_tlb_address_pgtables(struct kvm_vcpu *vcpu,
			gmm_struct_t *gmm,
			e2k_addr_t addr);
extern void mmu_pv_flush_tlb_page(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t addr);
extern void mmu_pv_flush_tlb_mm(struct kvm_vcpu *vcpu, gmm_struct_t *gmm);
extern void mmu_pv_flush_tlb_range(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			const e2k_addr_t start, const e2k_addr_t end);
extern void mmu_pv_flush_pmd_tlb_range(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			unsigned long start, unsigned long end);
extern void mmu_pv_flush_tlb_kernel_range(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			const e2k_addr_t start, const e2k_addr_t end);
extern void mmu_pv_flush_tlb_pgtables(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);
extern void mmu_pv_flush_tlb_range_and_pgtables(struct kvm_vcpu *vcpu,
			gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);
extern void mmu_pv_flush_tlb_page_and_pgtables(struct kvm_vcpu *vcpu,
			gmm_struct_t *gmm,
			unsigned long address);
extern void mmu_pv_flush_cpu_root_pt_mm(struct kvm_vcpu *vcpu, gmm_struct_t *gmm);
extern void mmu_pv_flush_cpu_root_pt(struct kvm_vcpu *vcpu);

extern long kvm_pv_sync_and_flush_tlb(struct kvm_vcpu *vcpu,
			mmu_spt_flush_t __user *flush_user);
extern long kvm_pv_sync_addr_range(struct kvm_vcpu *vcpu,
			gva_t start_gva, gva_t end_gva);

extern void mmu_pv_smp_flush_tlb_mm(struct kvm_vcpu *vcpu, gmm_struct_t *gmm);
extern void mmu_pv_smp_flush_tlb_page(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t addr);
extern void mmu_pv_smp_flush_tlb_range(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);
extern void mmu_pv_smp_flush_pmd_tlb_range(struct kvm_vcpu *vcpu, gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);
extern void mmu_pv_smp_flush_tlb_kernel_range(struct kvm_vcpu *vcpu,
			gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);
extern void mmu_pv_smp_flush_tlb_range_and_pgtables(struct kvm_vcpu *vcpu,
			gmm_struct_t *gmm,
			e2k_addr_t start, e2k_addr_t end);

extern void host_flush_shadow_pt_tlb_range(struct kvm_vcpu *vcpu,
			gva_t start, gva_t end, pgprot_t spte, int level);

extern void host_flush_shadow_pt_level_tlb(struct kvm *kvm, gmm_struct_t *gmm,
			gva_t gva, int level, pgprot_t new_spte, pgprot_t old_spte);

#ifndef	CONFIG_SMP
#define host_flush_tlb_mm(vcpu, gmm)	\
		mmu_pv_flush_tlb_mm(vcpu, gmm)
#define host_flush_tlb_page(vcpu, gmm, addr)	\
		mmu_pv_flush_tlb_page(vcpu, gmm, addr)
#define host_flush_tlb_range(vcpu, gmm, start, end) \
		mmu_pv_flush_tlb_range(vcpu, gmm, start, end)
#define host_flush_pmd_tlb_range(vcpu, gmm, start, end) \
		mmu_pv_flush_pmd_tlb_range(vcpu, gmm, start, end)
#define host_flush_tlb_mm_range(vcpu, gmm, start, end) \
		mmu_pv_flush_tlb_range(vcpu, gmm, start, end)
#define host_flush_tlb_kernel_range(vcpu, gmm, start, end) \
		mmu_pv_flush_tlb_kernel_range(vcpu, gmm, start, end)
#define host_flush_tlb_range_and_pgtables(vcpu, gmm, start, end) \
		mmu_pv_flush_tlb_range_and_pgtables(vcpu, gmm, start, end)
#else	/* CONFIG_SMP */
#define host_flush_tlb_mm(vcpu, gmm)	\
		mmu_pv_smp_flush_tlb_mm(vcpu, gmm)
#define host_flush_tlb_page(vcpu, gmm, addr)	\
		mmu_pv_smp_flush_tlb_page(vcpu, gmm, addr)
#define host_flush_tlb_range(vcpu, gmm, start, end) \
		mmu_pv_smp_flush_tlb_range(vcpu, gmm, start, end)
#define host_flush_pmd_tlb_range(vcpu, gmm, start, end) \
		mmu_pv_smp_flush_pmd_tlb_range(vcpu, gmm, start, end)
#define host_flush_tlb_kernel_range(vcpu, gmm, start, end) \
		mmu_pv_smp_flush_tlb_kernel_range(vcpu, gmm, start, end)
#define host_flush_tlb_mm_range(vcpu, gmm, start, end) \
		mmu_pv_smp_flush_tlb_range(vcpu, gmm, start, end)
#define host_flush_tlb_range_and_pgtables(vcpu, gmm, start, end) \
		mmu_pv_smp_flush_tlb_range_and_pgtables(vcpu, gmm, start, end)
#endif	/* !CONFIG_SMP */

#endif /* _E2K_KVM_TLBFLUSH_H */
