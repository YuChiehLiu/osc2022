#include "string.h"
#include "math.h"
#include "buddy_sys.h"
#include "uart.h"

pf_entry pf[TOTAL_PAGE_FRAME]; // page frame
pf_entry fa[MAX_ORDER+1]; // free area : linked-list for each size

reserved_memory RMlist[100];
int RMindex=0;

void pf_init()
{
    for(int i=0 ; i<=MAX_ORDER ; i++)
    {
        fa[i].val=i;
        fa[i].next=NULL;
        fa[i].previous=&fa[i];
    }

    for(int i=0 ; i<TOTAL_PAGE_FRAME ; i++)
    {
        pf[i].index=i;
        pf[i].val=FREE;
        pf[i].allord=-1;
        pf[i].addr=0x00000000+i*MIN_PAGE_SIZE;
        pf[i].next=NULL;
        pf[i].previous=NULL;
    }   

    fa[MAX_ORDER].next=pf;
    fa[MAX_ORDER].previous=pf;
    pf[0].val=MAX_ORDER;
    pf[0].previous=&fa[MAX_ORDER];

    for(int i=pow(2, MAX_ORDER) ; i<TOTAL_PAGE_FRAME ; i+=pow(2, MAX_ORDER))
    {
        pf[i].val=MAX_ORDER;
        insert_fa(MAX_ORDER, i);
    }
    
}

int config_pf(int size_req)
{
    int req_order = size_req;

    // decide at which order to search FREE memory block
    for(int i=0 ; i<=MAX_ORDER ; i++)
    {
        if(req_order <= MIN_PAGE_SIZE*pow(2, i))
        {    
            req_order = i;
            break;
        }
    }

    int cfg_order=req_order;
    int cfg_page=-1;

    // configure a available page frame
    while(cfg_order <= MAX_ORDER)
    {
        if((cfg_page=find_free(cfg_order))==-1)
            cfg_order++;
        else
            break;
    }

    if(cfg_order > MAX_ORDER)
        return -1;

    pf[cfg_page].allord=req_order;

    cfg_order--;
    
    // release redundant memory block
    while(cfg_order >= req_order)
    {
        release_rmb(cfg_order, cfg_page);
        cfg_order--;
    }

    // check the configured page if contains reserved memory
    int RM_index=RM_index=is_include_RM(pf[cfg_page].addr, pf[cfg_page].addr+MIN_PAGE_SIZE*pow(2, pf[cfg_page].allord)-1);
    if(RM_index)
    {
        int RM_page = cfg_page;
        found_RM(RM_page, RM_index);
        cfg_page = config_pf(size_req);
        free_pf(RM_page);
        if(cfg_page==-1)
            return -1;
    }

    return cfg_page;
}

int find_free(int order)
{
    if(fa[order].next==NULL)
        return -1;
    else
    {
        fa[order].next->val = ALLOCATED;
        int index=fa[order].next->index;
        remove_fa(order, index);
        return index;
    }
}

void release_rmb(int order, int index)
{
    int buddy = index ^ (1<<order);

    pf[buddy].val = order;

    insert_fa(order, buddy);
}

void free_pf(int index)
{
    int free_order = pf[index].allord;
    int buddy = index ^ (1<<free_order);

    pf[index].allord = -1;
    
    if(free_order==MAX_ORDER)
    {
        pf[index].val=free_order;
        insert_fa(free_order, index);
        return;
    }

    if(is_buddy_free(free_order, index) && pf[buddy].val==free_order)
    {
        remove_fa(free_order, index);
        remove_fa(free_order, buddy);

        int merge_index = index & ~(1<<free_order);

        pf[index].val=FREE;
        pf[buddy].val=FREE;
        pf[merge_index].allord=free_order+1;

        free_pf(merge_index);
    }
    else
    {
        pf[index].val=free_order;
        insert_fa(free_order, index);
    }
}

int is_buddy_free(int order, int index)
{
    int buddy = index ^ (1<<order);

    if(pf[buddy].val!=ALLOCATED && pf[buddy].val!=FREE)
        return 1;
    return 0;
}

