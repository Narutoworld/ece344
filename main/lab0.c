#include "lab0.h"


int get_Debug_Info(int command){
    switch (command){
            case 1:
                return DB_LOCORE;
            case 2:
                return DB_SYSCALL;
            case 3:
                return DB_INTERRUPT;
            case 4:
                return DB_DEVICE;
            case 5:
                return DB_THREADS;           
            case 6:
                return DB_VM;            
            case 7:
                return DB_EXEC;
            case 8:
                return DB_VFS;
            case 9:
                return DB_SFS;
            case 10:
                return DB_NET;            
            case 11:
                return DB_NETFS;
            case 12:
                return DB_KMALLOC;
            default:
                break;
    }
      
   return 0;                   
}

int char_to_int(const char* command){
    int i = 0;
    int count = 0;
    int result = 0;
    while(command[i] != '\0'){
        i += 1;
    }
    while (i != 0){
        int temp = command[i-1] - '0';
        result += temp * ipow(10,count);
        count += 1;
        i -= 1;
    }
    return result;
}

int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    
    return result;
}
