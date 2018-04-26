#define DB_LOCORE      0x001
#define DB_SYSCALL     0x002
#define DB_INTERRUPT   0x004
#define DB_DEVICE      0x008
#define DB_THREADS     0x010
#define DB_VM          0x020
#define DB_EXEC        0x040
#define DB_VFS         0x080
#define DB_SFS         0x100
#define DB_NET         0x200
#define DB_NETFS       0x400
#define DB_KMALLOC     0x800

int get_Debug_Info(int command);
int char_to_int(const char* command);
int ipow(int base, int exp);
