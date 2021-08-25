/*
 * TLB context support & flushing
 */

#ifndef _E2K_TLB_CONTEXT_H_
#define _E2K_TLB_CONTEXT_H_

#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/mutex.h>

#include <asm/mmu_context.h>
#include <asm/mmu_regs.h>
#include <asm/trace-tlb-flush.h>

#undef	DEBUG_PT_MODE
#undef	DebugPT
#define	DEBUG_PT_MODE		0	/* Data Caches */
#define DebugPT(...)		DebugPrint(DEBUG_PT_MODE, ##__VA_ARGS__)

/*
 * TLB flushing:
 */

/*
 *  Flush all processes TLBs of the processor
 */
static inline void
mmu_pid_flush_tlb_all(bool trace_enabled)
{
	flush_TLB_all();
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_all(raw_smp_processor_id());
	}
}

/*
 * Flush a specified user mapping on the processor
 */
static inline void
mmu_pid_flush_tlb_mm(mm_context_t *context,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int cpu;
	unsigned long old_pid, pid;

	count_vm_tlb_event(NR_TLB_LOCAL_FLUSH_ALL);

	cpu = raw_smp_processor_id();
	old_pid = context->cpumsk[cpu];

	if (likely(is_active)) {
		unsigned long pid, flags;

		/* Should update right now */
		DebugPT("mm context will be reloaded\n");
		raw_all_irq_save(flags);
		cpu = smp_processor_id();
		pid = get_new_mmu_pid(context, cpu);
		reload_context_mask(pid);
		raw_all_irq_restore(flags);

		DebugPT("CPU #%d new mm context is 0x%lx\n",
				cpu, context->cpumsk[cpu]);
	} else {
#ifdef CONFIG_SMP
		/* Remove this cpu from mm_cpumask. This might be
		 * needed, for example, after sys_io_setup() if the
		 * kernel thread which was using this mm received
		 * flush ipi (unuse_mm() does not clear mm_cpumask).
		 * And maybe there are other such places where
		 * a kernel thread uses user mm. */
		if (likely(mm_cpumask != NULL)) {
			cpumask_clear_cpu(cpu, mm_cpumask);
		}
#endif
		context->cpumsk[cpu] = 0;
		pid = 0;
	}
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_mm(cpu, context, is_active, old_pid, pid);
	}
}

/*
 * Flush just one specified address of current process.
 */
static inline void
mmu_pid_flush_tlb_address(mm_context_t *context,
		e2k_addr_t addr,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	old_pid = context->cpumsk[cpu];

	if (unlikely(old_pid == 0)) {
		/* See comment in __flush_tlb_range(). */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = context->cpumsk[cpu];
	} else {
		count_vm_tlb_event(NR_TLB_LOCAL_FLUSH_ONE);
		pid = old_pid;
		flush_TLB_page(addr, CTX_HARDWARE(pid));
	}
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_address(cpu, context, addr, old_pid, pid);
	}
}

/*
 * Flush the TLB entries mapping the virtually mapped linear page
 * table corresponding to specified address of current process.
 */
static inline void
mmu_pid_flush_tlb_address_pgtables(mm_context_t *context,
		e2k_addr_t addr,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	old_pid = context->cpumsk[cpu];

	if (unlikely(old_pid == 0)) {
		/* See comment in __flush_tlb_range(). */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = context->cpumsk[cpu];
	} else {
		pid = old_pid;
		flush_TLB_page_begin();
		/* flush virtual mapping of PTE entry (third level) */
		__flush_TLB_page(pte_virt_offset(_PAGE_ALIGN_UP(addr,
								PTE_SIZE)),
				 CTX_HARDWARE(pid));
		/* flush virtual mapping of PMD entry (second level) */
		__flush_TLB_page(pmd_virt_offset(_PAGE_ALIGN_UP(addr,
								PMD_SIZE)),
				 CTX_HARDWARE(pid));
		/* flush virtual mapping of PUD entry (first level) */
		__flush_TLB_page(pud_virt_offset(_PAGE_ALIGN_UP(addr,
								PUD_SIZE)),
				 CTX_HARDWARE(pid));
		flush_TLB_page_end();
	}
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_address(cpu, context, addr, old_pid, pid);
		trace_mmu_pid_flush_tlb_address(cpu, context,
			pte_virt_offset(_PAGE_ALIGN_UP(addr, PTE_SIZE)),
			old_pid, pid);
		trace_mmu_pid_flush_tlb_address(cpu, context,
			pmd_virt_offset(_PAGE_ALIGN_UP(addr, PMD_SIZE)),
			old_pid, pid);
		trace_mmu_pid_flush_tlb_address(cpu, context,
			pud_virt_offset(_PAGE_ALIGN_UP(addr, PUD_SIZE)),
			old_pid, pid);
	}
}

