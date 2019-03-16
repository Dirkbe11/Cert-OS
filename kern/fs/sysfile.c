//
// File-system system calls.
//

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include <kern/thread/PTCBIntro/export.h>
#include <kern/lib/trap.h>
#include <kern/lib/syscall.h>
#include <kern/trap/TSyscallArg/export.h>
#include <kern/lib/pmap.h>
#include <thread/PCurID/export.h>
#include <kern/lib/spinlock.h>

#include "dir.h"
#include "path.h"
#include "file.h"
#include "fcntl.h"
#include "log.h"

char BUFF[10000];

static spinlock_t Block;

// //  if (!(VM_USERLO <= str_uva && str_uva + str_len <= VM_USERHI)) {
//     syscall_set_errno(tf, E_INVAL_ADDR);
//     return;
//   }



/**
 * This function is not a system call handler, but an auxiliary function
 * used by sys_open.
 * Allocate a file descriptor for the given file.
 * You should scan the list of open files for the current thread
 * and find the first file descriptor that is available.
 * Return the found descriptor or -1 if none of them are free.
 */
static int
fdalloc(struct file *f)
{
  //TODO
  unsigned int curr_id;
  curr_id = get_curid();

  //get the openfile array
  struct file **fp;
  fp = tcb_get_openfiles(curr_id);

  for (int i = 0; i < NOFILE; ++i)
  {
    if (fp[i] == 0)
    {
      tcb_set_openfiles(curr_id, i, f);
      return i;
    }
  }

  return -1;
}


/**
 * From the file indexed by the given file descriptor, read n bytes and save them
 * into the buffer in the user. As explained in the assignment specification,
 * you should first write to a kernel buffer then copy the data into user buffer
 * with pt_copyout.
 * Return Value: Upon successful completion, read() shall return a non-negative
 * integer indicating the number of bytes actually read. Otherwise, the
 * functions shall return -1 and set errno E_BADF to indicate the error.
 */
void sys_read(tf_t *tf)
{
  spinlock_acquire(&Block);
  //TODO
  memzero(BUFF, 10000);
  int fd;
  unsigned int buffer;
  //char* buffer;
  int data, data_read;
  unsigned int n, curr_id;

  curr_id = get_curid();

  struct file* file_ptr;
  //file_ptr = tcb_get_openfiles(curr_id);

  fd = syscall_get_arg2(tf);
  buffer = syscall_get_arg3(tf);
  n = syscall_get_arg4(tf);

  if (fd < 0)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    spinlock_release(&Block);
    return;
  }

  if(n < 0)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    spinlock_release(&Block);
    return;
  }

  /*if(buffer < VM_USERLO || buffer + n > VM_USERHI)
    {
      syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
      spinlock_release(&Block);
      return;
      }*/

  if(n > 10000)
    {
      syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
      spinlock_release(&Block);
      return;
    }

  file_ptr = tcb_get_openfiles(curr_id)[fd];
  //read to kernel buffer

  if(file_ptr <= 0)
    {
      syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
      spinlock_release(&Block);
      return;
    }
  
  data = file_read(file_ptr, BUFF, n);

  if (data < 0)
    {
      syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
      spinlock_release(&Block);
      return;
    }

   

  data_read = pt_copyout(BUFF, get_curid(), buffer, data);

  syscall_set_errno(tf, E_SUCC);
  syscall_set_retval1(tf, data_read);
  spinlock_release(&Block);
  return;

}

/**
 * Write n bytes of data in the user's buffer into the file indexed by the file descriptor.
 * You should first copy the data info an in-kernel buffer with pt_copyin and then
 * pass this buffer to appropriate file manipulation function.
 * Upon successful completion, write() shall return the number of bytes actually
 * written to the file associated with f. This number shall never be greater
 * than nbyte. Otherwise, -1 shall be returned and errno E_BADF set to indicate the
 * error.
 */
void sys_write(tf_t *tf)
{
  spinlock_acquire(&Block);
  memzero(BUFF, 10000);
  //TODO
  int fd, data;
  int p;
  //char *p;
  unsigned int n;
  unsigned int curr_id = get_curid();

  struct file *file_ptr;

  fd = syscall_get_arg2(tf);
  p = syscall_get_arg3(tf);
  n = syscall_get_arg4(tf);

  
  if(fd < 0)
    {
       syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
      spinlock_release(&Block);
      return;
    }
  
  file_ptr = tcb_get_openfiles(curr_id)[fd];
  
  if(n < 0 || n > 10000)
    {
       syscall_set_errno(tf, E_BADF);
       syscall_set_retval1(tf, -1);
       spinlock_release(&Block);
       return;
    }

  if(p < VM_USERLO || p + n > VM_USERHI)
    {
       syscall_set_errno(tf, E_BADF);
       syscall_set_retval1(tf, -1);
       spinlock_release(&Block);
       return;
    }

  if(file_ptr == 0)
    {
        syscall_set_errno(tf, E_BADF);
       syscall_set_retval1(tf, -1);
       spinlock_release(&Block);
       return;
    }

  
  
  int data0 = pt_copyin(curr_id, p, BUFF, n);
  /*if (data == -1)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    spinlock_release(&Block);
    return;
    }*/

  data = file_write(file_ptr, BUFF, data0);

  if (data == -1)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    spinlock_release(&Block);
    return;
  }

  syscall_set_errno(tf, E_SUCC);
  syscall_set_retval1(tf, data);
  spinlock_release(&Block);
  return;



}


