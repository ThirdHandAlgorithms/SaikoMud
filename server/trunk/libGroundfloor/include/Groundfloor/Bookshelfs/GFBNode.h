
#ifndef __GFBNode_H
#define __GFBNode_H

class TGFBNode;
class TGFBNodePath;

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFVector.h"
#include "GFBAttribute.h"
#include <vector>

class TGFBNodePath: public TGFFreeable {
   protected:
      std::vector<unsigned int> path;

      int current;
   public:
      /// path initialization to length zero
      TGFBNodePath();
      ~TGFBNodePath();

      /// appends i to path
      void append( unsigned int i );
      /// clears entire path vector
      void clear();

      /// resets path pointer
      unsigned int traverse_reset();

      /// returns previous value
      unsigned int traverse_previous();
      /// returns first or next value
      unsigned int traverse_next();

      /// returns true if path pointer is at the beginning of the path
      bool traverse_isBoF();
      /// returns true if path pointer is at the end of the path, or when the path length is zero
      bool traverse_isEoF();

      void readFromString( const char *sPath );
};

/// TGFBNode is a class tailored to represent simple XML Nodes containing attributes in key=value format, and nesting of child-nodes
class TGFBNode: public TGFFreeable {
   protected:
      TGFVector *arrAttributes;
      TGFVector *arrChildren;
      TGFBNode *parent;

   public:
      /// initialize node with no name
      TGFBNode();
      /// initialize node with given name
      TGFBNode( const char *sName );
      /// automatically deletes all attributes and child-nodes
      ~TGFBNode();

      TGFString name;
      TGFString content;

      TGFBNode *getParentNode();

      /// adds attribute to the node, given attribute is automatically deleted
      void addAttribute( TGFBAttribute *anAttrib );
      /// adds child-node to the node, given node is automatically deleted
      void addChildNode( TGFBNode *aNode );

      /// number of child-nodes
      unsigned int childCount() const;
      /// number of attributes
      unsigned int attributeCount() const;

      /// copy attributes from given node
      void copyAttributes( TGFBNode *aNode );

      /// returns attribute at index i, returns NULL when out of bounds
      TGFBAttribute *getAttribute( unsigned int i ) const;
      /// returns attribute if found by name
      TGFBAttribute *getAttributeByName( const char *sName ) const;
      /// returns node at index i, returns NULL when out of bounds
      TGFBNode *getChild( unsigned int i ) const;

      /// returns node that has the given name, or NULL
      TGFBNode *getChildByName( const char *sName ) const;

      /// returns node that has the given name and where the attribute with given name has the given value
      TGFBNode *getChildByNameAndAttribute( const char *sName, const char *sAttributeName, const TGFBValue *sAttributeValue ) const;

      /// returns node that contains the given attribute with the given value, searches the complete tree
      TGFBNode *findAttributeThroughRecursiveSearch( const char *sAttributeName, const TGFBValue *sAttributeValue );

      /// find or create and add an attribute with given name
      TGFBAttribute *findOrCreateAttribute( const char *sName );

      /// traverses 1 position in given NodePath and uses current path reference as index, returns NULL if anything went wrong
      TGFBNode *getChildAndTraverse( TGFBNodePath *aPath ) const;

      /// traverses the whole path and returns the last valid child-node in the path, if no valid nodes are found in the path; "this" is returned
      TGFBNode *getChildByPath( TGFBNodePath *aPath );

      /// links pointer to content from either the first child, or itself if no children
      TGFString *getFirstChildsContent();

      /// flatten all child contents into a newly allocated string
      TGFString *flattenChildContent();

};

#endif // __GFBNode_H
