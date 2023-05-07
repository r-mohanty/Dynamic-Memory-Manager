#ifndef __DMEMMGR_HH
#define __DMEMMGR_HH 1
#include <cassert>
#include <cstdlib>
#include <cinttypes>
#include <cstdio>
#include <new>


void* malloc_mgr(size_t sz, const char* file, long line);

void free_mgr(void* ptr, const char* file, long line);

void* calloc_mgr(size_t nmemb, size_t sz, const char* file, long line);

struct malloc_mgr_stats {
    unsigned long long nactive;
    unsigned long long active_size;
    unsigned long long ntotal;
    unsigned long long total_size;
    unsigned long long nfail;
    unsigned long long fail_size;       
    uintptr_t heap_min;                 
    uintptr_t heap_max;
    malloc_mgr_stats()
    {
        nactive = 0;
        active_size = 0;
        ntotal = 0;
        total_size = 0;
        nfail = 0;
        fail_size = 0;
        heap_min = 0;
        heap_max = 0;
    }
};

void get_statistics(malloc_mgr_stats* stats);

void print_statistics();

void print_leak_report();

void* base_malloc(size_t sz);
void base_free(void* ptr);
void base_allocator_disable(bool is_disabled);

#if !MALLOC_MGR_DISABLE
#define malloc(sz)          malloc_mgr((sz), __FILE__, __LINE__)
#define free(ptr)           free_mgr((ptr), __FILE__, __LINE__)
#define calloc(nmemb, sz)   calloc_mgr((nmemb), (sz), __FILE__, __LINE__)
#endif

template <typename T>
class dbg_allocator {
public:
    using value_type = T;
    dbg_allocator() noexcept = default;
    dbg_allocator(const dbg_allocator<T>&) noexcept = default;
    template <typename U> dbg_allocator(dbg_allocator<U>&) noexcept {}

    T* allocate(size_t n) {
        return reinterpret_cast<T*>(malloc_mgr(n * sizeof(T), "?", 0));
    }
    void deallocate(T* ptr, size_t) {
        free_mgr(ptr, "?", 0);
    }
};
template <typename T, typename U>
inline constexpr bool operator==(const dbg_allocator<T>&, const dbg_allocator<U>&) {
    return true;
}
template <typename T, typename U>
inline constexpr bool operator!=(const dbg_allocator<T>&, const dbg_allocator<U>&) {
    return false;
}

#endif
