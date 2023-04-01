#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  backtrace();
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64
sys_sigreturn(void) {
    struct proc *p = myproc();
    p->trapframe->a0 = p->reg_value[0]; 
    p->trapframe->a1 = p->reg_value[1]; 
    p->trapframe->a2 = p->reg_value[2]; 
    p->trapframe->a3 = p->reg_value[3]; 
    p->trapframe->a4 = p->reg_value[4];
    p->trapframe->a5 = p->reg_value[5]; 
    p->trapframe->a6 = p->reg_value[6]; 
    p->trapframe->a7 = p->reg_value[7]; 
    p->trapframe->t0 = p->reg_value[8]; 
    p->trapframe->t1 = p->reg_value[9];
    p->trapframe->t2 = p->reg_value[10]; 
    p->trapframe->t3 = p->reg_value[11]; 
    p->trapframe->t4 = p->reg_value[12]; 
    p->trapframe->t5 = p->reg_value[13]; 
    p->trapframe->t6 = p->reg_value[14];
    p->trapframe->epc = p->reg_value[15];
    p->trapframe->s0 = p->reg_value[16];
    p->trapframe->s1 = p->reg_value[17];
    p->trapframe->s2 = p->reg_value[18];
    p->trapframe->s3 = p->reg_value[19];
    p->trapframe->s4 = p->reg_value[20];
    p->trapframe->s5 = p->reg_value[21];
    p->trapframe->s6 = p->reg_value[22];
    p->trapframe->s7 = p->reg_value[23];
    p->trapframe->s8 = p->reg_value[24];
    p->trapframe->s9 = p->reg_value[25];
    p->trapframe->s10 = p->reg_value[26];
    p->trapframe->s11 = p->reg_value[27];
    p->trapframe->ra = p->reg_value[28];
    p->trapframe->sp = p->reg_value[29];
    p->prevent_re_entry = 0;

    return 0;
}

uint64 
sys_sigalarm(void) {
    int ticks;
    uint64 handler; 
    struct proc *p = myproc();

    if (argint(0, &ticks) < 0)
        return -1;
    if (argaddr(1, &handler) < 0)
        return -1;

    p->ticks = ticks;
    p->handler = handler;
    return 0;
}
