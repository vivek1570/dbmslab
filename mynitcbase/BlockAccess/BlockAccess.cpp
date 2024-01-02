#include "BlockAccess.h"

#include <cstring>
#include<bits/stdc++.h>



RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    // let block and slot denote the record id of the record being currently checked
int block=prevRecId.block;
int slot=prevRecId.slot;
    // if the current search index record is invalid(i.e. both block and slot = -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry relCatBuf_temp;
        RelCacheTable::getRelCatEntry(relId,&relCatBuf_temp);
        // block = first record block of the relation
        // slot = 0
        block=relCatBuf_temp.firstBlk;
        slot=0;
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)

        // block = search index's block
        // slot = search index's slot + 1
       slot+=1;
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    while (block != -1)
    {
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */

RecBuffer relCatBlock(block);
        

// RecBuffer::RecBuffer(int block) : BlockBuffer::BlockBuffer(block) {}

        // get the record with id (block, slot) using RecBuffer::getRecord()
       
        // get header of the block using RecBuffer::getHeader() function
        struct HeadInfo head;
       

        relCatBlock.getHeader(&head);
         Attribute attrCatRecord[head.numAttrs];
        relCatBlock.getRecord(attrCatRecord,slot);
        // RecBuffer::getHeader()
        // get slot map of the block using RecBuffer::getSlotMap() function
        //printf("abcd\n");
        unsigned char slot_map[head.numSlots];
        relCatBlock.getSlotMap(slot_map); 
        //printf("efgh\n");
        // If slot >= the number of slots per block(i.e. no more slots in this block)
        if(slot>=head.numSlots)
        {
            // update block = right block of block
            // update slot = 0
           block=head.rblock;
            slot=0;
            continue;  // continue to the beginning of this while loop
        }

        // if slot is free skip the loop
        // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        if(*(slot_map+slot)==SLOT_UNOCCUPIED)
        {
            slot++;
            continue;
            // increment slot and continue to the next record slot
        }

        // compare record's attribute value to the the given attrVal as below:
        /*
            firstly get the attribute offset for the attrName attribute
            from the attribute cache entry of the relation using
            AttrCacheTable::getAttrCatEntry()
        */
        /* use the attribute offset to get the value of the attribute from
           current record */
// int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {
    AttrCatEntry attrCatBuf;
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatBuf);
            
        int cmpVal; 
         // will store the difference between the attributes
        // set cmpVal using compareAttrs()
        int type=(attrVal.nVal==NUMBER)?0:1;
        cmpVal=compareAttrs(attrCatRecord[attrCatBuf.offset],attrVal,attrCatBuf.attrType);
        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
            RecId searchIndex;
            searchIndex.block=block;
            searchIndex.slot=slot;
            RelCacheTable::setSearchIndex(relId,&searchIndex);

            return {block,slot};
        }

        slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return {-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(0);
    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal,newName);
    char array[16];
    const char* source="RelName";
    strcpy(array,source);
    RecId x=BlockAccess::linearSearch(0,array,newRelationName,0);
    // search the relation catalog for an entry with "RelName" = newRelationName

    if(x.block!=-1 && x.slot!=-1)
    return E_RELEXIST;
    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;

    RelCacheTable::resetSearchIndex(0); 
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal,oldName);
    RecId y=BlockAccess::linearSearch(0,array,oldRelationName,0);
    // search the relation catalog for an entry with "RelName" = oldRelationName

    if(y.block==-1 && y.slot==-1)
    return E_RELNOTEXIST;
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
//    printf("block=%d slot=%d\n",y.block,y.slot);
   RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[6];
    relCatBlock.getRecord(relCatRecord,y.slot);
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,newName);
    // printf("rel anem=%s",relCatRecord[0].sVal);
    relCatBlock.setRecord(relCatRecord,y.slot);
    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
   RelCacheTable::resetSearchIndex(1);
    int numattr=relCatRecord[1].nVal;
     RecBuffer attrcatblock(ATTRCAT_BLOCK);
     Attribute attrcatrecord[6];
     RecId res;
    for(int i=0;i<numattr;i++)
    {
    res= BlockAccess::linearSearch(1, array, oldRelationName, 0);
        attrcatblock.getRecord(attrcatrecord,res.slot);
        strcpy(attrcatrecord[0].sVal,newName);
        attrcatblock.setRecord(attrcatrecord,res.slot);
    }
    //     return SUCCESS;
    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord

