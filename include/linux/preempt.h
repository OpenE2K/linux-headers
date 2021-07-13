/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PREEMPT_H
#define __LINUX_PREEMPT_H

/*
 * include/linux/preempt.h - macros for accessing and manipulating
 * preempt_count (used for kernel preemption, interrupt count, etc.)
 */

#include <linux/linkage.h>
#include <linux/list.h>

/*
 * We put the hardirq and softirq counter into the preemption
 * counter. The bitmask has the following meaning:
 *
 * - bits 0-7 are the preemption count (max preemption depth: 256)
 * - bits 8-15 are the softirq count (max # of softirqs: 256)
 *
 * The hardirq count could in theory be the same as the number of
 * interrupts in the system, but we run all interrupt handlers with
 * interrupts disabled, so we cannot have nesting interrupts. Though
 * there are a few palaeontologic drivers which reenable interrupts in
 * the handler, so we need more than one bit here.
 *
 *         PREEMPT_MASK:	0x000000ff
 *         SOFTIRQ_MASK:	0x0000ff00
 *         HARDIRQ_MASK:	0x000f0000
 *             NMI_MASK:	0x00100000
 * PREEMPT_NEED_RESCHED:	0x80000000
 */
#define PREEMPT_BITS	8
#define SOFTIRQ_BITS	8
#define HARDIRQ_BITS	4
#define NMI_BITS	1

#define PREEMPT_SHIFT	0
#define SOFTIRQ_SHIFT	(PREEMPT_SHIFT + PREEMPT_BITS)
#define HARDIRQ_SHIFT	(SOFTIRQ_SHIFT + SOFTIRQ_BITS)
#define NMI_SHIFT	(HARDIRQ_SHIFT + HARDIRQ_BITS)

#define __IRQ_MASK(x)	((1UL << (x))-1)

#define PREEMPT_MASK	(__IRQ_MASK(PREEMPT_BITS) << PREEMPT_SHIFT)
#define SOFTIRQ_MASK	(__IRQ_MASK(SOFTIRQ_BITS) << SOFTIRQ_SHIFT)
#define HARDIRQ_MASK	(__IRQ_MASK(HARDIRQ_BITS) << HARDIRQ_SHIFT)
#define NMI_MASK	(__IRQ_MASK(NMI_BITS)     << NMI_SHIFT)

#define PREEMPT_OFFSET	(1UL << PREEMPT_SHIFT)
#define SOFTIRQ_OFFSET	(1UL << SOFTIRQ_SHIFT)
#define HARDIRQ_OFFSET	(1UL << HARDIRQ_SHIFT)
#define NMI_OFFSET	(1UL << NMI_SHIFT)

#if defined(CONFIG_MCST) && defined(CONFIG_WATCH_PREEMPT)
extern int do_watch_preempt_disable;
extern void save_tm_prmtdsbl(int val);
extern void chck_tm_prmtdsbl(int val);

#define NEVER_PWATCH            0x00000001
#define NOPWATCH_DEAD           0x00000002
#define NOPWATCH_TSBGROW        0x00000004
#define NOPWATCH_LOCTIM         0x00000008
#define NOPWATCH_SCHED          0x00000010
#define NOPWATCH_EXITMM         0x00000020
#endif

#define SOFTIRQ_DISABLE_OFFSET	(2 * SOFTIRQ_OFFSET)

#define PREEMPT_DISABLED	(PREEMPT_DISABLE_OFFSET + PREEMPT_ENABLED)

/*
 * Disable preemption until the scheduler is running -- use an unconditional
 * value so that it also works on !PREEMPT_COUNT kernels.
 *
 * Reset by start_kernel()->sched_init()->init_idle()->init_idle_preempt_count().
 */
#define INIT_PREEMPT_COUNT	PREEMPT_OFFSET

/*
 * Initial preempt_count value; reflects the preempt_count schedule invariant
 * which states that during context switches:
 *
 *    preempt_count() == 2*PREEMPT_DISABLE_OFFSET
 *
 * Note: PREEMPT_DISABLE_OFFSET is 0 for !PREEMPT_COUNT kernels.
 * Note: See finish_task_switch().
 */
