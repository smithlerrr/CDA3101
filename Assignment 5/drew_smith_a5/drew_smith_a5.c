//Drew Smith
//CDA3101
//December 3rd, 2015
//Modular Data Cache Simulator
//////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct{ 
  char access;
  int size;
  int address;
} MemRef;

typedef struct{
  int index;
  int offset;
  int result;
  int memref;
} Trans;

typedef struct{
  int valid;
  int tag;
  int dirty;
  int age;
} Block;

int numSets, setSize, lineSize;                   
Block **cache;                                  

void OpenRequest(FILE **);                      
void ReadRequest(FILE **, int *, int *, int *); 
void Initialize(); 
int  CacheRead(Trans *, Block *);               
void CacheWrite(Trans *, Block *);                                                               
void Calculate(MemRef *, Trans *, Block *);     
void PrintData(int, MemRef *, Trans *, Block *);
void Layout();              
void PrintCache(int, int);                      

int main() 
{
  FILE *file;
  OpenRequest(&file);
  ReadRequest(&file, &numSets, &setSize, &lineSize);
  Initialize();
  
  int i = 1;
  int hits = 0;  
  int misses = 0;
  char line[10]; 

  MemRef ref;
  Trans t;
  Block b;
  
  Layout();
  while(fgets(line, sizeof(line), stdin)) 
  {
    sscanf(line, "%c:%d:%x", &ref.access, &ref.size, &ref.address);
    switch(ref.size) 
    {
      case 1: 
      case 2: 
      case 4: 
      case 8:
        if (ref.address % ref.size != 0) 
        {
          fprintf(stderr, "Error: Invalid size on line %d\n", i);
            break;
        }
      
        Calculate(&ref, &t, &b);
        
        t.result = CacheRead(&t, &b);
        if (t.result == 0) 
        {
          CacheWrite(&t, &b);
          misses++;
        } 
        else 
          hits++;
        
        PrintData(i, &ref, &t, &b);
        i++;
        break;

      default:
        fprintf(stderr, "Error: Invalid size on line %d\n", i);
    }   
  }

  PrintCache(hits, misses);
  fclose(file);
  for (i = 0; i < numSets; ++i)
    free(cache[i]);
  
  free(cache);
  return 0;
}

void OpenRequest(FILE **file) 
{
  *file = fopen("request", "r");
  if (!file) 
  {
    printf("Error: No input file.\n");
    exit(1);
  }
}

void ReadRequest(FILE **file, int *numSets, int *setSize, int *lineSize) 
{
  char line[20];
  
  fgets(line, sizeof(line), *file);
  sscanf(line, "%*[^:]: %d", numSets);
  
  fgets(line, sizeof(line), *file);
  sscanf(line, "%*[^:]: %d", setSize);
  
  fgets(line, sizeof(line), *file);
  sscanf(line, "%*[^:]: %d", lineSize);
}

void Initialize() 
{
    int i, j;
    cache = (Block**) malloc(numSets * sizeof(Block*));
    for (i = 0; i < numSets; ++i)
      cache[i] = (Block*) malloc(setSize * sizeof(Block));
    
    for (i = 0; i < numSets; ++i)
    {
      for (j = 0; j < setSize; ++j)
        cache[i][j].valid = 0; 
        cache[i][j].tag = 0; 
        cache[i][j].dirty = 0;
        cache[i][j].age = 0;
    }
}


int CacheRead(Trans *t, Block *b) 
{
  int i;
  for (i = 0; i < setSize; ++i)
    if (cache[(*t).index][i].valid)
      if (cache[(*t).index][i].tag == (*b).tag) 
      {
        (*t).memref = 0;
        return 1;
      }
  return 0;
}

void CacheWrite(Trans *t, Block *b) 
{
  int i; 
  int LRU = 0;
  (*t).memref = 1;
  Block *loc = cache[(*t).index];
  
  for (i = 0; i < setSize; ++i)
    ++loc[i].age;
  
  for (i = 0; i < setSize; ++i)
    if (!loc[i].valid) 
    {
      loc[i].valid = 1;
      loc[i].tag = (*b).tag;
      loc[i].dirty = (*b).dirty;
      loc[i].age = 0; 
      return;
    }
  
  for (i = 0; i < setSize; ++i)
    if (loc[i].age > loc[LRU].age)
      LRU = i;
  
  if (loc[LRU].dirty)
    ++(*t).memref;
  
  loc[LRU].tag = (*b).tag;
  loc[LRU].dirty = (*b).dirty;
  loc[LRU].age = 0;
}

void Calculate(MemRef *m, Trans *t, Block *b) 
{
  int bits = (int)((log(lineSize) / log(2)));
  (*b).valid = 1;
  (*b).tag = (*m).address >> (bits + (int)(log(numSets) / log(2)));
  (*t).index = ((*m).address >> bits) % numSets;
  (*t).offset = (*m).address % lineSize;
  (*b).dirty = ((*m).access == 'R') ? 0 : 1;
}

void Layout() 
{
  printf("Cache Configuration\n\n");
  printf("   %d %d-way set associative entries\n", numSets, setSize);
  printf("   of line size 8 bytes\n\n\n");
  printf("Results for Each Reference\n\n");
  printf("Ref  Access Address    Tag   Index Offset Result Memrefs\n");
  printf("---- ------ -------- ------- ----- ------ ------ -------\n");
}

void PrintData(int i, MemRef *m, Trans *t, Block *b) 
{
  printf("%4d %6s %8x %7d %5d %6d %6s %7d\n", 
    i, 
    ((*m).access == 'W') ? "Write" : "Read", 
    (*m).address, 
    (*b).tag, 
    (*t).index, 
    (*t).offset, 
    ((*t).result == 0) ? "Miss" : "Hit",
    (*t).memref
    );
}

void PrintCache(int hits, int misses) 
{
  int accesses = hits + misses;
  printf("\n\nCache Statistics:\n");
  printf("Total Hits: %d\n", hits);
  printf("Total Misses: %d\n", misses);
  printf("Total Accesses: %d\n", accesses);
  printf("Hit Ratio: %f\n", (float)hits / accesses);
  printf("Miss ratio: %f\n\n", (float)misses / accesses);
}