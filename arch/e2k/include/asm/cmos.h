#ifndef _ASM_CMOS_H
#define _ASM_CMOS_H

#include <asm/io.h>
#include <asm/e2k.h>

#ifndef RTC_PORT
#define RTC_PORT(x)	(0x70 + (x))
#define RTC_ALWAYS_BCD	1	/* RTC operates in binary mode */
#endif

static inline char mc146818_cmos_read(char addr)
{
	WARN_ONCE(1, "Warning: CMOS_READ attempted on a machine without a functioning CMOS\n");
	return 0;
}

static inline void mc146818_cmos_write(char val, char addr)
{
	WARN_ONCE(1, "Warning: CMOS_WRITE attempted on a machine without a functioning CMOS\n");
}

#define CMOS_READ(addr) mc146818_cmos_read(addr)
#define CMOS_WRITE(val, addr) mc146818_cmos_write(val, addr)

#endif