return SUCCESS;

}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
       RelCacheTable::resetSearchIndex(0);

    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal,relName);
    char name[16];
    strcpy(name,"RelName");
    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    RecId res;
    res= BlockAccess::linearSearch(0,name,relNameAttr,0);
    if(res.block==-1 && res.slot==-1)
    {
        return E_RELNOTEXIST;
    }

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(1);
    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
       RecBuffer attrcatblock(ATTRCAT_BLOCK);
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
            res=BlockAccess::linearSearch(1,name,relNameAttr,0);
        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        if(res.block==-1 && res.slot==-1)
        break;

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        attrcatblock.getRecord(attrCatEntryRecord,res.slot);
        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if(strcmp(attrCatEntryRecord[1].sVal,oldName)==0)
        {
            attrToRenameRecId.block=res.block;
            attrToRenameRecId.slot=res.slot;
        }
        if(strcmp(attrCatEntryRecord[1].sVal,newName)==0)
        return E_ATTREXIST;
        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.block==-1 && attrToRenameRecId.slot==-1)
    return E_ATTRNOTEXIST;
    
    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer block_need(attrToRenameRecId.block);
    block_need.getRecord(attrCatEntryRecord,attrToRenameRecId.slot);
    strcpy(attrCatEntryRecord[1].sVal,newName);
    block_need.setRecord(attrCatEntryRecord,attrToRenameRecId.slot);

    return SUCCESS;
}


int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relcatbuff;
    RelCacheTable::getRelCatEntry(relId,&relcatbuff);
    // printf ("inserting into relId: %d\n", relId);

    int blockNum = relcatbuff.firstBlk;
    /* first record block of the relation (from the rel-cat entry)*/
    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relcatbuff.numSlotsPerBlk;/* number of slots per record block */
    int numOfAttributes = relcatbuff.numAttrs;/* number of attributes of the relation */

    int prevBlockNum = -1;/* block number of the last element in the linked list = -1 */
    // printf("num of slots in relcat=%d\n",)
    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
   int slot=0;
   int f=1;
    while (blockNum != -1) {
        RecBuffer relCatBlock(blockNum);

         struct HeadInfo head;
       

        relCatBlock.getHeader(&head);

        unsigned char slot_map[numOfSlots];
        relCatBlock.getSlotMap(slot_map); 
         if(slot>=numOfSlots)
        {
            // update block = right block of block
            // update slot = 0
            prevBlockNum=blockNum;
            blockNum=head.rblock;
            slot=0;
            continue;  // continue to the beginning of this while loop
        }
        if(*(slot_map+slot)==SLOT_UNOCCUPIED)
        {
             rec_id.block=blockNum;
             rec_id.slot=slot;
            // RelCacheTable::setSearchIndex(relId,&rec_id);
            break;
        }
        slot++;

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */

        /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */
        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
    }
    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id.block==-1 && rec_id.slot==-1)
    {
        if(relId==0)
        return E_MAXRELATIONS;
        RecBuffer newBlock; 
        int ret=newBlock.getBlockNum(); 
        if (ret == E_DISKFULL) {
            return E_DISKFULL;
        }
        rec_id.block=ret;
        rec_id.slot=0;

       struct HeadInfo head;
       newBlock.getHeader(&head);
       head.blockType=REC;
       head.pblock=-1;
       if(prevBlockNum==-1)
       head.lblock=-1;
       else head.lblock=prevBlockNum;
       head.rblock=-1;
       head.numEntries=0;
       head.numSlots=numOfSlots;
       head.numAttrs=numOfAttributes;
        newBlock.setHeader(&head);

       int fg=numOfSlots;
       unsigned char slotMap[fg];
       for(int i=0;i<fg;i++)
       {
        slotMap[i]=SLOT_UNOCCUPIED;
       }
       newBlock.setSlotMap(slotMap);
        // if prevBlockNum != -1
        if(prevBlockNum!=-1)
        {
            RecBuffer prevblk(prevBlockNum);
            struct HeadInfo head;
            prevblk.getHeader(&head);
            head.rblock=rec_id.block;
            prevblk.setHeader(&head);
        }
        else
        {
        
            relcatbuff.firstBlk=ret;
            
            // RelCacheTable::relCache[relId]->recId.block=rec_id.block;

            RelCacheTable::setRelCatEntry(relId,&relcatbuff);
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)
        }
        relcatbuff.lastBlk=ret;
        RelCacheTable::setRelCatEntry(relId,&relcatbuff);
        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
    }
    // create a RecBuffer object for rec_id.blockF
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer rec_id_blk(rec_id.block);
    rec_id_blk.setRecord(record,rec_id.slot);
    // printf("%d",)
   
    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
       unsigned char sloti[numOfSlots];
       rec_id_blk.getSlotMap(sloti);
       sloti[rec_id.slot]=SLOT_OCCUPIED;
        // for(int i=0;i<numOfSlots;i++)
        // {
        //     printf("h=%d ",(int)sloti[i]);
        // }
        // printf("in blk")

       rec_id_blk.setSlotMap(sloti);
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)

    struct HeadInfo rec_head;
    rec_id_blk.getHeader(&rec_head);

    rec_head.numEntries+=1;
    rec_id_blk.setHeader(&rec_head);
    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    relcatbuff.numRecs+=1;

    RelCacheTable::setRelCatEntry(relId,&relcatbuff);


    /* B+ Tree Insertions */
    // (the following section is only relevant once indexing has been implemented)

    int flag = SUCCESS;
    // Iterate over all the attributes of the relation
    // (let attrOffset be iterator ranging from 0 to numOfAttributes-1)
    for(int i=0;i<numOfAttributes;i++)
    {
        // get the attribute catalog entry for the attribute from the attribute cache
        // (use AttrCacheTable::getAttrCatEntry() with args relId and attrOffset)
        AttrCatEntry attrcat_i;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrcat_i);

        // get the root block field from the attribute catalog entry
        int rootblk=attrcat_i.rootBlock;


        // if index exists for the attribute(i.e. rootBlock != -1)
        if(rootblk!=-1)
        {
            /* insert the new record into the attribute's bplus tree using
             BPlusTree::bPlusInsert()*/

            int retVal = BPlusTree::bPlusInsert(relId, attrcat_i.attrName,record[i], rec_id);

            if (retVal == E_DISKFULL) {
                //(index for this attribute has been destroyed)
                flag = E_INDEX_BLOCKS_RELEASED;
            }
        }
    }

    return flag;
}