#define FORK_PREEMPT_COUNT	(2*PREEMPT_DISABLE_OFFSET + PREEMPT_ENABLED)

/* preempt_count() and related functions, depends on PREEMPT_NEED_RESCHED */
#include <asm/preempt.h>

#define hardirq_count()	(preempt_count() & HARDIRQ_MASK)
#define irq_count()	(preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK \
				 | NMI_MASK))
/*
 * Are we doing bottom half or hardware interrupt processing?
 *
 * in_irq()       - We're in (hard) IRQ context
 * in_softirq()   - We have BH disabled, or are processing softirqs
 * in_interrupt() - We're in NMI,IRQ,SoftIRQ context or have BH disabled
 * in_serving_softirq() - We're in softirq context
 * in_nmi()       - We're in NMI context
 * in_task()	  - We're in task context
 *
 * Note: due to the BH disabled confusion: in_softirq(),in_interrupt() really
 *       should not be used in new code.
 */
#define in_irq()		(hardirq_count())
#define in_interrupt()		(irq_count())
#define in_nmi()		(preempt_count() & NMI_MASK)
#define in_task()		(!(preempt_count() & \
				   (NMI_MASK | HARDIRQ_MASK | SOFTIRQ_OFFSET)))
#ifdef CONFIG_PREEMPT_RT

#define softirq_count()		((long)current->softirq_count)
#define in_softirq()		(softirq_count())
#define in_serving_softirq()	(current->softirq_count & SOFTIRQ_OFFSET)

#else

#define softirq_count()		(preempt_count() & SOFTIRQ_MASK)
#define in_softirq()		(softirq_count())
#define in_serving_softirq()	(softirq_count() & SOFTIRQ_OFFSET)

#endif

/*
 * The preempt_count offset after preempt_disable();
 */
#if defined(CONFIG_PREEMPT_COUNT)
# define PREEMPT_DISABLE_OFFSET	PREEMPT_OFFSET
#else
# define PREEMPT_DISABLE_OFFSET	0
#endif

/*
 * The preempt_count offset after spin_lock()
 */
#if !defined(CONFIG_PREEMPT_RT)
#define PREEMPT_LOCK_OFFSET	PREEMPT_DISABLE_OFFSET
#else
#define PREEMPT_LOCK_OFFSET	0
#endif

/*
 * The preempt_count offset needed for things like:
 *
 *  spin_lock_bh()
 *
 * Which need to disable both preemption (CONFIG_PREEMPT_COUNT) and
 * softirqs, such that unlock sequences of:
 *
 *  spin_unlock();
 *  local_bh_enable();
 *
 * Work as expected.
 */
#define SOFTIRQ_LOCK_OFFSET (SOFTIRQ_DISABLE_OFFSET + PREEMPT_LOCK_OFFSET)

/*
 * Are we running in atomic context?  WARNING: this macro cannot
 * always detect atomic context; in particular, it cannot know about
 * held spinlocks in non-preemptible kernels.  Thus it should not be
 * used in the general case to determine whether sleeping is possible.
 * Do not use in_atomic() in driver code.
 */
#define in_atomic()	(preempt_count() != 0)

/*
 * Check whether we were atomic before we did preempt_disable():
 * (used by the scheduler)
 */
#define in_atomic_preempt_off() (preempt_count() != PREEMPT_DISABLE_OFFSET)

#if defined(CONFIG_DEBUG_PREEMPT) || defined(CONFIG_TRACE_PREEMPT_TOGGLE)
extern void preempt_count_add(int val);
extern void preempt_count_sub(int val);
#define preempt_count_dec_and_test() \
	({ preempt_count_sub(1); should_resched(0); })
#else


# if defined(CONFIG_MCST) && defined(CONFIG_WATCH_PREEMPT)
#  define preempt_count_add(val)                                         \
	do {                                                            \
		__preempt_count_add(val);                               \
		if (unlikely(do_watch_preempt_disable))                 \
			save_tm_prmtdsbl(val);                          \
	} while (0)
#  ifdef CONFIG_DEBUG_PREEMPT_COUNT
#   define preempt_count_sub(val)                                         \
	do {                                                            \
		WARN_ONCE((val > *preempt_count_ptr()),                 \
			"preempt count(0x%08x) < val(%d)\n",            \
			*preempt_count_ptr(), val);                     \
		if (unlikely(do_watch_preempt_disable))                 \
			chck_tm_prmtdsbl(val);                          \
		__preempt_count_sub(val);                               \
	} while (0)
