#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 0
#define LINELENGTH 10
#define CACHE_SIZE 8192
#define BLOCK_SIZE 8
#define TAG 18 
#define INDEX 12 
#define OFFSET 2 

typedef struct Cache_ *Cache;
typedef struct Block_ *Block;

struct Block_ {
    int valid;
    char* tag;
    int dirty;
};

struct Cache_ {
    int hits;
    int misses;
    int reads;
    int writes;
    int cache_size;
    int block_size;
    int numLines;
    int write_policy;
    Block* blocks;    
};

void createCache();
int readFromCache(Cache, char*);
int writeToCache(Cache, char*);
char *getBinary(unsigned int);
char *formatBinary(char*);
int btoi(char *bin);
void destroyCache(Cache);
void printCache(Cache);

int main(int argc, char **argv)
{
    int write_policy, lines, size_in_bytes, i, j;
    Cache cache;
    FILE *file;
    char mode, address[100];
    char buffer[LINELENGTH];
    
    file = fopen("test.txt", "r");
    if(!file)
    {
        fprintf(stderr, "Error: Could not open file.\n");
        return 0; 
    }

    createCache();

    lines = 0;
    
    while(fgets(buffer, LINELENGTH, file) != NULL)
    {
      i = 0;
      mode = buffer[i];
      size_in_bytes = buffer[i+2];
      
      i = 4;
      j = 0;
      while(buffer[i] != '\0')
      {
          address[j] = buffer[i];   //POSSIBLE SEG FAULT
          i++;
          j++;
      }
      
      if(DEBUG) 
        printf("%i: %c %s\n", lines, mode, address);
      
      if(mode == 'R')
      {
          readFromCache(cache, address);
      }
      
      else if(mode == 'W')
      {
          writeToCache(cache, address);
      }
      
      else
      {
          printf("%i: ERROR!!!!\n", lines);
          fclose(file);
          destroyCache(cache);
          cache = NULL;
          
          return 0;
      }
      lines++;
      
    }

    cache->numLines = lines;
    
    if(DEBUG) 
    {
      printf("Number of Lines: %i\n", cache->numLines);   
      printf("CACHE HITS: %i\n", cache->hits);
      printf("CACHE MISSES: %i\n", cache->misses);
      printf("MEMORY READS: %i\n", cache->reads);
      printf("MEMORY WRITES: %i\n", cache->writes);
    }
    
    fclose(file);
    destroyCache(cache);
    cache = NULL;
    return 1;
}

void createCache()
{
    Cache cache;
    int i;
    int write_policy;

    cache->hits = 0;
    cache->misses = 0;
    cache->reads = 0;
    cache->writes = 0;
    cache->write_policy = 0;
    cache->cache_size = CACHE_SIZE;
    cache->block_size = BLOCK_SIZE;
    cache->blocks[i]->valid = 0;
    cache->blocks[i]->dirty = 0;
    cache->blocks[i]->tag = NULL;

    printf("mommas tits");
}

