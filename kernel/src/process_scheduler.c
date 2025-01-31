#include <mman.h>
#include <process_scheduler.h>
#include <stdint.h>
#include <string.h>

void fetch_registers(struct registers *regs)
{
    asm("movq %%cr3, %0" : "=r"(regs->cr3)::);
    asm("pushfq; pop %0;" : "=m"(regs->eflags) : : "memory");
    asm("movq %%rsp, %0" : "=m"(regs->rsp) : : "%rsp");
    asm("movq %%rax, %0" : "=m"(regs->rax) : : "%rax");
    asm("movq %%rbx, %0" : "=m"(regs->rbx) : : "%rbx");
    asm("movq %%rcx, %0" : "=m"(regs->rcx) : : "%rcx");
    asm("movq %%rdx, %0" : "=m"(regs->rdx) : : "%rdx");
    asm("movq %%rsi, %0" : "=m"(regs->rsi) : : "%rsi");
    asm("movq %%rdi, %0" : "=m"(regs->rdi) : : "%rdi");
    asm("movq %%rbp, %0" : "=m"(regs->rbp) : : "%rbp");
    asm("movq %%r8, %0" : "=m"(regs->r8) : : "%r8");
    asm("movq %%r9, %0" : "=m"(regs->r9) : : "%r9");
    asm("movq %%r10, %0" : "=m"(regs->r10) : : "%r10");
    asm("movq %%r11, %0" : "=m"(regs->r11) : : "%r11");
    asm("movq %%r12, %0" : "=m"(regs->r12) : : "%r12");
    asm("movq %%r13, %0" : "=m"(regs->r13) : : "%r13");
    asm("movq %%r14, %0" : "=m"(regs->r14) : : "%r14");
    asm("movq %%r15, %0" : "=m"(regs->r15) : : "%r15");
}

void load_registers(struct registers *regs)
{
    asm("movq %0, %%cr3" : "=r"(regs->cr3)::);
    asm("pushq %0; popfq;" : "=m"(regs->eflags) : : "memory");
    asm("movq %0, %%rsp" : "=m"(regs->rsp) : : "%rsp");
    asm("movq %0, %%rax" : "=m"(regs->rax) : : "%rax");
    asm("movq %0, %%rbx" : "=m"(regs->rbx) : : "%rbx");
    asm("movq %0, %%rcx" : "=m"(regs->rcx) : : "%rcx");
    asm("movq %0, %%rdx" : "=m"(regs->rdx) : : "%rdx");
    asm("movq %0, %%rsi" : "=m"(regs->rsi) : : "%rsi");
    asm("movq %0, %%rdi" : "=m"(regs->rdi) : : "%rdi");
    asm("movq %0, %%rbp" : "=m"(regs->rbp) : : "%rbp");
    asm("movq %0, %%r8" : "=m"(regs->r8) : : "%r8");
    asm("movq %0, %%r9" : "=m"(regs->r9) : : "%r9");
    asm("movq %0, %%r10" : "=m"(regs->r10) : : "%r10");
    asm("movq %0, %%r11" : "=m"(regs->r11) : : "%r11");
    asm("movq %0, %%r12" : "=m"(regs->r12) : : "%r12");
    asm("movq %0, %%r13" : "=m"(regs->r13) : : "%r13");
    asm("movq %0, %%r14" : "=m"(regs->r14) : : "%r14");
    asm("movq %0, %%r15" : "=m"(regs->r15) : : "%r15");
}

extern struct process kernel_task;

int create_process(void(*f), void *stack_ptr, void *cr3 /*, size_t flags*/)
{
    struct process *task = (struct process *)kalloc(sizeof(struct process));
    task->execution_ptr = f;
    memset((void *)&task->regs, 0, sizeof(struct registers));
    task->regs.cr3 = (uint64_t)cr3;
    task->regs.cr3 = (uint64_t)stack_ptr;
    task->next = NULL;

    struct process *curr = &kernel_task;

    while (curr->next != NULL)
    {
        curr = curr->next;
    }

    curr->next = task;

    return 0;
}