#  else
#   define preempt_count_sub(val)                                         \
	do {                                                            \
		if (unlikely(do_watch_preempt_disable))                 \
			chck_tm_prmtdsbl(val);                          \
		__preempt_count_sub(val);                               \
	} while (0)
#  endif
# else
#  define preempt_count_add(val)        __preempt_count_add(val)
#  define preempt_count_sub(val)        __preempt_count_sub(val)
# endif

#define preempt_count_dec_and_test() __preempt_count_dec_and_test()
#endif

#define __preempt_count_inc() __preempt_count_add(1)
#define __preempt_count_dec() __preempt_count_sub(1)

#define preempt_count_inc() preempt_count_add(1)
#define preempt_count_dec() preempt_count_sub(1)

#ifdef CONFIG_PREEMPT_LAZY
#define add_preempt_lazy_count(val)	do { preempt_lazy_count() += (val); } while (0)
#define sub_preempt_lazy_count(val)	do { preempt_lazy_count() -= (val); } while (0)
#define inc_preempt_lazy_count()	add_preempt_lazy_count(1)
#define dec_preempt_lazy_count()	sub_preempt_lazy_count(1)
#define preempt_lazy_count()		(current_thread_info()->preempt_lazy_count)
#else
#define add_preempt_lazy_count(val)	do { } while (0)
#define sub_preempt_lazy_count(val)	do { } while (0)
#define inc_preempt_lazy_count()	do { } while (0)
#define dec_preempt_lazy_count()	do { } while (0)
#define preempt_lazy_count()		(0)
#endif

#ifdef CONFIG_PREEMPT_COUNT

#define preempt_disable() \
do { \
	preempt_count_inc(); \
	barrier(); \
} while (0)

#define preempt_lazy_disable() \
do { \
	inc_preempt_lazy_count(); \
	barrier(); \
} while (0)

#define sched_preempt_enable_no_resched() \
do { \
	barrier(); \
	preempt_count_dec(); \
} while (0)

#ifdef CONFIG_PREEMPT_RT
# define preempt_enable_no_resched() sched_preempt_enable_no_resched()
# define preempt_check_resched_rt() preempt_check_resched()
#else
# define preempt_enable_no_resched() preempt_enable()
# define preempt_check_resched_rt() barrier();
#endif

#define preemptible()	(preempt_count() == 0 && !irqs_disabled())

#if defined(CONFIG_SMP) && defined(CONFIG_PREEMPT_RT)

extern void migrate_disable(void);
extern void migrate_enable(void);

int __migrate_disabled(struct task_struct *p);

#elif !defined(CONFIG_SMP) && defined(CONFIG_PREEMPT_RT)

extern void migrate_disable(void);
extern void migrate_enable(void);
static inline int __migrate_disabled(struct task_struct *p)
{
	return 0;
}

#else
#define migrate_disable()		preempt_disable()
#define migrate_enable()		preempt_enable()
static inline int __migrate_disabled(struct task_struct *p)
{
	return 0;
}
#endif

#ifdef CONFIG_PREEMPTION
#define preempt_enable() \
do { \
	barrier(); \
	if (unlikely(preempt_count_dec_and_test())) \
		__preempt_schedule(); \
} while (0)

#define preempt_enable_notrace() \
do { \
	barrier(); \
	if (unlikely(__preempt_count_dec_and_test())) \
		__preempt_schedule_notrace(); \
} while (0)

#define preempt_check_resched() \
do { \
	if (should_resched(0)) \
		__preempt_schedule(); \
} while (0)

#define preempt_lazy_enable() \
do { \
	dec_preempt_lazy_count(); \
	barrier(); \
	preempt_check_resched(); \
} while (0)

#else /* !CONFIG_PREEMPTION */
#define preempt_enable() \
do { \
	barrier(); \
	preempt_count_dec(); \
} while (0)

#define preempt_lazy_enable() \
do { \
	dec_preempt_lazy_count(); \
	barrier(); \
} while (0)