/*
 * Flush just one page of a specified user.
 */
static inline void
mmu_pid_flush_tlb_page(mm_context_t *context,
		e2k_addr_t addr,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	old_pid = context->cpumsk[cpu];

	if (unlikely(old_pid == 0)) {
		/* See comment in __flush_tlb_range(). */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = context->cpumsk[cpu];
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_address(cpu, context, addr,
							old_pid, pid);
		}
		return;
	}

	count_vm_tlb_event(NR_TLB_LOCAL_FLUSH_ONE);

	pid = old_pid;
	flush_TLB_page_begin();
	__flush_TLB_page(addr, CTX_HARDWARE(pid));
	/* flush virtual mapping of PTE entry (third level) */
	__flush_TLB_page(pte_virt_offset(addr), CTX_HARDWARE(pid));
	flush_TLB_page_end();
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_address(cpu, context, addr, old_pid, pid);
		trace_mmu_pid_flush_tlb_address(cpu, context,
			pte_virt_offset(_PAGE_ALIGN_UP(addr, PTE_SIZE)),
			old_pid, pid);
	}
}

/*
 * Flush a specified range of pages
 */

/* If the number of pages to be flushed is below this value,
 * then only those pages will be flushed.
 *
 * Flushing one page takes ~150 cycles, flushing the whole mm
 * takes ~400 cycles. Also note that __flush_tlb_range() may
 * be called repeatedly for the same process so high values
 * are bad. */
#define FLUSH_TLB_RANGE_MAX_PAGES 8

static inline void
mmu_pid_flush_tlb_range(mm_context_t *context,
		const e2k_addr_t start, const e2k_addr_t end,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	const long pages_num = (PAGE_ALIGN_DOWN(end) - PAGE_ALIGN_UP(start))
			/ PAGE_SIZE;
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	BUG_ON(start > end);

	DebugPT("range start 0x%lx end 0x%lx context 0x%lx PID 0x%lx CPU #%d\n",
		PAGE_ALIGN_UP(start), PAGE_ALIGN_DOWN(end),
		CTX_HARDWARE(context->cpumsk[cpu]),
		context->cpumsk[cpu], cpu);

	old_pid = CTX_HARDWARE(context->cpumsk[cpu]);
	if (pages_num <= FLUSH_TLB_RANGE_MAX_PAGES) {
		unsigned long page, pmd_start, pmd_end;

		if (unlikely(old_pid == 0)) {
			/* We were trying to flush a range of pages,
			 * but someone is flushing the whole mm.
			 * Now we cannot flush pages (we do not know
			 * the context) so we have to flush the whole mm.
			 *
			 * Even if we will receive the flush ipi we will
			 * just end up flushing mm twice - which is OK
			 * considering how rare this case is. */
			goto flush_mm;
		}

		count_vm_tlb_events(NR_TLB_LOCAL_FLUSH_ONE, pages_num);

		pid = old_pid;
		flush_TLB_page_begin();
		for (page = PAGE_ALIGN_UP(start); page < end;
				page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		/*
		 * flush virtual mapping of PTE entry (third level)
		 *
		 * Needed because Linux assumes that flush_tlb_*()
		 * interfaces flush both pte and pmd levels (this
		 * may be changed in future versions, in which case
		 * this flush can be removed).
		 */
		pmd_start = pte_virt_offset(round_down(start, PMD_SIZE));
		pmd_end = pte_virt_offset(round_up(end, PMD_SIZE));
		for (page = round_down(pmd_start, PAGE_SIZE);
				page < pmd_end; page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		flush_TLB_page_end();
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
			trace_mmu_pid_flush_tlb_range(cpu, context,
					pmd_start, pmd_end, old_pid, pid);
		}
	} else {
flush_mm:
		/* Too many pages to flush.
		 * It is faster to change the context instead.
		 * If mm != current->active_mm then setting this
		 * CPU's mm context to 0 will do the trick,
		 * otherwise we duly increment it. */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = CTX_HARDWARE(context->cpumsk[cpu]);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
		}
	}
}

