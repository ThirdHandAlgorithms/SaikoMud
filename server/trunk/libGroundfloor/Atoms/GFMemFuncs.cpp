
#include "GFMemFuncs.h"
#include "GFDefines.h"
#include "GFLockable.h"

#include <cstring>
#include <cstdio>

#ifndef gfmin
   #define gfmin(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define C_SMALLBLOCKSIZE 256

static char *smallblock = NULL;
static char *smalleob = NULL;

static unsigned long smallblockmap  = 0;
static unsigned long smallblocksize = 0x80;


#define C_MAPARRAY_BLOCKSIZE 1024
static char *blocks[C_BLOCKCOUNT];
static char *eobs[C_BLOCKCOUNT];
static unsigned long maps[C_BLOCKCOUNT];
static unsigned long counters[C_BLOCKCOUNT];



inline bool initMapArray() {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      blocks[i]   = reinterpret_cast<char *>( malloc( C_MAPARRAY_BLOCKSIZE ) );
      eobs[i]     = blocks[i] + C_MAPARRAY_BLOCKSIZE;
      memset( blocks[i], 0, C_MAPARRAY_BLOCKSIZE );

      counters[i] = 4;
   }

   return true;
}

inline bool finiMapArray() {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      counters[i] = 0;
      memset( blocks[i], 0, C_MAPARRAY_BLOCKSIZE );
      free( blocks[i] );
   }

   return true;
}

inline void *allocInMapArray( unsigned int c ) {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      if ( counters[i] != 0 ) {
         counters[i]--;

         void *ptrAddr = NULL;
         unsigned long map = maps[i];
         if ( (map & 0x000000FF) == 0) {
            map |= c;
            ptrAddr = blocks[i];
         } else if ( (map & 0x0000FF00) == 0) {
            map |= (c << 8);
            ptrAddr = (blocks[i] + 0x0100);
         } else if ( (map & 0x00FF0000) == 0) {
            map |= (c << 16);
            ptrAddr = (blocks[i] + 0x0200);
         } else if ( (map & 0xFF000000) == 0) {
            map |= (c << 24);
            ptrAddr = (blocks[i] + 0x0300);
         }

         if ( ptrAddr != NULL ) {
            maps[i] = map;
            return ptrAddr;
         }
      }
   }

   return NULL;
}

inline void *reallocInMapArray( void *pOld, unsigned int c ) {
   void *p = NULL;
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      char *block = blocks[i];
      if ( (pOld >= block) && (pOld < eobs[i]) ) {
         int j = reinterpret_cast<char *>( pOld ) - block;

         if (j < C_SMALLBLOCKSIZE) {
            if ( c < C_SMALLBLOCKSIZE ) {
               maps[i] = (maps[i] & 0xFFFFFF00) | c;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, C_SMALLBLOCKSIZE );
               maps[i] &= 0xFFFFFF00;
               counters[i]++;
               return p;
            }
         } else if (j < 0x200) {
            if ( c < C_SMALLBLOCKSIZE ) {
               maps[i] = (maps[i] & 0xFFFF00FF) | (c << 8);
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, C_SMALLBLOCKSIZE );
               maps[i] &= 0xFFFF00FF;
               counters[i]++;
               return p;
            }
         } else if (j < 0x300) {
            if ( c < C_SMALLBLOCKSIZE ) {
               maps[i] = (maps[i] & 0xFF00FFFF) | (c << 16);
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, C_SMALLBLOCKSIZE );
               maps[i] &= 0xFF00FFFF;
               counters[i]++;
               return p;
            }
         } else if (j < 0x400) {
            if ( c < C_SMALLBLOCKSIZE ) {
               maps[i] = (maps[i] & 0x00FFFFFF) | (c << 24);
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, C_SMALLBLOCKSIZE );
               maps[i] &= 0x00FFFFFF;
               counters[i]++;
               return p;
            }
         }
      }
   }

   return p;
}

