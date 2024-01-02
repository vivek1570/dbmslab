#include "OpenRelTable.h"

#include <cstring>
#include<stdlib.h>
#include<bits/stdc++.h>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
// printf("hello\n");
  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if(!strcmp(RELCAT_RELNAME,relName)){
    return RELCAT_RELID;
  }
  if(!strcmp(ATTRCAT_RELNAME,relName)){
    return ATTRCAT_RELID;
  }
for(int i=2;i<12;i++)
{
  
  if(!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName,relName)==0)
  return i;
}
  return E_RELNOTOPEN;
}

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free=true;
  }
  

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);
  // OpenRelTable::tableMetaInfo[]
  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

    // printf("in open table relid _recid=%d\n",RelCacheTable:)
  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

// set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]

  //RecBuffer relCatBlock(ATTRCAT_BLOCK);
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = ATTRCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  AttrCacheTable::attrCache[RELCAT_RELID]; 
  struct AttrCacheEntry* attrCacheEntry;
  struct AttrCacheEntry* temp;;
  attrCacheEntry=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  AttrCacheTable::attrCache[RELCAT_RELID]=attrCacheEntry;
  attrCatBlock.getRecord(attrCatRecord,0);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
  attrCacheEntry->recId.block=ATTRCAT_BLOCK;
  attrCacheEntry->recId.slot=0;
  for(int i=1;i<6;i++){
    temp=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    temp->next=NULL;
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(temp->attrCatEntry));
    temp->recId.block=ATTRCAT_BLOCK;
    temp->recId.slot=i;
    attrCacheEntry->next=temp;
    attrCacheEntry=attrCacheEntry->next;
  }

  attrCacheEntry->next=NULL;




struct AttrCacheEntry* aattrCacheEntry;
  AttrCacheTable::attrCache[ATTRCAT_RELID] ;
  aattrCacheEntry=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  AttrCacheTable::attrCache[ATTRCAT_RELID]=aattrCacheEntry;
  attrCatBlock.getRecord(attrCatRecord,6);
  
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &aattrCacheEntry->attrCatEntry);
  aattrCacheEntry->recId.block=ATTRCAT_BLOCK;
  aattrCacheEntry->recId.slot=6;
  for(int i=7;i<12;i++){
    temp=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    temp->next=NULL;
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);
    temp->recId.block=ATTRCAT_BLOCK;
    temp->recId.slot=i;
    aattrCacheEntry->next=temp;
    aattrCacheEntry=aattrCacheEntry->next;
  }
  attrCacheEntry->next=NULL;

  tableMetaInfo[0].free=false;
  tableMetaInfo[1].free=false;
  strcpy(tableMetaInfo[0].relName,"RELATIONCAT");
  strcpy(tableMetaInfo[1].relName,"ATTRIBUTECAT");
}


OpenRelTable::~OpenRelTable() {

    for(int i=2;i<MAX_OPEN;i++)
    {
        if(!tableMetaInfo[i].free)
        {
            OpenRelTable::closeRel(i);
        }
    }

    /**** Closing the catalog relations in the relation cache ****/

    //releasing the relation cache entry of the attribute catalog

    if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty)
     {
        RelCatEntry relcatent_attr;
        RelCacheTable::getRelCatEntry(1,&relcatent_attr);
        Attribute record[6];
        
        RelCacheTable::relCatEntryToRecord(&relcatent_attr,record);
        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        

        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId=RelCacheTable::relCache[ATTRCAT_RELID]->recId;
        RecBuffer relCatBlock(4);
        relCatBlock.setRecord(record,1);
        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    }
    // delete
    /* free the memory dynamically allocated to this RelCacheEntry */

    delete RelCacheTable::relCache[1];

    //releasing the relation cache entry of the relation catalog

    // if(/* RelCatEntry of the RELCAT_RELID-th RelCacheEntry has been modified */) 
    if(RelCacheTable::relCache[RELCAT_RELID]->dirty)
    {
        RelCatEntry relcatent_rel;
        RelCacheTable::getRelCatEntry(0,&relcatent_rel);
        Attribute record1[6];
        RelCacheTable::relCatEntryToRecord(&relcatent_rel,record1);

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */

        // declaring an object of RecBuffer class to write back to the buffer
        RecId recid1=RelCacheTable::relCache[RELCAT_RELID]->recId;
        RecBuffer relCatBlock1(recid1.block);
        relCatBlock1.setRecord(record1,recid1.slot);
        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    }
    // free the memory dynamically allocated for this RelCacheEntry
  //   delete RelCacheTable::relCache[RELCAT_RELID];
  //     RelCacheTable::relCache[RELCAT_RELID]=nullptr;
  // printf("hello from ~open 5\n");
    delete RelCacheTable::relCache[0];
  RelCacheTable::relCache[0]=nullptr;
  AttrCacheEntry *p=AttrCacheTable::attrCache[0];
  while(p!=nullptr)
  {
    AttrCacheEntry *t=p->next;
    delete p;
    p=t;
  }
  p=AttrCacheTable::attrCache[1];
  while(p!=nullptr)
  {
    AttrCacheEntry *t=p->next;
    delete p;
    p=t;
  }


  
    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for(int i=0;i<12;i++)
  {
    if(tableMetaInfo[i].free)
    {
      return i;
    }
    // else {
    //   printf("%s\n",tableMetaInfo[i].relName);
    // }
  }
  return E_CACHEFULL;

  // if found return the relation id, else return E_CACHEFULL.
}


