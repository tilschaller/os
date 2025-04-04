#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <mman.h>
#include <limine.h>
#include <ssfn.h>
#include <string.h>
#include <stdbool.h>

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_memmap_request memmap_request = {
  .id = LIMINE_MEMMAP_REQUEST, 
  .revision = 0};

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, 
  .revision = 0};

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_executable_address_request exe_addr_request = {
  .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST, 
  .revision = 0};

static void init_pmm(uintptr_t phys_mem);
static void init_vmm();
static void init_paging(uintptr_t phys_mem);

struct vm_chunk* vm_objs = NULL;

// waterbucket like allocator
size_t pt_mem_offset = 0;
uintptr_t pt_phys_start = 0;
uintptr_t pt_virt_start = HIGHER_HALF + 0x40000000 - 4*PAGE_SIZE;

void init_mman() {
  // first find a suitable RAM segment
  // SIZE: 8MB
  // LAYOUT:  
  // ---------4MB-----------------2MB----------2MB------
  // |    page tables       |  stack     |   mman obj  |
  // ---------------------------------------------------
  //
  // mman objects
  //-----1MB-------1MB-----
  //|    pmm   |   vmm    |
  //-----------------------
  uintptr_t phys = 0;
  for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
    if (memmap_request.response->entries[i]->type == LIMINE_MEMMAP_USABLE && memmap_request.response->entries[i]->length >= 5*PAGE_SIZE) {
      phys = memmap_request.response->entries[i]->base;
      break;
    }
  }

  if (phys == 0) {
    printk("No suitable memory segment found\n");
    abort();
  }

  // aligning phys to 2MB
  phys = ((phys + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

  printk("The physical memory segment is at: %x\n", phys);
  printk("The framebuffer is at: %x\n", ssfn_dst.ptr);
  printk("The kernel is at: %x\n", exe_addr_request.response->physical_base);

  if ((uintptr_t)ssfn_dst.ptr % PAGE_SIZE != 0) {
    printk("Warning: framebuffer is not page aligned\n");
  }

  if (exe_addr_request.response->physical_base % 0x1000 != 0) {
    printk("Warning: kernel is not page aligned\n");
  }

  // clearing the memory segment
  memset((void*)phys + hhdm_request.response->offset, 0, 4*PAGE_SIZE);

  init_pmm(phys);
  init_paging(phys);

  //switching to new page table
  asm("mov %0, %%cr3" : : "r"(phys) : "memory"); 
  //switching to new stack
  asm("mov %0, %%rsp" ::"r"(HIGHER_HALF + 0x40000000 - 2*PAGE_SIZE) : "%rsp");

  ssfn_dst.ptr = (void*)HIGHER_HALF;

  extern __attribute__((noreturn))
  void kmain();
  kmain();
}

void init_pmm(uintptr_t phys_mem) {
  // creating first pm_chunk object at the reserved space
  struct pm_chunk *cur = (struct pm_chunk*)(phys_mem + 3*PAGE_SIZE + hhdm_request.response->offset);

  // parsing the memmap
  for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
    if (memmap_request.response->entries[i]->type == LIMINE_MEMMAP_USABLE || memmap_request.response->entries[i]->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) { 
      if (memmap_request.response->entries[i]->length >= PAGE_SIZE) {
      
        cur->base = memmap_request.response->entries[i]->base;
        if (cur->base % PAGE_SIZE != 0) {
          cur->base = ((cur->base + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
        }
        size_t length = memmap_request.response->entries[i]->length - (cur->base - memmap_request.response->entries[i]->base);
        if (length < PAGE_SIZE) {
          continue;
        }
        cur->pages = length >> 21;
        cur->next = (struct pm_chunk*)((uintptr_t)cur + sizeof(struct pm_chunk) + sizeof(uint8_t) * ((cur->pages + 7) >> 3));
        cur = cur->next;
      }
    }
  }
  cur->next = NULL;

  // filling the bitmaps - we need to reserve space for: (4 Pages) 
  // - stack (2MB)
  // - mman objects (2MB)
  // - page tables (4 MB)
  // - kernel and framebuffer dont need to be mapped because they arent in the structs
  cur = (struct pm_chunk*)(phys_mem + 3*PAGE_SIZE + hhdm_request.response->offset);
  while (true) {
    if (cur->base == phys_mem) {
      pmm_set_page_used(cur->bitmap, 0);
      pmm_set_page_used(cur->bitmap, 1);
      pmm_set_page_used(cur->bitmap, 2);
      pmm_set_page_used(cur->bitmap, 3);
      cur = NULL;
      break;
    }
    if (cur->next != NULL) {
      cur = cur->next;
    } else {
      break; 
    }
  }
  if (cur != NULL) {
    printk("Error: could not fill bitmaps in pmm_init\n");
    abort();
  }
}

void init_paging(uintptr_t phys_mem) {
  //layout of paging table
  //|HIGHER_HALF                                                                  | |KERNEL_SPACE                             |        
  //|------(1GB-8MB)---------4MB------------------2MB---------2MB-----------------| |----------1GB----------------------------|
  //|   framebuffer    |   page table     |    stack    |     mman objects        | |     kernel            |   free          |
  //|-----------------------------------------------------------------------------| |-----------------------------------------|
  //| phys_framebuffer | void* phys_mem                                           | |phys_kernel_base                         |
  
  uint64_t offset = hhdm_request.response->offset;
  pt_phys_start = phys_mem;
  phys_mem += offset; // at this point its actually virtual

  uint64_t *ptl_4 = (uint64_t*)phys_mem;  
  uint64_t *ptl_3_hh = (uint64_t*)(phys_mem + sizeof(uint64_t) * 512);
  uint64_t *ptl_3_krn = (uint64_t*)(phys_mem + sizeof(uint64_t) * 1024);
  uint64_t *ptl_2_hh = (uint64_t*)(phys_mem + sizeof(uint64_t) * 1536);
  uint64_t *ptl_2_krn = (uint64_t*)(phys_mem + sizeof(uint64_t) * 2048);
  uint64_t *ptl_1_krn = (uint64_t*)(phys_mem + sizeof(uint64_t) * 3072);
  pt_mem_offset = sizeof(uint64_t) * (3072 + 512 * 512);  

  // mapping frambuffer 
  ptl_4[256] = (uintptr_t)ptl_3_hh - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT;
  ptl_3_hh[0] = (uintptr_t)ptl_2_hh - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT;
  for (int i = 0; i < 512; i++) {
    ptl_2_hh[i] = (uintptr_t)ssfn_dst.ptr - offset + (PAGE_SIZE * i) | PT_FLAG_WRITE | PT_FLAG_PRESENT | PT_FLAG_2MB; 
  }

  // mapping stack
  ptl_2_hh[510] = phys_mem - offset + (PAGE_SIZE * 2) | PT_FLAG_WRITE | PT_FLAG_PRESENT | PT_FLAG_2MB;
  // mapping mman objects
  ptl_2_hh[511] = phys_mem - offset + (PAGE_SIZE * 3) | PT_FLAG_WRITE | PT_FLAG_PRESENT | PT_FLAG_2MB;
 
  // mapping kernel
  ptl_4[511] = (uintptr_t)ptl_3_krn - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT;
  ptl_3_krn[510] = (uintptr_t)ptl_2_krn - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT;
  for (int i = 0; i < 512; i++) {
    ptl_2_krn[i] = (uintptr_t)&ptl_1_krn[i*512] - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT;
    for (int j = 0; j < 512; j++) {
      ptl_1_krn[512*i+j] = exe_addr_request.response->physical_base + (0x1000 * (512*i+j)) | PT_FLAG_WRITE | PT_FLAG_PRESENT;
    }
  }

  // mapping pt using recursive paging
  ptl_2_hh[508] = (uintptr_t) phys_mem - offset | PT_FLAG_WRITE | PT_FLAG_PRESENT | PT_FLAG_2MB;
  ptl_2_hh[509] = (uintptr_t) phys_mem - offset + PAGE_SIZE | PT_FLAG_WRITE | PT_FLAG_PRESENT | PT_FLAG_2MB;
} 

void pmm_set_page_used(uint8_t *bitmap_entry, int bit) {
  *bitmap_entry = *bitmap_entry | 1 << bit; 
}

void pmm_set_page_free(uint8_t *bitmap_entry, int bit) {
  *bitmap_entry = *bitmap_entry & ~ (1 << bit); 
}

int pmm_get_page_status(uint8_t *bitmap_entry, int bit) {
  return (*bitmap_entry >> bit) & 0b1;
}

int vmm_get_page_status(uint64_t *pt_index) {
  return *pt_index & 0b1;
}

void init_vmm() {
 
}

// rewrite when adding userspace
// TODO: add proper allocation for page tables and add unmap function
void map_memory(void *root_table, void *phys, void *virt, uint64_t flags) {
  uint64_t ptl_4_index = ((uintptr_t)virt >> 39) & 0x1FF;
  uint64_t ptl_3_index = ((uintptr_t)virt >> 30) & 0x1FF;
  uint64_t ptl_2_index = ((uintptr_t)virt >> 21) & 0x1FF;
 
  uint64_t *ptl_4 = (uint64_t*)root_table;
  uint64_t *ptl_3;
  uint64_t *ptl_2;

  if (vmm_get_page_status(&ptl_4[ptl_4_index])) {
    ptl_3 = (uint64_t*)(ptl_4[ptl_4_index] & ~0xFFF);
    ptl_3 = ptl_3 + pt_virt_start - pt_phys_start;
  } else {
    ptl_3 = (uint64_t*)(pt_mem_offset + pt_virt_start);
    ptl_4[ptl_4_index] = pt_mem_offset + pt_phys_start | PT_FLAG_WRITE | PT_FLAG_PRESENT; 
    pt_mem_offset += sizeof(uint64_t) * 512;
    if (pt_mem_offset > 2*PAGE_SIZE) {
      printk("FIXME: space for page tables is full\n");
      abort();
    }
  }

  if (vmm_get_page_status(&ptl_3[ptl_3_index])) {
      ptl_2 = (uint64_t*)(ptl_3[ptl_3_index] & ~0xFFF);
      ptl_2 = ptl_3 + pt_virt_start - pt_phys_start;
    } else {
      ptl_2 = (uint64_t*)(pt_mem_offset + pt_virt_start);
      ptl_3[ptl_3_index] = pt_mem_offset + pt_phys_start | PT_FLAG_PRESENT | PT_FLAG_WRITE;
      pt_mem_offset += sizeof(uint64_t) * 512;
      if (pt_mem_offset > 2*PAGE_SIZE) {
        printk("FIXME: space for page tables is full\n");
        abort();
    }
  }

  ptl_2[ptl_2_index] = (uint64_t)phys | PT_FLAG_PRESENT | PT_FLAG_2MB | PT_FLAG_WRITE;
}

void* vmm_alloc(size_t length, size_t flags, void* arg) {

}