inline bool freeInMapArray( void *p ) {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      char *block = blocks[i];
      if ( (p >= block) && (p < eobs[i]) ) {
         int j = reinterpret_cast<char *>( p ) - block;

         unsigned long map = maps[i];
         if (j < 0x100) {
            if ( (map & 0x000000FF) != 0 ) {
               map &= 0xFFFFFF00;
               counters[i]++;
            }
         } else if (j < 0x200) {
            if ( (maps[i] & 0x0000FF00) != 0 ) {
               map &= 0xFFFF00FF;
               counters[i]++;
            }
         } else if (j < 0x300) {
            if ( (map & 0x00FF0000) != 0 ) {
               map &= 0xFF00FFFF;
               counters[i]++;
            }
         } else if (j < 0x400) {
            if ( (map & 0xFF000000) != 0 ) {
               map &= 0x00FFFFFF;
               counters[i]++;
            }
         }
         maps[i] = map;

         return true;
      }
   }

   return false;
}

int GFgetBytesAllocatedInMapArray( void *p ) {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      char *block = blocks[i];
      if ( (p >= block) && (p < eobs[i]) ) {
         int j = reinterpret_cast<char *>( p ) - block;
         if (j < 0x100) {
            return (maps[i] & 0x000000FF);
         } else if (j < 0x200) {
            return (maps[i] & 0x0000FF00) >> 8;
         } else if (j < 0x300) {
            return (maps[i] & 0x00FF0000) >> 16;
         } else if (j < 0x400) {
            return (maps[i] & 0xFF000000) >> 24;
         }
      }
   }

   return -1;
}


void GFprintSmallMemoryInMapArray() {
   for ( unsigned int i = 0; i < C_BLOCKCOUNT; i++ ) {
      char *block             = blocks[i];
      unsigned long map       = maps[i];
      unsigned long counter   = counters[i];

      printf( "Block[%d] == %x (%d slots remaining) (addr %x - %x)\r\n", i, map, counter, block, block + 0x400 );
      printf( "Block[%d]-0: %x (%s)\r\n", i, (map & 0x000000FF), block );
      printf( "Block[%d]-1: %x (%s)\r\n", i, (map & 0x0000FF00) >> 8, block + 0x100 );
      printf( "Block[%d]-2: %x (%s)\r\n", i, (map & 0x00FF0000) >> 16, block + 0x200 );
      printf( "Block[%d]-3: %x (%s)\r\n\r\n", i, (map & 0xFF000000) >> 24, block + 0x300 );
   }
}

//------------------------------------------------------------------------------

bool GFinitSmallMemory() {
   smallblock = static_cast<char *>( malloc( smallblocksize ) );
   memset( smallblock, 0, smallblocksize );
   smalleob = smallblock + smallblocksize;

   initMapArray();

   return true;
}

bool GFfiniSmallMemory() {
   finiMapArray();

   if ( smallblock != NULL ) {
      memset( smallblock, 0, smallblocksize );
      free( smallblock );
      smallblock = NULL;
   }

   return true;
}

void *GFallocSmallMemory( unsigned int c ) {
   if ( c == 0 ) {
      return malloc( 0 );
   }

   void *p = NULL;
   if ( c < 16 ) {
      if ( (smallblockmap & 0x0000000F) == 0) {
         smallblockmap |= c;
         return smallblock;
      } else if ( (smallblockmap & 0x000000F0) == 0) {
         smallblockmap |= (c << 4);
         return (smallblock + 0x10);
      } else if ( (smallblockmap & 0x00000F00) == 0) {
         smallblockmap |= (c << 8);
         return (smallblock + 0x20);
      } else if ( (smallblockmap & 0x0000F000) == 0) {
         smallblockmap |= (c << 12);
         return (smallblock + 0x30);
      } else if ( (smallblockmap & 0x000F0000) == 0) {
         smallblockmap |= (c << 16);
         return (smallblock + 0x40);
      } else if ( (smallblockmap & 0x00F00000) == 0) {
         smallblockmap |= (c << 20);
         return (smallblock + 0x50);
      } else if ( (smallblockmap & 0x0F000000) == 0) {
         smallblockmap |= (c << 24);
         return (smallblock + 0x60);
      } else if ( (smallblockmap & 0xF0000000) == 0) {
         smallblockmap |= (c << 28);
         return (smallblock + 0x70);
      }
   } else if ( c < 256 ) {
      p = allocInMapArray( c );
   }

   if ( p == NULL ) {
      p = malloc( c );
   }


   if ( p == NULL ) {
       printf( "Out Of Memory 1!\n" );
   }

   return p;
}