int OpenRelTable::openRel(char relName[ATTR_SIZE]){
	int relId = OpenRelTable::getRelId(relName);
	if (relId >= 0 && relId < MAX_OPEN){
		return relId;
	}

	relId = OpenRelTable::getFreeOpenRelTableEntry();
	if (relId == E_CACHEFULL){
		return E_CACHEFULL;
	}

	HeadInfo head;
	Attribute record[RELCAT_NO_ATTRS];

	RelCacheTable::resetSearchIndex(RELCAT_RELID);
	char attrName[ATTR_SIZE] = RELCAT_ATTR_RELNAME;
	Attribute attrVal;
	strcpy (attrVal.sVal, relName);

	RecId relCatId = BlockAccess::linearSearch (RELCAT_RELID, attrName, attrVal, EQ);
	if (relCatId.block == -1 && relCatId.slot == -1){
		return E_RELNOTEXIST;
	}

	tableMetaInfo[relId].free = false;
	strcpy (tableMetaInfo[relId].relName, relName);
	RecBuffer recBuffer(relCatId.block);
	recBuffer.getRecord(record, relCatId.slot);

	RelCacheEntry relCacheEntry;
	RelCacheTable::recordToRelCatEntry (record, &relCacheEntry.relCatEntry);
	
	relCacheEntry.recId = relCatId;
	relCacheEntry.searchIndex.block = -1;
	relCacheEntry.searchIndex.slot = -1;
	relCacheEntry.dirty = false;
	RelCacheTable::relCache[relId] = new RelCacheEntry;
	*RelCacheTable::relCache[relId] = relCacheEntry;

	//insert attrcat
	
	int attrBlock = ATTRCAT_BLOCK;
	AttrCacheEntry *dummy_head = new AttrCacheEntry, *p = dummy_head;
	AttrCacheEntry attrCacheEntry;

	RecId attrRecId;
	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
	char attrCatRelName[ATTR_SIZE];
	strcpy (attrCatRelName, ATTRCAT_ATTR_RELNAME);
	while (true){
		attrRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attrCatRelName, attrVal, EQ);
		if (attrRecId.block == -1 && attrRecId.slot == -1){
			break;
		}
		p->next = new AttrCacheEntry;
		p = p->next;;
		RecBuffer attrCatBuffer(attrRecId.block);
		attrCatBuffer.getRecord (record, attrRecId.slot);
		AttrCacheTable::recordToAttrCatEntry (record, &attrCacheEntry.attrCatEntry);
		attrCacheEntry.dirty = false;
	        attrCacheEntry.recId = attrRecId;
		*p = attrCacheEntry;
	}

	p->next = nullptr;
	AttrCacheTable::attrCache[relId] = dummy_head->next;
	delete dummy_head;
	return relId;
}
// AttrCacheEntry *createAttrCacheEntryList(int size)
// {
// 	AttrCacheEntry *head = nullptr, *curr = nullptr;
// 	head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
// 	size--;
// 	while (size--)
// 	{
// 		curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
// 		curr = curr->next;
// 	}
// 	curr->next = nullptr;

// 	return head;
// }

// int OpenRelTable::openRel(char relName[ATTR_SIZE]) 
// {
// 	// (checked using OpenRelTable::getRelId())
// 	int relId = getRelId(relName);
//   	if(relId >= 0){
//     	// return that relation id;
// 		return relId;
//   	}