int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    RecId recId;

    /* get the attribute catalog entry from the attribute cache corresponding
    to the relation with Id=relid and with attribute_name=attrName  */
    AttrCatEntry attrcat;
    int x=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcat);
    if(x!=SUCCESS)
    return x;
    // if this call returns an error, return the appropriate error code

    // get rootBlock from the attribute catalog entry
    /* if Index does not exist for the attribute (check rootBlock == -1) */
    if(attrcat.rootBlock==-1) {

        /* search for the record id (recid) corresponding to the attribute with
           attribute name attrName, with value attrval and satisfying the
           condition op using linearSearch()
        */
       recId=BlockAccess::linearSearch(relId,attrName,attrVal,op);
    }

    else{
        // (index exists for the attribute)

        /* search for the record id (recid) correspoding to the attribute with
        attribute name attrName and with value attrval and satisfying the
        condition op using BPlusTree::bPlusSearch() */
        recId=BPlusTree::bPlusSearch(relId,attrName,attrVal,op);
    }


    // if there's no record satisfying the given condition (recId = {-1, -1})
    //     return E_NOTFOUND;
    if(recId.block==-1 && recId.slot==-1)
    return E_NOTFOUND;

     RecBuffer blk(recId.block);
   blk.getRecord(record,recId.slot);

    return SUCCESS;
    /* Copy the record with record id (recId) to the record buffer (record).
       For this, instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */

}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
    if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0)
    return E_NOTPERMITTED;
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
       RelCacheTable::resetSearchIndex(0);

    Attribute relNameAttr; // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName
    strcpy(relNameAttr.sVal,relName);

    //  linearSearch on the relation catalog for RelName = relNameAttr
    char name[16];
    strcpy(name,"RelName");
    RecId y=linearSearch(0,name,relNameAttr,0);
    RecId f_blk=y;

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST
    if(y.block==-1 && y.slot==-1)
    return E_RELNOTEXIST;

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */
    //    RecBuffer recbl(RELCAT_BLOCK);
     RecBuffer relCatBlock(RELCAT_BLOCK);

       relCatBlock.getRecord(relCatEntryRecord,y.slot);
        int firstblk=relCatEntryRecord[3].nVal;
        int numAttrs=relCatEntryRecord[1].nVal;
    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    /*

     Delete all the record blocks of the relation
    */
   while(firstblk!=-1)
   {
    RecBuffer blki(firstblk);
    struct HeadInfo head;
    blki.getHeader(&head);
    int bl=head.rblock;
    blki.releaseBlock();
    // int bl=firstblk;
    firstblk=bl;
   }
    // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1


    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog
    RelCacheTable::resetSearchIndex(1);

    int numberOfAttributesDeleted = 0;


    while(true) {
        // RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
           RecId attrRecid=linearSearch(1,name,relNameAttr,0);
        //    printf("\nrecid and slot in attr:%d and %d\n",attrRecid.block,attrRecid.slot);

        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if(attrRecid.block==-1 && attrRecid.slot==-1)
        break;

        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
        RecBuffer attrBlk(attrRecid.block);
        struct HeadInfo attrhead;
        attrBlk.getHeader(&attrhead);
        Attribute attrrecord[6];
        attrBlk.getRecord(attrrecord,attrRecid.slot);

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrrecord[4].nVal;
        int x;
        /* get root block from the record */
        // (This will be used later to delete any indexes if it exists)

        int num_slots=attrhead.numSlots;
        unsigned char slot_map[num_slots];
       x=attrBlk.getSlotMap(slot_map);
       if(x!=SUCCESS)
       return E_RELNOTEXIST;
       slot_map[attrRecid.slot]=SLOT_UNOCCUPIED;
       x=attrBlk.setSlotMap(slot_map);
       if(x!=SUCCESS)
       return E_RELNOTEXIST;

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap

        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
           attrhead.numEntries=attrhead.numEntries-1;
           x=attrBlk.setHeader(&attrhead);
           if(x!=SUCCESS)
           return E_RELNOTEXIST;


        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if(attrhead.numEntries==0)
        {
            // printf("keri\n");
            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */
           int y=attrhead.lblock;
           RecBuffer attrBlk(y);
            struct HeadInfo attrhead_of_y;
            attrBlk.getHeader(&attrhead_of_y);
            attrhead_of_y.rblock=attrhead.rblock;
           

            // create a RecBuffer for lblock and call appropriate methods

            if (attrhead.rblock!=-1/* header.rblock != -1 */) {
                int p=attrhead.rblock;
                 RecBuffer attrBlk(p);
            struct HeadInfo attrhead_of_p;
            attrBlk.getHeader(&attrhead_of_p);
            attrhead_of_p.lblock=attrhead.lblock;
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods

            } else {
                relCatEntryRecord[4].nVal=y;
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            // call releaseBlock()
            attrBlk.releaseBlock();
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
            BPlusTree::bPlusDestroy(rootBlock);
        }
    }

    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block
    //  RecBuffer relCatBlock(RELCAT_BLOCK);

    struct HeadInfo relCatHeader;
    relCatBlock.getHeader(&relCatHeader);
    int slots=relCatHeader.numSlots;


    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
       relCatHeader.numEntries=relCatHeader.numEntries-1;
       relCatBlock.setHeader(&relCatHeader);

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    unsigned char relCatSlotMap[slots];
    relCatBlock.getSlotMap(relCatSlotMap);
    relCatSlotMap[f_blk.slot]=SLOT_UNOCCUPIED;
    relCatBlock.setSlotMap(relCatSlotMap);

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
        RelCatEntry relcat;
    RelCacheTable::getRelCatEntry(0,&relcat);
    relcat.numRecs=relcat.numRecs-1;
    RelCacheTable::setRelCatEntry(0,&relcat);
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted
    RelCatEntry attrcat;
    RelCacheTable::getRelCatEntry(1,&attrcat);
    attrcat.numRecs=attrcat.numRecs-numberOfAttributesDeleted;
    RelCacheTable::setRelCatEntry(1,&attrcat);

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    return SUCCESS;
}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)
        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry buffrptr;
        RelCacheTable::getRelCatEntry(relId,&buffrptr);
        block=buffrptr.firstBlk;
        slot=0;
        // block = first record block of the relation
        // slot = 0
    }
    else
    {
        // (a project/search operation is already in progress)

        // block = previous search index's block
        // slot = previous search index's slot + 1
        block=prevRecId.block;
        slot=prevRecId.slot+1;
    }
    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
        RecBuffer blk(block);
        HeadInfo head;
        blk.getHeader(&head);
        unsigned char slot_map[head.numSlots];
        blk.getSlotMap(slot_map);
        // if(/* slot >= the number of slots per block*/)
        if(slot>=head.numSlots)
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            // set block = -1 and fail the loop condition )
            block=head.rblock;
            slot=0;
        }
        else if(slot_map[slot]==SLOT_UNOCCUPIED)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)

            // increment slot
            slot++;
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId,&nextRecId);
    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
   RecBuffer blki(block);
   blki.getRecord(record,slot);

    return SUCCESS;
}