#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <mman.h>
#include <limine.h>
#include <ssfn.h>

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_memmap_request memmap_request = {
  .id = LIMINE_MEMMAP_REQUEST, 
  .revision = 0};

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST, 
  .revision = 0};

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_executable_address_request exe_addr_request = {
  .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST, 
  .revision = 0};

static void init_pmm();
static void init_vmm();
static void init_paging();

void init_mman() {
  // first find a suitable RAM segment
  // SIZE: 8MB
  // LAYOUT:  
  // ---------4MB-----------------2MB----------2MB------
  // |    page tables       |  mman data |   stack     |
  // ---------------------------------------------------
  uintptr_t phys = 0;
  for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
    if (memmap_request.response->entries[i]->type == LIMINE_MEMMAP_USABLE && memmap_request.response->entries[i]->length >= 4*PAGE_SIZE) {
      phys = memmap_request.response->entries[i]->base;
    }
  }

  if (phys == 0) {
    abort();
  }
    
  if (phys % PAGE_SIZE != 0) {
    printk("Warning: memory segment is not page aligned\n");
  }

  if ((uintptr_t)ssfn_dst.ptr % PAGE_SIZE != 0) {
    printk("Warning: framebuffer is not page aligned\n");
  }

  if (exe_addr_request.response->physical_base % PAGE_SIZE != 0) {
    printk("Warning: kernel is not page aligned\n");
    printk("Its at: %x\n", exe_addr_request.response->physical_base);
  }

  init_pmm();
  init_vmm();
  init_paging();

  extern __attribute__((noreturn))
  void kmain();
  kmain();
}

void init_pmm() {

}

void init_vmm() {

}

void init_paging() {
  //layout of paging table
  //                   HIGHER_HALF                                     KERNEL_SPACE                                      
  //|---------1GB------------2MB---------2MB-----------------| |----------1GB----------------------------|
  //|   framebuffer    |    stack    |   mman objects        | |     kernel            |   free          |
  //|----------------------------------------------------------------------------------------------------|
  
  
}