static inline void
mmu_pid_flush_pmd_tlb_range(mm_context_t *context,
		unsigned long start, unsigned long end,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	long pages_num;
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	BUG_ON(start > end);

	end = round_up(end, PMD_SIZE);
	start = round_down(start, PMD_SIZE);

	pages_num = (end - start) / PMD_SIZE;

	old_pid = CTX_HARDWARE(context->cpumsk[cpu]);
	if (pages_num <= FLUSH_TLB_RANGE_MAX_PAGES) {
		unsigned long pmd_start, pmd_end;
		e2k_addr_t page;

		if (unlikely(old_pid == 0)) {
			/* We were trying to flush a range of pages,
			 * but someone is flushing the whole mm.
			 * Now we cannot flush pages (we do not know
			 * the context) so we have to flush the whole mm.
			 *
			 * Even if we will receive the flush ipi we will
			 * just end up flushing mm twice - which is OK
			 * considering how rare this case is. */
			goto flush_mm;
		}

		count_vm_tlb_events(NR_TLB_LOCAL_FLUSH_ONE,
				pages_num * (PMD_SIZE / PTE_SIZE));

		pid = old_pid;
		flush_TLB_page_begin();
		for (page = start; page < end; page += PMD_SIZE)
			__flush_TLB_page(page, pid);
		/*
		 * flush virtual mapping of PTE entry (third level).
		 *
		 * When flushing high order page table entries,
		 * we must also flush all links below it. E.g. when
		 * flushing PMD, also flush PMD->PTE link (i.e. DTLB
		 * entry for address 0xff8000000000|(address >> 9)).
		 *
		 * Otherwise the following can happen:
		 * 1) High-order page is allocated.
		 * 2) Someone accesses the PMD->PTE link (e.g. half-spec. load)
		 * and creates invalid entry in DTLB.
		 * 3) High-order page is split into 4 Kb pages.
		 * 4) Someone accesses the PMD->PTE link address (e.g. DTLB
		 * entry probe) and reads the invalid entry created earlier.
		 */
		pmd_start = pte_virt_offset(round_down(start, PMD_SIZE));
		pmd_end = pte_virt_offset(round_up(end, PMD_SIZE));
		for (page = round_down(pmd_start, PAGE_SIZE);
				page < pmd_end; page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
			trace_mmu_pid_flush_tlb_range(cpu, context,
					pmd_start, pmd_end, old_pid, pid);
		}
		flush_TLB_page_end();
	} else {
flush_mm:
		/* Too many pages to flush.
		 * It is faster to change the context instead.
		 * If mm != current->active_mm then setting this
		 * CPU's mm context to 0 will do the trick,
		 * otherwise we duly increment it. */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = CTX_HARDWARE(context->cpumsk[cpu]);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
		}
	}
}

/*
 * Flush the TLB entries mapping the virtually mapped linear page
 * table corresponding to address range [start : end].
 */
