//
// File descriptors

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
//
#include "file.h"
#include "spinlock.h"

struct semaphore {
  struct spinlock lock;
  int count;
  int ref; //assigned or not
  uint q;
};

struct {
  struct spinlock lock;
  struct semaphore semaphore[NSEM_MAX];
} semaphore_table;

struct mutex {
	struct spinlock lock;
	int locked; //Locked or not
	int ref; //Assigned or not
	uint q;
};

struct {
	struct spinlock lock;
	struct mutex mutex[NCOND_MAX];
} mutex_table;

struct condition {
	struct spinlock lock;
	int value;
	int ref; //Assigned or not
	int alarm_clock;
};

struct {
	struct spinlock lock;
	struct condition condition[NCOND_MAX];
} condition_table;

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}
void
semaphore_init(void)
{
  initlock(&semaphore_table.lock, "semaphore_table");
}

// Allocate a semaphore structure.
int
sem_alloc(void)
{
  struct semaphore *s;
  int k = 0;

  acquire(&semaphore_table.lock);
  for(s = semaphore_table.semaphore; s < semaphore_table.semaphore + NSEM_MAX; s++){
    if(s->ref == 0){
      s->ref = 1;
      release(&semaphore_table.lock);
      return k;
    }
		k++;
  }
  release(&semaphore_table.lock);
  return -1;
}

int
sem_init(int sn, int n)
{
  struct semaphore *s;
  
  s = &semaphore_table.semaphore[sn];
  acquire(&s->lock);
  s->count = n;
  release(&s->lock);
  return 0;
}

int
sem_destroy(int sn)
{
	 struct semaphore *s;
  
  s = &semaphore_table.semaphore[sn];
  s->ref = 0;
  return 0;
}

int
sem_wait(int sn)
{  
  struct semaphore *s;
  
  s = &semaphore_table.semaphore[sn];
  acquire(&s->lock);
  s->count--;
  if (s->count < 0)
    sleep(&s->q, &s->lock); 
  release(&s->lock);
  return 0;
}

int
sem_post(int sn)
{
  struct semaphore *s;
    
 s = &semaphore_table.semaphore[sn];
      
   acquire(&s->lock);
   s->count++;
   if (s->count <= 0)
     wakeup(&s->q);
   release(&s->lock); 
   return 0;
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);
  
  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_trans();
    iput(ff.ip);
    commit_trans();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((LOGSIZE-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_trans();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      commit_trans();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

//Initialise the mutex table
void mutex_init() {
	initlock(&mutex_table.lock, "mutex table");
}

void cond_lock(int cond) {
	struct mutex* mutex;
	mutex = &mutex_table.mutex[cond];
	acquire(&mutex->lock);
	while (mutex->locked == 0) {
		sleep(&mutex->q, &mutex->lock);
	}
	mutex->locked = 0;
	release(&mutex->lock);
}

void cond_unlock(int cond) {
	struct mutex* mutex;
	mutex = &mutex_table.mutex[cond];
	acquire(&mutex->lock);
	mutex->locked = 1;
	release(&mutex->lock);
}

//Initialise the condition table
void condition_init(void) {
	initlock(&condition_table.lock, "condition table");
}

//Alloc a condition structure
int cond_alloc(void) {
	struct condition* cond;
	struct mutex* mutex;
	int counter;
	acquire(&condition_table.lock);
	acquire(&mutex_table.lock);
	for (counter = 0; counter < NCOND_MAX; counter++) {
		cond = &condition_table.condition[counter];	
		mutex = &mutex_table.mutex[counter];
		if (cond->ref == 0) {
			cond->ref = 1;
			mutex->locked = 1;
			release(&mutex_table.lock);
			release(&condition_table.lock);
			return counter;
		}
		counter++;
	}
	release(&mutex_table.lock);
	release(&condition_table.lock);
	return -1;
}

//Set the initial value of a condition
void cond_set(int cond, int val) {
	struct condition* condition = &condition_table.condition[cond];
	acquire(&condition->lock);	
	condition->value = val;
	release(&condition->lock);
	return;
}

//Get the value of a condition
int cond_get(int cond, int val) {
	struct condition* condition = &condition_table.condition[cond];
	return condition->value;
}

//Release a condition structure
void cond_destroy(int cond) {
	acquire(&condition_table.lock);
	condition_table.condition[cond].ref = 0;
	release (&condition_table.lock);
	return;
}

//Wait on the condition
void cond_wait(int cond) {
	struct condition* condition = &condition_table.condition[cond];
	acquire(&condition->lock);
	cond_unlock(cond);
	sleep(&condition->alarm_clock, &condition->lock);
	cond_lock(cond);
	release(&condition->lock);
}

//Awake one waiting thread
void cond_signal(int cond) {
	struct condition* condition = &condition_table.condition[cond];
	wakeup(&condition->alarm_clock);
}

//Awake all the waiting threads
void cond_broadcast(int cond) {
	struct condition* condition = &condition_table.condition[cond];
	wakeup(&condition->alarm_clock);
}













