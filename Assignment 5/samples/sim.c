#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "sim.h"

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

unsigned int htoi(const char str[])
{
    /* Local Variables */
    unsigned int result;
    int i;

    i = 0;
    result = 0;
    
    if(str[i] == '0' && str[i+1] == 'x')
    {
        i = i + 2;
    }

    while(str[i] != '\0')
    {
        result = result * 16;
        if(str[i] >= '0' && str[i] <= '9')
        {
            result = result + (str[i] - '0');
        }
        else if(tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f')
        {
            result = result + (tolower(str[i]) - 'a') + 10;
        }
        i++;
    }

    return result;
}

char *getBinary(unsigned int num)
{
    char* bstring;
    int i;
    
    /* Calculate the Binary String */
    
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

void parseMemoryAddress(char *address)
{
    unsigned int dec;
    char *bstring, *bformatted, *tag, *index, *offset;
    int i;
    
    dec = htoi(address);
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring);
    
    if(DEBUG)
    {
        printf("Hex: %s\n", address);
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
    
    printf("Tag: %s (%i)\n", tag, btoi(tag));
    printf("Index: %s (%i)\n", index, btoi(index));
    printf("Offset: %s (%i)\n", offset, btoi(offset));
}

int main(int argc, char **argv)
{
    /* Local Variables */
    int write_policy, counter, i, j;
    Cache cache;
    FILE *file;
    char mode, address[100];
    
    /* Technically a line shouldn't be longer than 25 characters, but
       allocate extra space in the buffer just in case */
    char buffer[LINELENGTH];
    
    /* Help Menu
     *
     * If the help flag is present or there are fewer than
     * three arguments, print the usage menu and return. 
     */
     
    if(argc < 3 || strcmp(argv[1], "-h") == 0)
    {
        fprintf(stderr, 
        "Usage: ./sim [-h] <write policy> <trace file>\n\n<write policy> is one of: \n\twt - simulate a write through cache. \n\twb - simulate a write back cache \n\n<trace file> is the name of a file that contains a memory access trace.\n");
        return 0;
    }
    
    /* Write Policy */
    if(strcmp(argv[1], "wt") == 0)
    {
        write_policy = 0;
        if(DEBUG) printf("Write Policy: Write Through\n");
    }
    else if(strcmp(argv[1], "wb") == 0)
    {
        write_policy = 1;
        if(DEBUG) printf("Write Policy: Write Back\n");
    }
    else
    {
        fprintf(stderr, "Invalid Write Policy.\nUsage: ./sim [-h] <write policy> <trace file>\n");
        return 0;
    }
    
    /* Open the file for reading. */
    file = fopen( argv[2], "r" );
    if( file == NULL )
    {
        fprintf(stderr, "Error: Could not open file.\n");
        return 0; 
    }

    cache = createCache(CACHE_SIZE, BLOCK_SIZE, write_policy);
    
    counter = 0;
    
    while( fgets(buffer, LINELENGTH, file) != NULL )
    {
        if(buffer[0] != '#')
        {
            i = 0;
            while(buffer[i] != ' ')
            {
                i++;
            }
            
            mode = buffer[i+1];
            
            i = i+2;
            j = 0;
            
            while(buffer[i] != '\0')
            {
                address[j] = buffer[i];
                i++;
                j++;
            }
            
            address[j-1] = '\0';
            
            if(DEBUG) printf("%i: %c %s\n", counter, mode, address);
            
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
                printf("%i: ERROR!!!!\n", counter);
                fclose(file);
                destroyCache(cache);
                cache = NULL;
                
                return 0;
            }
            counter++;
        }
    }
    
    if(DEBUG) printf("Num Lines: %i\n", counter);
    
    printf("CACHE HITS: %i\nCACHE MISSES: %i\nMEMORY READS: %i\nMEMORY WRITES: %i\n", cache->hits, cache->misses, cache->reads, cache->writes);
    
    /* Close the file, destroy the cache. */
    
    fclose(file);
    destroyCache(cache);
    cache = NULL;
    
    return 1;
}

Cache createCache(int cache_size, int block_size, int write_policy)
{
    /* Local Variables */
    Cache cache;
    int i;
    
    /* Validate Inputs */
    if(cache_size <= 0)
    {
        fprintf(stderr, "Cache size must be greater than 0 bytes...\n");
        return NULL;
    }
    
    if(block_size <= 0)
    {
        fprintf(stderr, "Block size must be greater than 0 bytes...\n");
        return NULL;
    }
    
    if(write_policy != 0 && write_policy != 1)
    {
        fprintf(stderr, "Write policy must be either \"Write Through\" or \"Write Back\".\n");
        return NULL;
    }
    
    /* Lets make a cache! */
    cache = (Cache) malloc( sizeof( struct Cache_ ) );
    if(cache == NULL)
    {
        fprintf(stderr, "Could not allocate memory for cache.\n");
        return NULL;
    }
    
    cache->hits = 0;
    cache->misses = 0;
    cache->reads = 0;
    cache->writes = 0;
    
    cache->write_policy = write_policy;
    
    cache->cache_size = CACHE_SIZE;
    cache->block_size = BLOCK_SIZE;
    
    /* Calculate numLines */
    cache->numLines = (int)(CACHE_SIZE / BLOCK_SIZE);
    
    cache->blocks = (Block*) malloc( sizeof(Block) * cache->numLines );
    assert(cache->blocks != NULL);
        
    /* By default insert blocks where valid = 0 */
    for(i = 0; i < cache->numLines; i++)
    {
        cache->blocks[i] = (Block) malloc( sizeof( struct Block_ ) );
        assert(cache->blocks[i] != NULL);
        cache->blocks[i]->valid = 0;
        cache->blocks[i]->dirty = 0;
        cache->blocks[i]->tag = NULL;
    }
    
    return cache;
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

int readFromCache(Cache cache, char* address)
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
    
    /* Convert and parse necessary values */
    
    dec = htoi(address);
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring);
    
    if(DEBUG)
    {
        printf("Hex: %s\n", address);
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
    
    /* Convert and parse necessary values */
    
    dec = htoi(address);
    bstring = getBinary(dec);
    bformatted = formatBinary(bstring);
    
    if(DEBUG)
    {
        printf("Hex: %s\n", address);
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

void printCache(Cache cache)
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
        printf("Cache:\n\tCACHE HITS: %i\n\tCACHE MISSES: %i\n\tMEMORY READS: %i\n\tMEMORY WRITES: %i\n\n\tCACHE SIZE: %i Bytes\n\tBLOCK SIZE: %i Bytes\n\tNUM LINES: %i\n", cache->hits, cache->misses, cache->reads, cache->writes, cache->cache_size, cache->block_size, cache->numLines);
    }
}