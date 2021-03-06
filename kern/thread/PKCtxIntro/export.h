#ifndef _KERN_THREAD_PKCTXINTRO_H_
#define _KERN_THREAD_PKCTXINTRO_H_

#ifdef _KERN_

void kctx_set_esp(unsigned int pid, void *esp);
void kctx_set_eip(unsigned int pid, void *eip);
void kctx_switch(unsigned int from_pid, unsigned int to_pid);

#endif /* _KERN_ */

#endif /* !_KERN_THREAD_PKCTXINTRO_H_ */