static inline void
mmu_pid_flush_tlb_pgtables(mm_context_t *context,
		e2k_addr_t start, e2k_addr_t end,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	const long pages_num = (PAGE_ALIGN_DOWN(end) - PAGE_ALIGN_UP(start))
			/ PAGE_SIZE;
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, pid;

	BUG_ON(start > end);

	DebugPT("range start 0x%lx end 0x%lx context 0x%lx pid 0x%lx CPU #%d\n",
		PAGE_ALIGN_UP(start), PAGE_ALIGN_DOWN(end),
		CTX_HARDWARE(context->cpumsk[cpu]),
		context->cpumsk[cpu], cpu);

	old_pid = CTX_HARDWARE(context->cpumsk[cpu]);
	if (pages_num <= FLUSH_TLB_RANGE_MAX_PAGES) {
		e2k_addr_t page;
		unsigned long range_begin, range_end;

		if (unlikely(old_pid == 0)) {
			/* We were trying to flush a range of pages,
			 * but someone is flushing the whole mm.
			 * Now we cannot flush pages (we do not know
			 * the context) so we have to flush the whole mm.
			 *
			 * Even if we will receive the flush ipi we will
			 * just end up flushing mm twice - which is OK
			 * considering how rare this case is. */
			goto flush_mm;
		}

		pid = old_pid;
		flush_TLB_page_begin();

		/* flush virtual mapping of PTE entries (third level) */
		range_begin = pte_virt_offset(_PAGE_ALIGN_UP(start, PTE_SIZE));
		range_end = pte_virt_offset(_PAGE_ALIGN_DOWN(end, PTE_SIZE));
		for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
				page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					range_begin, range_end, old_pid, pid);
		}

		/* flush virtual mapping of PMD entries (second level) */
		range_begin = pmd_virt_offset(_PAGE_ALIGN_UP(start, PMD_SIZE));
		range_end = pmd_virt_offset(_PAGE_ALIGN_DOWN(end, PMD_SIZE));
		for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
				page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					range_begin, range_end, old_pid, pid);
		}

		/* flush virtual mapping of PUD entries (first level) */
		range_begin = pud_virt_offset(_PAGE_ALIGN_UP(start, PUD_SIZE));
		range_end = pud_virt_offset(_PAGE_ALIGN_DOWN(end, PUD_SIZE));
		for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
				page += PAGE_SIZE)
			__flush_TLB_page(page, pid);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					range_begin, range_end, old_pid, pid);
		}

		flush_TLB_page_end();
	} else {
flush_mm:
		/* Too many pages to flush.
		 * It is faster to change the context instead.
		 * If mm != current->active_mm then setting this
		 * CPU's mm context to 0 will do the trick,
		 * otherwise we duly increment it. */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = CTX_HARDWARE(context->cpumsk[cpu]);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
		}
	}
}

/*
 * Flush a specified range of pages and the TLB entries mapping the virtually
 * mapped linear page table corresponding to address range [start : end].
 */
static inline void
mmu_pid_flush_tlb_range_and_pgtables(mm_context_t *context,
		e2k_addr_t start, e2k_addr_t end,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	mmu_pid_flush_tlb_range(context, start, end, is_active, mm_cpumask,
				trace_enabled);
	mmu_pid_flush_tlb_pgtables(context, start, end, is_active, mm_cpumask,
					trace_enabled);
}

static inline void
mmu_pid_flush_tlb_page_and_pgtables(mm_context_t *context,
		unsigned long address,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	unsigned long page;
	unsigned long start = address, end = address + E2K_MAX_FORMAT;
	unsigned long range_begin, range_end;
	unsigned long pid = context->cpumsk[raw_smp_processor_id()];
	int cpu = raw_smp_processor_id();
	unsigned long old_pid, new_pid;

	old_pid = context->cpumsk[cpu];
	if (unlikely(old_pid == 0)) {
		/* See comment in __flush_tlb_range(). */
		mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);
		pid = CTX_HARDWARE(context->cpumsk[cpu]);
		if (unlikely(trace_enabled)) {
			trace_mmu_pid_flush_tlb_range(cpu, context,
					start, end, old_pid, pid);
		}
		return;
	}

	new_pid = old_pid;
	pid = CTX_HARDWARE(new_pid);

	flush_TLB_page_begin();

	/* flush virtual mapping of PUD entries (first level) */
	range_begin = pud_virt_offset(_PAGE_ALIGN_UP(start, PUD_SIZE));
	range_end = pud_virt_offset(_PAGE_ALIGN_DOWN(end, PUD_SIZE));
	for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
			page += PAGE_SIZE)
		__flush_TLB_page(page, pid);
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_range(cpu, context,
				range_begin, range_end, old_pid, new_pid);
	}

	/* flush virtual mapping of PMD entries (second level) */
	range_begin = pmd_virt_offset(_PAGE_ALIGN_UP(start, PMD_SIZE));
	range_end = pmd_virt_offset(_PAGE_ALIGN_DOWN(end, PMD_SIZE));
	for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
			page += PAGE_SIZE)
		__flush_TLB_page(page, pid);
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_range(cpu, context,
				range_begin, range_end, old_pid, new_pid);
	}

	/* flush virtual mapping of PTE entries (third level) */
	range_begin = pte_virt_offset(_PAGE_ALIGN_UP(start, PTE_SIZE));
	range_end = pte_virt_offset(_PAGE_ALIGN_DOWN(end, PTE_SIZE));
	for (page = PAGE_ALIGN_UP(range_begin); page < range_end;
			page += PAGE_SIZE)
		__flush_TLB_page(page, pid);
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_range(cpu, context,
				range_begin, range_end, old_pid, new_pid);
	}

	for (page = PAGE_ALIGN_UP(start); page < end; page += PAGE_SIZE)
		__flush_TLB_page(page, pid);
	if (unlikely(trace_enabled)) {
		trace_mmu_pid_flush_tlb_range(cpu, context,
				start, end, old_pid, pid);
	}

	flush_TLB_page_end();
}