//   	// TODO: find a free slot in the Open Relation Table
//     // TODO: using OpenRelTable::getFreeOpenRelTableEntry().

//   	// let relId be used to store the free slot.
// 	relId = OpenRelTable::getFreeOpenRelTableEntry();
//   	if (relId == E_CACHEFULL) return E_CACHEFULL;

//   	/****** Setting up Relation Cache entry for the relation ******/

//   	// TODO: search for the entry with relation name, relName, 
// 	// TODO: in the Relation Catalog using BlockAccess::linearSearch().
//     //* Care should be taken to reset the searchIndex of the relation RELCAT_RELID
//     //* before calling linearSearch().

//   	// relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
// 	Attribute attrVal; strcpy(attrVal.sVal, relName);
// 	RelCacheTable::resetSearchIndex(RELCAT_RELID);

//     Attribute attri;
//   strcpy(attri.sVal,relName);
//   char array[16];
//   strcpy(array,"RelName");
//   RecId relcatRecId =BlockAccess::linearSearch(0,array,attrVal,0);

//   	// RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, attrVal, EQ);

// 	if (relcatRecId.block==-1 && relcatRecId.slot==-1) {
// 		//! the relation is not found in the Relation Catalog
// 		return E_RELNOTEXIST;
// 	}

// 	// TODO: read the record entry corresponding to relcatRecId and create a relCacheEntry
// 	// TODO: on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
// 	// RecBuffer relationBuffer (relcatRecId.block);
//   RecBuffer relationBuffer (RELCAT_BLOCK);
// 	Attribute relationRecord [RELCAT_NO_ATTRS];
// 	RelCacheEntry *relCacheBuffer = nullptr;

// 	relationBuffer.getRecord(relationRecord, relcatRecId.slot);

// 	//* NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
// 	relCacheBuffer = (RelCacheEntry*) malloc (sizeof(RelCacheEntry));
// 	RelCacheTable::recordToRelCatEntry(relationRecord, &(relCacheBuffer->relCatEntry));

// 	// update the recId field of this Relation Cache entry to relcatRecId.
// 	relCacheBuffer->recId.block = relcatRecId.block;
// 	relCacheBuffer->recId.slot = relcatRecId.slot;
	
// 	// use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
// 	RelCacheTable::relCache[relId] = relCacheBuffer;	


//   	/****** Setting up Attribute Cache entry for the relation ******/

// 	// {
// 	// 	RecId attrcatRecId;

// 	// 	read the record entry corresponding to attrcatRecId and create an
// 	// 	Attribute Cache entry on it using RecBuffer::getRecord() and
// 	// 	AttrCacheTable::recordToAttrCatEntry().
// 	// 	update the recId field of this Attribute Cache entry to attrcatRecId.
// 	// 	add the Attribute Cache entry to the linked list of listHead.
// 	// NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
// 	// }

// 	Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

// 	// let listHead be used to hold the head of the linked list of attrCache entries.
// 	AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

// 	int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
// 	head = createAttrCacheEntryList(numberOfAttributes);
// 	attrCacheEntry = head;

// 	// TODO: iterate over all the entries in the Attribute Catalog corresponding to each
// 	// TODO: attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
// 	// * care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
// 	// * corresponding to Attribute Catalog before the first call to linearSearch().

// 	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
// 	while (numberOfAttributes--)
// 	{
// 		// // AttrCacheTable::resetSearchIndex(relId, attr);
// 		// let attrcatRecId store a valid record id an entry of the relation, relName, in the Attribute Catalog.
// 		RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, array, attrVal, EQ);

// 		// RecBuffer attrCatBlock(attrcatRecId.block);
//     RecBuffer attrCatBlock(ATTRCAT_BLOCK);
// 		attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);

// 		AttrCacheTable::recordToAttrCatEntry(
// 			attrCatRecord,
// 			&(attrCacheEntry->attrCatEntry)
// 		);

// 		attrCacheEntry->recId.block = attrcatRecId.block;
// 		attrCacheEntry->recId.slot = attrcatRecId.slot;

// 		attrCacheEntry = attrCacheEntry->next;
// 	}

// 	// set the relIdth entry of the AttrCacheTable to listHead.
// 	AttrCacheTable::attrCache[relId] = head;

//   	/****** Setting up metadata in the Open Relation Table for the relation******/

// 	// update the relIdth entry of the tableMetaInfo with free as false and
// 	// relName as the input.
// 	tableMetaInfo[relId].free = false;
// 	strcpy(tableMetaInfo[relId].relName, relName);

