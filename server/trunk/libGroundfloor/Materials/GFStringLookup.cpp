
#include "GFStringLookup.h"


//------------------------------------------------------------------------------------

TGFSimpleStringLookupObject::TGFSimpleStringLookupObject( const char *sName, TGFFreeable *anObject ) : TGFFreeable() {
    name.setValue_ansi( sName );
    object = anObject;
}


TGFSimpleStringLookupObject:: TGFSimpleStringLookupObject( const TGFString *sName, TGFFreeable *anObject ) : TGFFreeable() {
    name.setValue( sName );
    object = anObject;
}

TGFSimpleStringLookupObject::~TGFSimpleStringLookupObject() {
    // don't delete the object, we don't own it
    object = NULL;
}

//------------------------------------------------------------------------------------

TGFSimpleStringLookup::TGFSimpleStringLookup() : TGFVector() {
}

TGFSimpleStringLookup::~TGFSimpleStringLookup() {
}

void TGFSimpleStringLookup::addObjectByString( const char *sName, TGFFreeable *anObject ) {
    this->addElement( new TGFSimpleStringLookupObject( sName, anObject ) );
}


void TGFSimpleStringLookup::insertObjectSortedByLength( const TGFString *sName, TGFFreeable *anObject, bool bAscending ) {
    unsigned long i, c;
    TGFSimpleStringLookupObject *obj;
    TGFString s(sName);

    bool bInserted = false;
    TGFSimpleStringLookupObject *lobj = new TGFSimpleStringLookupObject( sName, anObject );

    c = this->iElementCount;
    for ( i = 0; i < c; i++ ) {
        obj = static_cast<TGFSimpleStringLookupObject *>( paVector[i] );
        if ( obj != NULL ) {
           if ( bAscending ) {
              if ( s.getLength() <= obj->name.getLength() ) {
                   this->insertAt( i, lobj );
                   bInserted = true;
                   break;
              }
           } else {
              if ( s.getLength() >= obj->name.getLength() ) {
                   this->insertAt( i, lobj );
                   bInserted = true;
                   break;
              }
           }
        }
    }

    if ( !bInserted ) {
       this->addElement( lobj );
    }
}

void TGFSimpleStringLookup::insertObjectSortedByLength( const char *sName, TGFFreeable *anObject, bool bAscending ) {
    unsigned long i, c;
    TGFSimpleStringLookupObject *obj;
    TGFString s(sName);

    bool bInserted = false;
    TGFSimpleStringLookupObject *lobj = new TGFSimpleStringLookupObject( sName, anObject );

    c = this->iElementCount;
    for ( i = 0; i < c; i++ ) {
        obj = static_cast<TGFSimpleStringLookupObject *>( paVector[i] );
        if ( obj != NULL ) {
           if ( bAscending ) {
              if ( s.getLength() <= obj->name.getLength() ) {
                   this->insertAt( i, lobj );
                   bInserted = true;
                   break;
              }
           } else {
              if ( s.getLength() >= obj->name.getLength() ) {
                   this->insertAt( i, lobj );
                   bInserted = true;
                   break;
              }
           }
        }
    }

    if ( !bInserted ) {
       this->addElement( lobj );
    }
}


TGFFreeable *TGFSimpleStringLookup::getObjectByString( const TGFString *sName ) {
    unsigned long i, c;
    TGFSimpleStringLookupObject *obj;

    c = this->iElementCount;
    for ( i = 0; i < c; i++ ) {
        obj = static_cast<TGFSimpleStringLookupObject *>( paVector[i] );
        if ( obj != NULL ) {
            if ( sName->match( &(obj->name) ) ) {
                return obj->object;
            }
        }
    }

    return NULL;
}

TGFFreeable *TGFSimpleStringLookup::getObjectByString( const char *sName ) {
    TGFString tmp;
    tmp.setValue_ansi( sName );

    return getObjectByString( &tmp );
}

void TGFSimpleStringLookup::removeObjectByString( const char *sName ) {
    TGFString tmp;
    tmp.setValue_ansi( sName );

    unsigned long i, c;
    TGFSimpleStringLookupObject *obj;

    c = this->iElementCount;
    for ( i = 0; i < c; i++ ) {
        obj = static_cast<TGFSimpleStringLookupObject *>( paVector[i] );
        if ( obj != NULL ) {
            if ( tmp.match( &(obj->name) ) ) {
                paVector[i] = NULL;

                delete obj;
                break;
            }
        }
    }
}
