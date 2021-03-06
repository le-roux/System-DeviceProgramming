
struct stat;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int sem_alloc (void);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int sem_init (int sn, int n);
int sem_destroy (int sn);
int sem_wait (int sn);
int sem_post (int sn);
int cond_alloc();
void cond_set(int cond, int val);
int cond_get(int cond);
void cond_destroy(int cond);
void cond_wait(int cond);
void cond_signal(int cond);
void cond_signal(int cond);
void cond_broadcast(int cond);
void cond_lock(int cond);
void cond_unlock(int cond);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