void *GFreallocSmallMemory( void *pOld, unsigned int c ) {
   void *p = NULL;

   if ( pOld != NULL ) {
      long j = static_cast<char *>(pOld) - smallblock;
      if ( (j >= 0) && (j < smallblocksize) ) {
         if (j < 0x10) {
            if ( c < 16 ) {
               smallblockmap &= 0xFFFFFFF0;
               smallblockmap |= c;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x0000000F) );
               smallblockmap &= 0xFFFFFFF0;
               return p;
            }
         } else if (j < 0x20) {
            if ( c < 16 ) {
               smallblockmap &= 0xFFFFFF0F;
               smallblockmap |= c << 4;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x000000F0) >> 4 );
               smallblockmap &= 0xFFFFFF0F;
               return p;
            }
         } else if (j < 0x30) {
            if ( c < 16 ) {
               smallblockmap &= 0xFFFFF0FF;
               smallblockmap |= c << 8;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x00000F00) >> 8 );
               smallblockmap &= 0xFFFFF0FF;
               return p;
            }
         } else if (j < 0x40) {
            if ( c < 16 ) {
               smallblockmap &= 0xFFFF0FFF;
               smallblockmap |= c << 12;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x0000F000) >> 12 );
               smallblockmap &= 0xFFFF0FFF;
               return p;
            }
         } else if (j < 0x50) {
            if ( c < 16 ) {
               smallblockmap &= 0xFFF0FFFF;
               smallblockmap |= c << 16;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x000F0000) >> 16 );
               smallblockmap &= 0xFFF0FFFF;
               return p;
            }
         } else if (j < 0x60) {
            if ( c < 16 ) {
               smallblockmap &= 0xFF0FFFFF;
               smallblockmap |= c << 20;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x00F00000) >> 20 );
               smallblockmap &= 0xFF0FFFFF;
               return p;
            }
         } else if (j < 0x70) {
            if ( c < 16 ) {
               smallblockmap &= 0xF0FFFFFF;
               smallblockmap |= c << 24;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0x0F000000) >> 24 );
               smallblockmap &= 0xF0FFFFFF;
               return p;
            }
         } else if (j < 0x80) {
            if ( c < 16 ) {
               smallblockmap &= 0x0FFFFFFF;
               smallblockmap |= c << 28;
               return pOld;
            } else {
               p = GFallocSmallMemory( c );
               memcpy( p, pOld, (smallblockmap & 0xF0000000) >> 28 );
               smallblockmap &= 0x0FFFFFFF;
               return p;
            }
         }
      } else {
         p = reallocInMapArray( pOld, c );
         if ( p == NULL ) {
            p = realloc( pOld, c );

           if ( p == NULL ) {
               printf( "Out Of Memory.. old: %x, new: %x, size: %d ... for real! (j=%d, size=%d)\n", pOld, p, c, j, smallblocksize );
           }
         }
      }
   }

   if ( p == NULL ) {
       printf( "Out Of Memory 2!\n" );
   }

   return p;
}