#ifdef	CONFIG_SMP

/*
 * Flush a specified user mapping
 */

static inline void
mmu_pid_smp_flush_tlb_mm(mm_context_t *context,
		void (*flush_ipi_func)(void *data), void *flush_ipi_data,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	preempt_disable();

	/* Signal to all users of this mm that it has been flushed.
	 * Invalid context will be updated while activating or switching to. */
	memset(context->cpumsk, 0, nr_cpu_ids * sizeof(context->cpumsk[0]));

	/* See comment about memory barriers in do_switch_mm(). */
	smp_mb();

	mmu_pid_flush_tlb_mm(context, is_active, mm_cpumask, trace_enabled);

	/* Check that mm_cpumask() has some other CPU set */
	if (cpumask_any_but(mm_cpumask, smp_processor_id()) < nr_cpu_ids) {
		/* Send flush ipi to all other cpus in mm_cpumask(). */
		count_vm_tlb_event(NR_TLB_REMOTE_FLUSH);
		smp_call_function_many(mm_cpumask, flush_ipi_func,
					flush_ipi_data, 1);
	}

	preempt_enable();
}

/*
 * Flush a single page from TLB
 */

static inline void
mmu_pid_smp_flush_tlb_page(mm_context_t *context, const e2k_addr_t addr,
		void (*flush_ipi_func)(void *data), void *flush_ipi_data,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int i, cpu;

	preempt_disable();

	cpu = smp_processor_id();

	/* See comment in smp_flush_tlb_range() */
	for (i = 0; i < nr_cpu_ids; i++) {
		if (i == cpu)
			continue;
		context->cpumsk[i] = 0;
	}

	mmu_pid_flush_tlb_page(context, addr, is_active, mm_cpumask,
				trace_enabled);

	/* See comment about memory barriers in do_switch_mm(). */
	smp_mb();

	/* Check that mm_cpumask() has some other CPU set */
	if (cpumask_any_but(mm_cpumask, cpu) < nr_cpu_ids) {
		/* Send flush ipi to all other cpus in mm_cpumask(). */
		count_vm_tlb_event(NR_TLB_REMOTE_FLUSH);
		smp_call_function_many(mm_cpumask, flush_ipi_func,
					flush_ipi_data, 1);
	}

	preempt_enable();
}

/*
 * Flush a range of pages
 */

