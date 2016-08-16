#include <iostream.h>

/**
   All of our writes to the next level of memory go through a
   WriteBuffer.  If we create a writebuffer of size 0, this is a
   special case which models no write buffer.  In this case, all
   writes cost the full memory latency.  Otherwise, items can be added
   to the writebuffer, at no cost, provided that the buffer is not
   full.  If it is full, we must wait until it has completely written
   the current item.  When performing a read, we must be sure to first
   request that the WriteBuffer relinquish the bus (ie. finish writing
   the current item, if any exists).  
*/
class WriteBuffer {
public:
  /**
     Create a new WriteBuffer.
     @param size the maximum number of elements in the WriteBuffer.
     @param cyclesPerItem the number of cycles to write a single item
  */
  WriteBuffer(int size, int cyclesPerItem);

  /**
    Complete the current write.
    @returns the number of cycles we waited
  */
  int relinquishBus();   

  /**
     Add one item to the writebuffer
     @returns the number of cycles we waited (if the buffer was full).
  */
  int addItem(int addr);

  /**
     Give the write buffer some cycles.
     @returns the number of cycles it actually consumed.
  */
  int advanceCycles(int cycles);

private:
  int memLatency;
  int maxCycles;
  int cyclesUntilEmpty;
  int isEmpty() { return cyclesUntilEmpty == 0; }
  int isFull() { return cyclesUntilEmpty > maxCycles - memLatency; }
};
  


/**
   CacheBlock implements the abstraction of a single cache line.  This is
   where hit and miss policies are determined.
*/   
class CacheBlock {
public:
  /** Make a new Cacheblock. */
  CacheBlock();

  /**
     Read this address.
     @returns 1 for a hit, 0 for a miss
     @param addr the address to read.
     @param writeBuffer the writeBuffer we're using.
     @param cycles the number of cycles we stalled.
  */
  int read(int addr, WriteBuffer& writeBuffer, int& cycles);

  /**
     Write to this address.
     @returns 1 for a hit, 0 for a miss
     @param writeBuffer the writeBuffer we're using.
     @param cycles the number of cycles we stalled.
  */
  int write(int addr, WriteBuffer& writeBuffer, int& cycles);

 protected:
  int tag;
  int valid;
};
 

/**
   A cache is just an array of CacheBlocks.  Caches also gather statistics
   about themselves.
*/

class Cache {
public:

  /**
     Create a new Cache.
     @param lines how many lines in the cache
     @param blockSizeInBytes the size of the lines.
     @param wb a WriteBuffer to use. 
  */
  Cache(int lines, int blockSizeInBytes, WriteBuffer& wb);

  /**
     Read this address
     @returns 1 for hit, 0 for miss
  */
  int read(int addr);

  /**
     Write this address
     @returns 1 for hit, 0 for miss
  */
  int write(int addr);

  /** Dump the statistics to the output stream.  */
  void report(ostream& os);

  /** Answer the total number of references. */
  int references() { return readHits+readMisses+writeHits+writeMisses; }

private:
  CacheBlock* blocks;
  WriteBuffer& writeBuffer;
  int numBlocks, blockSize;
  int readHits, readMisses, writeHits, writeMisses;
  int readStallCycles, writeStallCycles;
  void updateStats(int hit, int cycles, int& hits, int& misses, int& cycleCt);
};




