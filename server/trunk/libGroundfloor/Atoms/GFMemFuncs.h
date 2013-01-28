
#ifndef __GFMemFuncs_H
#define __GFMemFuncs_H

#include <cstdlib>

#define C_BLOCKCOUNT 1


// TODO: don't use these, introduces too many strange bugs when multithreading, not thread-safe?

// Default defines for using small memory maps
//#define GFMalloc(a) GFallocSmallMemory(a)
//#define GFFree(a) GFfreeSmallMemory(a)
//#define GFRealloc(a,c) GFreallocSmallMemory(a,c)


// Traceable defines for using small memory maps
//#define GFMalloc(a) GFallocTracedMemory(a,__FILE__,__LINE__,__FUNCTION__)
//#define GFFree(a) GFfreeTracedMemory(a,__FILE__,__LINE__,__FUNCTION__)
//#define GFRealloc(a,c) GFreallocTracedMemory(a,c,__FILE__,__LINE__,__FUNCTION__)


// To use the system defaults directly, use these defines
#define GFMalloc(a) malloc(a)
#define GFFree(a) free(a)
#define GFRealloc(a,c) realloc(a,c)

/** GFMemFuncs
 *
 * Groundfloor Memfuncs uses a small memory mapping technique
 *  that pre-allocates space and handles user allocations manually
 *  through optimized code.
 * The mapping system is able to speedup allocation and freeing
 *  by 20-50% depending on size and amount of the allocations.
 *
 * There are 5 reserved memory maps, giving 8 blocks of 16 bytes and
 *  C_BLOCKCOUNT times 4 blocks of 256 bytes. Note that the larger blocks take longer
 *  to assign as it contains a loop-structure and some necessary checks.
 * If the user requests an amount of memory that cannot be allocated by
 *  the Small Memory Mapping, it automatically reverts to the systems malloc/free.
 */

/// initializes the small memory maps (is automatically called by initGroundfloor())
bool GFinitSmallMemory();
/// finalizes the small memory maps, freeing the memory blocks and fills them with zeros
bool GFfiniSmallMemory();

/// allocates a piece of memory with given size
void *GFallocSmallMemory( unsigned int c );

/// re-allocates p or grows size of p
void *GFreallocSmallMemory( void *pOld, unsigned int c );

/// frees memory associated with given pointer
void GFfreeSmallMemory( void *p );

/// debugging method to check the size that was allocated with given pointer
long GFgetBytesAllocated( void *p );
/// debugging method to printf() the internal state of the memory maps
void GFprintSmallMemoryMap();

/// function to perform a simple check if a pointer is within bounds
bool GFisValidPointer( void *p, void *pDerived );


void GFnullifyMem( void *p, unsigned int count );

//------------------------------------------------------------------------------

/** Allocation class you can request when you're using traced memory **/
class TGFAllocation {
   public:
      TGFAllocation();
      ~TGFAllocation();

      void *addr;
      unsigned long size;
      unsigned long owner;
};

void *GFallocTracedMemory( unsigned int c, const char *file, long line, const char* func );
void *GFreallocTracedMemory( void *pOrg, unsigned int c, const char *file, long line, const char* func );
void GFfreeTracedMemory( void *p, const char *file, long line, const char* func );

void GFsetVerboseTrace( bool b );
void GFsetBlockForNonTracedFree( bool b );
void GFsetEnableTracing( bool b );

/// To retreive the allocation details of the given pointer
TGFAllocation *GFgetTrace( void *p );


#endif //__GFMemFuncs_H
