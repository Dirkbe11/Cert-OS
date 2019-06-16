**Certified Operating System**  
CURRENTLY UNDER CONSTRUCTION.  
You may have issues if running the operating system using ubuntu on a virtual machine.  

Cert is a small operating system built using C and run using the qemu emulator. 

**Details:**
The operating system was build by adding to an extensive already existing codebase. 
```
The features added include:
•	Physical Memory Manager
•	Virtual Memory Manager
•	Thread & Process Management
•	Trap Handling
•	Copy-on-write 
•	Multi-core Support
•	Preemptive Multitasking
•	Thread Scheduler : sleep on resource wait
•	User Shell 
```

REQUIREMENTS:  
-qemu  
-gcc 32-bit multilib

RUNNING:
-In the home directory,  
1.) run make  
2.) run make qemu

