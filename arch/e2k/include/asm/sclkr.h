#ifndef _ASM_E2K_SCLKR_H
#define _ASM_E2K_SCLKR_H

#include <linux/clocksource.h>
#include <linux/types.h>
#include <linux/percpu.h>
#include <linux/sched.h>

#include <asm/cpu_regs.h>
#include <asm-l/l_timer.h>

extern __interrupt u64 fast_syscall_read_sclkr(void);

/* SCLKR/SCLKM1/SCLKM2 implemented only on machine from e2s */
extern unsigned long native_read_SCLKR_reg_value(void);
extern unsigned long native_read_SCLKM1_reg_value(void);
extern unsigned long native_read_SCLKM2_reg_value(void);
extern void native_write_SCLKR_reg_value(unsigned long reg_value);
extern void native_write_SCLKM1_reg_value(unsigned long reg_value);
extern void native_write_SCLKM2_reg_value(unsigned long reg_value);

struct prev_sclkr {
	atomic64_t res;
} ____cacheline_aligned_in_smp;
extern struct prev_sclkr prev_sclkr;

#define SCLKR_NO	0
#define SCLKR_INT	1
#define SCLKR_RTC	2
#define SCLKR_EXT	3

extern struct clocksource clocksource_sclkr;
extern long long sclkr_sched_offset;
extern int sclkr_initialized;


#define SCLKR_SRC_LEN	4
extern char sclkr_src[SCLKR_SRC_LEN];
extern int sclkr_mode;
extern int sclk_register(void *);
extern struct clocksource clocksource_sclkr;
extern int proc_sclkr(struct ctl_table *, int,
	void __user *, size_t *, loff_t *);
extern void sclk_set_deviat(int dev);
extern u64 raw_read_sclkr(void);
DECLARE_PER_CPU(int, ema_freq);

extern __interrupt u64 fast_syscall_read_sclkr(void);
extern struct clocksource *curr_clocksource;
extern int redpill;
extern int check_sclkr_monotonic;

#define xchg_prev_sclkr_res(res) \
	__api_atomic64_fetch_xchg_if_below(res, &prev_sclkr.res.counter, RELAXED_MB)

static __always_inline u64 _sclkr_to_ns(u64 sclkr, u64 freq)
{
	u64 sclkr_sec, sclkr_lo, res;
	e2k_sclkm1_t sclkm1 = READ_SCLKM1_REG();
	/* we can not use __this_cpu_read/write(ema_freq) in fast syscall : */

	sclkr_sec = sclkr >> 32;
	sclkr_lo = (u64) (u32) sclkr;

	if (sclkr_lo >= freq)
		sclkr_lo = freq - 1;

	res = sclkr_sec * NSEC_PER_SEC + sclkr_lo * NSEC_PER_SEC / freq;

	/* sclkm3 has a summary time when guest was out of cpu */
	if (!redpill && sclkm1.sclkm3)
		res -= READ_SCLKM3_REG();
	return res;
}

static __always_inline u64 sclkr_to_ns(u64 sclkr, u64 freq)
{
	u64 res, before;
	res = _sclkr_to_ns(sclkr, freq);
	if (check_sclkr_monotonic) {
		before = xchg_prev_sclkr_res(res);
		if (before > res)
			res = before;
	}

	return res;
}

static __always_inline u64 sclkr2ns(u64 sclkr, u64 freq)
{
/* Do not check monotonic as kernel/sched/clock.c says^
 * cpu_clock(i) provides a fast (execution time) high resolution
 * clock with bounded drift between CPUs. The value of cpu_clock(i)
 * is monotonic for constant i.
*/
	return _sclkr_to_ns(sclkr, freq);
}

static inline bool use_sclkr_sched_clock(void)
{
	return sclkr_initialized;
}

#endif	/* _ASM_E2K_SCLKR_H */
