#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include "inode.h"
#include "dir.h"

// Directories

int
dir_namecmp(const char *s, const char *t)
{
  return strncmp(s, t, DIRSIZ);
}

/**
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode*
dir_lookup(struct inode *dp, char *name, uint32_t *poff)
{
  uint32_t off, inum;
  struct dirent de;

  if(dp->type != T_DIR)
    KERN_PANIC("dir_lookup not DIR");

  //TODO

    
  //need to cycle through file

  for (off = 0; off < dp->size; off += sizeof(de))
  {
    //move to dirent
    if(inode_read(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    {
      KERN_PANIC("INCORRECT SIZE");
    }

    if(de.inum == 0)
      continue;
    
    if (!dir_namecmp(de.name, name))
    {
      if(poff != 0)
	*poff = off;

      inum = de.inum;
      return(inode_get(dp->dev, inum));
    }
  }
  
  return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int
dir_link(struct inode *dp, char *name, uint32_t inum)
{
  // TODO: Check that name is not present.
  
  // TODO: Look for an empty dirent.
  uint32_t off;
  struct inode* node_ptr;
  struct dirent de;

  node_ptr = dir_lookup(dp, name, 0);
  if(node_ptr != 0)
  {
      inode_put(node_ptr);
      return (-1);
  }

  for (off = 0; off < dp->size; off += sizeof(de))
  {
    //move to dirent
    inode_read(dp, (char*)&de, off, sizeof(de));

    if (de.inum == 0)
    {
      break;
    }

  }

    de.inum = inum;
    strncpy(de.name, name, DIRSIZ);
    inode_write(dp, (char*)&de, off, sizeof(de));
  // KERN_PANIC("ERROR IN DIR_LINK! NO EMPTY SUB_DIR ENTRY!!");


  return 0;
}