#define preempt_enable_notrace() \
do { \
	barrier(); \
	__preempt_count_dec(); \
} while (0)

#define preempt_check_resched() do { } while (0)
#endif /* CONFIG_PREEMPTION */

#define preempt_disable_notrace() \
do { \
	__preempt_count_inc(); \
	barrier(); \
} while (0)

#define preempt_enable_no_resched_notrace() \
do { \
	barrier(); \
	__preempt_count_dec(); \
} while (0)

#else /* !CONFIG_PREEMPT_COUNT */

/*
 * Even if we don't have any preemption, we need preempt disable/enable
 * to be barriers, so that we don't have things like get_user/put_user
 * that can cause faults and scheduling migrate into our preempt-protected
 * region.
 */
#define preempt_disable()			barrier()
#define sched_preempt_enable_no_resched()	barrier()
#define preempt_enable_no_resched()		barrier()
#define preempt_enable()			barrier()
#define preempt_check_resched()			do { } while (0)

#define preempt_disable_notrace()		barrier()
#define preempt_enable_no_resched_notrace()	barrier()
#define preempt_enable_notrace()		barrier()
#define preempt_check_resched_rt()		barrier()
#define preemptible()				0

#define migrate_disable()			barrier()
#define migrate_enable()			barrier()

static inline int __migrate_disabled(struct task_struct *p)
{
	return 0;
}
#endif /* CONFIG_PREEMPT_COUNT */

/* In our kernels local_irq_enable() and spin_unlock_no_resched()
 * play preemption tricks */
#if defined MODULE && !defined CONFIG_MCST
/*
 * Modules have no business playing preemption tricks.
 */
#undef sched_preempt_enable_no_resched
#undef preempt_enable_no_resched
#undef preempt_enable_no_resched_notrace
#undef preempt_check_resched
#endif

#define preempt_set_need_resched() \
do { \
	set_preempt_need_resched(); \
} while (0)
#define preempt_fold_need_resched() \
do { \
	if (tif_need_resched_now()) \
		set_preempt_need_resched(); \
} while (0)

#ifdef CONFIG_PREEMPT_RT
# define preempt_disable_rt()		preempt_disable()
# define preempt_enable_rt()		preempt_enable()
# define preempt_disable_nort()		barrier()
# define preempt_enable_nort()		barrier()
#else
# define preempt_disable_rt()		barrier()
# define preempt_enable_rt()		barrier()
# define preempt_disable_nort()		preempt_disable()
# define preempt_enable_nort()		preempt_enable()
#endif

#ifdef CONFIG_PREEMPT_NOTIFIERS

struct preempt_notifier;

/**
 * preempt_ops - notifiers called when a task is preempted and rescheduled
 * @sched_in: we're about to be rescheduled:
 *    notifier: struct preempt_notifier for the task being scheduled
 *    cpu:  cpu we're scheduled on
 * @sched_out: we've just been preempted
 *    notifier: struct preempt_notifier for the task being preempted
 *    next: the task that's kicking us out
 *
 * Please note that sched_in and out are called under different
 * contexts.  sched_out is called with rq lock held and irq disabled
 * while sched_in is called without rq lock and irq enabled.  This
 * difference is intentional and depended upon by its users.
 */
struct preempt_ops {
	void (*sched_in)(struct preempt_notifier *notifier, int cpu);
	void (*sched_out)(struct preempt_notifier *notifier,
			  struct task_struct *next);
};

/**
 * preempt_notifier - key for installing preemption notifiers
 * @link: internal use
 * @ops: defines the notifier functions to be called
 *
 * Usually used in conjunction with container_of().
 */
struct preempt_notifier {
	struct hlist_node link;
	struct preempt_ops *ops;
};

void preempt_notifier_inc(void);
void preempt_notifier_dec(void);
void preempt_notifier_register(struct preempt_notifier *notifier);
void preempt_notifier_unregister(struct preempt_notifier *notifier);

static inline void preempt_notifier_init(struct preempt_notifier *notifier,
				     struct preempt_ops *ops)
{
	INIT_HLIST_NODE(&notifier->link);
	notifier->ops = ops;
}

#endif

#endif /* __LINUX_PREEMPT_H */