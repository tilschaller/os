#include <mman.h>
#include <limine.h>
#include <stddef.h>
#include <stdint.h>
#include <kstdio.h>
#include <string.h>
#include <ssfn.h>

#define PAGE_SIZE 0x200000
#define HIGHER_HALF 0xffff800000000000
#define KERNEL_OFFSET 0xffffffff80000000

#define PT_FLAG_PRESENT 1
#define PT_FLAG_WRITE 1 << 1
#define PT_FLAG_USER 1 << 2
#define PT_FLAG_NX 1 << 63
#define PT_FLAG_2MB 1 << 7

extern void kmain();

static int pmm_get_bit_status(int bit_number);
static void pmm_set_bit_used(int bit_number);
static void pmm_set_bit_free(int bit_number);

uint64_t *bitmap;

struct GDTEntry {
    uint16_t limit_low;      // Lower 16 bits of the limit
    uint16_t base_low;       // Lower 16 bits of the base
    uint8_t base_middle;     // Middle 8 bits of the base
    uint8_t access;          // Access byte
    uint8_t granularity;     // Granularity and higher limit
    uint8_t base_high;       // Higher 8 bits of the base
} __attribute__((packed));

struct GDTR {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

__attribute__((aligned(16))) struct GDTEntry gdt[] = {
    // Null Descriptor
    {0},

    // 16-bit Code Segment Descriptor
    {0xFFFF, 0x0000, 0x00, 0x9A, 0x00, 0x00},

    // 16-bit Data Segment Descriptor
    {0xFFFF, 0x0000, 0x00, 0x92, 0x00, 0x00},

    // 32-bit Code Segment Descriptor
    {0xFFFF, 0x0000, 0x00, 0x9A, 0xCF, 0x00},

    // 32-bit Data Segment Descriptor
    {0xFFFF, 0x0000, 0x00, 0x92, 0xCF, 0x00},

    // 64-bit Code Segment Descriptor
    {0x0000, 0x0000, 0x00, 0x9A, 0x20, 0x00},

    // 64-bit Data Segment Descriptor
    {0x0000, 0x0000, 0x00, 0x92, 0x00, 0x00},
};

struct GDTR gdtr = {
    .limit = sizeof(gdt) - 1,
    .base = (uint64_t)&gdt,
};

uint64_t find_mem(struct limine_memmap_entry **entries, int entry_count) {
	uint64_t mem_ptr = 0; 
	uint64_t size = 0;

	for (int i = 0; i < entry_count; ++i) {
		if (entries[i]->type == 0 && entries[i]->length >= 0x40200000) {
			mem_ptr = entries[i]->base;
			size = entries[i]->length;
			break;
		}
	}

	if (size == 0 || mem_ptr == 0) {
		return 0;
	}

	kprintf("Found at least 2GB of continuous RAM\n");

	mem_ptr = ((mem_ptr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

	return mem_ptr;
}

static int pmm_get_bit_status(int bit_number) {
	int column, row, temp;
	column = bit_number % 64;
	row = bit_number >> 6;
	temp = bitmap[row];
	return temp & (1 << column);
}

static void pmm_set_bit_used(int bit_number) {
	int column, row;
	column = bit_number % 64;
	row = bit_number >> 6;
	bitmap[row] = bitmap[row] | 1 << column;
}

static void pmm_set_bit_free(int bit_number) {
	int column, row;
	column = bit_number % 64;
	row = bit_number >> 6;
	bitmap[row] = bitmap[row] & ~(1 << column);
}

uint64_t root_table_4[512] __attribute__((aligned(0x1000))) = {0};

uint64_t high_table_3[512] __attribute__((aligned(0x1000))) = {0};
uint64_t high_table_2[2][512] __attribute__((aligned(0x1000))) = {0};

uint64_t kernel_table_3[512] __attribute__((aligned(0x1000))) = {0};
uint64_t kernel_table_2[512] __attribute__((aligned(0x1000))) = {0};
uint64_t kernel_table_1[512][512] __attribute__((aligned(0x1000))) = {0};

int init_paging(uint64_t free_mem, uint64_t framebuffer_addr, uint64_t phys_kernel_addr) {
  if (framebuffer_addr % PAGE_SIZE != 0) {
    return -1;
  }

	//filling page tables
	root_table_4[256] = ((uint64_t)&high_table_3[0] - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;
	root_table_4[511] = ((uint64_t)&kernel_table_3 - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;

	high_table_3[0] = ((uint64_t)&high_table_2[0][0] - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;
	high_table_3[1] = ((uint64_t)&high_table_2[1][0] - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;

	kernel_table_3[510] = ((uint64_t)&kernel_table_2[0] - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;

	for (int i = 0; i < 512; i++) {
		high_table_2[0][i] = (free_mem + i*PAGE_SIZE) | PT_FLAG_PRESENT | PT_FLAG_WRITE | PT_FLAG_2MB;
		high_table_2[1][i] = (framebuffer_addr + i*PAGE_SIZE) | PT_FLAG_PRESENT | PT_FLAG_WRITE | PT_FLAG_2MB;
		kernel_table_2[i] = ((uint64_t)&kernel_table_1[i][0] - KERNEL_OFFSET + phys_kernel_addr) | PT_FLAG_PRESENT | PT_FLAG_WRITE;
		for (int j = 0; j < 512; j++) {
			kernel_table_1[i][j] = (phys_kernel_addr + (i * 0x1000 * 512) + (j * 0x1000)) | PT_FLAG_PRESENT | PT_FLAG_WRITE;
		}
	}

	//loading gdtr
	asm volatile("lgdt %0" :: "m"(gdtr));

	//loading cr3
	asm volatile("mov %0, %%cr3" : : "r" ((uint64_t)root_table_4 - KERNEL_OFFSET + phys_kernel_addr) : "memory");

	//changing stack pointer to mapped mem
	asm volatile("mov %%rsp, %0":: "r" (HIGHER_HALF + 0x40000000 - PAGE_SIZE): "%rsp");

	ssfn_dst.ptr = (void *)(HIGHER_HALF + 0x40000000); //should point to framebuffer

	kprintf("Changed GDT and paging table\n");

	kmain();

	//doesnt return
  return 0;
}

#define USED 1
#define FREE 0

struct mem_chunk {
  void *ptr;
  size_t size;
  struct mem_chunk *next;
  struct mem_chunk *prev;
  int state;
};

struct mem_chunk first_chunk = {
  .ptr = (void *)HIGHER_HALF,
  .size =  0x40000000 - PAGE_SIZE,
  .next = NULL,
  .prev = NULL,
  .state = FREE,
};

void *kalloc(size_t size) {  
  struct mem_chunk *cur;
  cur = &first_chunk;

  //parse the mem_chunk_structs
  while (1) {
    if (cur->size == size && cur->state == FREE) {
      break;
    }
    if (cur->size >= size + sizeof(struct mem_chunk) && cur->state == FREE) {
      break;
    }
    if (cur->next != 0) {
      cur = cur->next;
    } else {
      kprintf("Error: Could not find RAM for kernel");
      return NULL;
    }
  }

  if (cur->size == size) {
    cur->state = USED;
    return cur->ptr;
  }

  struct mem_chunk *new;
  new = cur->ptr + cur->size;

  new->ptr = (void *)new + sizeof(struct mem_chunk);
  new->size = size;
  new->next = NULL;
  new->prev = cur;
  new->state = USED;
  
  if (cur->next != NULL) {
    new->next = cur->next;
  }

  cur->next = new;

  return new->ptr;
}

static void merge_mem_chunks();

void kfree(void *ptr) {
  struct mem_chunk *to_delete, *prev;
  to_delete = ptr - sizeof(struct mem_chunk);
  prev = to_delete->prev;
  
  if (prev->state == USED) {
    to_delete->state = FREE;
  } else {
    prev->size = to_delete->size + sizeof(struct mem_chunk);
    prev->next = to_delete->next;
  }

  merge_mem_chunks();
}

static void merge_mem_chunks() {
  //TODO: look for fragmentation in the mem chunks
}