int readFromCache(Cache cache, char* address)
{
    unsigned int dec;
    char *bstring, *bformatted, *tag, *index, *offset;
    int i;
    Block block;
  
    if(cache == NULL)
    {
        fprintf(stderr, "Error: Must supply a valid cache to write to.\n");
        return 0;
    }
    
    if(address == NULL)
    {
        fprintf(stderr, "Error: Must supply a valid memory address.\n");
        return 0;
    }
    
    dec = (int)address;
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring);
    
    if(DEBUG)
    {
        printf("Decimal: %u\n", dec);
        printf("Binary: %s\n", bstring);
        printf("Formatted: %s\n", bformatted);
    }
        
    i = 0;
    
    tag = (char *) malloc( sizeof(char) * (TAG + 1) );
    assert(tag != NULL);
    tag[TAG] = '\0';
    
    for(i = 0; i < TAG; i++)
    {
        tag[i] = bformatted[i];
    }
    
    index = (char *) malloc( sizeof(char) * (INDEX + 1) );
    assert(index != NULL);
    index[INDEX] = '\0';
    
    for(i = TAG + 1; i < INDEX + TAG + 1; i++)
    {
        index[i - TAG - 1] = bformatted[i];
    }
    
    offset = (char *) malloc( sizeof(char) * (OFFSET + 1) );
    assert(offset != NULL);
    offset[OFFSET] = '\0';
    
    for(i = INDEX + TAG + 2; i < OFFSET + INDEX + TAG + 2; i++)
    {
        offset[i - INDEX - TAG - 2] = bformatted[i];
    }
    
    if(DEBUG)
    {
        printf("Tag: %s (%i)\n", tag, btoi(tag));
        printf("Index: %s (%i)\n", index, btoi(index));
        printf("Offset: %s (%i)\n", offset, btoi(offset));
    }
    
    block = cache->blocks[btoi(index)];
    
    if(DEBUG)
    {
        printf("Attempting to read data from cache slot %i.\n", btoi(index));
    }
    
    if(block->valid == 1 && strcmp(block->tag, tag) == 0)
    {
        cache->hits++;
        free(tag);
    }
    else
    {        
        cache->misses++;
        cache->reads++;
        
        if(cache->write_policy == 1 && block->dirty == 1)
        {
            cache->writes++;
            block->dirty = 0;
        }
        
        block->valid = 1;
        
        if(block->tag != NULL)
        {
            free(block->tag);
        }
        
        block->tag = tag;
    }
    
    
    free(bstring);
    free(bformatted);
    free(offset);
    free(index);
    return 1;
}

int writeToCache(Cache cache, char* address)
{
    unsigned int dec;
    char *bstring, *bformatted, *tag, *index, *offset;
    int i;
    Block block;
    
    
    /* Validate inputs */
    if(cache == NULL)
    {
        fprintf(stderr, "Error: Must supply a valid cache to write to.\n");
        return 0;
    }
    
    if(address == NULL)
    {
        fprintf(stderr, "Error: Must supply a valid memory address.\n");
        return 0;
    }
    
    dec = (int)address;
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring);
    
    if(DEBUG)
    {
      printf("Decimal: %u\n", dec);
      printf("Binary: %s\n", bstring);
      printf("Formatted: %s\n", bformatted);
    }
        
    i = 0;
    
    tag = (char *) malloc( sizeof(char) * (TAG + 1) );
    assert(tag != NULL);
    tag[TAG] = '\0';
    
    for(i = 0; i < TAG; i++)
    {
        tag[i] = bformatted[i];
    }
    
    index = (char *) malloc( sizeof(char) * (INDEX + 1) );
    assert(index != NULL);
    index[INDEX] = '\0';
    
    for(i = TAG + 1; i < INDEX + TAG + 1; i++)
    {
        index[i - TAG - 1] = bformatted[i];
    }
    
    offset = (char *) malloc( sizeof(char) * (OFFSET + 1) );
    assert(offset != NULL);
    offset[OFFSET] = '\0';
    
    for(i = INDEX + TAG + 2; i < OFFSET + INDEX + TAG + 2; i++)
    {
        offset[i - INDEX - TAG - 2] = bformatted[i];
    }
    
    if(DEBUG)
    {
        printf("Tag: %s (%i)\n", tag, btoi(tag));
        printf("Index: %s (%i)\n", index, btoi(index));
        printf("Offset: %s (%i)\n", offset, btoi(offset));
    }
    
    /* Get the block */
    
    block = cache->blocks[btoi(index)];
    
    if(DEBUG)
    {
        printf("Attempting to write data to cache slot %i.\n", btoi(index));
    }
    
    if(block->valid == 1 && strcmp(block->tag, tag) == 0)
    {
        if(cache->write_policy == 0)
        {
            cache->writes++;
        }
        block->dirty = 1;
        cache->hits++;
        free(tag);
    }
    else
    {
        cache->misses++;
        cache->reads++;
        
        if(cache->write_policy == 0)
        {
            cache->writes++;
        }
        
        if(cache->write_policy == 1 && block->dirty == 1)
        {
            cache->writes++;
        }        
        
        block->dirty = 1;
        
        block->valid = 1;
        
        if(block->tag != NULL)
        {
            free(block->tag);
        }
        
        block->tag = tag;
    }
    
    free(bstring);
    free(bformatted);
    free(offset);
    free(index);
    return 1;
}

