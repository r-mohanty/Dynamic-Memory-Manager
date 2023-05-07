#define MALLOC_MGR_DISABLE 1
#include "dmem_mgr.hh"
#include <cassert>
#include <cstring>
#include<unordered_set>
#include<unordered_map>
#include<utility>


malloc_mgr_stats curr_statistics;
bool freefail=false;
std::unordered_map<void*, std::pair<int, long>> add_map;

const char* fi;

void* malloc_mgr(size_t sz, const char* file, long line) {
    (void) file, (void) line;
    fi = file;
    size_t meta_sz = sizeof(double) + sizeof(double);
    size_t tot_sz = meta_sz + sz;
    tot_sz += (tot_sz%8);
    void* ptr = base_malloc(tot_sz);
    if(ptr==NULL || (tot_sz<meta_sz || tot_sz<sz))
    {
        ++curr_statistics.nfail;
        curr_statistics.fail_size += sz;
        return NULL;
    }
    ++curr_statistics.ntotal;
    ++curr_statistics.nactive;
    curr_statistics.total_size += sz;
    curr_statistics.active_size += sz;
    *((char*)ptr) = 5;
    ptr = (int*)ptr + 1;
    *((int*)ptr) = sz;
    ptr = (int*)ptr + 1;
    ptr = (char*)ptr + sz;
    *((char*)ptr) = 8;
    ptr = (char*)ptr - sz;
    if (!curr_statistics.heap_min || curr_statistics.heap_min > (uintptr_t) ptr)
        curr_statistics.heap_min = (uintptr_t)ptr;

    add_map[ptr] = std::make_pair(sz,line);
    uintptr_t temp = (uintptr_t)((char*)ptr+sz);
    if (!curr_statistics.heap_max || curr_statistics.heap_max < temp)
        curr_statistics.heap_max = temp;
    return ptr;
}

void free_mgr(void* ptr, const char* file, long line) {
    (void) file, (void) line;
    if(ptr==nullptr)
    {
        base_free(ptr);
        return;
    }
    if((uintptr_t)ptr < curr_statistics.heap_min || (uintptr_t)ptr > curr_statistics.heap_max)      //Out of head
    {
        fprintf(stderr,"MEMORY BUG???: invalid free of pointer %p, not in heap", ptr);
        freefail = true;
        return;
    }
    if((int)(*((char*)ptr - 8) == 27))     //double free
    {
        fprintf(stderr,"MEMORY BUG???: invalid free of pointer %p, double free", ptr);
        freefail = true;
        return;
    }
    if(((int)(*((char*)ptr - 8))) != 5)     //Wrong heap loation
    {
        fprintf(stderr,"MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated\n", file, line, ptr);
        if(!add_map.empty())
            fprintf(stderr, "%s:%ld: %p is %d bytes inside a %d byte region allocated here", file, add_map.begin()->second.second, ptr, (int)((uintptr_t)ptr - (uintptr_t)(add_map.begin()->first)), add_map.begin()->second.first);
        freefail = true;
        return;
    }
    if(add_map.find(ptr)==add_map.end())
    {
        fprintf(stderr,"MEMORY BUG %s:%ld: invalid free of pointer %p, not allocated", file, line, ptr);
        freefail = true;
        return;
    }
    ptr = (int*)ptr - 1;
    int sz = *((int*)ptr);
    ptr = (int*)ptr + 1;
    add_map.erase(ptr);
    if(*((char*)ptr + sz) != 8)
    {
        fprintf(stderr,"MEMORY BUG???: detected wild write during free of pointer %p\n", ptr);
        freefail = true;
        abort();
        return;
    }
    ptr = (double*)ptr - 1;
    --curr_statistics.nactive;
    curr_statistics.active_size -= sz;
    base_free(ptr);
    *((char*)ptr) = 27;
    return;
}

void* calloc_mgr(size_t nmemb, size_t sz, const char* file, long line) {
    size_t res = nmemb * sz;
    if((res/sz)!=nmemb || (res/nmemb)!=sz)
    {
        ++curr_statistics.nfail;
        curr_statistics.fail_size += res;
        return NULL;
    }
    void* ptr = malloc_mgr(res, file, line);
    if(ptr)
    {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

void get_statistics(malloc_mgr_stats* stats) {
    stats->nactive = curr_statistics.nactive;
    stats->active_size = curr_statistics.active_size;
    stats->ntotal = curr_statistics.ntotal;
    stats->total_size = curr_statistics.total_size;
    stats->nfail = curr_statistics.nfail;
    stats->fail_size = curr_statistics.fail_size;
    stats->heap_min = curr_statistics.heap_min;
    stats->heap_max = curr_statistics.heap_max;

    return;

}

void print_statistics() {
    if(!freefail)
    {
        malloc_mgr_stats stats;
        get_statistics(&stats);

        printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
               stats.nactive, stats.ntotal, stats.nfail);
        printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
               stats.active_size, stats.total_size, stats.fail_size);
    }
    else
        freefail = false;
}

void print_leak_report() {
    (void) fi;
    std::unordered_map<void*, std::pair<int, long>>::iterator it = add_map.begin();
    while(it!=add_map.end())
    {
        printf("LEAK CHECK: %s:%ld: allocated object %p with size %d\n", fi, it->second.second, it->first, it->second.first);
        ++it;
    }
    return;
}


void* drealloc(void* ptr, size_t sz, const char* file, long line)
{
    void *ptr2=nullptr;
    if(sz)
    {
        ptr2 = malloc_mgr(sz, file, line);
        if(ptr2)
            *((char*)ptr2) = *((char*)ptr);
        else
            return ptr;
    }
    free_mgr(ptr, file, line);
    return ptr2;
}