static inline void
mmu_pid_smp_flush_tlb_range(mm_context_t *context,
		const e2k_addr_t start, const e2k_addr_t end,
		void (*flush_ipi_func)(void *data), void *flush_ipi_data,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int cpu, i;

	preempt_disable();

	cpu = smp_processor_id();

	/* Signal to all users of this mm that it has been flushed.
	 * Invalid context will be updated while activating or switching to.
	 *
	 * Things to consider:
	 *
	 * 1) Clearing the whole context for CPUs to which we send the flush
	 * ipi looks unnecessary, but is needed to avoid race conditions. The
	 * problem is that there is a window between reading mm_cpumask() and
	 * deciding which context should be set to 0. In that window situation
	 * could have changed, so the only safe way is to set mm context on
	 * ALL cpus to 0.
	 *
	 * 2) Setting it to 0 essentially means that the cpus which receive the
	 * flush ipis cannot flush only a range of pages because they do not
	 * know the context, so they will flush the whole mm.
	 *
	 * 3) TODO FIXME This way of doing things is OK for 2 CPUs, for 4 CPUs,
	 * but it may become a problem for e2s with its 64 CPUs if there is a
	 * really-multi-threaded application running. If this is the case it
	 * would be better to implement scheme which will remember pending TLB
	 * flush requests. But such a scheme will greatly increase struct mm
	 * size (64 * 4 * 32 = 8 Kb for 64-processors system with a maximum
	 * of 4 simultaneously pending flushes each taking up 32 bytes).
	 *
	 * This problem (3) only gets worse when we are making all pages valid
	 * since EVERY mmap/sys_brk and some other calls will end up sending
	 * 63 flush ipis which will flush all the TLBs.
	 */
	for (i = 0; i < nr_cpu_ids; i++) {
		if (i == cpu)
			/* That being said, current CPU can still
			 * flush only the given range of pages. */
			continue;
		context->cpumsk[i] = 0;
	}

	mmu_pid_flush_tlb_range(context, start, end, is_active, mm_cpumask,
					trace_enabled);

	/* See comment about memory barriers in do_switch_mm(). */
	smp_mb();

	/* Check that mm_cpumask() has some other CPU set */
	if (cpumask_any_but(mm_cpumask, cpu) < nr_cpu_ids) {
		/* Send flush ipi to all other cpus in mm_cpumask(). */
		count_vm_tlb_event(NR_TLB_REMOTE_FLUSH);
		smp_call_function_many(mm_cpumask, flush_ipi_func,
					flush_ipi_data, 1);
	}

	preempt_enable();
}

/*
 * As native_smp_flush_tlb_range() but for pmd's
 */
static inline void
mmu_pid_smp_flush_pmd_tlb_range(mm_context_t *context,
		const e2k_addr_t start, const e2k_addr_t end,
		void (*flush_ipi_func)(void *data), void *flush_ipi_data,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)

{
	int cpu, i;

	preempt_disable();

	cpu = smp_processor_id();

	/* See comment in smp_flush_tlb_range() */
	for (i = 0; i < nr_cpu_ids; i++) {
		if (i == cpu)
			/* That being said, current CPU can still
			 * flush only the given range of pages. */
			continue;
		context->cpumsk[i] = 0;
	}

	mmu_pid_flush_pmd_tlb_range(context, start, end, is_active, mm_cpumask,
					trace_enabled);

	/* See comment about memory barriers in do_switch_mm(). */
	smp_mb();

	/* Check that mm_cpumask() has some other CPU set */
	if (cpumask_any_but(mm_cpumask, cpu) < nr_cpu_ids) {
		/* Send flush ipi to all other cpus in mm_cpumask(). */
		count_vm_tlb_event(NR_TLB_REMOTE_FLUSH);
		smp_call_function_many(mm_cpumask, flush_ipi_func,
					flush_ipi_data, 1);
	}

	preempt_enable();
}

/*
 * Flush a range of pages and page tables.
 */

static inline void
mmu_pid_smp_flush_tlb_range_and_pgtables(mm_context_t *context,
		const e2k_addr_t start, const e2k_addr_t end,
		void (*flush_ipi_func)(void *data), void *flush_ipi_data,
		bool is_active, cpumask_t *mm_cpumask,
		bool trace_enabled)
{
	int i, cpu;

	preempt_disable();

	cpu = smp_processor_id();

	/* See comment in smp_flush_tlb_range() */
	for (i = 0; i < nr_cpu_ids; i++) {
		if (i == cpu)
			continue;
		context->cpumsk[i] = 0;
	}

	mmu_pid_flush_tlb_range_and_pgtables(context, start, end, is_active,
						mm_cpumask, trace_enabled);

	/* See comment about memory barriers in do_switch_mm(). */
	smp_mb();

	/* Check that mm_cpumask() has some other CPU set */
	if (cpumask_any_but(mm_cpumask, cpu) < nr_cpu_ids) {
		/* Send flush ipi to all other cpus in mm_cpumask(). */
		count_vm_tlb_event(NR_TLB_REMOTE_FLUSH);
		smp_call_function_many(mm_cpumask, flush_ipi_func,
					flush_ipi_data, 1);
	}

	preempt_enable();
}

#endif	/* CONFIG_SMP */

#endif	/* _E2K_TLB_CONTEXT_H_ */
