#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <uio.h>
#include <elf.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vnode.h>
#include <kern/unistd.h>
#include <test.h>
#include <synch.h>
#include <vm.h>
/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

int sys_execv(const char *prog, char **args, int *retval)
{
    int result;
#if DEBUG_CODE & DEBUG_EXECV
    kprintf("exec: entered\n");
#endif
    if (prog == NULL || args == NULL || (int *)prog == 0x80000000 || (int *)prog == 0x40000000)
    {
        *retval = -1;
        return EFAULT;
    }
    if (strcmp(prog, "") == 0)
    {
        *retval = -1;
        return EINVAL;
    }
    if ((int *)args == 0x80000000 || (int *)args == 0x40000000)
    {
        *retval = -1;
        return EFAULT;
    }
    unsigned argc = 0;
    unsigned i = 0;
    unsigned j = 0;
    while (args[i] != NULL)
    {
        j += strlen(args[i]) + 1;
        i++;
        if ((int *)args[i] == 0x80000000 || (int *)args[i] == 0x40000000)
        {
            *retval = -1;
            return EFAULT;
        }
    }
    argc = i;
#if DEBUG_CODE & DEBUG_EXECV
    kprintf("exec:i=%d, j=%d\n", i, j);
#endif
    /* handle path name
 */
    unsigned path_size = strlen(prog);
    char path_buffer[path_size];
    result = copyinstr((const_userptr_t)prog, path_buffer, 256, NULL);
    /* handle array of string
 */
    char *args_kernel[argc];
    char buffer[j];
    char *iterater;
    iterater = &buffer[0];

    size_t size;
    i = 0;
    for (; i < argc; i++)
    {
        result = copyinstr((const_userptr_t)args[i], iterater, 256, &size);
        args_kernel[i] = iterater;
#if DEBUG_CODE & DEBUG_EXECV
        kprintf("exec: cpy: %s @%p\n",
                iterater, iterater);
#endif
        iterater += size;
    }
#if DEBUG_CODE & DEBUG_EXECV
    kprintf("exec: argsk: %p, argsk[0]: %p\n",
            args_kernel, args_kernel[0]);
    kprintf("ecec: argsk[0]string: %s\n", args_kernel[0]);
#endif
    as_destroy(curthread->t_vmspace);
    curthread->t_vmspace = NULL;

    runprogram(path_buffer, args_kernel, argc);

    // //check parameters
    // if (prog == NULL || args == NULL)
    // {
    //     *retval = -1;
    //     return EFAULT;
    // }
    // if (prog == '\0' || strcmp(prog, "") == 0)
    // {
    //     *retval = -1;
    //     return EINVAL;
    // }
    // char *pathname = kmalloc(128 * sizeof(char));

    // //copy path name from user space to kernel space
    // int result = copyinstr((const_userptr_t)prog, pathname, 128, NULL);

    // if (result != 0)
    // {
    //     kprintf("ERROR: copy path name from user space to kernel space");
    //     kfree(pathname);
    //     *retval = result;
    //     return EFAULT;
    // }

    // //get arg number and make a copy form args to argv
    // int argc = 0;
    // int i = 0;
    // while (args[i] != NULL)
    // {
    //     i++;
    // }
    // argc = i;

    // char **argv;
    // argv = kmalloc(sizeof(userptr_t) * (argc + 1));
    // if (argv == NULL)
    // {
    //     kprintf("ERROR: Allocate memory for argv");
    //     return ENOMEM;
    // }

    // //copy the address of each string
    // //args are the address of each string
    // result = copyin((const_userptr_t)args, argv, sizeof(userptr_t) * (argc + 1));

    // if (result != 0)
    // {
    //     kprintf("ERROR: copy args to argv");
    //     kfree(pathname);
    //     kfree(argv);
    //     *retval = result;
    //     return EFAULT;
    // }

    // //copy string from args to argv
    // int k;
    // for (i = 0; i < argc; i++)
    // {
    //     if (args[i] == 0x80000000)
    //     {
    //         kprintf("ERROR: user space conflict with reserved kernel space");
    //         kfree(argv);
    //         kfree(pathname);
    //         *retval = -1;
    //         return EFAULT;
    //     }

    //     argv[i] = kmalloc(sizeof(char) * (strlen(args[i]) + 1));
    //     //no available memory space, free everything
    //     if (argv[i] == NULL)
    //     {
    //         kprintf("ERROR: can not allocate memory for argv[i]");
    //         for (k = 0; k < i; k++)
    //         {
    //             kfree(argv[k]);
    //         }
    //         kfree(argv);
    //         kfree(pathname);
    //         return ENOMEM;
    //     }

    //     //copy
    //     result = copyinstr((const_userptr_t)args[i], argv[i], strlen(args[i]) + 1, NULL);

    //     if (result != 0)
    //     {
    //         kprintf("ERROR: try to copy each string from args[%d] to argv[%d]", i, i);
    //         for (k = 0; k < i; k++)
    //         {
    //             kfree(argv[k]);
    //         }
    //         kfree(argv);
    //         kfree(pathname);
    //         *retval = result;
    //         return EFAULT;
    //     }
    // }

    // //everything parameters in kernel
    // // open the file
    // struct vnode *vn;
    // //flag num 0
    // result = vfs_open(pathname, O_RDONLY, &vn);

    // if (result != 0)
    // {
    //     kprintf("ERROR: open file fails");
    //     i = 0;
    //     while (i < argc)
    //     {
    //         kfree(argv[i]);
    //         i++;
    //     }
    //     kfree(argv);
    //     kfree(pathname);
    //     return result;
    // }

    // // deal with virtual memory//
    // if (curthread->t_vmspace != NULL)
    // {
    //     as_destroy(curthread->t_vmspace);
    //     curthread->t_vmspace = NULL;
    // }

    // //create a new virtual memory space
    // curthread->t_vmspace = as_create();
    // // no memory, error handler
    // if (curthread->t_vmspace == NULL)
    // {
    //     kprintf("ERROR: vmspace is not allocated");
    //     vfs_close(vn);
    //     i = 0;
    //     while (i < argc)
    //     {
    //         kfree(argv[i]);
    //         i++;
    //     }
    //     kfree(argv);
    //     kfree(pathname);
    //     return ENOMEM;
    // }
    // as_activate(curthread->t_vmspace);
    // vaddr_t entrypoint, stackpointer;
    // result = load_elf(vn, entrypoint);
    // //entry point (initial PC)
    // if (result != 0)
    // {
    //     kprintf("ERROR: load elf file fail");
    //     vfs_close(vn);
    //     i = 0;
    //     while (i < argc)
    //     {
    //         kfree(argv[i]);
    //         i++;
    //     }
    //     kfree(argv);
    //     kfree(pathname);
    //     return result;
    // }

    // //done with the file
    // vfs_close(vn);

    // //deal with the stack
    // // find the stack pointer of the curent address space
    // result = as_define_stack(curthread->t_vmspace, &stackpointer);

    // if (result != 0)
    // {
    //     kprintf("ERROR: fail to get the defined stack");
    //     i = 0;
    //     while (i < argc)
    //     {
    //         kfree(argv[i]);
    //         i++;
    //     }
    //     kfree(argv);
    //     kfree(pathname);
    //     return result;
    // }
    // //kernel need to give the loaded information back to the userspace
    // int stringlength;
    // for (i = argc - 1; i >= 0; i--)
    // {
    //     int argvlength = strlen(argv[i]) + 1;
    //     //make sure everything is begin with the address divisible by 4
    //     if (argvlength <= 4)
    //     {
    //         stringlength = 1;
    //     }
    //     else if (argvlength % 4 == 0)
    //     {
    //         stringlength = argvlength / 4;
    //     }
    //     else if (argvlength % 4 != 0)
    //     {
    //         stringlength = argvlength / 4 + 1;
    //     }

    //     stackpointer = stackpointer - stringlength * 4;
    //     //kernel space to user space, push argv onto the stack
    //     result = copyoutstr(argv[i], (userptr_t)stackpointer, argvlength, NULL);

    //     if (result)
    //     {
    //         kprintf("ERROR: every string layer: try to copy each string from argv[%d] to stackpointer", i);
    //         int j = 0;
    //         while (j < argc)
    //         {
    //             kfree(argv[j]);
    //             j++;
    //         }
    //         kfree(argv);
    //         kfree(pathname);
    //         return result;
    //     }
    //     // free waste stuff
    //     kfree(argv[i]);
    //     // keep track where are the stings stored
    //     argv[i] = (char *)stackpointer;
    // }

    // //the end of argv (parameter passing for user prog) is NULL
    // int *null = NULL;
    // stackpointer = stackpointer - 4;
    // null = (int *)stackpointer;
    // *null = 0x00000000; // 4 bytes

    // for (i = argc - 1; i >= 0; i--)
    // {
    //     stackpointer = stackpointer - 4;
    //     result = copyout(&argv[i], (userptr_t)stackpointer, sizeof(vaddr_t));

    //     if (result != 0)
    //     {
    //         kprintf("ERROR: array of string layer: try to copy each string from argv[%d] to stackpointer", i);
    //         kfree(argv);
    //         kfree(pathname);
    //         return result;
    //     }
    // }
    // kfree(argv);
    // kfree(pathname);
    // //argv at the front of the stack easily to implement

    panic("md_usermode returned\n");
    return EINVAL;
}

