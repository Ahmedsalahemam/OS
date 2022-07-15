#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>
#define pages ((KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE)

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
struct frame_info{
	uint32 counter;
	uint32 address;
}best_fit_arr[((KERNEL_HEAP_MAX-KERNEL_HEAP_START))/PAGE_SIZE];
uint32 firstFreeVAInKHeap = KERNEL_HEAP_START ;
struct frame_info new_arr[((KERNEL_HEAP_MAX-KERNEL_HEAP_START))/PAGE_SIZE];
int32 kheap_data_arr[((KERNEL_HEAP_MAX-KERNEL_HEAP_START))/PAGE_SIZE];

uint32 perfecto(unsigned int size)     // perfecto = Best fit
{
	int cnt = 0;
	int min_cnt;
	uint32 min_address = -1;
	uint32 firstFreeAddress = -1;
	int idx = 0;
	for(int i = KERNEL_HEAP_START; i < KERNEL_HEAP_MAX; i += PAGE_SIZE)
	{
		uint32* ptr_table = NULL;
		struct Frame_Info* framePTR = get_frame_info(ptr_page_directory, (void *)i, &ptr_table);
		if(framePTR == NULL && firstFreeAddress == -1)
		{
			firstFreeAddress = i;
			++cnt;
		}
		else if(framePTR == NULL && firstFreeAddress != -1)
		{
			++cnt;
		}
		else if(framePTR != NULL && firstFreeAddress != -1)
		{
			firstFreeAddress = -1;
			cnt = 0;
			continue;
		}
		if(cnt >= size)
		{
			best_fit_arr[idx].counter = cnt;
			best_fit_arr[idx].address = firstFreeAddress;
			++idx;
		}// best_fit             new_array
		// 1- f66666 - 6        f66666 - 8
		// 2- f66666 - 7        f66667 - 5
		// 3- f66666 - 8
		// 4- f66667 - 5
	}
	if(idx > 0)
	{
		new_arr[0].counter = best_fit_arr[0].counter;
		new_arr[0].address = best_fit_arr[0].address;
		int new_idx = 0;
		for(int i = 1; i < idx; ++i)
		{
			if(best_fit_arr[i].address == new_arr[new_idx].address)
			{
				new_arr[new_idx].counter = best_fit_arr[i].counter;
			}
			else{
				new_idx++;
				new_arr[new_idx].counter = best_fit_arr[i].counter;
				new_arr[new_idx].address = best_fit_arr[i].address;
			}
		}
		min_cnt = new_arr[0].counter;
		min_address = new_arr[0].address;
		for(int i = 1; i < new_idx; ++i)
		{
			if(new_arr[i].counter < min_cnt)
			{
				min_cnt = new_arr[i].counter;
				min_address = new_arr[i].address;
			}
		}
		return min_address;
	}
	return -1;
}

uint32 NextFitStrat(unsigned int size)
{
	uint32 firstFreeAddress = 0;
	int round1 = 0;
	int cnt = 0;
	uint32 temp = firstFreeVAInKHeap;
	while(firstFreeAddress==0 && round1 < 2)
	{
		for(int i = temp; i < KERNEL_HEAP_MAX; i += PAGE_SIZE)
		{
			uint32* ptr_table = NULL;
			struct Frame_Info* framePTR = get_frame_info(ptr_page_directory, (void *)i, &ptr_table);
			if(framePTR == NULL && firstFreeAddress == 0)
			{
				firstFreeAddress = i;
				++cnt;
			}
			else if(framePTR == NULL && firstFreeAddress != 0)
			{
				++cnt;
			}
			else if(framePTR != NULL)
			{
				firstFreeAddress = cnt = 0;
			}
			if(cnt == size)
			{
				firstFreeVAInKHeap = i+PAGE_SIZE;
				return firstFreeAddress;
			}
		}
		++round1;
		firstFreeAddress = 0; cnt = 0;
		temp = KERNEL_HEAP_START;
	}

	return -1;
}

void* kmalloc(unsigned int size)
{
	size = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;
	uint32 allocationStartAddress;
	uint32 returnedAllocation;
	if(isKHeapPlacementStrategyNEXTFIT())
	{
		 allocationStartAddress = NextFitStrat(size);
	}
	if(isKHeapPlacementStrategyBESTFIT())
	{
		allocationStartAddress = perfecto(size);
	}
    if(allocationStartAddress == -1)
    	return NULL;
	if ((KERNEL_HEAP_MAX-allocationStartAddress)/PAGE_SIZE<size)
	{
	   return NULL;
	}
	returnedAllocation = allocationStartAddress;
    for (int i=0;i<size;i++)
	{
	   struct Frame_Info * frames_info_ptr =NULL;
       if(allocate_frame(&frames_info_ptr)!= E_NO_MEM)
       {
          map_frame(ptr_page_directory, frames_info_ptr, (void *)allocationStartAddress,PERM_WRITEABLE);
       }
       allocationStartAddress += PAGE_SIZE;
	 }
	int index =  (returnedAllocation - KERNEL_HEAP_START)/ PAGE_SIZE ;
 	kheap_data_arr[index] = size;
 	return (void *)returnedAllocation;
}

void kfree(void* virtual_address)
{
	int index =  ((uint32 ) virtual_address - KERNEL_HEAP_START)/ PAGE_SIZE ;
	for(int j=0 ; j< kheap_data_arr[index] ; j++)
	{
		uint32 va = ((uint32)virtual_address+(j*PAGE_SIZE)) ;
		unmap_frame(ptr_page_directory, (void*) va);
 	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	uint32 tito = KERNEL_HEAP_START;
	struct Frame_Info* fi;
	  uint32* ppt;
		while(tito<KERNEL_HEAP_MAX)
			{
			    fi=NULL;
				fi = get_frame_info(ptr_page_directory, (uint32*)tito, &ppt);
				if(fi != NULL)
				{
					uint32 padd = to_physical_address(fi);
					if(padd == physical_address)
					{
						return tito;
					}
				}
				tito+=PAGE_SIZE;
			}
	return 0;
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer
	uint32* phy_ptr = NULL;
	get_page_table(ptr_page_directory, (void *)virtual_address, &phy_ptr);
	uint32 offset = (virtual_address & 0x00000FFF);
	uint32 translated_PHaddress = (phy_ptr[PTX(virtual_address)] >> 12 << 12) | offset;
	return translated_PHaddress;
}
// 111001101110100111000111001000110
// 000000000000000000000111111111111
//                          00000FFF
