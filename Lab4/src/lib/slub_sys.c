#include "uart.h"
#include "string.h"
#include "math.h"
#include "buddy_sys.h"
#include "slub_sys.h"

chunk* pool[MAX_POOL_SIZE/MIN_CHUNK_SIZE+1]={0};

chunk node[3000];
int node_index=-1;

chunk* newnode()
{
    node_index++;
    return &node[node_index];
}

int is_pool_init(int msize)
{
    int pf_index=config_pf(MIN_PAGE_SIZE);

    if(pf_index==-1)
        return 0;
    
    chunk* tmp=NULL;
    for(int i=0 ; i<MIN_PAGE_SIZE/(msize*MIN_CHUNK_SIZE) ; i++)
    {
        chunk* new=newnode();
        new->size=msize*MIN_CHUNK_SIZE;
        new->addr=pf[pf_index].addr + i*new->size;
        new->val=FREE;
        new->next=NULL;
        new->previous=tmp;
        if(tmp==NULL)
        {
            pool[msize]=new;
            tmp=new;
        }
        else
        {
            tmp->next=new;
            tmp=tmp->next;
        }
    }

    return 1;
}

int malloc(int size_req)
{
    int msize = roundup_size(size_req);

    if(pool[msize]==NULL)
        if(!is_pool_init(msize))
            return -1;
  
    int alloc_addr=find_chunk(msize);
    
    return alloc_addr;
}

int free(int addr)
{
    int msize = find_pool(addr);
    chunk* tmp = pool[msize];

    if(msize==-1)
        return 0;

    while(tmp!=NULL)
    {
        if(addr==tmp->addr)
        {
            
            if(tmp->val==ALLOCATED)
            {
                tmp->val=FREE;
                if(is_returnable(msize))
                {
                    free_pf((pool[msize]->addr>>12) % TOTAL_PAGE_FRAME);
                    pool[msize]=NULL;
                }
                return 1;
            }
            else
                return 0;
                
        }
        else
            tmp=tmp->next;
    }

    return 0;
}

int roundup_size(int size)
{
    switch(size)
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
    }
    return 0;
}

int find_chunk(int msize)
{
    chunk* tmp=pool[msize];

    while(tmp!=NULL)
    {
        if(tmp->val==FREE)
        {
            tmp->val=ALLOCATED;
            return tmp->addr;
        }
        else
            tmp=tmp->next;
    }

    return -1;
}

int find_pool(int addr)
{
    addr=addr>>12;

    for(int i=0 ; i<MAX_POOL_SIZE/MIN_CHUNK_SIZE+1 ; i++)
    {
        if(pool[i]==NULL)
            continue;
        else
        {
            int align_addr=pool[i]->addr>>12;
            if(addr==align_addr)
                return i;
        }
    }

    return -1;
}

int is_returnable(int msize)
{
    chunk* tmp=pool[msize];

    while(tmp!=NULL)
    {
        if(tmp->val==ALLOCATED)
            return 0;
        else
            tmp=tmp->next;
    }

    return 1;
}