int sys_write(int fd, char *buf, size_t nbytes, int32_t *retval)
{

    //report err that fd is not valid]
    if (fd == 0 || fd > 2)
    {
        return EBADF;
    }
    //report err that address space pointed by bug is invalid
    if (buf == NULL)
    {
        return EFAULT;
    }
    int spl = splhigh();
    char buffer[sizeof(char) * (nbytes + 1)];
    int result = copyin((userptr_t)buf, buffer, nbytes);
    if (result != 0)
    {
        int dumptest = 1;
        splx(spl);
        return EFAULT;
    }

    int i;
    for (i = 0; i < nbytes; i++)
    {
        putch(buffer[i]);
    }
    buffer[nbytes] = '\0';

    *retval = nbytes;
    splx(spl);
    return 0;
}

int sys_read(int fd, char *buf, size_t nbytes, int32_t *retval)
{
    // check for contraints
    // fd = 0 is stdin for linux (guess for os161 as well)
    if (fd != 0)
    {
        *retval = -1;
        return EBADF;
    }
    if (buf == NULL)
    {
        *retval = -1;
        return EFAULT;
    }
    if (nbytes == 0)
    {
        *retval = -1;
        return EIO;
    }

    int i;
    for (i = 0; i < nbytes; i++)
    {
        char buffer = getch();
        int result = copyout(&buffer, (userptr_t)buf + i, sizeof(char));
        if (result != 0)
        {
            int dumptest = 0;
            *retval = -1;
            return result;
        }
    }
    *retval = i;
    return 0;
}

