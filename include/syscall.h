#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_waitpid(struct trapframe *tf, int32_t *retval);

#endif /* _SYSCALL_H_ */