// insert to corresponding free area
void insert_fa(int order, int index)
{
    pf_entry* tmp=fa[order].previous;
    
    tmp->next=&pf[index];
    tmp->next->previous=tmp;
    tmp->next->next=NULL;
    fa[order].previous=&pf[index];
}

// remove from located free area
void remove_fa(int order, int index)
{
    if(pf[index].next!=NULL)
        pf[index].next->previous=pf[index].previous;
    else
        fa[order].previous=pf[index].previous;
    pf[index].previous->next=pf[index].next;
    pf[index].next=NULL;
    pf[index].previous=NULL;
}

int is_freeable(int index)
{
    if(pf[index].val != ALLOCATED)
        return 0;
    else
        return 1;
}

void memory_reserve(int start, int end, char* name)
{
    RMindex++;
    RMlist[RMindex].start=start;
    RMlist[RMindex].end=end;
    strcpy(name, RMlist[RMindex].name);
}

// return value : if including RM, return which no. of RM. Otherwise, return 0.
int is_include_RM(int start, int end)
{
    for(int i=1 ; i<=RMindex ; i++)
    {
        if(start>RMlist[i].start && start>RMlist[i].end)
            continue;
        else if(start<RMlist[i].start && end<RMlist[i].start)
            continue;
        else
            return i;
    }
    return 0;
}

/* below functions just for demo */
void demo()
{
    pf_entry* tmp;
    char order_c[2];
    char index_c[10];
    
    asyn_write("\n");
    for(int i=MAX_ORDER ; i>-1 ; i--)
    {
        tmp=&fa[i];
        strset(order_c, '\0', 2);
        itoa(i, order_c, 1);
        uart_puts("order ");
        uart_puts(order_c);
        uart_puts(" : ");
        while(tmp->next!=NULL)
        {
            int count=1;
            int tmp_index=tmp->next->index;

            while(tmp_index/10 != 0)
            {
                tmp_index/=10;
                count++;
            }
            
            switch(tmp->next->val)
            {
                case ALLOCATED:
                    uart_puts("ALLOCATED");
                    uart_puts(" -> ");
                    break;
                default:
                    strset(index_c, '\0', 10);
                    itoa(tmp->next->index, index_c, count);
                    uart_puts(index_c);
                    uart_puts(" -> ");
                    break;
            }
            tmp=tmp->next;
        }
        uart_puts("NULL\n\r");
    }
}

void demo_opp()
{
    pf_entry* tmp;
    char order_c[3];
    char index_c[10];

    for(int i=MAX_ORDER ; i>-1 ; i--)
    {
        tmp=&fa[i];
        strset(order_c, '\0', 2);
        itoa(i, order_c, 2);
        uart_puts("order ");
        uart_puts(order_c);
        uart_puts(" : ");

        uart_puts("NULL -> ");

        while(tmp->previous!=&fa[i])
        {
            int count=1;
            int tmp_index=tmp->previous->index;

            while(tmp_index/10 != 0)
            {
                tmp_index/=10;
                count++;
            }
            
            switch(tmp->previous->val)
            {
                case ALLOCATED:
                    uart_puts("ALLOCATED");
                    uart_puts(" -> ");
                    break;
                default:
                    strset(index_c, '\0', 10);
                    itoa(tmp->previous->index, index_c, count);
                    uart_puts(index_c);
                    uart_puts(" -> ");
                    break;
            }
            tmp=tmp->previous;
        }
        char del[11] = {127,127,127,127,'\n','\r','\0'}; // 127 : ASCII "del"
        uart_puts(del);
    }
}

void found_RM(int pf_index, int RM_index)
{
    char index_c[10];
    strset(index_c, '\0', 10);
    int count=1;
    int tmp_index=pf_index;
    while(tmp_index/10 != 0)
    {
        tmp_index/=10;
        count++;
    }
    itoa(pf_index, index_c, count);
    asyn_write("Page Frame ");
    asyn_write(index_c);
    asyn_write(" has reserved memory : ");
    asyn_write(RMlist[RM_index].name);
    asyn_write("\n");
}