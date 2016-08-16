#include <stdio.h>
#include "cachesim.h"

const int LINESIZE = 4;     // line size in bytes, must be power of 2, min=4 !!
const int SIZE     = 8192;  // total capacity in bytes, must be power of 2
const int WB_DEPTH = 1;     // depth of Wr buffer, 0 means none

// You shouldn't need to change any of the below constants

const int LINES    = SIZE/LINESIZE; // number of lines in the cache
const int WPLINE   = LINESIZE/4;    // words per line

// ------------------------------------------------------------------------
// Memory accesses take place as follows:
//   1 cycle to send address
//   10 cycles RAM latency to get first word
//   1 cycle to send each word
//   words in successive memory locations can be supplied at shorter
//   latency of 3 cycles
// Reading 1 word  = 1 + 10 + 1
// Reading n successive words = as above + (3+1)*(n-1)
// We assume writes don't generally occur to successive locations,
// so we must pay the full penalty for each write...
// ------------------------------------------------------------------------

const int SEND_LINES         = 1;
const int DRAM_LATENCY       = 10;
const int DRAM_LATENCY_NEXT  = 3;
const int SEND_WORD          = 1;
const int RM_PENALTY         = SEND_LINES + DRAM_LATENCY + SEND_WORD + 
                               ((WPLINE-1) * (DRAM_LATENCY_NEXT+SEND_WORD));
const int WM_PENALTY         = SEND_LINES + DRAM_LATENCY + SEND_WORD;

// ------------------------------------------------------------------------
// Below is the code for implementing write buffers.  You shouldn't
// need to study it unless you're doing extra credit. 
// ------------------------------------------------------------------------

int 
WriteBuffer::relinquishBus() 
{
  if (maxCycles == 0)  // no write buffer
    return 0;
  else {
    int temp = cyclesUntilEmpty % memLatency;
    cyclesUntilEmpty -= temp;
    return temp;
  }
}

int 
WriteBuffer::addItem(int addr)  // addr is currently not used 
{
  if (maxCycles == 0) // no write buffer
    return memLatency;  // always pay the full penalty
  else {
    int cycles = 0;
    if (isFull())
      cycles = relinquishBus();
    cyclesUntilEmpty += memLatency;
    return cycles;
  }
}

int 
WriteBuffer::advanceCycles(int cycles) 
{ 
  if (maxCycles > 0) {
    int temp = cyclesUntilEmpty;
    if (cyclesUntilEmpty < cycles)
      cyclesUntilEmpty = 0;
    else {
      cyclesUntilEmpty = cyclesUntilEmpty - cycles;
      temp = cycles;
    }
    return temp;  // how many cycles did we actually consume?
  }
  return 0;
}

WriteBuffer::WriteBuffer(int size, int cyclesPerItem) 
{
  printf ("wb size = %d\n", size);
  memLatency = cyclesPerItem;
  cyclesUntilEmpty = 0;
  maxCycles = size * memLatency;
}
  
// ------------------------------------------------------------------------
// Below are two important global functions which, given an address,
// return the cache index or tag.
// You need to fill in the details for these two functions:
// ------------------------------------------------------------------------

int INDEX(int addr) { return 0; }
int TAG(int addr) { return 0; }

// ------------------------------------------------------------------------
// Below is the implementation of CacheBlock's methods.
// The interesting ones are read and write.
// ------------------------------------------------------------------------

CacheBlock::CacheBlock() 
{
  valid = 0;
}

// ------------------------------------------------------------------------
// For a WriteThrough cache:
//  Readhits:  we can provide the data directly, so no stall occurs,
//     and we give the writebuffer one cycle.
//  Readmisses: we need to fetch the data from memory.  This means first
//     contending for the bus with the writebuffer, and then actually 
//     fetching the data from memory, setting the line to valid, and updating
//     the tag.
// Currently implements no write-buffer.  You need to add the right lines
// of code to use the write-buffer.  Later, you'll modify this method to
// implement a write-back policy.
// ------------------------------------------------------------------------

int 
CacheBlock::read(int addr, WriteBuffer& writeBuffer, int& cycles) 
{
  if (valid && tag == TAG(addr)) {  // read hit
    cycles = 0;  // don't stall on a read hit
    return 1;
  }
  else {         // read miss
    cycles = RM_PENALTY;   // now fetch item from memory
    valid = 1;
    tag = TAG(addr);
    return 0;
  }
}

