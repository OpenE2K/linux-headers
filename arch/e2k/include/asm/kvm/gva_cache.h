#ifndef GVA_CACHE_E2K_H
#define GVA_CACHE_E2K_H


#include <linux/kvm_types.h>

#include <asm/page.h>
#include <asm/kvm/mmu_exc.h>

/* Format of address record in gva cache */
typedef union {
	struct {
		u64 addr : 64 - PAGE_SHIFT;
		u64 flags : PAGE_SHIFT;
	} fields;
	u64 word;
} cache_addr_t;

#define ADDR_VALID_MASK		(1 << 0)

#define GVA_ADDR		gva.fields.addr
#define GVA_FLAGS		gva.fields.flags
#define GVA_WHOLE		gva.word

/* Cache "cell" with gva -> gpa,hva translation */
typedef struct gva_cache_cell {
	cache_addr_t gva;
	gfn_t gfn;
	u32 pte_access;
	u32 level;
	u64 replace_data;
} gva_cache_cell_t;

/* gva -> gpa cache size */
#define KVM_GVA_CACHE_SZ		PAGE_SIZE
/* 2 ^ KVM_GVA_CACHE_BUCKET_BITS buckets in cache */
#define KVM_GVA_CACHE_BUCKET_BITS	5
#define KVM_GVA_CACHE_BUCKETS		(1 << KVM_GVA_CACHE_BUCKET_BITS)
#define KVM_GVA_CACHE_BUCKET_SZ		\
		(KVM_GVA_CACHE_SZ / KVM_GVA_CACHE_BUCKETS)
#define KVM_GVA_CACHE_BUCKET_LEN	\
	(KVM_GVA_CACHE_BUCKET_SZ / sizeof(gva_cache_cell_t))
#define KVM_GVA_CACHE_LEN		\
	(KVM_GVA_CACHE_SZ / sizeof(gva_cache_cell_t))

typedef enum REPLACE_POLICY {
	LRU = 0,
	RAND = 1
} replace_policy_t;

/*
 * gva cache stores translations from gva to gpa and hva to avoid
 * guest page table and kvm memory slots lookup.
 */
typedef struct gva_cache {
	spinlock_t bucket_locks[KVM_GVA_CACHE_BUCKETS];
	gva_cache_cell_t *data;
	replace_policy_t replace_policy;
} gva_cache_t;

typedef struct gva_translation {
	gva_t gva;
	gpa_t gpa;
	u32 pte_access;
	u32 level;
} gva_translation_t;

typedef struct gva_cache_query {
	gva_t gva;
	u32 access;
} gva_cache_query_t;


typedef gpa_t (*gva_tranlslator_t)(struct kvm_vcpu *, gva_t,
			u32, struct kvm_arch_exception*);

gpa_t gva_cache_translate(gva_cache_t *cache, gva_t gva, u32 access,
			struct kvm_vcpu *vcpu, kvm_arch_exception_t *exc,
			gva_tranlslator_t gva_translate);
void gva_cache_fetch_addr(gva_cache_t *cache, gva_t gva, gpa_t gpa,
			u32 access);
void gva_cache_flush_addr(gva_cache_t *cache, gva_t gva);
gva_cache_t *gva_cache_init(void);
void gva_cache_erase(gva_cache_t *cache);


#ifdef CONFIG_KVM_GVA_CACHE_STAT

int gva_cache_stat_dev_init(void);

/* Statistics of access to gva->gpa cache */
typedef struct gva_caches_stat {
	u64 accesses;
	u64 hits;
	u64 misses;

	u64 sum_hit_time;
	u64 sum_miss_pen;

	u64 conflict_misses;
	u64 cold_misses;

	u64 flushes;
	u64 fetches;
} gva_caches_stat_t;

extern gva_caches_stat_t caches_stat;

#define gva_cache_stat_lookup_start(start)			\
({								\
	caches_stat.accesses++;					\
	start = ktime_get_ns();					\
})

#define gva_cache_stat_lookup_hit_end(start, stop)		\
({								\
	stop = ktime_get_ns();					\
	caches_stat.hits++;					\
	caches_stat.sum_hit_time += (stop - start);		\
})

#define gva_cache_stat_lookup_miss_start(start)			\
({								\
	caches_stat.misses++;					\
	start = ktime_get_ns();					\
})

#define gva_cache_stat_lookup_miss_stop(start, stop)		\
({								\
	stop = ktime_get_ns();					\
	caches_stat.sum_miss_pen += (stop - start);		\
})

#define gva_cache_stat_lookup_miss_stop(start, stop)		\
({								\
	stop = ktime_get_ns();					\
	caches_stat.sum_miss_pen += (stop - start);		\
})

#define gva_cache_stat_lookup_miss_conflict(is_conflict)	\
({								\
	if (is_conflict)					\
		caches_stat.conflict_misses++;			\
	else							\
		caches_stat.cold_misses++;			\
})

#define gva_cache_stat_replace_conflict(is_conflict, conflict)	\
({								\
	if (is_conflict)					\
		*is_conflict = conflict;			\
})

#define gva_cache_stat_fetch()					\
({								\
	caches_stat.accesses++;					\
	caches_stat.fetches++;					\
})

#define gva_cache_stat_flush()					\
({								\
	caches_stat.accesses++;					\
	caches_stat.flushes++;					\
})

#else /* CONFIG_KVM_GVA_CACHE_STAT */

#define gva_cache_stat_lookup_start(start)
#define gva_cache_stat_lookup_hit_end(start, stop)
#define gva_cache_stat_lookup_miss_start(start)
#define gva_cache_stat_lookup_miss_stop(start, stop)
#define gva_cache_stat_lookup_miss_stop(start, stop)
#define gva_cache_stat_lookup_miss_conflict(is_conflict)
#define gva_cache_stat_replace_conflict(is_conflict, conflict)
#define gva_cache_stat_fetch()
#define gva_cache_stat_flush()

#endif /* CONFIG_KVM_GVA_CACHE_STAT */

#endif /* GVA_CACHE_H */
