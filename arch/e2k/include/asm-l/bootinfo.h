#ifndef _L_BOOTINFO_H_
#define _L_BOOTINFO_H_

#if defined(__KERNEL__) || defined(__KVM_BOOTINFO_SUPPORT__)

/*
 * 0x0:
 * 0x1: extended command line
 */
#define BOOTBLOCK_VER			0x1

#define	KSTRMAX_SIZE			128
#define	KSTRMAX_SIZE_EX			512
#define	BIOS_INFO_SIGN_SIZE		8
#define KERNEL_ARGS_STRING_EX_SIGN_SIZE	22
#define	BOOT_VER_STR_SIZE		128
#define	BOOTBLOCK_SIZE			0x1000	/* 1 PAGE_SIZE */
#define	X86BOOT_SIGNATURE		0x8086
#define	ROMLOADER_SIGNATURE		0xe200
#define	KVM_GUEST_SIGNATURE		0x20e2
#define	BIOS_INFO_SIGNATURE		"E2KBIOS"
#define	KVM_INFO_SIGNATURE		"E2KKVM"
#define KERNEL_ARGS_STRING_EX_SIGNATURE "KERNEL_ARGS_STRING_EX"
#define BOOT_KERNEL_ARGS_STRING_EX_SIGNATURE	\
		boot_va_to_pa(KERNEL_ARGS_STRING_EX_SIGNATURE)

/*
 * Below is boot information that comes out of the x86 code of Linux/E2K
 * loader proto.
 */

/* L_MAX_NODE_PHYS_BANKS = 4 sometimes is not enough, so we increase it to
 * an arbitary value (8 now). The old L_MAX_NODE_PHYS_BANKS we rename to
 * L_MAX_NODE_PHYS_BANKS_FUSTY and take in mind for boot_info compatibility.
 *
 * L_MAX_NODE_PHYS_BANKS_FUSTY and L_MAX_MEM_NUMNODES describe max size of
 * array of memory banks on all nodes and should be in accordance with old value
 * of L_MAX_PHYS_BANKS for compatibility with boot_info old structure (bank)
 * size, so L_MAX_NODE_PHYS_BANKS_FUSTY * L_MAX_MEM_NUMNODES should be
 * equal to 32.
 */
#define L_MAX_NODE_PHYS_BANKS		64	/* max number of memory banks */
						/* on one node */
#define L_MAX_NODE_PHYS_BANKS_FUSTY	4	/* fusty max number of memory */
						/* banks on one node */
#define L_MAX_PHYS_BANKS_EX		64	/* max number of memory banks */
						/* in banks_ex field of */
						/* boot_info */
#define L_MAX_MEM_NUMNODES		8	/* max number of nodes in the */
						/* list of memory banks on */
						/* each node */
#define	L_MAX_BUSY_AREAS	4	/* max number of busy areas */
					/* occupied by BIOS and should be */
					/* kept unchanged by kernel to */
					/* support recovery mode */

#ifndef	__ASSEMBLY__

typedef struct bank_info {
	__u64	address;	/* start address of bank */
	__u64	size;		/* size of bank in bytes */
} bank_info_t;

typedef struct node_banks {
	bank_info_t banks[L_MAX_NODE_PHYS_BANKS_FUSTY];	/* memory banks array */
							/* of a node */
} node_banks_t;

typedef struct boot_times {
	__u64 arch;
	__u64 unpack;
	__u64 pci;
	__u64 drivers1;
	__u64 drivers2;
	__u64 menu;
	__u64 sm;
	__u64 kernel;
	__u64 reserved[8];
} boot_times_t;

typedef struct bios_info {
	__u8	signature[BIOS_INFO_SIGN_SIZE];		/* signature, */
							/* 'E2KBIOS' */
	__u8	boot_ver[BOOT_VER_STR_SIZE];		/* boot version */
	__u8	mb_type;				/* mother board type */
	__u8	chipset_type;				/* chipset type */
	__u8	cpu_type;				/* cpu type */
	__u8	kernel_args_string_ex[KSTRMAX_SIZE_EX];	/* extended command */
							/* line of kernel */
							/* used to pass */
							/* command line */
							/* from e2k BIOS */
	__u8	reserved1;				/* reserved1 */
	__u32	cache_lines_damaged;			/* number of damaged */
							/* cache lines */
	__u64	nodes_mem_slabs_deprecated[52];		/* array of slabs */
							/* accessible memory */
							/* on each node */
							/* accessible memory */
							/* on each node */
	bank_info_t banks_ex[L_MAX_PHYS_BANKS_EX];	/* extended array of */
							/* descriptors of */
							/* banks of available */
							/* physical memory */
	__u64	devtree;				/* devtree pointer */
	__u32	bootlog_addr;				/* bootlog address */
	__u32	bootlog_len;				/* bootlog length */
	__u8	uuid[16];				/* UUID boot device */
} bios_info_t;

