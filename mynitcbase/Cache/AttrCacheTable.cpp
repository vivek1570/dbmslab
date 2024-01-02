#include "AttrCacheTable.h"

#include <cstring>
#include<bits/stdc++.h>


AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

/* returns the attrOffset-th attribute for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/

int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise

  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // if there's no entry at the rel-id
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }

  // traverse the linked list of attribute cache entries
  AttrCacheEntry* po=attrCache[relId];
  // while(po!=NULL)
  // {
  //   printf("relname=%s\n",po->attrCatEntry.relName);
  //   printf("attrName=%s\n",po->attrCatEntry.attrName);
  //   po=po->next;
  // }
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    //printf("hello %d %d %d \n",attrCache[relId]->attrCatEntry.offset,attrOffset,entry->attrCatEntry.offset);
    if (entry->attrCatEntry.offset == attrOffset) {
      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf =entry->attrCatEntry;
     // printf("hi %s off: %d",attrCache[relId]->attrCatEntry.attrName,attrCatBuf->offset);
      return SUCCESS;
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}

int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {

  // check that relId is valid and corresponds to an open relation
  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }  
  // iterate over the entries in the attribute cache and set attrCatBuf to the entry that
  //    matches attrName
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    //printf("hello %d %d %d \n",attrCache[relId]->attrCatEntry.offset,attrOffset,entry->attrCatEntry.offset);
    std::string a=entry->attrCatEntry.attrName;
    std::string b=attrName;
    if (a==b) {
      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf =entry->attrCatEntry;
     // printf("hi %s off: %d",attrCache[relId]->attrCatEntry.attrName,attrCatBuf->offset);
      return SUCCESS;
    }
  }

  // no attribute with name attrName for the relation
  return E_ATTRNOTEXIST;
}

/* Converts a attribute catalog record to AttrCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct AttrCatEntry type.
*/
void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                          AttrCatEntry* attrCatEntry) {
  strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
  strcpy(attrCatEntry->attrName, record[1].sVal);

 attrCatEntry->attrType = (int)record[2].nVal;
 attrCatEntry->offset = (int)record[5].nVal;
 attrCatEntry->primaryFlag = (bool)record[3].nVal;
 attrCatEntry->rootBlock = (int)record[4].nVal;

  // copy the rest of the fields in the record to the attrCacheEntry struct
}

int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  // if(/*relId is outside the range [0, MAX_OPEN-1]*/) 
  if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }

  // if(/*entry corresponding to the relId in the Attribute Cache Table is free*/) 
  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }

  // for(/* each attribute corresponding to relation with relId */)
  // for(int )
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) 
  {
    // if (/* attrName/offset field of the AttrCatEntry
    //     is equal to the input attrName/attrOffset */)
    if(strcmp(attrName,entry->attrCatEntry.attrName)==0)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      searchIndex->block=entry->searchIndex.block;
      searchIndex->index=entry->searchIndex.index;
      return SUCCESS;
    }
  }
  return E_ATTRNOTEXIST;
}

int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {

 if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }

  // if(/*entry corresponding to the relId in the Attribute Cache Table is free*/) 
  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }

  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) 
  {
    // if (/* attrName/offset field of the AttrCatEntry
    //     is equal to the input attrName/attrOffset */)
    if(attrOffset==entry->attrCatEntry.offset)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      searchIndex->block=entry->searchIndex.block;
      searchIndex->index=entry->searchIndex.index;
      return SUCCESS;
    }
  }
  return E_ATTRNOTEXIST;
}

int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }
  // if(/*entry corresponding to the relId in the Attribute Cache Table is free*/) 
  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) 
  {
    // if (/* attrName/offset field of the AttrCatEntry
    //     is equal to the input attrName/attrOffset */)
    if(strcmp(attrName,entry->attrCatEntry.attrName)==0)
    {
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.
    entry->searchIndex.block=searchIndex->block;
    entry->searchIndex.index=searchIndex->index;
      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}

int AttrCacheTable::setSearchIndex(int relId,int attrOffset, IndexId *searchIndex) {

  if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }
  // if(/*entry corresponding to the relId in the Attribute Cache Table is free*/) 
  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) 
  {
    // if (/* attrName/offset field of the AttrCatEntry
    //     is equal to the input attrName/attrOffset */)
    if(attrOffset==entry->attrCatEntry.offset)
    {
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.
    entry->searchIndex.block=searchIndex->block;
    entry->searchIndex.index=searchIndex->index;
      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}
int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {

  // declare an IndexId having value {-1, -1}
  IndexId Index_Id={-1,-1};
  return AttrCacheTable::setSearchIndex(relId,attrName,&Index_Id);
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
}
int AttrCacheTable::resetSearchIndex(int relId,int attrOffset) {

  // declare an IndexId having value {-1, -1}
  IndexId index_Id={-1,-1};
  return AttrCacheTable::setSearchIndex(relId,attrOffset,&index_Id);
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
}

int AttrCacheTable::setAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf) {

  // if(/*relId is outside the range [0, MAX_OPEN-1]*/) 
  if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }

  // if(/*entry corresponding to the relId in the Attribute Cache Table is free*/) 
  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }

  // for(/* each attribute corresponding to relation with relId */)
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next)
  {
    if(strcmp(attrName,entry->attrCatEntry.attrName)==0)
    {
      // copy the attrCatBuf to the corresponding Attribute Catalog entry in
      // the Attribute Cache Table.
      strcpy(entry->attrCatEntry.attrName,attrCatBuf->attrName);
      entry->attrCatEntry.attrType=attrCatBuf->attrType;
      entry->attrCatEntry.offset=attrCatBuf->offset;
      entry->attrCatEntry.primaryFlag=attrCatBuf->primaryFlag;
      strcpy(entry->attrCatEntry.relName,attrCatBuf->relName);
      entry->attrCatEntry.rootBlock=attrCatBuf->rootBlock;

      // set the dirty flag of the corresponding Attribute Cache entry in the
      // Attribute Cache Table.
      entry->dirty=true;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}

int AttrCacheTable::setAttrCatEntry(int relId,int attrOffset, AttrCatEntry *attrCatBuf) {

  if(relId<0 || relId>11)
  {
    return E_OUTOFBOUND;
  }

  if(attrCache[relId]==nullptr)
  {
    return E_RELNOTOPEN;
  }

  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next)
  {
    if(attrOffset==entry->attrCatEntry.offset)
    {
      // copy the attrCatBuf to the corresponding Attribute Catalog entry in
      // the Attribute Cache Table.

      // set the dirty flag of the corresponding Attribute Cache entry in the
      // Attribute Cache Table.
      strcpy(entry->attrCatEntry.attrName,attrCatBuf->attrName);
      entry->attrCatEntry.attrType=attrCatBuf->attrType;
      entry->attrCatEntry.offset=attrCatBuf->offset;
      entry->attrCatEntry.primaryFlag=attrCatBuf->primaryFlag;
      strcpy(entry->attrCatEntry.relName,attrCatBuf->relName);
      entry->attrCatEntry.rootBlock=attrCatBuf->rootBlock;

      entry->dirty=true;
      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}

void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry * x,union Attribute record[])
{
  strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal,x->attrName);
  strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal,x->relName);
  record[ATTRCAT_ATTR_TYPE_INDEX].nVal=x->attrType;
  record[3].nVal=x->primaryFlag;
  record[5].nVal=x->offset;
  record[4].nVal=x->rootBlock;
}