void md_forkentry(void *tf, unsigned long *new_as)
{
    /*
     * This function is provided as a reminder. You need to write
     * both it and the code that calls it.
     *
     * Thus, you can trash it and do things another way if you prefer.
     */
    int spl = splhigh();
    curthread->t_vmspace = (struct addrspace *)new_as;
    struct trapframe tf_st = *(struct trapframe *)tf;
    kfree(tf);
    tf_st.tf_v0 = 0;
    tf_st.tf_a3 = 0;
    tf_st.tf_epc += 4;
#if (DEBUG_CODE & DEBUG_FORK) != 0
//kprintf("Child fork pid: %d\n", curthread->pid);
#endif
    as_activate(new_as);
    splx(spl);
    mips_usermode(&tf_st);
}

int sys_fork(struct trapframe *tf, int32_t *retval)
{

    int result;
    struct addrspace *new_as;
    result = as_copy(curthread->t_vmspace, &new_as);
    if (result)
        return result;

    struct trapframe *tf_copy = kmalloc(sizeof(struct trapframe));
    if (tf_copy == NULL)
    {
        kfree(new_as);
        return ENOMEM;
    }
    memcpy(tf_copy, tf, sizeof(struct trapframe));

    struct thread *child;
    result = thread_fork(curthread->t_name, tf_copy,
                         (unsigned long)new_as, md_forkentry, &child);
    if (result)
    {
        kfree(new_as);
        kfree(tf_copy);
        return result;
    }

    child->parent_pid = curthread->pid;
    *retval = child->pid;

#if DEBUG_CODE & DEBUG_FORK
//kprintf("Parent fork pid:%d ->child pid %d\n", curthread->pid, child->pid);
#endif
    return 0;
}

int sys_getpid(int32_t *retval)
{
#if DEBUG_CODE & DEBUG_EXIT
//kprintf("getpid: %d\n", curthread->pid);
#endif
    assert(curthread->pid)
        *retval = curthread->pid;
    return 0;
}

int sys_waitpid(struct trapframe *tf, int32_t *retval)
{
#if DEBUG_WP & DEBUG_CODE
    kprintf("\nwaitpid: child: %d, parent: %d\n", tf->tf_a0, curthread->pid);
#endif
    int check = tf->tf_a0;
    int result;
    if (check <= 0)
    {
        *retval = -1;
        return EINVAL;
    }
    if (check > get_array_size())
    {
        *retval = -1;
        return EINVAL;
    }
    struct thread *temp = pid_get_thread(tf->tf_a0);

#if DEBUG_WP & DEBUG_CODE
    kprintf("\nwaitpid: temp: %p, p_pid: %d\n", temp, temp->parent_pid);
#endif

    if (tf->tf_a1 != 0x010)
    {
#if DEBUG_WP & DEBUG_CODE
        kprintf("\nwaitpid: entered: %p\n", tf->tf_a1);
#endif
        if (tf->tf_a1 == NULL)
        {
#if DEBUG_WP & DEBUG_CODE
            kprintf("\nwaitpid: entered1: %p\n", tf->tf_a1);
#endif
            *retval = -1;
            return EFAULT;
        }
        if (tf->tf_a1 == 0x40000000 || tf->tf_a1 == 0x80000000)
        {
            *retval = -1;
            return EFAULT;
        }
        int check2 = tf->tf_a1 & 0x00000003;
        if (check2 != 0)
        {
            *retval = -1;
            return -1;
        }
        if (!temp)
        {
            *retval = -1;
            return EINVAL;
        }
        if (tf->tf_a0 == curthread->pid)
        {
            *retval = -1;
            return EINVAL;
        }
        if ((temp != NULL) && (temp->parent_pid != curthread->pid))
        {
            *retval = -1;
            return EINVAL;
        }
        if (tf->tf_a2 != 0)
        {
            *retval = -1;
            return EINVAL;
        }
    }
#if DEBUG_WP & DEBUG_CODE
    kprintf("\nwaitpid: entered2: %p\n", tf->tf_a1);
#endif

    lock_acquire(temp->thread_lock);
    lock_release(temp->thread_lock);

    if (tf->tf_a1 != 0x010)
    {
        *retval = tf->tf_a0;
        int *exit_code = tf->tf_a1;
        *exit_code = temp->exit_code;
        invalid_exit_thread(tf->tf_a0);
    }
    return 0;
}

