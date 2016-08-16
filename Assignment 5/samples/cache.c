/*************************************************************************
 *                                                                       *
 *            Name: Adam Stallard                                        *
 *           Class: CDA3101                                              *
 *      Assignment: Implementing a Data Cache Simulator                  *
 *         Compile: "gcc -o datacache -lm datacache.c"                   *
 *                                                                       *
 *        This program accepts an input file and manages a cache using   *
 *    a configuration specified from a local config file. It does this   *
 *    by translating memory references to cache locations and updating   *
 *    a cache using an LRU replacement policy. The policy is naively     *
 *    updated using a counter. Unfortunately this is O(n) and could be   *
 *    reduced by implementing a pair of hash maps. However that would    *
 *    have been a great deal more effort. :)                             *
 *                                                                       *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


/* An incoming reference to memory */
typedef struct { 
	char access_t;
	int	size,
		hex_addr;
} memReference;

/* Translate reference into cache information */
typedef struct {
	int index,
		offset,
		result,
		memrefs;
} trans;

/* Data inside each cache block */
typedef struct {
	int	valid,
		tag,
		dirty,
		age;
} block;

int	num_sets, set_sz, lne_sz;                       /* config file info      */
block **cache;                                      /* dynm pointer to cache */

void get_config(FILE **);                           /* open config file      */
void scan_config(FILE **, int *, int *, int *);     /* scan config file      */
void init_cache();                                  /* alloc cache w/config  */
void header();                                      /* intro / table header  */
void translate(mem_ref *, trans *, block *);        /* calculate row data    */
void table_entry(int, mem_ref *, trans *, block *); /* print row data        */
int  found_in_cache(trans *, block *);              /* cache lookup          */
void add_to_cache(trans *, block *);                /* cache write           */
void statistics(int, int);                          /* end message / results */

int main() {

	/* configuration management */
	FILE *trace_config;
	get_config(&trace_config);
	scan_config(&trace_config, &num_sets, &set_sz, &lne_sz);
	init_cache();
	
	/* variable declarations */
	int i = 1,      /* reference counter  */
		hits = 0,   /* hits counter       */
		misses = 0; /* misses counter     */
	char line[10];	/* line from stdin    */

	/* instrances of typedefs */
	mem_ref ref;
	trans t;
	block b;
	
	header();
	while(fgets(line, sizeof(line), stdin)) {
		sscanf(line, "%c:%d:%x", &ref.access_t, &ref.size, &ref.hex_addr);
		
		/* memory reference (in)validation */
		switch(ref.size) {
		/* validate reference sizes */
		case 1: case 2: case 4: case 8:
		
			/* validate reference address*/
			if (ref.hex_addr % ref.size != 0) {
				fprintf(stderr, 
					"line %d has misaligned reference at address %x for size %d\n",
					i, ref.hex_addr, ref.size);
				break;
			}
			
			/* calculate cache info & data from mem ref */
			translate(&ref, &t, &b);
			
			/* look in cache and update on a miss */
			t.result = found_in_cache(&t, &b);
			if (t.result == 0) {
				add_to_cache(&t, &b);
				++misses;
			} else ++hits;
			
			/* print report on current reference */
			table_entry(i, &ref, &t, &b);
			++i;
			break;
			
		/* invalid reference size */
		default:
			fprintf(stderr, "line %d has illegal size %d\n", i, ref.size);
		}	
	}
	
	/* print results concering all references */
	statistics(hits, misses);
	
	/* deallocate dynamic memory */
	fclose(trace_config);
	for (i = 0; i < num_sets; ++i)
		free(cache[i]);
	free(cache);
	
	/* exit successfully */
	return 0;
}

void get_config(FILE **trace_config) {
	*trace_config = fopen("trace.config", "r");
	if (!trace_config) {
		printf("missing input file\n");
	    exit(1);
	}
}

void scan_config(FILE **trace_config, int *num_sets, int *set_sz, int *lne_sz) {
	char line[20];
	fgets(line, sizeof(line), *trace_config);
	sscanf(line, "%*[^:]: %d", num_sets);
	fgets(line, sizeof(line), *trace_config);
	sscanf(line, "%*[^:]: %d", set_sz);
	fgets(line, sizeof(line), *trace_config);
	sscanf(line, "%*[^:]: %d", lne_sz);
}

void init_cache() {
	int i, j;
	cache = (block**) malloc(num_sets * sizeof(block*));
	for (i = 0; i < num_sets; ++i)
		cache[i] = (block*) malloc(set_sz * sizeof(block));
	for (i = 0; i < num_sets; ++i)
		for (j = 0; j < set_sz; ++j)
			cache[i][j].valid 
			= cache[i][j].tag 
			= cache[i][j].dirty 
			= cache[i][j].age 
			= 0;
}

void translate(mem_ref *m, trans *t, block *b) {
	int bits = (int)((log(lne_sz) / log(2)));
	(*b).valid = 1;
	(*b).tag = (*m).hex_addr >> (bits + (int)(log(num_sets) / log(2)));
	(*t).index = ((*m).hex_addr >> bits) % num_sets;
	(*t).offset = (*m).hex_addr % lne_sz;
	(*b).dirty = ((*m).access_t == 'R') ? 0 : 1;
}

void header() {
	printf("Cache Configuration\n\n");
	printf("   %d %d-way set associative entries\n", num_sets, set_sz);
	printf("   of line size 8 bytes\n\n\n");
	printf("Results for Each Reference\n\n");
	printf("Ref  Access Address    Tag   Index Offset Result Memrefs\n");
	printf("---- ------ -------- ------- ----- ------ ------ -------\n");
}

void table_entry(int i, mem_ref *m, trans *t, block *b) {
	printf("%4d %6s %8x %7d %5d %6d %6s %7d\n", 
		i, 
		((*m).access_t == 'W') ? "write" : "read", 
		(*m).hex_addr, 
		(*b).tag, 
		(*t).index, 
		(*t).offset, 
		((*t).result == 0) ? "miss" : "hit",
		(*t).memrefs
	);
}

int found_in_cache(trans *t, block *b) {
	int i;
	for (i = 0; i < set_sz; ++i)
		if (cache[(*t).index][i].valid)
			if (cache[(*t).index][i].tag == (*b).tag) {
				(*t).memrefs = 0;
				return 1;
			}
	return 0;
}

void add_to_cache(trans *t, block * b) {
	int i, LRU = 0;
	(*t).memrefs = 1;
	block * loc = cache[(*t).index];
	for (i = 0; i < set_sz; ++i)
		++loc[i].age;
	for (i = 0; i < set_sz; ++i)
		if (!loc[i].valid) {
			loc[i].valid = 1;
			loc[i].tag = (*b).tag;
			loc[i].dirty = (*b).dirty;
			loc[i].age = 0;	
			return;
		}
	for (i = 0; i < set_sz; ++i)
		if (loc[i].age > loc[LRU].age)
			LRU = i;
	if (loc[LRU].dirty)
		++(*t).memrefs;
	loc[LRU].tag = (*b).tag;
	loc[LRU].dirty = (*b).dirty;
	loc[LRU].age = 0;
}

void statistics(int hits, int misses) {
	int accesses = hits + misses;
	printf("\n\nSimulation Summary Statistics\n");
	printf("-----------------------------\n");
	printf("Total hits       : %d\n", hits);
	printf("Total misses     : %d\n", misses);
	printf("Total accesses   : %d\n", accesses);
	printf("Hit ratio        : %f\n", (float)hits / accesses);
	printf("Miss ratio       : %f\n\n", (float)misses / accesses);
}