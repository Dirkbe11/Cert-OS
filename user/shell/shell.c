#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <font.h>
#include <gcc.h>

//#define gcc_aligned(mult)       __attribute__((aligned (mult)))
//#include <kern/dev/console.h>


struct VGAD {

  char vga_buffer[38400] gcc_aligned(4096);
  int cur1;
  int cur2;
};

struct VGAD video;

char str_buffer[10000];


int run(char* command)
{
  //volatile char *vga = (volatile char *) 0xB8000;
  //*vga++ = 'x';
  
  int length = strlen(command);
  /*
  while(1){
    

    if(strncmp("ls", command, 2) == 0)
      {
	printf("HI\n");
      }
    
    memzero(command, length);
    return 0;
  */
     
}

int main()
{
  int i = 1;
  int k = 15;
  
  while(1){
  memzero(str_buffer, strlen(str_buffer));
  sys_readline(str_buffer);
  printf(str_buffer);
  if(strncmp("save", str_buffer, 2) == 0)
    {
      k = 15;
      i = 1;
    
  memzero(str_buffer, strlen(str_buffer));
  for(int l = 0; l < 10 * 38399; l++)
    {
      sys_map(video.vga_buffer);
      i++;
      if(i == 38399)
	{
	  if (k == 15)
	    k = 0;
	 else if (k == 0)
	    k = 15;
	  i = 0;
	  
	}
      
      video.vga_buffer[i] = k;
     
      for(int j = 0; j < 9999999; j++)
	;
	
    }
    }

  for(int o = 0; o < 38399; o ++)
    video.vga_buffer[o] = 0;

  memzero(str_buffer, strlen(str_buffer));
  }

  return 0;
}
