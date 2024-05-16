/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access
 * and runs at high speed
 */

#include "mm.h"
#include <stdlib.h>

#define MAX_PID 1024
typedef struct
{
   BYTE *storage;
} TLBCache;

static TLBCache cache[MAX_PID] = {0};
#define init_tlbcache(mp, sz, ...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct memphy_struct *mp, int pid, int pgnum, BYTE *value)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   if (pid < 0 || pid >= MAX_PID || mp == NULL)
      return -1;
   TLBCache *tlb_cache = &cache[pid];
   if (tlb_cache->storage != NULL && tlb_cache->storage[pgnum] != NULL)
   {
      // When cache hit
      *value = tlb_cache->storage[pgnum];
   }
   else
   {
      // When cache miss
      int result = TLBMEMPHY_read(mp, pgnum, value);
      if (result == -1)
         return -1;
      // Cache the fetched page
      if (tlb_cache->storage == NULL)
      {
         tlb_cache->storage = (BYTE *)malloc(mp->maxsz * sizeof(BYTE));
         if (tlb_cache->storage == NULL)
            return -1; // Memory allocation failed
      }
      tlb_cache->storage[pgnum] = *value;
   }
   return 0;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   if (pid < 0 || pid >= MAX_PID || mp == NULL)
      return -1; // Return -1 if PID is invalid or mp is invalid

   // Initialize a pointer to the TLB cache of the PID
   TLBCache *tlb_cache = &cache[pid];

   // Write the value to both cache and memory
   if (tlb_cache->storage == NULL)
   {
      tlb_cache->storage = (BYTE *)malloc(mp->maxsz * sizeof(BYTE));
      if (tlb_cache->storage == NULL)
         return -1;  //Return -1 if memory allocation fails
   }
   tlb_cache->storage[pgnum] = value;
   int result = TLBMEMPHY_write(mp, pgnum, value);
   if (result == -1)
      return -1; // Unable to write to memory
   return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL)
      return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}

/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct *mp, int addr, BYTE data)
{
   if (mp == NULL)
      return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */

int TLBMEMPHY_dump(struct memphy_struct *mp)
{
   /*TODO dump memphy contnt mp->storage
    *     for tracing the memory content
    */
   if (mp == NULL)
      return -1;

   printf("Dumping memphy content:\n");
   for (int i = 0; i < mp->maxsz; i++)
   {
      printf("%02X ", mp->storage[i]); // Print the value of each BYTE of memphy_struct
      if ((i + 1) % 16 == 0)
         printf("\n");
   }
   printf("\n");
   return 0;
}

/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
   mp->maxsz = max_size;

   mp->rdmflg = 1;

   return 0;
}

// #endif

