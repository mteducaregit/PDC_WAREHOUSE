/***
*heapinit.c -  Initialze the heap
*
*       Copyright (c) 1989-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*******************************************************************************/


#ifdef WINHEAP


#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>

HANDLE _crtheap;

/*
 * Dummy definition of _amblksiz. Included primarily so the dll will build
 * without having to change crtlib.c (there is an access function for _amblksiz
 * defined in crtlib.c).
 */
unsigned int _amblksiz = BYTES_PER_PARA;


/***
*_heap_init() - Initialize the heap
*
*Purpose:
*       Setup the initial C library heap.
*
*       NOTES:
*       (1) This routine should only be called once!
*       (2) This routine must be called before any other heap requests.
*
*Entry:
*       <void>
*Exit:
*       Returns 1 if successful, 0 otherwise.
*
*Exceptions:
*       If heap cannot be initialized, the program will be terminated
*       with a fatal runtime error.
*
*******************************************************************************/

int __cdecl _heap_init (
        int mtflag
        )
{
        //  Initialize the "big-block" heap first.
        if ( (_crtheap = HeapCreate( mtflag ? 0 : HEAP_NO_SERIALIZE,
                                     BYTES_PER_PAGE, 0 )) == NULL )
            return 0;

        //  Initialize the small-block heap
        if (__sbh_heap_init() == 0)
        {
            HeapDestroy(_crtheap);
            return 0;
        }
        return 1;
}

/***
*_heap_term() - return the heap to the OS
*
*Purpose:
*
*       NOTES:
*       (1) This routine should only be called once!
*       (2) This routine must be called AFTER any other heap requests.
*
*Entry:
*       <void>
*Exit:
*       <void>
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _heap_term (void)
{
    PHEADER         pHeader = __sbh_pHeaderList;
    int             cntHeader;

    //  scan through all the headers
    for (cntHeader = 0; cntHeader < __sbh_cntHeaderList; cntHeader++)
    {
        //  decommit and release the address space for the region
        VirtualFree(pHeader->pHeapData, BYTES_PER_REGION, MEM_DECOMMIT);
        VirtualFree(pHeader->pHeapData, 0, MEM_RELEASE);

        //  free the region data structure
        HeapFree(_crtheap, 0, pHeader->pRegion);

        pHeader++;
    }
    //  free the header list
    HeapFree(_crtheap, 0, __sbh_pHeaderList);

    //  destroy the large-block heap
    HeapDestroy(_crtheap);
}


#else  /* WINHEAP */


#ifdef _WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <dos.h>
#include <heap.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Heap descriptor
 */


struct _heap_desc_ _heap_desc = {
        &_heap_desc.sentinel,           /* pfirstdesc */
        &_heap_desc.sentinel,           /* proverdesc */
        NULL,                           /* emptylist */
        NULL,                           /* sentinel.pnextdesc */
        NULL                            /* sentinel.pblock */
        };

/*
 * Array of region structures
 * [Note: We count on the fact that this is always initialized to zero
 * by the compiler.]
 */

struct _heap_region_ _heap_regions[_HEAP_REGIONMAX];

void ** _heap_descpages;        /* linked list of pages used for descriptors */

/*
 * Control parameter locations
 */

unsigned int _heap_resetsize = 0xffffffff;

/* NOTE: Currenlty, _heap_growsize is a #define to _amblksiz */
unsigned int _heap_growsize   = _HEAP_GROWSIZE;         /* region inc size */
unsigned int _heap_regionsize = _HEAP_REGIONSIZE_L;     /* region size */
unsigned int _heap_maxregsize = _HEAP_MAXREGSIZE_L;     /* max region size */


/***
*_heap_init() - Initialize the heap
*
*Purpose:
*       Setup the initial C library heap.  All necessary memory and
*       data bases are init'd appropriately so future requests work
*       correctly.
*
*       NOTES:
*       (1) This routine should only be called once!
*       (2) This routine must be called before any other heap requests.
*
*
*Entry:
*       <void>
*Exit:
*       <void>
*
*Exceptions:
*       If heap cannot be initialized, the program will be terminated
*       with a fatal runtime error.
*
*******************************************************************************/

void __cdecl _heap_init (
        void
        )
{
        /*
         * Test for Win32S or Phar Lap TNT environment
         * which cannot allocate uncommitted memory
         * without actually allocating physical memory
         *
         * High bit of _osver is set for both of those environments
         * -AND- the Windows version will be less than 4.0.
         */

        if ( ( _osver & 0x8000 ) && ( _winmajor < 4 ) )
        {
                _heap_regionsize = _HEAP_REGIONSIZE_S;
                _heap_maxregsize = _HEAP_MAXREGSIZE_S;
        }
}