void GFfreeSmallMemory( void *p ) {
   if ( p != NULL ) {
      long j = static_cast<char *>(p) - smallblock;

      if ( (j >= 0) && (j < smallblocksize) ) {
         if (j < 0x10) {
            smallblockmap &= 0xFFFFFFF0;
         } else if (j < 0x20) {
            smallblockmap &= 0xFFFFFF0F;
         } else if (j < 0x30) {
            smallblockmap &= 0xFFFFF0FF;
         } else if (j < 0x40) {
            smallblockmap &= 0xFFFF0FFF;
         } else if (j < 0x50) {
            smallblockmap &= 0xFFF0FFFF;
         } else if (j < 0x60) {
            smallblockmap &= 0xFF0FFFFF;
         } else if (j < 0x70) {
            smallblockmap &= 0xF0FFFFFF;
         } else if (j < 0x80) {
            smallblockmap &= 0x0FFFFFFF;
         }
      } else {
         if ( !freeInMapArray(p) ) {
            free( p );
         }
      }
   }
}

long GFgetBytesAllocated( void *p ) {
   __int64 j = static_cast<char *>(p) - smallblock;

   if ( (j >= 0) && (j < smallblocksize) ) {
      if (j < 0x10) {
         return (smallblockmap & 0x0000000F);
      } else if (j < 0x20) {
         return (smallblockmap & 0x000000F0) >> 4;
      } else if (j < 0x30) {
         return (smallblockmap & 0x00000F00) >> 8;
      } else if (j < 0x40) {
         return (smallblockmap & 0x0000F000) >> 12;
      } else if (j < 0x50) {
         return (smallblockmap & 0x000F0000) >> 16;
      } else if (j < 0x60) {
         return (smallblockmap & 0x00F00000) >> 20;
      } else if (j < 0x70) {
         return (smallblockmap & 0x0F000000) >> 24;
      } else if (j < 0x80) {
         return (smallblockmap & 0xF0000000) >> 28;
      }
   } else {
      return GFgetBytesAllocatedInMapArray( p );
   }

   return -1;
}

void GFprintSmallMemoryMap() {
   printf( "SmallBlock == %x (%x - %x)\r\n", smallblockmap, smallblock, smallblock + 0x80  );
   printf( "SmallBlock-0: %x (%s)\r\n", (smallblockmap & 0x0000000F), smallblock );
   printf( "SmallBlock-1: %x (%s)\r\n", (smallblockmap & 0x000000F0) >> 4, smallblock + 0x10 );
   printf( "SmallBlock-2: %x (%s)\r\n", (smallblockmap & 0x00000F00) >> 8, smallblock + 0x20 );
   printf( "SmallBlock-3: %x (%s)\r\n", (smallblockmap & 0x0000F000) >> 12, smallblock + 0x30 );
   printf( "SmallBlock-4: %x (%s)\r\n", (smallblockmap & 0x000F0000) >> 16, smallblock + 0x40 );
   printf( "SmallBlock-5: %x (%s)\r\n", (smallblockmap & 0x00F00000) >> 20, smallblock + 0x50 );
   printf( "SmallBlock-6: %x (%s)\r\n", (smallblockmap & 0x0F000000) >> 24, smallblock + 0x60 );
   printf( "SmallBlock-7: %x (%s)\r\n\r\n", (smallblockmap & 0xF0000000) >> 28, smallblock + 0x70 );

   GFprintSmallMemoryInMapArray();
}


bool GFisValidPointer( void *p, void *pDerived ) {
   long c = GFgetBytesAllocated(p);
   if ( c != -1 ) {
      long pDiff = reinterpret_cast<long>(pDerived) - reinterpret_cast<long>(p);
      if ( pDiff >= 0 ) {
         if ( pDiff < c ) {
            return true;
         }
      }
   }

   return false;
}

void GFnullifyMem( void *p, unsigned int count ) {
    memset( p, 0, count );
    /*
   unsigned long *pAddr = reinterpret_cast<unsigned long *>( p );
   unsigned long iNull = 0;

   unsigned int c = count / sizeof(unsigned long);
   unsigned int i = c + 1;
   while ( i != 0 ) {
      i--;
      pAddr[i] = iNull;
   }

   unsigned int iDone = (c * sizeof(unsigned long));
   unsigned char *pAddrC = reinterpret_cast<unsigned char *>( p );
   for ( i = iDone; i < count; i++ ) {
      pAddrC[i] = 0;
   }*/
}
