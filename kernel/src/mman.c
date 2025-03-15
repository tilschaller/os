#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <mman.h>
#include <limine.h>
#include <ssfn.h>
#include <string.h>

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
static void init_paging(uintptr_t phys_mem);

void init_mman() {
  // first find a suitable RAM segment
  // SIZE: 8MB
  // LAYOUT:  
  // ---------4MB-----------------2MB----------2MB------
  // |    page tables       |  stack     |   mman obj  |
  // ---------------------------------------------------
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

  init_pmm();
  init_vmm();
  init_paging(phys);

  //switch to new page table
  asm("mov %0, %%cr3" : : "r"(phys) : "memory"); 
  //switch to new stack
  asm("mov %0, %%rsp" ::"r"(HIGHER_HALF + 0x40000000 - 2*PAGE_SIZE) : "%rsp");

  ssfn_dst.ptr = (void*)HIGHER_HALF;

  extern __attribute__((noreturn))
  void kmain();
  kmain();
}

void init_pmm() {

}

void init_vmm() {

}

void init_paging(uintptr_t phys_mem) {
  //layout of paging table
  //                   HIGHER_HALF                                     KERNEL_SPACE                                      
  //|------(1GB-4MB)---------2MB---------2MB-----------------| |----------1GB----------------------------|
  //|   framebuffer    |    stack    |   mman objects        | |     kernel            |   free          |
  //|--------------------------------------------------------| |-----------------------------------------|
  //|  phys_framebuffer| void* phys_mem                      | |phys_kernel_base
  
  uint64_t offset = hhdm_request.response->offset;
  phys_mem += offset;

  uint64_t *ptl_4 = (uint64_t*)phys_mem;  
  uint64_t *ptl_3_hh = (uint64_t*)phys_mem + sizeof(uint64_t) * 512;
  uint64_t *ptl_3_krn = (uint64_t*)phys_mem + sizeof(uint64_t) * 1024;
  uint64_t *ptl_2_hh = (uint64_t*)phys_mem + sizeof(uint64_t) * 1536;
  uint64_t *ptl_2_krn = (uint64_t*)phys_mem + sizeof(uint64_t) * 2048;
  uint64_t *ptl_1_krn = (uint64_t*)phys_mem + sizeof(uint64_t) * 3072;

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
}
