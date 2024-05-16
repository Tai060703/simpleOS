//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
/*
 *  MEMPHY_mv_csr - move MEMPHY cursor
 *  @mp: memphy struct
 *  @offset: offset
 */
int MEMPHY_mv_csr(struct memphy_struct *mp, int offset)
{
   int numstep = 0;

   mp->cursor = 0;
   while(numstep < offset && numstep < mp->maxsz){
     /* Traverse sequentially */
     mp->cursor = (mp->cursor + 1) % mp->maxsz;
     numstep++;
   }

   return 0;
}

/*
 *  MEMPHY_seq_read - read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_seq_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   if (!mp->rdmflg)
     return -1; /* Not compatible mode for sequential read */

   MEMPHY_mv_csr(mp, addr);
   *value = (BYTE) mp->storage[addr];

   return 0;
}

/*
 *  MEMPHY_read read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   if (mp->rdmflg)
      *value = mp->storage[addr];
   else /* Sequential access device */
      return MEMPHY_seq_read(mp, addr, value);

   return 0;
}

/*
 *  MEMPHY_seq_write - write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_seq_write(struct memphy_struct * mp, int addr, BYTE value)
{

   if (mp == NULL)
     return -1;

   if (!mp->rdmflg)
     return -1; /* Not compatible mode for sequential read */

   MEMPHY_mv_csr(mp, addr);
   mp->storage[addr] = value;

   return 0;
}

/*
 *  MEMPHY_write-write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   if (mp->rdmflg) {
      mp->storage[addr] = data;
   }
      
   else /* Sequential access device */
      return MEMPHY_seq_write(mp, addr, data);

   return 0;
}

/*
 *  MEMPHY_format-format MEMPHY device
 *  @mp: memphy struct
 */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
    /* This setting come with fixed constant PAGESZ */
    int numfp = mp->maxsz / pagesz;
    struct framephy_struct *newfst, *fst;
    int iter = 0;

    if (numfp <= 0)
      return -1;

    /* Init head of free framephy list */ 
    fst = malloc(sizeof(struct framephy_struct));
    fst->fpn = iter;
    mp->free_fp_list = fst;

    /* We have list with first element, fill in the rest num-1 element member*/
    for (iter = 1; iter < numfp ; iter++)
    {
       newfst =  malloc(sizeof(struct framephy_struct));
       newfst->fpn = iter;
       newfst->fp_next = NULL;
       fst->fp_next = newfst;
       fst = newfst;
    }

    return 0;
}

int MEMPHY_get_freefp(struct memphy_struct *mp, int *retfpn)
{
   struct framephy_struct *fp = mp->free_fp_list;
   if (fp == NULL)
     return -1;

   *retfpn = fp->fpn;
   mp->free_fp_list = fp->fp_next;

   /* MEMPHY is iteratively used up until its exhausted
    * No garbage collector acting then it not been released
    */
   free(fp);

   return 0;
}

int MEMPHY_dump(struct memphy_struct * mp)
{
    /*TODO dump memphy contnt mp->storage 
     *     for tracing the memory content
     */
    printf("----------------MEMORY CONTENT-------------- \n");
    printf("Address: Content \n");
    for (int i = 0; i < mp->maxsz; i++)
      if (mp->storage[i]) printf("0x%08x: %08x \n", i, mp->storage[i]);
    return 0;
}

int MEMPHY_put_freefp(struct memphy_struct *mp, int fpn)
{
   struct framephy_struct *fp = mp->free_fp_list;
   struct framephy_struct *newnode = malloc(sizeof(struct framephy_struct));

   /* Create new node with value fpn */
   newnode->fpn = fpn;
   newnode->fp_next = fp;
   mp->free_fp_list = newnode;

   return 0;
}
int MEMPHY_remove_usedfp(struct memphy_struct *mem_phy, int frame_number){
   // Start with the first frame in the used frame list
   struct framephy_struct *current_frame = mem_phy->used_fp_list;

   // Check if the used frame list is empty
   if (!current_frame) return -1;

   // If the first frame matches the frame number, remove it from the list
   if (current_frame->fpn == frame_number) {
      mem_phy->used_fp_list = mem_phy->used_fp_list->fp_next;
      current_frame->fp_next = NULL;
      return 0;
   }
   else {
      // Iterate through the list to find the frame with the specified number
      while (current_frame->fp_next) {
         if (current_frame->fp_next->fpn == frame_number) {
            // Remove the frame from the list
            struct framephy_struct *ret = current_frame->fp_next;
            current_frame->fp_next = current_frame->fp_next->fp_next;
            ret->fp_next = NULL;
            return 0;
         }
         else current_frame = current_frame->fp_next;
      }
      return -1; // Return -1 if the specified frame number is not found
   }
}

int MEMPHY_put_usedfp(struct memphy_struct *mem_phy, int frame_number, struct mm_struct *owner)
{
   struct framephy_struct *current_frame = mem_phy->used_fp_list;
   struct framephy_struct *new_frame_node = malloc(sizeof(struct framephy_struct));

   /* Create a new node with the specified frame number */
   new_frame_node->fpn = frame_number;
   new_frame_node->owner = owner;
   new_frame_node->fp_next = NULL;

   /* Push the new frame to the end of the list */
   if (!current_frame) {
      mem_phy->used_fp_list = new_frame_node;
   }
   else {
      while (current_frame->fp_next) {
         current_frame = current_frame->fp_next;
      } 
      current_frame->fp_next = new_frame_node;
   }
   return 0;
}

struct framephy_struct* MEMPHY_get_usedfp(struct memphy_struct *mem_phy)
{
   struct framephy_struct *current_frame = mem_phy->used_fp_list;

   // Return NULL if the used frame list is empty
   if (current_frame == NULL)
     return NULL;

   // Move to the next frame in the list
   mem_phy->used_fp_list = current_frame->fp_next;

   /* 
    * The used frames are iteratively retrieved until the memory is exhausted.
    * If there's no garbage collector acting, frames may not be released.
    */
   return current_frame;
}

/*
 *  Init MEMPHY struct
 */
int init_memphy(struct memphy_struct *mp, int max_size, int randomflg)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;
   mp->used_fp_list = NULL;
   MEMPHY_format(mp,PAGING_PAGESZ);

   mp->rdmflg = (randomflg != 0)?1:0;

   if (!mp->rdmflg )   /* Not Ramdom acess device, then it serial device*/
      mp->cursor = 0;

   return 0;
}

//#endif
