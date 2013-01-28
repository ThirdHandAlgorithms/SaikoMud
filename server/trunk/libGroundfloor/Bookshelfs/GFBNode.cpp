
#include "GFBNode.h"

//------------------------------------------------------------------------------

TGFBNodePath::TGFBNodePath() : TGFFreeable() {
   current = -1;
}

TGFBNodePath::~TGFBNodePath() {
}

void TGFBNodePath::append( unsigned int i ) {
   path.push_back( i );
}

void TGFBNodePath::clear() {
   path.clear();
   current = -1;
}

unsigned int TGFBNodePath::traverse_reset() {
   current = -1;

   return current;
}

unsigned int TGFBNodePath::traverse_previous() {
   current--;

   return path[current];
}

unsigned int TGFBNodePath::traverse_next() {
   current++;

   return path[current];
}

bool TGFBNodePath::traverse_isBoF() {
   return (current <= -1);
}

bool TGFBNodePath::traverse_isEoF() {
   int c = path.size() - 1;
   return (current >= c);
}

void TGFBNodePath::readFromString( const char *sPath ) {
    this->clear();

    unsigned int len = strlen( sPath );
    TGFString *nextItem = new TGFString();
    for ( unsigned int i = 0; i < len; i++ ) {
        char c = sPath[i];
        if ( (c >= '0') && (c <= '9') ) {
            nextItem->append( c );
        } else {
            unsigned int p = atoi( nextItem->getValue() );
            this->append( p );
            nextItem->setLength( 0 );
        }
    }
    if ( nextItem->getLength() != 0 ) {
        unsigned int p = atoi( nextItem->getValue() );
        this->append( p );
    }
    delete nextItem;
}

//------------------------------------------------------------------------------

TGFBNode::TGFBNode() : TGFFreeable() {
   arrAttributes = NULL;
   arrChildren = NULL;
}

TGFBNode::TGFBNode( const char *sName ) : TGFFreeable() {
   arrAttributes = NULL;
   arrChildren = NULL;

   parent = NULL;

   name.setValue_ansi( sName );
}

TGFBNode::~TGFBNode() {
   if ( arrAttributes != NULL ) {
      delete arrAttributes;
   }
   if ( arrChildren != NULL ) {
      delete arrChildren;
   }
}

TGFBNode *TGFBNode::getParentNode() {
   return parent;
}

void TGFBNode::addAttribute( TGFBAttribute *anAttrib ) {
   if ( arrAttributes == NULL ) {
      arrAttributes = new TGFVector();
   }

   arrAttributes->addElement( anAttrib );
}

void TGFBNode::addChildNode( TGFBNode *aNode ) {
   if ( arrChildren == NULL ) {
      arrChildren = new TGFVector();
   }

   arrChildren->addElement( aNode );
   aNode->parent = this;
}

unsigned int TGFBNode::childCount() const {
   if ( arrChildren != NULL ) {
      return arrChildren->size();
   }

   return 0;
}

unsigned int TGFBNode::attributeCount() const {
   if ( arrAttributes != NULL ) {
      return arrAttributes->size();
   }

   return 0;
}

TGFBAttribute *TGFBNode::getAttribute( unsigned int i ) const {
   if ( arrAttributes != NULL ) {
      return static_cast<TGFBAttribute *>( arrAttributes->elementAt( i ) );
   }

   return NULL;
}

TGFBAttribute *TGFBNode::getAttributeByName( const char *sName ) const {
   if ( arrAttributes != NULL ) {
      unsigned int c = arrAttributes->size();
      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBAttribute *attrib = static_cast<TGFBAttribute *>( arrAttributes->elementAt( i ) );
         if ( attrib->key.match_ansi( sName ) ) {
            return attrib;
         }
      }
   }

   return NULL;
}

void TGFBNode::copyAttributes( TGFBNode *aNode ) {
    unsigned int c = aNode->attributeCount();
    for ( unsigned int i = 0; i < c; i++ ) {
        TGFBAttribute *attrib = new TGFBAttribute( static_cast<TGFBAttribute *>( aNode->arrAttributes->elementAt( i ) ) );
        this->addAttribute( attrib );
    }
}


