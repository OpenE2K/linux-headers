/*
 *  Elbrus MAC Kernel (elmac) security module
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2,
 *      as published by the Free Software Foundation.
 */

#ifndef	_UAPI_LINUX_ELMAC_H
#define	_UAPI_LINUX_ELMAC_H

#include <linux/capability.h>

#ifndef PACKED
#define EL_PACKED __attribute__((aligned(1), packed))
#else
#define EL_PACKED PACKED
#endif

/*
 * elmac label
 */
typedef struct _el_mac_label {
        __u8	level;
        __u64	category;
} EL_PACKED elmac_label_t;

/*
 * elmac context
 */
typedef struct _el_mac_context {
        __u32		attr;
        elmac_label_t	mac;
} EL_PACKED elmac_context_t;


#define ELMAC_LABEL_LEV(label)		((label).level)
#define ELMAC_LABEL_LCAT(label)		((label).category)
#define ELMAC_CONTEXT_LEV(context)	((context).mac.level)
#define ELMAC_CONTEXT_CAT(context)	((context).mac.category)
#define ELMAC_CONTEXT_ATTR(context)	((context).attr)
#define ELMAC_CONTEXT_ATTR_CHECK(context, a) \
					((context).attr & ELMAC_ATTR_##a)

/*
 * Objects special attr
 */
#define ELMAC_ATTR_IGNORER_CAT	0x00000001
#define ELMAC_ATTR_IGNOREW_CAT	0x00000002
#define ELMAC_ATTR_IGNOREX_CAT	0x00000004
#define ELMAC_ATTR_IGNORER_LVL	0x00000008
#define ELMAC_ATTR_IGNOREW_LVL	0x00000010
#define ELMAC_ATTR_IGNOREX_LVL	0x00000020
#define ELMAC_ATTR_IGNORER	((ELMAC_ATTR_IGNORER_CAT) | \
				(ELMAC_ATTR_IGNORER_LVL))
#define ELMAC_ATTR_IGNOREW	((ELMAC_ATTR_IGNOREW_CAT) | \
				(ELMAC_ATTR_IGNOREW_LVL))
#define ELMAC_ATTR_IGNOREX	((ELMAC_ATTR_IGNOREX_CAT) | \
				(ELMAC_ATTR_IGNOREX_LVL))
#define ELMAC_ATTR_IGNORE	((ELMAC_ATTR_IGNORER) | (ELMAC_ATTR_IGNOREW) | \
				(ELMAC_ATTR_IGNOREX))

/*
 * Subjects special attr
 */
#define ELMAC_ATTR_PRIVSOCK		0x00000040
#define ELMAC_ATTR_CAP_READ_SEARCH	0x00000080


#define ELMAC_MAX_LEVEL		255
#define ELMAC_MAX_CATEGORY	((__u64)(-1))

#define ELMAC_LABEL_MAXLEN	sizeof(elmac_label_t)
#define ELMAC_LABEL_STRLEN	(ELMAC_LABEL_MAXLEN + 1)
#define ELMAC_CONTEXT_MAXLEN	sizeof(elmac_context_t)

#define ELMAC_IPSO_OPTION 	"-IPSO"

#ifdef __KERNEL__
#ifdef CONFIG_MCST_SECURITY_ELMAC
extern bool elmac_is_enabled(void);
#else
static inline bool elmac_is_enabled(void)
{
	return false;
}
#endif	/* CONFIG_MCST_SECURITY_ELMAC */
#endif	/* __KERNEL__ */

#endif	/* _UAPI_LINUX_ELMAC_H */