// ------------------------------------------------------------------------
// For a WriteThrough cache:
//  Writehits:  we always update the next level of our hierarchy, so we
//     need to stick the word we're writing into the writebuffer
//  WriteMisses: since we're modeling a the writearound policy, we don't
//     make any changes to the cache, but just write to the next level
//     of our hierarchy, which again means giving the word to the writebuffer.
// Currently implements no write-buffer.  You need to add the right lines
// of code to use the write-buffer.  Later, you'll modify this method to
// implement a write-back policy.
// ------------------------------------------------------------------------

int 
CacheBlock::write(int addr, WriteBuffer& writeBuffer, int& cycles) 
{
  // With WT, on a hit or miss, we always write around to memory...
  cycles = WM_PENALTY;
  return (valid && tag == TAG(addr));  // was it a hit?
}

// ------------------------------------------------------------------------
// Below is the implementation of Cache's methods.
// Caches don't do much besides pass the buck along to the appropriate block
// ------------------------------------------------------------------------
  
Cache::Cache(int lines, int blockSizeInBytes, WriteBuffer& wb) :
  writeBuffer(wb)
{
    blocks = new CacheBlock[lines];
    numBlocks = lines;    
    blockSize = blockSizeInBytes;
    readHits = readMisses = readStallCycles = 0;
    writeHits = writeMisses = writeStallCycles = 0;
}


// ------------------------------------------------------------------------
// read:  find the block and pass the buck.  Update the statistics.
// ------------------------------------------------------------------------

int 
Cache::read(int addr) 
{
  int cycles = 0;
  int hit = 0;
  hit = blocks[INDEX(addr)].read(addr, writeBuffer, cycles);
  updateStats(hit, cycles, readHits, readMisses, readStallCycles);
  return hit;
}

// ------------------------------------------------------------------------
// write:  find the block and pass the buck.  Update the statistics.
// ------------------------------------------------------------------------

int 
Cache::write(int addr) 
{
  int cycles = 0;
  int hit = 0;
  hit = blocks[INDEX(addr)].write(addr, writeBuffer, cycles);
  updateStats(hit, cycles, writeHits, writeMisses, writeStallCycles);
  return hit;
}

void 
Cache::updateStats(int hit, int cycles, int& hits, int& misses, int& cycleCt) 
{
  if (hit)
    hits++;
  else
    misses++;
  cycleCt = cycleCt + cycles;
}  

void 
Cache::report(ostream& os) {
  int totalHits = readHits + writeHits;
  int totalMisses = readMisses + writeMisses;
  int totalRefs = totalHits + totalMisses;
  int cycles = readStallCycles + writeStallCycles;
  os << "# Lines = " << numBlocks << " linesize = " << blockSize 
     << " bytes"<< endl;
  os << "Reads:  hits = " << readHits << ", misses = " << readMisses << endl;
  os << "Writes: hits = " << writeHits << ", misses = "<< writeMisses<< endl;
  os << "Miss rate = "<< ((float)totalMisses/totalRefs * 100.0)<< "%" << endl; 
  os << "Stall Cycles: reads = " << readStallCycles <<
    ", writes = " << writeStallCycles << ", total = " << cycles << endl;
  os << "Total accesses = " << totalRefs << endl;
}

// ------------------------------------------------------------------------
// Main.
// The format of the data file is <type> <addr> ... <type> <addr> <eof>
// <type> is a decimal, where 0 means instruction fetch, 1 means data load
// and 2 means data store.  <addr> is a hex address that is being accessed.
// Main just reads <type> <addr> pairs from standard input, dispatching
// the accesses to the cache.
// When an <eof> is encountered, the loop is exited and the cache stats are
// reported.
// ------------------------------------------------------------------------

void main () 
{
  WriteBuffer wb(WB_DEPTH, WM_PENALTY);
  Cache cache(LINES, LINESIZE, wb);
  int i, iloads, type, addr, hit;
  iloads = 0;

  while (cin >> type >> hex >> addr) {
    if (type == 0 || type == 1) {  // a load of some sort
      if (type == 0) {
	iloads++;
      }
      hit = cache.read(addr);
    }
    else {                         // store 
      hit = cache.write(addr);
    }
  }

  // print stats

  cout << "\n** Cache Statistics **\n\n";
  cache.report(cout);
  cout << "# Instructions = " << iloads << " references/ins = " <<
    ((float)cache.references()) / iloads << endl;
}