//   	return relId;
// }


// int OpenRelTable::closeRel(int relId) {
//    if (relId==0 || relId==1/* rel-id corresponds to relation catalog or attribute catalog*/) {
//     return E_NOTPERMITTED;
//   }

//   if ( 0 >relId ||relId>=12 ) {
//     return E_OUTOFBOUND;
//   }
//   if(tableMetaInfo[relId].free)
//   return E_RELNOTOPEN;

//   /****** Releasing the Relation Cache entry of the relation ******/
//   if (RelCacheTable::relCache[relId]->dirty/* RelCatEntry of the relId-th Relation Cache entry has been modified */)
//   {
    
//     Attribute record[6];
//     RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry),record);
    
//     int slot=RelCacheTable::relCache[relId]->recId.slot;
//     int blk=RelCacheTable::relCache[relId]->recId.block;
//     // printf("in open rel blk:%d  slot:%d\n",blk,slot);
//     // blk=4;
//     RecBuffer relCatBlock(blk);


//     relCatBlock.setRecord(record,slot);
//   // printf("hello 7\n");
//     // Write back to the buffer using relCatBlock.setRecord() with recId.slot
//   }

//   /****** Releasing the Attribute Cache entry of the relation ******/
//   // for all the entries in the linked list of the relIdth Attribute Cache entry.
//   for (AttrCacheEntry* entry = AttrCacheTable::attrCache[relId]; entry != nullptr; entry = entry->next)
//     {
//         // if the entry has been modified:
//         if(entry->dirty)
//         {
            
//               Attribute record[6];
//              AttrCacheTable::attrCatEntryToRecord(&(entry->attrCatEntry),record);
//              int slot=entry->recId.slot;
//              int block=entry->recId.block;
//              RecBuffer attrCatblock(block);
//              attrCatblock.setRecord(record,slot);
//         }

//         // free the memory dynamically alloted to this entry in Attribute
//         // Cache linked list and assign nullptr to that entry
//         delete entry;
//         entry=nullptr;
//     }

//  delete RelCacheTable::relCache[relId];
//   RelCacheTable::relCache[relId]=nullptr;
  

//   /****** Set the Open Relation Table entry of the relation as free ******/

//   // update `metainfo` to set `relId` as a free slot
//   // printf("rel name in openrel closerel: %s",RelCacheTable::relCache[relId]->relCatEntry.relName);
//   OpenRelTable::tableMetaInfo[relId].free=true;
//   // OpenRelTable::tableMetaInfo[relId].relName=

//   return SUCCESS;
// }
int OpenRelTable::closeRel(int relId) {
  	if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) return E_NOTPERMITTED;

  	if (0 > relId || relId >= MAX_OPEN) return E_OUTOFBOUND;

  	if (tableMetaInfo[relId].free) return E_RELNOTOPEN;

	if (RelCacheTable::relCache[relId]->dirty == true) {
		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute relCatBuffer [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

		// declaring an object of RecBuffer class to write back to the buffer
		RecId recId = RelCacheTable::relCache[relId]->recId;
		RecBuffer relCatBlock(recId.block);

		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatBuffer, RelCacheTable::relCache[relId]->recId.slot);
	}

	// free the memory allocated in the relation and attribute caches which was
	// allocated in the OpenRelTable::openRel() function
	free (RelCacheTable::relCache[relId]);
	
	// // RelCacheEntry *relCacheBuffer = RelCacheTable::relCache[relId];

	//* because we are not modifying the attribute cache at this stage,
	//* write-back is not required. We will do it in subsequent
  	//* stages when it becomes needed)

	AttrCacheEntry *head = AttrCacheTable::attrCache[relId];
	AttrCacheEntry *next = head->next;

	while (true) {
		if (head->dirty)
		{
			Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
			AttrCacheTable::attrCatEntryToRecord(&(head->attrCatEntry), attrCatRecord);

			RecBuffer attrCatBlockBuffer (head->recId.block);
			attrCatBlockBuffer.setRecord(attrCatRecord, head->recId.slot);
		}


		free (head);
		head = next;

		if (head == NULL) break;
		next = next->next;
	}

	// update `tableMetaInfo` to set `relId` as a free slot
	// update `relCache` and `attrCache` to set the entry at `relId` to nullptr
	tableMetaInfo[relId].free = true;
	RelCacheTable::relCache[relId] = nullptr;
	AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}