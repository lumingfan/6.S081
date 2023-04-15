// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKET_NUM 13


struct buf buf[NBUF];
struct {
  struct spinlock lock;

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache[BUCKET_NUM];

void
binit(void)
{
  struct buf *b;

  for (int i = 0; i < BUCKET_NUM; ++i) {
      initlock(&bcache[i].lock, "bcache");
      bcache[i].head.prev = &bcache[i].head;
      bcache[i].head.next = &bcache[i].head;
  }

  // Create linked list of buffers
  for(b = buf; b < buf+NBUF; b++){
    b->next = bcache[0].head.next;
    b->prev = &bcache[0].head;
    b->timestamps = ticks;
    initsleeplock(&b->lock, "buffer");
    bcache[0].head.next->prev = b;
    bcache[0].head.next = b;
  }
}


static int hash(uint dev, uint blockno) {
    return (dev * 31 + blockno) % BUCKET_NUM;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer

static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  
  int id = hash(dev, blockno);

  acquire(&bcache[id].lock);

  // Is the block already cached?
  for(b = bcache[id].head.next; b != &bcache[id].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[id].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (int i = 0; i < BUCKET_NUM; ++i) {
      if (i != id)
          acquire(&bcache[i].lock);
      for(b = bcache[i].head.prev; b != &bcache[i].head; b = b->prev){
        if(b->refcnt == 0) {
         b->next->prev = b->prev;
         b->prev->next = b->next;
         b->next = bcache[id].head.next;
         b->prev = &bcache[id].head;
         bcache[id].head.next->prev = b;
         bcache[id].head.next = b;     

          b->dev = dev;
          b->blockno = blockno;
          b->valid = 0;
          b->refcnt = 1;
          release(&bcache[id].lock);
          if (i != id)
              release(&bcache[i].lock);
          acquiresleep(&b->lock);
          return b;
        }
      }
      if (i != id)
          release(&bcache[i].lock);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");


  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
      b->timestamps = ticks;
  }

  releasesleep(&b->lock);

  
}

void
bpin(struct buf *b) {
  int id = hash(b->dev, b->blockno);
  acquire(&bcache[id].lock);
  b->refcnt++;
  release(&bcache[id].lock);
}

void
bunpin(struct buf *b) {
  int id = hash(b->dev, b->blockno);
  acquire(&bcache[id].lock);
  b->refcnt--;
  release(&bcache[id].lock);
}


