#include <lib/debug.h>
#include <lib/types.h>
#include <lib/x86.h>
#include <lib/trap.h>
#include <lib/syscall.h>
#include <dev/intr.h>
#include <lib/pmap.h>
#include <pcpu/PCPUIntro/export.h>
#include <lib/string.h>

#include <vmm/MPTKern/export.h>
#include "import.h"
#include <kern/dev/console.h>
static char sys_buf[NUM_IDS][PAGESIZE];

/**
 * Copies a string from user into buffer and prints it to the screen.
 * This is called by the user level "printf" library as a system call.
 */
void sys_puts(tf_t *tf)
{
  unsigned int cur_pid;
  unsigned int str_uva, str_len;
  unsigned int remain, cur_pos, nbytes;

  cur_pid = get_curid();
  str_uva = syscall_get_arg2(tf);
  str_len = syscall_get_arg3(tf);

  if (!(VM_USERLO <= str_uva && str_uva + str_len <= VM_USERHI)) {
    syscall_set_errno(tf, E_INVAL_ADDR);
    return;
  }

  remain = str_len;
  cur_pos = str_uva;

  while (remain) {
    if (remain < PAGESIZE - 1)
      nbytes = remain;
    else
      nbytes = PAGESIZE - 1;

    if (pt_copyin(cur_pid,
		  cur_pos, sys_buf[cur_pid], nbytes) != nbytes) {
      syscall_set_errno(tf, E_MEM);
      return;
    }

    sys_buf[cur_pid][nbytes] = '\0';
    KERN_INFO("%s", sys_buf[cur_pid]);

    remain -= nbytes;
    cur_pos += nbytes;
  }

  syscall_set_errno(tf, E_SUCC);
}

extern uint8_t _binary___obj_user_pingpong_ping_start[];
extern uint8_t _binary___obj_user_pingpong_pong_start[];
extern uint8_t _binary___obj_user_pingpong_ding_start[];
extern uint8_t _binary___obj_user_fstest_fstest_start[];

extern uint8_t _binary___obj_user_shell_shell_start[];

/**
 * Spawns a new child process.
 * The user level library function sys_spawn (defined in user/include/syscall.h)
 * takes two arguments [elf_id] and [quota], and returns the new child process id
 * or NUM_IDS (as failure), with appropriate error number.
 * Currently, we have three user processes defined in user/pingpong/ directory,
 * ping, pong, and ding.
 * The linker ELF addresses for those compiled binaries are defined above.
 * Since we do not yet have a file system implemented in mCertiKOS,
 * we statically loading the ELF binraries in to the memory based on the
 * first parameter [elf_id], i.e., ping, pong, and ding corresponds to
 * the elf_id of 1, 2, and 3, respectively.
 * If the parameter [elf_id] is none of those three, then it should return
 * NUM_IDS with the error number E_INVAL_PID. The same error case apply
 * when the proc_create fails.
 * Otherwise, you mark it as successful, and return the new child process id.
 */
void sys_spawn(tf_t *tf)
{
  unsigned int new_pid;
  unsigned int elf_id, quota;
  void *elf_addr;
  unsigned int qok, nc, curid;

  elf_id = syscall_get_arg2(tf);
  quota = syscall_get_arg3(tf);

  qok = container_can_consume(curid, quota);
  nc = container_get_nchildren(curid);
  curid = get_curid();
  if (qok == 0) {
    syscall_set_errno(tf, E_EXCEEDS_QUOTA);
    syscall_set_retval1(tf, NUM_IDS);
    return;
  }
  else if (NUM_IDS < curid * MAX_CHILDREN + 1 + MAX_CHILDREN) {
    syscall_set_errno(tf, E_MAX_NUM_CHILDEN_REACHED);
    syscall_set_retval1(tf, NUM_IDS);
    return;
  }
  else if (nc == MAX_CHILDREN) {
    syscall_set_errno(tf, E_INVAL_CHILD_ID);
    syscall_set_retval1(tf, NUM_IDS);
    return;
  }

  if (elf_id == 1) {
    elf_addr = _binary___obj_user_pingpong_ping_start;
  } else if (elf_id == 2) {
    elf_addr = _binary___obj_user_pingpong_pong_start;
  } else if (elf_id == 3) {
    elf_addr = _binary___obj_user_pingpong_ding_start;
  } else if (elf_id == 4) {
    //elf_addr = _binary___obj_user_shell_shell_start;


    elf_addr = _binary___obj_user_fstest_fstest_start;
  }else if (elf_id == 5)
    {
      elf_addr = _binary___obj_user_shell_shell_start; 
    }else {
    syscall_set_errno(tf, E_INVAL_PID);
    syscall_set_retval1(tf, NUM_IDS);
    return;
  }

  new_pid = proc_create(elf_addr, quota);

  if (new_pid == NUM_IDS) {
    syscall_set_errno(tf, E_INVAL_PID);
    syscall_set_retval1(tf, NUM_IDS);
  } else {
    syscall_set_errno(tf, E_SUCC);
    syscall_set_retval1(tf, new_pid);
  }
}

/**
 * Yields to another thread/process.
 * The user level library function sys_yield (defined in user/include/syscall.h)
 * does not take any argument and does not have any return values.
 * Do not forget to set the error number as E_SUCC.
 */
void sys_yield(tf_t *tf)
{
  thread_yield();
  syscall_set_errno(tf, E_SUCC);
}

void sys_readline(tf_t *tf)
{
  char* line = readline("#");
  int data = syscall_get_arg2(tf);
  //read to kernel buff/ then to buffer given
  int length = strnlen(line, 1022);
  pt_copyout(line, get_curid(), data, length);
  syscall_set_errno(tf, E_SUCC);
  syscall_set_retval1(tf, 0);
}

void sys_map(tf_t *tf)
{
  unsigned int buff_ptr = syscall_get_arg2(tf);

  // map_page(get_curid(), buff_ptr, (0x000A0000/4096), 7);
  
  for (unsigned int i = 0; i < 10; i++)
    {
      unsigned int cur = get_curid();
      map_page(cur, buff_ptr + (4096 * i),(0x000A0000/4096) + i, (PTE_P | PTE_W | PTE_U));
    }
  
  syscall_set_errno(tf, E_SUCC);
  syscall_set_retval1(tf, 0);
}

void sys_produce(tf_t *tf)
{
  unsigned int i;
  for(i = 0; i < 5; i++) {
    intr_local_disable();
    KERN_DEBUG("CPU %d: Process %d: Produced %d\n", get_pcpu_idx(), get_curid(), i);
    intr_local_enable();
  }
  syscall_set_errno(tf, E_SUCC);
}

void sys_consume(tf_t *tf)
{
  unsigned int i;
  for(i = 0; i < 5; i++) {
    intr_local_disable();
    KERN_DEBUG("CPU %d: Process %d: Consumed %d\n", get_pcpu_idx(), get_curid(), i);
    intr_local_enable();
  }
  syscall_set_errno(tf, E_SUCC);
}