/***
*_heap_term() - Clean-up the heap
*
*Purpose:
*       This routine will decommit and release ALL of the CRT heap.
*       All memory malloc-ed by the CRT will then be invalid.
*
*       NOTES:
*       (1) This routine should only be called once!
*       (2) This routine must be called AFTER any other heap requests.
*
*Entry:
*       <void>
*Exit:
*       <void>
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _heap_term (
        void
        )
{
    int index;
    void **pageptr;

    /*
     * Loop through the region descriptor table, decommitting
     * and releasing (freeing up) each region that is in use.
     */

    for ( index=0 ; index < _HEAP_REGIONMAX ; index++ ) {
        void * regbase ;

        if ( (regbase = _heap_regions[index]._regbase)
          && VirtualFree(regbase, _heap_regions[index]._currsize, MEM_DECOMMIT)
          && VirtualFree(regbase, 0, MEM_RELEASE) )
                regbase = _heap_regions[index]._regbase = NULL ;
    }

    /*
     * Now we need to decommit and release the pages used for descriptors
     * _heap_descpages points to the head of a singly-linked list of the pages.
     */

    pageptr = _heap_descpages;

    while ( pageptr ) {
        void **nextpage;

        nextpage = *pageptr;

        if(!VirtualFree(pageptr, 0, MEM_RELEASE))
            break;      /* if the linked list is corrupted, give up */

        pageptr = nextpage;
    }

}



/***
* _heap_grow_emptylist() - Grow the empty heap descriptor list
*
*Purpose:
*       (1) Get memory from OS
*       (2) Form it into a linked list of empty heap descriptors
*       (3) Attach it to the master empty list
*
*       NOTE:  This routine assumes that the emptylist is NULL
*       when called (i.e., there are no available empty heap descriptors).
*
*Entry:
*       (void)
*
*Exit:
*       1, if the empty heap descriptor list was grown
*       0, if the empty heap descriptor list could not be grown.
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl _heap_grow_emptylist (
        void
        )
{
        REG1 _PBLKDESC first;
        REG2 _PBLKDESC next;
        _PBLKDESC last;

        /*
         * Get memory for the new empty heap descriptors
         *
         * Note that last is used to hold the returned pointer because
         * first (and next) are register class.
         */

        if ( !(last = VirtualAlloc(NULL,
                                   _HEAP_EMPTYLIST_SIZE,
                                   MEM_COMMIT,
                                   PAGE_READWRITE)) )
                return 0;

        /*
         * Add this descriptor block to the front of the list
         *
         * Advance "last" to skip over the
         */

        *(void **)last = _heap_descpages;
        _heap_descpages = (void **)(last++);


        /*
         * Init the empty heap descriptor list.
         */

        _heap_desc.emptylist = first = last;


        /*
         * Carve the memory into an empty list
         */

        last = (_PBLKDESC) ((char *) first + _HEAP_EMPTYLIST_SIZE - 2 * sizeof(_BLKDESC));
        next = (_PBLKDESC) ((char *) first + sizeof(_BLKDESC));

        while ( first < last ) {

                /* Init this descriptor */
                first->pnextdesc = next;

                /* onto the next block */

                first = next++;

        }

        /*
         * Take care of the last descriptor (end of the empty list)
         */

        last->pnextdesc = NULL;


        return 1;
}


/***
*__getempty() - get an empty heap descriptor
*
*Purpose:
*       Get a descriptor from the list of empty heap descriptors. If the list
*       is empty, call _heap_grow_emptylist.
*
*Entry:
*       no arguments
*
*Exit:
*       If successful, a pointer to the descriptor is returned.
*       Otherwise, NULL is returned.
*
*Exceptions:
*
*******************************************************************************/

_PBLKDESC __cdecl __getempty(
        void
        )
{
        _PBLKDESC pdesc;

        if ( (_heap_desc.emptylist == NULL) && (_heap_grow_emptylist()
              == 0) )
                return NULL;

        pdesc = _heap_desc.emptylist;

        _heap_desc.emptylist = pdesc->pnextdesc;

        return pdesc;
}


#else  /* _WIN32 */

#if defined (_M_MPPC) || defined (_M_M68K)


#include <cruntime.h>
#include <dos.h>
#include <heap.h>
#include <malloc.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <dbgint.h>

#include <macos\types.h>
#include <macos\errors.h>
#include <macos\memory.h>               // Mac OS interface header
#include <macos\lowmem.h>
#include <macos\segload.h>

#define _HEAP_EMPTYLIST_SIZE    (1 * _PAGESIZE_)

#define DSErrCode   (*(short*)(0x0af0))

/*
 * Heap descriptor
 */

struct _heap_desc_ _heap_desc = {
        &_heap_desc.sentinel,           /* pfirstdesc */
        &_heap_desc.sentinel,           /* proverdesc */
        NULL,                           /* emptylist */
        NULL,                           /* sentinel.pnextdesc */
        NULL                            /* sentinel.pblock */
        };

/*
 * Array of region structures
 * [Note: We count on the fact that this is always initialized to zero
 * by the compiler.]
 */

Handle hHeapRegions = NULL;
int _heap_region_table_cur = 0;

/*
 * Control parameter locations
 */

unsigned int _heap_resetsize = 0xffffffff;