/**
 * Return Value: Upon successful completion, 0 shall be returned; otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_close(tf_t *tf)
{
  int fd;
  unsigned int curr_id;
  curr_id = get_curid();

  struct file * file_ptr;
  fd = syscall_get_arg2(tf);

  if(fd < 0)
    {
       syscall_set_errno(tf, E_BADF);
       syscall_set_retval1(tf, -1);
       return;
    }

  file_ptr = tcb_get_openfiles(curr_id)[fd];
  
  if (file_ptr <= 0)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    return;
  }

  tcb_set_openfiles(curr_id, fd, 0);
  file_close(file_ptr);
  syscall_set_errno(tf, E_SUCC);
  syscall_set_retval1(tf, 0);
  return;



  //TODO
}

/**
 * Return Value: Upon successful completion, 0 shall be returned. Otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_fstat(tf_t *tf)
{
  //TODO
  int fd, curr_id;
  struct file* file_ptr;
  struct file_stat* st;
  struct file_stat state;
  int FSTAT;
  st = (struct file_stat *)syscall_get_arg3(tf);
  curr_id = get_curid();

  fd = syscall_get_arg2(tf);
  
  if (fd < 0)
  {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
    return;
  }
  
  // st = (struct file_stat*)syscall_get_arg3(tf);

  file_ptr = tcb_get_openfiles(curr_id)[fd];
  if(file_ptr <= 0)
    {
       syscall_set_errno(tf, E_BADF);
       syscall_set_retval1(tf, -1);
       spinlock_release(&Block);
       return;
    }

  //st = syscall_get_arg3(tf);
  file_stat(file_ptr, st);
  //syscall_set_errno(tf, E_SUCC);
  //syscall_set_retval1(tf, 0);
  //pt_copyin(curr_id, st, &state, sizeof(struct file_stat));

  FSTAT = file_stat(file_ptr, st);
  
  if(FSTAT != 0)
    {
      syscall_set_errno(tf, E_BADF);
      syscall_set_retval1(tf, -1);
    }
  else{
    syscall_set_errno(tf, E_SUCC);
    syscall_set_retval1(tf, FSTAT);
  }
  

  return;

}

/**
 * Create the path new as a link to the same inode as old.
 */
void sys_link(tf_t *tf)
{
  char name[DIRSIZ], new[128], old[128];
  struct inode *dp, *ip;
  unsigned int old_lengther = syscall_get_arg4(tf);
  unsigned int new_lengther = syscall_get_arg5(tf);

  if(old_lengther >= 128)
    {
      old_lengther = 127;
    }
  if(new_lengther >= 128)
    {
      new_lengther = 127;
    }
  
  pt_copyin(get_curid(), syscall_get_arg2(tf), old, old_lengther);
  pt_copyin(get_curid(), syscall_get_arg3(tf), new, new_lengther);

  old[old_lengther] = 0;
  new[new_lengther] = 0;
  
  if((ip = namei(old)) == 0){
    syscall_set_errno(tf, E_NEXIST);
    return;
  }
  
  begin_trans();

  inode_lock(ip);
  if(ip->type == T_DIR){
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }

  ip->nlink++;
  inode_update(ip);
  inode_unlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  inode_lock(dp);
  if(   dp->dev != ip->dev
     || dir_link(dp, name, ip->inum) < 0){
    inode_unlockput(dp);
    goto bad;
  }
  inode_unlockput(dp);
  inode_put(ip);

  commit_trans();

  syscall_set_errno(tf, E_SUCC);
  return;

bad:
  inode_lock(ip);
  ip->nlink--;
  inode_update(ip);
  inode_unlockput(ip);
  commit_trans();
  syscall_set_errno(tf, E_DISK_OP);
  return;
}