void destroyCache(Cache cache)
{
    int i;
    
    if(cache != NULL)
    {
        for( i = 0; i < cache->numLines; i++ )
        {
            if(cache->blocks[i]->tag != NULL)
            {
                free(cache->blocks[i]->tag);
            }
            
            free(cache->blocks[i]);
        }
        free(cache->blocks);
        free(cache);
    }
    return;
}
 
char *getBinary(unsigned int num)
{
    char* bstring;
    int i;
    
    bstring = (char*) malloc(sizeof(char) * 33);
    assert(bstring != NULL);
    
    bstring[32] = '\0';
    
    for( i = 0; i < 32; i++ )
    {
        bstring[32 - 1 - i] = (num == ((1 << i) | num)) ? '1' : '0';
    }
    
    return bstring;
}

char *formatBinary(char *bstring)
{
    char *formatted;
    int i;
    
    /* Format for Output */
    
    formatted = (char *) malloc(sizeof(char) * 35);
    assert(formatted != NULL);
    
    formatted[34] = '\0';
    
    for(i = 0; i < TAG; i++)
    {
        formatted[i] = bstring[i];
    }
    
    formatted[TAG] = ' ';
    
    for(i = TAG + 1; i < INDEX + TAG + 1; i++)
    {
        formatted[i] = bstring[i - 1];
    }
    
    formatted[INDEX + TAG + 1] = ' ';
    
    for(i = INDEX + TAG + 2; i < OFFSET + INDEX + TAG + 2; i++)
    {
        formatted[i] = bstring[i - 2];
    }

    return formatted;
}

int btoi(char *bin)
{
    int  b, k, m, n;
    int  len, sum;

    sum = 0;
    len = strlen(bin) - 1;

    for(k = 0; k <= len; k++)
    {
        n = (bin[k] - '0'); 
        if ((n > 1) || (n < 0))
        {
            return 0;
        }
        for(b = 1, m = len; m > k; m--)
        {
            b *= 2;
        }
        sum = sum + n * b;
    }
    return(sum);
}

/*void printCache(Cache cache)
{
    int i;
    char* tag;
    
    if(cache != NULL)
    {        
        for(i = 0; i < cache->numLines; i++)
        {
            tag = "NULL";
            if(cache->blocks[i]->tag != NULL)
            {
                tag = cache->blocks[i]->tag;
            }
            
            printf("[%i]: { valid: %i, tag: %s }\n", i, cache->blocks[i]->valid, tag);
        }
      print("Cache:\n");
      printf("CACHE TAG: %i\n,", cache->tag);
      printf("CACHE BlOCK INDEX: %i\n,", cache->blocks);
      printf("CACHE HITS: %i\n,", cache->hits);
      printf("CACHE HIT RATIO: %i\n,", (float)cache->hits / accesses);  
      printf("CACHE MISSES: %i\n", cache->misses);
      printf("CACHE MISS RATIO: %i\n,", (float)cache->misses / accesses); 
      printf("MEMORY READS: %i\n", cache->reads);
      printf("MEMORY WRITES: %i\n", cache->writes);
      printf("CACHE SIZE: %i\n", cache->cache_size); 
      printf("BLOCK SIZE: %i\n", cache->block_size); 
      printf("NUM LINES: %i\n", cache->numLines);      
    }
}*/