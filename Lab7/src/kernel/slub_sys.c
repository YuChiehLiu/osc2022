#include "uart.h"
#include "string.h"
#include "math.h"
#include "buddy_sys.h"
#include "slub_sys.h"

chunk *pool[MAX_POOL_SIZE / MIN_CHUNK_SIZE + 1] = {0};

chunk node[3000];
int node_index = -1;

chunk *newnode()
{
    node_index++;
    return &node[node_index];
}

void *malloc(int size_req)
{
    int msize = roundup_size(size_req);

    if (msize == 0)
    {
        int alloc_page = config_pf(size_req);
        if (alloc_page == -1)
            return (void *)-1;
        return (void *)pf[alloc_page].addr;
    }

    if (pool[msize] == NULL)
        if (!is_pool_init(msize))
            return (void *)-1;

    unsigned long alloc_addr = find_chunk(msize);
    if (alloc_addr == 0)
    {
        if (!pool_extend(msize))
            return (void *)-1;
        alloc_addr = find_chunk(msize);
    }

    return (void *)alloc_addr;
}

/* if pool is empty */
int is_pool_init(int msize)
{
    int pf_index = config_pf(MIN_PAGE_SIZE);

    if (pf_index == -1)
        return 0;

    chunk *tmp = NULL;
    for (int i = 0; i < MIN_PAGE_SIZE / (msize * MIN_CHUNK_SIZE); i++)
    {
        chunk *new = newnode();
        new->size = msize *MIN_CHUNK_SIZE;
        new->addr = pf[pf_index].addr + i *new->size;
        new->val = FREE;
        new->next = NULL;
        new->previous = tmp;
        if (tmp == NULL)
        {
            pool[msize] = new;
            new->nexthead = NULL;
            tmp = new;
        }
        else
        {
            tmp->next = new;
            tmp = tmp->next;
        }
    }

    return 1;
}

/* if all chunks in pool are allocated */
int pool_extend(int msize)
{
    int pf_index = config_pf(MIN_PAGE_SIZE);

    if (pf_index == -1)
        return 0;

    chunk *tmp = pool[msize];
    chunk *head = NULL;
    for (int i = 0; i < MIN_PAGE_SIZE / (msize * MIN_CHUNK_SIZE); i++)
    {
        chunk *new = newnode();
        new->size = msize *MIN_CHUNK_SIZE;
        new->addr = pf[pf_index].addr + i *new->size;
        new->val = FREE;
        if (tmp == pool[msize])
        {
            head = new;
            new->next = tmp;
            new->previous = NULL;
            new->nexthead = tmp;
            tmp->previous = new;
            tmp = new;
        }
        else
        {
            new->next = tmp->next;
            new->previous = tmp;
            tmp->next->previous = new;
            tmp->next = new;
            tmp = tmp->next;
        }
    }
    pool[msize] = head;

    return 1;
}

int free(void* addr)
{
    chunk *free_node_head = find_pool((unsigned long)addr);
    int pf_index = ((unsigned long)addr >> 12) % TOTAL_PAGE_FRAME;

    if (free_node_head == NULL)
    {
        if (pf[pf_index].val == ALLOCATED)
        {
            free_pf(pf_index);
            return 1;
        }
        else
            return 0;
    }

    int msize = free_node_head->size / MIN_CHUNK_SIZE;

    chunk *tmp = free_node_head;
    while (tmp != NULL)
    {
        if ((unsigned long)addr == tmp->addr)
        {
            if (tmp->val == ALLOCATED)
            {
                tmp->val = FREE;
                if (is_returnable(free_node_head))
                {
                    free_pf((free_node_head->addr >> 12) % TOTAL_PAGE_FRAME);
                    if (free_node_head->previous == NULL)
                    {
                        pool[msize] = free_node_head->nexthead;
                        free_node_head->nexthead->previous = NULL;
                    }
                    else
                    {
                        free_node_head->previous->next = free_node_head->nexthead;
                        free_node_head->nexthead->previous = free_node_head->previous;
                    }
                    chunk *headtmp = pool[msize];
                    while (headtmp != NULL)
                    {
                        if (headtmp->nexthead == free_node_head)
                            headtmp->nexthead = free_node_head->nexthead;
                        else
                            headtmp = headtmp->nexthead;
                    }
                }
                return 1;
            }
            else
                return 0;
        }
        else
            tmp = tmp->next;
    }

    return 0;
}

int roundup_size(int size)
{
    switch (size)
    {
    case 1 ... 16:
        return 1;
    case 17 ... 32:
        return 2;
    case 43 ... 48:
        return 3;
    case 49 ... 96:
        return 6;
    case 97 ... 128:
        return 8;
    case 129 ... 192:
        return 12;
    case 193 ... 256:
        return 16;
    case 257 ... 512:
        return 32;
    case 513 ... 1024:
        return 64;
    case 1025 ... 2048:
        return 128;
    }
    return 0;
}

/* at requied size pool find a available memory */
unsigned long find_chunk(int msize)
{
    chunk *tmp = pool[msize];

    while (tmp != NULL)
    {
        if (tmp->val == FREE)
        {
            tmp->val = ALLOCATED;
            return tmp->addr;
        }
        else
            tmp = tmp->next;
    }

    return 0;
}

/* find the freed memory belong to which pool and return the page head node*/
chunk *find_pool(unsigned long addr)
{
    addr = addr >> 12; // same page(pool) memories has same prefix addr. (4KB=2^12)

    for (int i = 0; i < MAX_POOL_SIZE / MIN_CHUNK_SIZE + 1; i++)
    {
        if (pool[i] == NULL)
            continue;
        else
        {
            chunk *tmp = pool[i];
            while (tmp != NULL)
            {
                unsigned long align_addr = tmp->addr >> 12;
                if (addr == align_addr)
                    return tmp;
                else
                    tmp = tmp->nexthead;
            }
        }
    }

    return NULL;
}

/* check all chunks belonged to same page are all free */
int is_returnable(chunk *head)
{
    chunk *tmp = head;

    for (int i = 0; i < MIN_PAGE_SIZE / head->size; i++)
    {
        if (tmp->val == ALLOCATED)
            return 0;
        else
            tmp = tmp->next;
    }

    return 1;
}