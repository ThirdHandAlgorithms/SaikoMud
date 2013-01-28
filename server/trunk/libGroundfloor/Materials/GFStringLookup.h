//---------------------------------------------------------------------------

#ifndef GFStringLookupH
#define GFStringLookupH
//---------------------------------------------------------------------------

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFString.h"
#include "../Molecules/GFVector.h"

class TGFSimpleStringLookupObject: public TGFFreeable {
    public:
        TGFSimpleStringLookupObject( const char *sName, TGFFreeable *anObject );
        TGFSimpleStringLookupObject( const TGFString *sName, TGFFreeable *anObject );
        ~TGFSimpleStringLookupObject();

        TGFString name;
        TGFFreeable *object;
};

class TGFSimpleStringLookup: public TGFVector {
    public:
        TGFSimpleStringLookup();
        ~TGFSimpleStringLookup();

        void addObjectByString( const char *sName, TGFFreeable *anObject );
        TGFFreeable *getObjectByString( const TGFString *sName );
        TGFFreeable *getObjectByString( const char *sName );
        void removeObjectByString( const char *sName );

        void insertObjectSortedByLength( const char *sName, TGFFreeable *anObject, bool bAscending = true );
        void insertObjectSortedByLength( const TGFString *sName, TGFFreeable *anObject, bool bAscending = true );
};


//---------------------------------------------------------------------------
#endif