/* NOTE: Currenlty, _heap_growsize is a #define to _amblksiz */
unsigned int _heap_growsize   = _HEAP_GROWSIZE;         /* region inc size */
unsigned int _heap_regionsize = _HEAP_REGIONSIZE;       /* region size */


/***
*_heap_init() - Initialize the heap
*
*Purpose:
*       Setup the initial C library heap.  All necessary memory and
*       data bases are init'd appropriately so future requests work
*       correctly.
*
*       NOTES:
*       (1) This routine should only be called once!
*       (2) This routine must be called before any other heap requests.
*
*
*Entry:
*       <void>
*Exit:
*       <void>
*
*Exceptions:
*       If heap cannot be initialized, the program will be terminated
*       with a fatal runtime error.
*
*******************************************************************************/

void __cdecl _heap_init (
        void
        )
{

#define _INITREGIONSZ 0x1000

        /*LATER -- do we need to do anything to init heap? Yes, in case user not malloc first*/
        int oldregionsz = _heap_regionsize;     /* save current region size */

        struct _heap_region_ *pHeapRegions;
        void *p;
        void *p2;

        if (hHeapRegions == NULL)
                {
                hHeapRegions = NewHandle(sizeof(struct _heap_region_)*_HEAP_REGIONMAX);
                if (hHeapRegions == NULL)
                        {
                        DSErrCode = appMemFullErr;
                        ExitToShell();
                        }
                HLock(hHeapRegions);
                pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
                memset(pHeapRegions, 0, sizeof(struct _heap_region_)*_HEAP_REGIONMAX);
                _heap_region_table_cur = _HEAP_REGIONMAX;
                }


        _heap_regionsize = _INITREGIONSZ;       /* set region size to 64 Kb */

        /* make sure we have enough memory to do initialization */
        if ((p = NewPtr(_HEAP_EMPTYLIST_SIZE)) == NULL)
                {
                DSErrCode = appMemFullErr;
                ExitToShell();
                }

        if ((p2 = NewPtr(_heap_regionsize)) == NULL)
                {
                DSErrCode = appMemFullErr;
                ExitToShell();
                }

        if (p)
                {
                DisposePtr(p);
                }
        if (p2)
                {
                DisposePtr(p2);
                }

        p = _malloc_base(4);
        if (p == NULL)
                {
                DSErrCode = appMemFullErr;
                ExitToShell();
                }
        _free_base( p );                /* malloc, then free a block */
        _heap_regionsize = oldregionsz;         /* restore region size */

}



/***
* _heap_grow_emptylist() - Grow the empty heap descriptor list
*
*Purpose:
*       (1) Get memory from OS
*       (2) Form it into a linked list of empty heap descriptors
*       (3) Attach it to the master empty list
*
*       NOTE:  This routine assumes that the emptylist is NULL
*       when called (i.e., there are no available empty heap descriptors).
*
*Entry:
*       (void)
*
*Exit:
*       (void)
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl _heap_grow_emptylist (
        void
        )
{
        REG1 _PBLKDESC first;
        REG2 _PBLKDESC next;
        _PBLKDESC last;


        /*
         * Get memory for the new empty heap descriptors
         *
         * Note that last is used to hold the returned pointer because
         * first (and next) are register class.
         */

        if ((last = (_PBLKDESC)NewPtr(_HEAP_EMPTYLIST_SIZE)) == NULL)
                {
                return 0;
                }

        /*
         * Init the empty heap descriptor list.
         */

        _heap_desc.emptylist = first = last;


        /*
         * Carve the memory into an empty list
         */

        last = (_PBLKDESC) ((char *) first + _HEAP_EMPTYLIST_SIZE - sizeof(_BLKDESC));
        next = (_PBLKDESC) ((char *) first + sizeof(_BLKDESC));

        while ( first < last ) {

                /* Init this descriptor */
                first->pnextdesc = next;

                /* onto the next block */

                first = next++;

        }

        /*
         * Take care of the last descriptor (end of the empty list)
         */

        last->pnextdesc = NULL;


        return 1;

}

/***
*__getempty() - get an empty heap descriptor
*
*Purpose:
*       Get a descriptor from the list of empty heap descriptors. If the list
*       is empty, call _heap_grow_emptylist.
*
*Entry:
*       no arguments
*
*Exit:
*       If successful, a pointer to the descriptor is returned.
*       Otherwise, NULL is returned.
*
*Exceptions:
*
*******************************************************************************/

_PBLKDESC __cdecl __getempty(
        void
        )
{
        _PBLKDESC pdesc;

        if ( (_heap_desc.emptylist == NULL) && (_heap_grow_emptylist()
              == 0) )
                return NULL;

        pdesc = _heap_desc.emptylist;

        _heap_desc.emptylist = pdesc->pnextdesc;

        return pdesc;
}


#endif  /* defined (_M_MPPC) || defined (_M_M68K) */

#endif  /* _WIN32 */

#endif  /* WINHEAP */