typedef struct boot_info {
	__u16	signature;	/* signature, 0x8086 */
	__u8	target_mdl;	/* target cpu model number */
	__u8	reserved1;	/* reserved1 */
	__u16	reserved2;	/* reserved2 */
	__u8	vga_mode;	/* vga mode */
	__u8	num_of_banks;	/* number of available physical memory banks */
				/* see below bank array */
				/* total number on all nodes or 0 */
	__u64	kernel_base;	/* base address to load kernel image */
				/* if 0 then BIOS can load at any address */
				/* but address should be large page size */
				/* aligned - 4 Mb */
	__u64	kernel_size;	/* kernel image byte's size */
	__u64	ramdisk_base;	/* base address to load RAM-disk */
				/* now not used */
	__u64	ramdisk_size;	/* RAM-disk byte's size */

	__u16	num_of_cpus;	/* number of started physical CPU(s) */
	__u16	mach_flags;	/* machine identifacition flags */
				/* should be set by our romloader and BIOS */
	__u16	num_of_busy;	/* number of busy areas occupied by BIOS */
				/* see below busy array */
	__u16	num_of_nodes;	/* number of nodes on NUMA system */
	__u64	mp_table_base;	/* MP-table base address */
	__u64	serial_base;	/* base address of serial port for Am85c30 */
				/* Used for debugging purpose */
	__u64	nodes_map;	/* online nodes map */
	__u64	mach_serialn;	/* serial number of the machine */
	__u8	mac_addr[6];	/* base MAC address for ethernet cards */
	__u16	reserved3;	/* reserved3 */

	char	kernel_args_string[KSTRMAX_SIZE]; /* command line of kernel */
						  /* used to pass command line */
						  /* from e2k BIOS */
	node_banks_t	nodes_mem[L_MAX_MEM_NUMNODES];	/* array of */
						/* descriptors of banks of */
						/* available physical memory */
						/* on each node */
	bank_info_t	busy[L_MAX_BUSY_AREAS];	/* descriptors of areas */
						/* occupied by BIOS, all this */
						/* shoud be kept in system */
						/* recovery mode */
	u64		cntp_info_deprecated[32];	/* control points */
							/* info to save and */
							/* restore them state */
	u64		dmp_deprecated[20];	/* Info for future work of */
						/* dump analyzer */
	__u64		reserved4[13];	/* reserved4 */
	__u8		mb_name[16];	/* Motherboard product name */
	__u32		reserved5;	/* reserved5 */
	__u32		kernel_csum;	/* kernel image control sum */
	bios_info_t	bios;		/* extended BIOS info */
					/* SHOULD BE LAST ITEM into this */
					/* structure */
} boot_info_t;