/**
 * Is the directory dp empty except for "." and ".." ?
 */
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(inode_read(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      KERN_PANIC("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

void sys_unlink(tf_t *tf)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], path[128];
  uint32_t off;
  unsigned int lengther;
  lengther = syscall_get_arg3(tf);

  if(lengther >= 128)
    {
      lengther = 127;
    }
  
  pt_copyin(get_curid(), syscall_get_arg2(tf), path, lengther);

  path[lengther] = 0;
  
  if((dp = nameiparent(path, name)) == 0){
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }
  
  begin_trans();

  inode_lock(dp);

  // Cannot unlink "." or "..".
  if(   dir_namecmp(name, ".") == 0
     || dir_namecmp(name, "..") == 0)
    goto bad;

  if((ip = dir_lookup(dp, name, &off)) == 0)
    goto bad;
  inode_lock(ip);

  if(ip->nlink < 1)
    KERN_PANIC("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    inode_unlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(inode_write(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    KERN_PANIC("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    inode_update(dp);
  }
  inode_unlockput(dp);

  ip->nlink--;
  inode_update(ip);
  inode_unlockput(ip);

  commit_trans();

  syscall_set_errno(tf, E_SUCC);
  return;
  
bad:
  inode_unlockput(dp);
  commit_trans();
  syscall_set_errno(tf, E_DISK_OP);
  return;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  uint32_t off;
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  inode_lock(dp);

  if((ip = dir_lookup(dp, name, &off)) != 0){
    inode_unlockput(dp);
    inode_lock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    inode_unlockput(ip);
    return 0;
  }

  if((ip = inode_alloc(dp->dev, type)) == 0)
    KERN_PANIC("create: ialloc");

  inode_lock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  inode_update(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    inode_update(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(   dir_link(ip, ".", ip->inum) < 0
       || dir_link(ip, "..", dp->inum) < 0)
      KERN_PANIC("create dots");
  }

  if(dir_link(dp, name, ip->inum) < 0)
    KERN_PANIC("create: dir_link");

  inode_unlockput(dp);

  return ip;
}

void sys_open(tf_t *tf)
{
  char path[128];
  int fd, omode;
  struct file *f;
  struct inode *ip;
  unsigned int lengther;
  lengther = syscall_get_arg4(tf);

  if (lengther >= 128)
    {
      lengther = 127;
    }
  

  static int first = TRUE;
  if (first) {
    first = FALSE;
    log_init();
  }

  pt_copyin(get_curid(), syscall_get_arg2(tf), path, lengther);
  omode = syscall_get_arg3(tf);

  path[lengther] = 0;
  
  if (!path)
    KERN_PANIC("sys_open: no path");
  
  if(omode & O_CREATE){
    begin_trans();
    ip = create(path, T_FILE, 0, 0);
    commit_trans();
    if(ip == 0){
      syscall_set_retval1(tf, -1);
      syscall_set_errno(tf, E_CREATE);
      return;
    }
  } else {
    if((ip = namei(path)) == 0){
      syscall_set_retval1(tf, -1);
      syscall_set_errno(tf, E_NEXIST);
      return;
    }
    inode_lock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      inode_unlockput(ip);
      syscall_set_retval1(tf, -1);
      syscall_set_errno(tf, E_DISK_OP);
      return;
    }
  }

  if((f = file_alloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      file_close(f);
    inode_unlockput(ip);
    syscall_set_retval1(tf, -1);
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }
  inode_unlock(ip);

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  syscall_set_retval1(tf, fd);
  syscall_set_errno(tf, E_SUCC);
}

void sys_mkdir(tf_t *tf)
{
  char path[128];
  struct inode *ip;
  unsigned int lengther;
  lengther = syscall_get_arg3(tf);

  if(lengther >= 128)
    {
      lengther = 127;
    }
  
  pt_copyin(get_curid(), syscall_get_arg2(tf), path, lengther);

  path[lengther] = 0;
  
  begin_trans();
  if((ip = (struct inode*)create(path, T_DIR, 0, 0)) == 0){
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }
  inode_unlockput(ip);
  commit_trans();
  syscall_set_errno(tf, E_SUCC);
}

void sys_chdir(tf_t *tf)
{
  char path[128];
  struct inode *ip;
  int pid = get_curid();
  unsigned int lengther;
  lengther = syscall_get_arg3(tf);

  if(lengther >= 128)
    {
      lengther = 127;
    }
  
  pt_copyin(get_curid(), syscall_get_arg2(tf), path, lengther);

  path[lengther] = 0;
  
  if((ip = namei(path)) == 0){
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }
  inode_lock(ip);
  if(ip->type != T_DIR){
    inode_unlockput(ip);
    syscall_set_errno(tf, E_DISK_OP);
    return;
  }
  inode_unlock(ip);
  inode_put(tcb_get_cwd(pid));
  tcb_set_cwd(pid, ip);
  syscall_set_errno(tf, E_SUCC);
}