int sys__exit(struct trapframe *tf)
{
    curthread->exit_code = tf->tf_a0;
#if DEBUG_CODE & DEBUG_EXIT
    kprintf("pid %d exits with code %d\n", curthread->pid,
            curthread->exit_code);
#endif

    thread_exit();
    return 0;
}

time_t sys___time(time_t *secs, unsigned long *nsecs, int* retval)
{   
    if(secs == 0x40000000 || secs == 0x80000000 || nsecs == 0x40000000 || nsecs == 0x80000000)
    {
        *retval = -1; 
        return EFAULT;
    }
    if(secs != NULL && nsecs != NULL)
    {
        gettime(secs,nsecs);
        *retval = *secs;
        return 0;
    }
    else if(secs == NULL && nsecs != NULL)
    {
        time_t temp;
        gettime(&temp, nsecs);
        *retval = temp;
        return 0;
    }
    else if(secs != NULL && nsecs == NULL)
    {
        unsigned long temp;
        gettime(secs, &temp);
        *retval = *secs;
        return 0;
    }
    else
    {
        *retval = -1;
        return EINVAL;
    }

}

int sys_sbrk(intptr_t amount, int* retval)
{
    if (amount < -8191)
    {
        *retval = -1;
        return EINVAL;
    }
    if (amount > 4096*1024*256 -1)
    {
        *retval = -1;
        return ENOMEM;
    }
    if ((amount % 4) != 0)
    {
        *retval = -1;
        return EINVAL;
    }
    *retval = 0;
    return 0;
}


void mips_syscall(struct trapframe *tf)
{
    int callno;
    int32_t retval;
    int err;

    assert(curspl == 0);

    callno = tf->tf_v0;

    /*
     * Initialize retval to 0. Many of the system calls don't
     * really return a value, just 0 for success and -1 on
     * error. Since retval is the value returned on success,
     * initialize it to 0 by default; thus it's not necessary to
     * deal with it except for calls that return other values, 
     * like write.
     */

    retval = 0;

    switch (callno)
    {
    case SYS_reboot:
        err = sys_reboot(tf->tf_a0);
        break;

    case SYS_read: //5
        err = sys_read(tf->tf_a0, (void *)tf->tf_a1, tf->tf_a2, &retval);
        break;

    case SYS_write: //6
        err = sys_write(tf->tf_a0, (void *)tf->tf_a1, tf->tf_a2, &retval);
        break;

    case SYS_execv: //1
        err = sys_execv((const char *)tf->tf_a0, (char **)tf->tf_a1, &retval);
        break;

    case SYS_fork: //2
        err = sys_fork(tf, &retval);
        break;

    case SYS_getpid: //11
        err = sys_getpid(&retval);
        break;

    case SYS_waitpid: //3
        err = sys_waitpid(tf, &retval);
        break;

    case SYS__exit: //0
        err = sys__exit(tf);
        break;
        /* Add stuff here */
    
    case SYS___time:
        err = sys___time((time_t*)tf->tf_a0, (unsigned long*)tf->tf_a1, &retval);
        break;

    case SYS_sbrk:
        err = sys_sbrk(tf->tf_a0, &retval);
        break;

    default:
        kprintf("Unknown syscall %d\n", callno);
        err = ENOSYS;
        break;
    }

    if (err)
    {
        /*
         * Return the error code. This gets converted at
         * userlevel to a return value of -1 and the error
         * code in errno.
         */
        tf->tf_v0 = err;
        tf->tf_a3 = 1; /* signal an error */
    }
    else
    {
        /* Success. */
        tf->tf_v0 = retval;
        tf->tf_a3 = 0; /* signal no error */
    }

    /*
     * Now, advance the program counter, to avoid restarting
     * the syscall over and over again.
     */

    tf->tf_epc += 4;

    /* Make sure the syscall code didn't forget to lower spl */
    assert(curspl == 0);
}
