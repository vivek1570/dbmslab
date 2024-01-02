#include "Schema.h"

#include <cmath>
#include <cstring>
#include<bits/stdc++.h>

int Schema::openRel(char relName[ATTR_SIZE]) {
  int ret = OpenRelTable::openRel(relName);

  // the OpenRelTable::openRel() function returns the rel-id if successful
  // a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
  // error codes will be negative
  if(ret >= 0){
    return SUCCESS;
  }

  //otherwise it returns an error message
  return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE]) {
  if (relName=="ATTRIBUTECAT" ||relName=="RELATIONCAT") {
    return E_NOTPERMITTED;
  }

  // this function returns the rel-id of a relation if it is open or
  // E_RELNOTOPEN if it is not. we will implement this later.
  int relId = OpenRelTable::getRelId(relName);


  if (relId==E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  return OpenRelTable::closeRel(relId);
}

int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]) {
  if(strcmp(oldRelName,"RELATIONCAT")==0 ||
    strcmp(oldRelName,"ATTRIBUTECAT")==0||
    strcmp(newRelName,"RELATIONCAT")==0||
    strcmp(newRelName,"ATTRIBUTECAT")==0
  )
  {
    return E_NOTPERMITTED;
  }
    // if the oldRelName or newRelName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

int x=OpenRelTable::getRelId(oldRelName);
if(x>=0 && x<12)
{
  return E_RELOPEN;
}
// return 1;
    // if the relation is open
    //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
    //    return E_RELOPEN
    int retVal = BlockAccess::renameRelation(oldRelName, newRelName);
    return retVal;
}

int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName) {
   if(strcmp(oldAttrName,"RELATIONCAT")==0 ||
    strcmp(oldAttrName,"ATTRIBUTECAT")==0||
    strcmp(newAttrName,"RELATIONCAT")==0||
    strcmp(newAttrName,"ATTRIBUTECAT")==0
  )
  {
    return E_NOTPERMITTED;
  }
    // if the relName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
  int x=OpenRelTable::getRelId(oldAttrName);
  if(x>=0 && x<12)
  {
    return E_RELOPEN;
  }
    // if the relation is open
        //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
        //    return E_RELOPEN
  int retVal = BlockAccess::renameAttribute(relName,oldAttrName, newAttrName);
    return retVal;
    // Call BlockAccess::renameAttribute with appropriate arguments.

    // return the value returned by the above renameAttribute() call
}


