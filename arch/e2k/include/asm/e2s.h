#ifndef _ASM_E2S_H_
#define _ASM_E2S_H_

/*
 * Machine (based on E4C processor) topology:
 * E4C is NUMA system on distributed memory and can have several nodes.
 * Each node can have some memory (faster to access) and max 4 CPUs (cores)
 * Node number is the same as chip-processor number
 * Some nodes (CPUs) can be without memory
 * LAPIC cluster number is the same as node number
 */

#ifndef __ASSEMBLY__
struct pt_regs;

extern void boot_e2s_setup_arch(void);
extern void e2s_setup_machine(void);
extern void setup_APIC_vector_handler(int vector,
		void (*handler)(struct pt_regs *), bool system, char *name);
extern void sic_error_interrupt(struct pt_regs *regs);
#endif

#define	E2S_NR_NODE_CPUS		4
#define	E2S_MAX_NR_NODE_CPUS		E2S_NR_NODE_CPUS

#define	E2S_NODE_IOLINKS		1

#define	E2S_PCICFG_AREA_PHYS_BASE	ES2_PCICFG_AREA_PHYS_BASE
#define	E2S_PCICFG_AREA_SIZE		ES2_PCICFG_AREA_SIZE

#define E2S_NSR_AREA_PHYS_BASE		ES2_NSR_AREA_PHYS_BASE

#define	E2S_NBSR_AREA_OFFSET		ES2_NBSR_AREA_OFFSET
#define	E2S_NBSR_AREA_SIZE		ES2_NBSR_AREA_SIZE

#define E2S_COPSR_AREA_PHYS_BASE	ES2_COPSR_AREA_PHYS_BASE
#define	E2S_COPSR_AREA_SIZE		ES2_COPSR_AREA_SIZE

#define E2S_MLT_SIZE			ES2_MLT_SIZE

#define	E2S_TLB_LINES_BITS_NUM		ES2_TLB_LINES_BITS_NUM
#define E2S_TLB_ADDR_LINE_NUM		ES2_TLB_ADDR_LINE_NUM
#define E2S_TLB_ADDR_LINE_NUM2		0x000000001fe00000
#define E2S_TLB_ADDR_LINE_NUM_SHIFT2	21
#define E2S_TLB_ADDR_SET_NUM		ES2_TLB_ADDR_SET_NUM
#define E2S_TLB_ADDR_SET_NUM_SHIFT	ES2_TLB_ADDR_SET_NUM_SHIFT

#define E2S_SIC_MC_SIZE			0xa4
#define E2S_SIC_MC_COUNT		3
#define E2S_SIC_MC1_ECC			0x440

#define E2S_CLOCK_TICK_RATE		ES2_CLOCK_TICK_RATE

#define E2S_L1_CACHE_SHIFT		ES2_L1_CACHE_SHIFT
#define E2S_L1_CACHE_BYTES		ES2_L1_CACHE_BYTES
#define E2S_L2_CACHE_SHIFT		ES2_L2_CACHE_SHIFT
#define E2S_L2_CACHE_BYTES		ES2_L2_CACHE_BYTES

#endif /* _ASM_E2S_H_ */