TGFBNode *TGFBNode::getChildByName( const char *sName ) const {
   if ( arrChildren != NULL ) {
      unsigned int c = arrChildren->size();
      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBNode *child = static_cast<TGFBNode *>( arrChildren->elementAt( i ) );
         if ( child->name.match_ansi( sName ) ) {
            return child;
         }
      }
   }

   return NULL;
}

TGFBNode *TGFBNode::getChildByNameAndAttribute( const char *sName, const char *sAttributeName, const TGFBValue *sAttributeValue ) const {
   if ( arrChildren != NULL ) {
      unsigned int c = arrChildren->size();
      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBNode *child = static_cast<TGFBNode *>( arrChildren->elementAt( i ) );
         if ( child->name.match_ansi( sName ) ) {
            TGFBAttribute *attrib = child->getAttributeByName( sAttributeName );
            if ( attrib != NULL ) {
               if ( attrib->value.equals( sAttributeValue ) ) {
                  return child;
               }
            } else {
               if ( sAttributeValue == NULL ) {
                  return child;
               }
            }
         }
      }
   }

   return NULL;
}

TGFBNode *TGFBNode::findAttributeThroughRecursiveSearch( const char *sAttributeName, const TGFBValue *sAttributeValue ) {
    TGFBAttribute *attrib = this->getAttributeByName( sAttributeName );
    if ( attrib != NULL ) {
        if ( attrib->value.equals( sAttributeValue ) ) {
            return this;
        }
    }

    if ( arrChildren != NULL ) {
        unsigned int c = arrChildren->size();
        for ( unsigned int i = 0; i < c; i++ ) {
            TGFBNode *child = static_cast<TGFBNode *>( arrChildren->elementAt( i ) );

            if ( child != NULL ) {
                TGFBNode *foundNode = child->findAttributeThroughRecursiveSearch( sAttributeName, sAttributeValue );
                if ( foundNode != NULL ) {
                    return foundNode;
                }
            }
        }
    }

    return NULL;
}

TGFBNode *TGFBNode::getChild( unsigned int i ) const {
   if ( arrChildren != NULL ) {
      return static_cast<TGFBNode *>( arrChildren->elementAt( i ) );
   }

   return NULL;
}

TGFBAttribute *TGFBNode::findOrCreateAttribute( const char *sName ) {
   TGFBAttribute *attr = getAttributeByName( sName );
   if ( attr == NULL ) {
      attr = new TGFBAttribute( sName );
      addAttribute( attr );
   }

   return attr;
}

TGFString *TGFBNode::getFirstChildsContent() {
    if ( arrChildren != NULL ) {
        if ( arrChildren->size() != 0 ) {
            TGFBNode *child = static_cast<TGFBNode *>( arrChildren->elementAt(0) );
            if ( child != NULL ) {
                return &child->content;
            }
        }
    }

    return &content;
}

TGFString *TGFBNode::flattenChildContent() {
    TGFString *sNewContent = new TGFString();
    sNewContent->append( &content );

    if ( arrChildren != NULL ) {
        unsigned int c = arrChildren->size();
        for ( unsigned int i = 0; i < c; i++ ) {
            TGFBNode *child = static_cast<TGFBNode *>( arrChildren->elementAt(i) );
            if ( child != NULL ) {
                TGFString *tmp = child->flattenChildContent();
                sNewContent->append( tmp );
                delete tmp;
                if ( (i+1) != c ) {
                    if ( sNewContent->getLength() != 0 ) {
                        sNewContent->append_ansi( " " );
                    }
                }
            }
        }
    }

    return sNewContent;
}

//--------------------------------------------

TGFBNode *TGFBNode::getChildAndTraverse( TGFBNodePath *aPath ) const {
   if ( !aPath->traverse_isEoF() ) {
      unsigned int i = aPath->traverse_next();

      return getChild( i );
   }

   return NULL;
}

TGFBNode *TGFBNode::getChildByPath( TGFBNodePath *aPath ) {
   TGFBNode *newChild = this->getChildAndTraverse( aPath );
   if ( newChild != NULL ) {
      return newChild->getChildByPath( aPath );
   } else {
      return this;
   }
}