typedef struct bootblock_struct {
	boot_info_t	info;			/* general kernel<->BIOS info */
	__u8					/* zip area to make size of */
						/* bootblock struct - constant */
			gap[BOOTBLOCK_SIZE -
				sizeof (boot_info_t)  -
				sizeof (boot_times_t) -
				1 -		/* u8  : bootblock_ver */
				4 -		/* u32 : reserved1 */
				2 -		/* u16 : kernel_flags */
				1 -		/* u8  : reserved2 */
				5 -		/* u8  : number of cnt points */
						/* u8  : current # of cnt point */
						/* u8  : number of cnt points */
						/*	 ready in the memory */
						/* u8  : number of cnt points */
						/*	 saved on the disk */
						/* u8  : all control points */
						/*	 is created */
				8 -		/* u64 : dump sector */
				8 -		/* u64 : cnt point sector */
				2 -		/* u16 : dump device */
				2 -		/* u16 : cnt point device */
				2 -		/* u16 : boot_flags */
				2];		/* u16 : x86_marker */
	__u8		bootblock_ver;		/* bootblock version number */
	__u32		reserved1;		/* reserved1 */
	boot_times_t	boot_times;		/* boot load times */
	__u16		kernel_flags;		/* kernel flags, boot should */
						/* not modify it */
	__u8		reserved2;		/* reserved2 */

	__u8	cnt_points_num_deprecated;	/* number of control points */
						/* all memory will be devided */
						/* on this number of parts */
	__u8	cur_cnt_point_deprecated;	/* current # of active */
						/* control point (running */
						/* part) */
	__u8	mem_cnt_points_deprecated;	/* number of started control */
						/* points (ready in the memory) */
	__u8	disk_cnt_points_deprecated;	/* number of control points */
						/* saved on the disk (ready  */
						/* to be loaded from disk) */
	__u8	cnt_points_created_deprecated;	/* all control points created */
						/* in the memory and on disk */
	__u64	dump_sector_deprecated;		/* start sector # to dump */
						/* physical memory */
	__u64	cnt_point_sector_deprecated;	/* start sector # to save */
						/* restore control points */
	__u16	dump_dev_deprecated;		/* disk # to dump memory */
	__u16	cnt_point_dev_deprecated;	/* disk # for save/restore */
						/* control point */

	__u16		boot_flags;		/* boot flags: if non */
						/* zero then this structure */
						/* is recovery info */
						/* structure instead of boot */
						/* info structure */
	__u16		x86_marker;		/* marker of the end of x86 */
						/* boot block (0xAA55) */
} bootblock_struct_t;

extern	bootblock_struct_t *bootblock_virt;	/* bootblock structure */
						/* virtual pointer */
#endif /* ! __ASSEMBLY__ */

/*
 * Boot block flags to elaborate boot modes
 */

#define	RECOVERY_BB_FLAG	0x0001	/* recovery flag: if non zero then */
					/* this structure is recovery info */
					/* structure instead of boot info */
					/* structure */
					/* BIOS should not clear memory */
					/* and should keep current state of */
					/* physical memory */
#define	CNT_POINT_BB_FLAG	0x0002	/* kernel restarted in the mode of */
					/* control point creation */
					/* BIOS should read kernel image from */
					/* the disk to the specified area of */
					/* the memory and start kernel (this */
					/* flag should be with */
					/* RECOVERY_BB_FLAG flag) */
#define NO_READ_IMAGE_BB_FLAG   0x0004	/* BIOS should not read kernel image */
					/* from disk and start current */
					/* image in the specified area of */
					/* the memory (this flag should be */
					/* with RECOVERY_BB_FLAG flag) */
#define	DUMP_ANALYZE_BB_FLAG	0x0008	/* This flag is used only by kernel */
					/* to indicate dump analyzer mode */
#define	MEMORY_DUMP_BB_FLAG	0x0010	/* BIOS should dump all physical */
					/* memory before start all other */
					/* actions */

/*
 * The machine identification flags
 */

#define	SIMULATOR_MACH_FLAG		0x0001	/* system is running on */
						/* simulator */
#define	PROTOTYPE_MACH_FLAG_DEPRECATED	0x0002	/* machine is prototype */
#define	IOHUB_MACH_FLAG			0x0004	/* machine has IOHUB */
#define OLDMGA_MACH_FLAG		0x0008	/* MGA card has old firmware */
#define MULTILINK_MACH_FLAG		0x0010	/* some nodes are connected */
						/* by sevral IP links */
#define	MSI_MACH_FLAG			0x0020	/* boot inits right values in */
						/* apic to support MSI. */
						/* Meanfull for e2k only. For */
						/* v9 it always true */
#define	KVM_GUEST_MACH_FLAG		0x0100	/* system is running */
						/* as KVM guest */

/*
 * The chipset types
 */

#define CHIPSET_TYPE_PIIX4	0x01	/* PIIX4 */
#define CHIPSET_TYPE_IOHUB	0x02	/* IOHUB */

/*
 * The chipset types names
 */

#define GET_CHIPSET_TYPE_NAME(type)			\
({							\
	char *name;					\
							\
	switch (type) {					\
		case CHIPSET_TYPE_PIIX4:		\
			name = "PIIX4";			\
			break;				\
		case CHIPSET_TYPE_IOHUB:		\
			name = "IOHUB";			\
			break;				\
		default:				\
			name = "?????";			\
	}						\
							\
	name;						\
})

extern char *mcst_mb_name;

#endif /* __KERNEL__ || __KVM_BOOTINFO_SUPPORT__ */

#endif /* _L_BOOTINFO_H_ */