int Schema::createRel(char relName[],int nAttrs, char attrs[][ATTR_SIZE],int attrtype[]){

    // declare variable relNameAsAttribute of type Attribute
    // copy the relName into relNameAsAttribute.sVal
    Attribute relNameAsAttribute;
    strcpy(relNameAsAttribute.sVal,relName);
    
    // declare a variable targetRelId of type RecId
    RecId targetRecId;
    targetRecId.block=-1;
    targetRecId.slot=-1;
    
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    // Reset the searchIndex using RelCacheTable::resetSearhIndex()
    // Search the relation catalog (relId given by the constant RELCAT_RELID)
    // for attribute value attribute "RelName" = relNameAsAttribute using
    // BlockAccess::linearSearch() with OP = EQ
    
     char name[16];
    strcpy(name,"RelName");
    targetRecId=BlockAccess::linearSearch(0,name,relNameAsAttribute,0);

    // if a relation with name `relName` already exists  ( linearSearch() does
    //                                                     not return {-1,-1} )
    //     return E_RELEXIST;
    
    if(targetRecId.block!=-1 && targetRecId.slot!=-1)
    {
      return E_RELEXIST;
    }

    // compare every pair of attributes of attrNames[] array
    // if any attribute names have same string value,
    //     return E_DUPLICATEATTR (i.e 2 attributes have same value)
    for(int i=0;i<nAttrs;i++)
    {
        for(int j=0;j<nAttrs;j++)
        {
          if(i!=j && strcmp(attrs[i],attrs[j])==0)
          {
            return E_DUPLICATEATTR;
            break;
          }
        }
    }

    /* declare relCatRecord of type Attribute which will be used to store the
       record corresponding to the new relation which will be inserted
       into relation catalog */
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // fill relCatRecord fields as given below
    // offset RELCAT_REL_NAME_INDEX: relName
    // offset RELCAT_NO_ATTRIBUTES_INDEX: numOfAttributes
    // offset RELCAT_NO_RECORDS_INDEX: 0`
    // offset RELCAT_FIRST_BLOCK_INDEX: -1
    // offset RELCAT_LAST_BLOCK_INDEX: -1
    // offset RELCAT_NO_SLOTS_PER_BLOCK_INDEX: floor((2016 / (16 * nAttrs + 1)))
    // (number of slots is calculated as specified in the physical layer docs)
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,relName);
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal=nAttrs;
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal=0;
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal=-1;
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal=-1;
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal=floor((2016 / (16 * nAttrs + 1)));
    
    // retVal = BlockAccess::insert(RELCAT_RELID(=0), relCatRecord);
    int retVal=BlockAccess::insert(0,relCatRecord);
    
    if(retVal!=SUCCESS)
    return retVal;
    // if BlockAccess::insert fails return retVal
    // (this call could fail if there is no more space in the relation catalog)

    // iterate through 0 to numOfAttributes - 1 :
    for(int i=0;i<nAttrs;i++)
    {
        /* declare Attribute attrCatRecord[6] to store the attribute catalog
           record corresponding to i'th attribute of the argument passed*/
           Attribute attrCatRecord[6];
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relName);
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrs[i]);
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal=attrtype[i];
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal=-1;
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal=-1;
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal=i;
        // retVal = BlockAccess::insert(ATTRCAT_RELID(=1), attrCatRecord);
        retVal=BlockAccess::insert(1,attrCatRecord);
        if(retVal!=SUCCESS)
        {
          retVal;
        }
        /* if attribute catalog insert fails:
             delete the relation by calling deleteRel(targetrel) of schema layer
             return E_DISKFULL
             // (this is necessary because we had already created the
             //  relation catalog entry which needs to be removed)
        */
    }

    return SUCCESS;
}
int Schema::deleteRel(char *relName) {
  if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0)
    return E_NOTPERMITTED;
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
  int x=OpenRelTable::getRelId(relName);
  // printf("x is:%d\n",x);
    // get the rel-id using appropriate method of OpenRelTable class by
    // passing relation name as argument

    // if relation is opened in open relation table, return E_RELOPENa
  //  if(OpenRelTable::tableMetaInfo[x].free==false)
  if(x>=0 && x<12)
  return E_RELOPEN;

    

    // Call BlockAccess:  :deleteRelation() with appropriate argument.
    return BlockAccess::deleteRelation(relName);

    // return the value returned by the above deleteRelation() call

    /* the only that should be returned from deleteRelation() is E_RELNOTEXIST.
       The deleteRelation call may return E_OUTOFBOUND from the call to
       loadBlockAndGetBufferPtr, but if your implementation so far has been
       correct, it should not reach that point. That error could only occur
       if the BlockBuffer was initialized with an invalid block number.
    */
}

int Schema::createIndex(char relName[ATTR_SIZE],char attrName[ATTR_SIZE]){
    // if the relName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    // get the relation's rel-id using OpenRelTable::getRelId() method

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)

    // create a bplus tree using BPlusTree::bPlusCreate() and return the value
     if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0)
    return E_NOTPERMITTED;
     int x=OpenRelTable::getRelId(relName);
     if(x<0 || x>11)
     return E_RELNOTOPEN;
    return BPlusTree::bPlusCreate(x, attrName);
}

int Schema::dropIndex(char *relName, char *attrName) {
    // if the relName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    // get the rel-id using OpenRelTable::getRelId()

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)

    // get the attribute catalog entry corresponding to the attribute
    // using AttrCacheTable::getAttrCatEntry()

    // if getAttrCatEntry() fails, return E_ATTRNOTEXIST
     if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0)
    return E_NOTPERMITTED;
     int x=OpenRelTable::getRelId(relName);
     if(x<0 || x>11)
     return E_RELNOTOPEN;
     AttrCatEntry attrcatbuf;
     int ret=AttrCacheTable::getAttrCatEntry(x,attrName,&attrcatbuf);
     if(ret!=SUCCESS)
     return ret;

    int rootBlock = attrcatbuf.rootBlock;/* get the root block from attrcat entry */

    // if (/* attribute does not have an index (rootBlock = -1) */) 
    if(rootBlock==-1)
    {
        return E_NOINDEX;
    }

    // destroy the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
    BPlusTree::bPlusDestroy(rootBlock);

    // set rootBlock = -1 in the attribute cache entry of the attribute using
    // AttrCacheTable::setAttrCatEntry()
    attrcatbuf.rootBlock=-1;
    AttrCacheTable::setAttrCatEntry(x,attrName,&attrcatbuf);

    return SUCCESS;
}