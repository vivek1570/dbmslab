#include "BPlusTree.h"

#include <cstring>
#include<bits/stdc++.h>

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    IndexId searchIndex;

    AttrCacheTable::getSearchIndex(relId,attrName,&searchIndex);

    AttrCatEntry attrCatEntry;
  
     AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    int block=-1, index=-1;

    if(searchIndex.block==-1 && searchIndex.index==-1)
     {

        block = attrCatEntry.rootBlock;
        index = 0;
        if(block==-1)
        {
            return RecId{-1, -1};
        }
    } else {
        block = searchIndex.block;
        index = searchIndex.index + 1;  // search is resumed from the next index.

        IndLeaf leaf(block);

        HeadInfo leafHead;
        
        leaf.getHeader(&leafHead);

        if (index >= leafHead.numEntries) {

            block=leafHead.rblock;
            index=0;
            if (block == -1) {
                return RecId{-1, -1};
            }
        }
    }

    /******  Traverse through all the internal nodes according to value
             of attrVal and the operator op                             ******/

    /* (This section is only needed when
        - search restarts from the root block (when searchIndex is reset by caller)
        - root is not a leaf
        If there was a valid search index, then we are already at a leaf block
        and the test condition in the following loop will fail)
    */

    while(StaticBuffer::getStaticBlockType(block)==IND_INTERNAL) {  //use StaticBuffer::getStaticBlockType()
        // load the block into internalBlk using IndInternal::IndInternal().
        IndInternal internalBlk(block);

        HeadInfo intHead;

        // load the header of internalBlk into intHead using BlockBuffer::getHeader()
        internalBlk.getHeader(&intHead);

        // declare intEntry which will be used to store an entry of internalBlk.
        InternalEntry intEntry;
        // if (/* op is one of NE, LT, LE */) 
        if(op==NE ||op==LT|| op==LE)
        {
            /*
            - NE: need to search the entire linked list of leaf indices of the B+ Tree,
            starting from the leftmost leaf index. Thus, always move to the left.

            - LT and LE: the attribute values are arranged in ascending order in the
            leaf indices of the B+ Tree. Values that satisfy these conditions, if
            any exist, will always be found in the left-most leaf index. Thus,
            always move to the left.
            */
           internalBlk.getEntry(&intEntry,0);
            // load entry in the first slot of the block into intEntry
            // using IndInternal::getEntry().

            block = intEntry.lChild;

        } else {
            /*
            - EQ, GT and GE: move to the left child of the first entry that is
            greater than (or equal to) attrVal
            (we are trying to find the first entry that satisfies the condition.
            since the values are in ascending order we move to the left child which
            might contain more entries that satisfy the condition)
            */

            /*
             traverse through all entries of internalBlk and find an entry that
             satisfies the condition.
             if op == EQ or GE, then intEntry.attrVal >= attrVal
             if op == GT, then intEntry.attrVal > attrVal
             Hint: the helper function compareAttrs() can be used for comparing
            */
            // if (/* such an entry is found*/) 
            int i;
        for(i=0;i<intHead.numEntries;i++)
        {
            internalBlk.getEntry(&intEntry,i);
                       int x=compareAttrs(intEntry.attrVal,attrVal,attrCatEntry.attrType);

                 if((op==EQ && x>=0)||(op == GT && x > 0) ||
                (op == GE && x >= 0))
                break;
            
        }
        if(i>=0 && i<intHead.numEntries){
                // move to the left child of that entry
                block = intEntry.lChild;
                 // left child of the entry

            } else {
                // move to the right child of the last entry of the block
                // i.e numEntries - 1 th entry of the block

                block =  intEntry.rChild;
                // right child of last entry
            }
           
        }
    }

    // NOTE: `block` now has the block number of a leaf index block.

    /******  Identify the first leaf index entry from the current position
                that satisfies our condition (moving right)             ******/

    while (block != -1) {
        // load the block into leafBlk using IndLeaf::IndLeaf().
        IndLeaf leafBlk(block);
        HeadInfo leafHead;

        // load the header to leafHead using BlockBuffer::getHeader().
        leafBlk.getHeader(&leafHead);

        // declare leafEntry which will be used to store an entry from leafBlk
        Index leafEntry;

        // while (/*index < numEntries in leafBlk*/) 
        while(index<leafHead.numEntries)
        {
            // load entry corresponding to block and index into leafEntry
            // using IndLeaf::getEntry().
            
            leafBlk.getEntry(&leafEntry,index);

            int cmpVal = compareAttrs(leafEntry.attrVal,attrVal,attrCatEntry.attrType);
            /* comparison between leafEntry's attribute value
                            and input attrVal using compareAttrs()*/

            
            if(
                (op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == NE && cmpVal != 0)
            ) {
                // (entry satisfying the condition found)

                // set search index to {block, index}
                // leafEntry.block=block;
                // leafEntry.slot=index;
                IndexId set={block,index};
                AttrCacheTable::setSearchIndex(relId,attrName,&set);

                // return the recId {leafEntry.block, leafEntry.slot}.
                return {leafEntry.block,leafEntry.slot};

            } else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
                /*future entries will not satisfy EQ, LE, LT since the values
                    are arranged in ascending order in the leaves */

                // return RecId {-1, -1};
                return {-1,-1};
            }

            // search next index.
            ++index;
        }
        /*only for NE operation do we have to check the entire linked list;
        for all the other op it is guaranteed that the block being searched
        will have an entry, if it exists, satisying that op. */
        if (op != NE) {
            break;
        }

        // block = next block in the linked list, i.e., the rblock in leafHead.
        // update index to 0.
        block=leafHead.rblock;
        index=0;
    }
    return {-1,-1};
    // no entry satisying the op was found; return the recId {-1,-1}
}

int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

    if(relId==0 || relId==1)
    return E_NOTPERMITTED;

    AttrCatEntry attrCatBuff;
    int x=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatBuff);

    if(x!=SUCCESS)
    return x;
    

    // if (/* an index already exists for the attribute (check rootBlock field) */) 
    if(attrCatBuff.rootBlock!=-1)
    {
        return SUCCESS;
    }

    /******Creating a new B+ Tree ******/

    // get a free leaf block using constructor 1 to allocate a new block
    IndLeaf rootBlockBuf;

    // (if the block could not be allocated, the appropriate error code
    //  will be stored in the blockNum member field of the object)

    // declare rootBlock to store the blockNumber of the new leaf block
    int rootBlock = rootBlockBuf.getBlockNum();
    if (rootBlock == E_DISKFULL) {
        return E_DISKFULL;
    }
    attrCatBuff.rootBlock=rootBlock;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatBuff);
    // if there is no more disk space for creating an index
    RelCatEntry relCatEntry;
    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().
    RelCacheTable::getRelCatEntry(relId,&relCatEntry);

    int block = relCatEntry.firstBlk/* first record block of the relation */;

    /***** Traverse all the blocks in the relation and insert them one
           by one into the B+ Tree *****/
    while (block != -1) {

        // declare a RecBuffer object for `block` (using appropriate constructor)
        RecBuffer blk(block);

        unsigned char slotMap[relCatEntry.numSlotsPerBlk];

        // load the slot map into slotMap using RecBuffer::getSlotMap().
        blk.getSlotMap(slotMap);
       
        // int numslots=head.numSlots;

        // for every occupied slot of the block
        for(int i=0;i<relCatEntry.numSlotsPerBlk;i++)
        {
            if(slotMap[i]==SLOT_OCCUPIED)
            {
                Attribute record[relCatEntry.numAttrs];
            // load the record corresponding to the slot into `record`
            // using RecBuffer::getRecord().
            blk.getRecord(record,i);
            // declare recId and store the rec-id of this record in it
            // RecId recId{block, slot};
            RecId recId={block,i};

            // insert the attribute value corresponding to attrName from the record
            // into the B+ tree using bPlusInsert.
            // (note that bPlusInsert will destroy any existing bplus tree if
            // insert fails i.e when disk is full)
            int retVal = bPlusInsert(relId, attrName,record[attrCatBuff.offset], recId);
            if (retVal == E_DISKFULL) {
                // (unable to get enough blocks to build the B+ Tree.)
                return E_DISKFULL;
            }
            }
        }
         HeadInfo head;
        blk.getHeader(&head);
        // get the header of the block using BlockBuffer::getHeader()
        // set block = rblock of current block (from the header)
        block=head.rblock;
    }
    return SUCCESS;
}

int BPlusTree::bPlusDestroy(int rootBlockNum) {
    // if (/*rootBlockNum lies outside the valid range [0,DISK_BLOCKS-1]*/) 
    if(rootBlockNum<0 || rootBlockNum>=DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }

    int type = StaticBuffer::getStaticBlockType(rootBlockNum);
    /* type of block (using StaticBuffer::getStaticBlockType())*/

    if (type == IND_LEAF) {
        // declare an instance of IndLeaf for rootBlockNum using appropriate
        // constructor
        IndLeaf leafblk(rootBlockNum);

        // release the block using BlockBuffer::releaseBlock().
        leafblk.releaseBlock();

        return SUCCESS;

    } else if (type == IND_INTERNAL) {
        // declare an instance of IndInternal for rootBlockNum using appropriate
        // constructor
        IndInternal indblk(rootBlockNum);
        HeadInfo head;
        indblk.getHeader(&head);

        InternalEntry internal;
        indblk.getEntry(&internal,0);
        bPlusDestroy(internal.lChild);
        for(int i=0;i<head.numEntries;i++)
        {
            indblk.getEntry(&internal,i);
        bPlusDestroy(internal.rChild);
        }
            
		indblk.releaseBlock();
        // load the header of the block using BlockBuffer::getHeader().

        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */

        // release the block using BlockBuffer::releaseBlock().

        return SUCCESS;

    } else {
        // (block is not an index block.)
        return E_INVALIDBLOCK;
    }
}
int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrcat;
    int ret=AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcat);
    if(ret!=SUCCESS)
    return ret;
    // if getAttrCatEntry() failed
    //     return the error codej
    int blockNum = attrcat.rootBlock;
    /* rootBlock of B+ Tree (from attrCatEntry) */

    // if (/* there is no index on attribute (rootBlock is -1) */) 
    if(blockNum==-1)
    {
        return E_NOINDEX;
    }

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    int leafBlkNum = findLeafToInsert(blockNum,attrVal,attrcat.attrType);
    // insertIntoLeaf(relId,attrName,blockNum,);
    Index index;
    if(attrcat.attrType==NUMBER)
    index.attrVal.nVal=attrVal.nVal;
    if(attrcat.attrType==STRING)
    strcpy(index.attrVal.sVal,attrVal.sVal);
    index.block=recId.block;
    index.slot=recId.slot;
    ret=insertIntoLeaf(relId,attrName,leafBlkNum,index);
    /* findLeafToInsert(root block num, attrVal, attribute type) */
    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.
    // declare a struct Index with attrVal = attrVal, block = recId.block and
    // slot = recId.slot to pass as argument to the function.
    // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
    // NOTE: the insertIntoLeaf() function will propagate the insertion to the
    //       required internal nodes by calling the required helper functions
    //       like insertIntoInternal() or createNewRoot()

    // if (/*insertIntoLeaf() returns E_DISKFULL */) 
    if(ret==E_DISKFULL)
    {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
        bPlusDestroy(blockNum);

        // update the rootBlock of attribute catalog cache entry to -1 using
        attrcat.rootBlock=-1;
        AttrCacheTable::setAttrCatEntry(relId,attrName,&attrcat);
        // AttrCacheTable::setAttrCatEntry().

        return E_DISKFULL;
    }

    return SUCCESS;
}

int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {
    int blockNum = rootBlock;

    // while (/*block is not of type IND_LEAF */) 
    while(StaticBuffer::getStaticBlockType(blockNum)!=IND_LEAF)
    {  // use StaticBuffer::getStaticBlockType()
        // declare an IndInternal object for block using appropriate constructor
        IndInternal indblk(blockNum);
        HeadInfo head;
        indblk.getHeader(&head);
        // get header of the block using BlockBuffer::getHeader()
        int numEntries=head.numEntries;
        int i=0;
        InternalEntry internal;
        for( i=0;i<numEntries;i++)
        {
            indblk.getEntry(&internal,i);
            int cmp=compareAttrs(attrVal,internal.attrVal,attrType);
            if(cmp<=0) break;
        }
        /* iterate through all the entries, to find the first entry whose
             attribute value >= value to be inserted.
             NOTE: the helper function compareAttrs() declared in BlockBuffer.h
                   can be used to compare two Attribute values. */
        // if (/*no such entry is found*/) 
        if(i==numEntries)
        {
            // set blockNum = rChild of (nEntries-1)'th entry of the block
            // (i.e. rightmost child of the block)
            InternalEntry entry;
            indblk.getEntry(&entry,head.numEntries-1);
            blockNum=entry.rChild;

        } else {
            // set blockNum = lChild of the entry that was found
            InternalEntry entry;
            indblk.getEntry(&entry,i);
            blockNum=entry.lChild;
        }
    }

    return blockNum;
}

int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int leafBlockNum, Index indexEntry) 
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // declare an IndLeaf instance for the block using appropriate constructor
    IndLeaf leafBlock (leafBlockNum);

    HeadInfo blockHeader;
    // store the header of the leaf index block into blockHeader
    // using BlockBuffer::getHeader()
    leafBlock.getHeader(&blockHeader);

    // the following variable will be used to store a list of index entries with
    // existing indices + the new index to insert
    Index indices[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array indices.
    Also insert `indexEntry` at appropriate position in the indices array maintaining
    the ascending order.
    - use IndLeaf::getEntry() to get the entry
    - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
    */

    bool inserted = false;
    for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
    {
        Index entry;
        leafBlock.getEntry(&entry, entryindex);

        if (compareAttrs(entry.attrVal, indexEntry.attrVal, attrCatEntryBuffer.attrType) <= 0)
        {
            // the current entry element is lesser than the one to be inserted
            indices[entryindex] = entry;
        }
        else
        {
            // the indexEntry is to be placed here
            indices[entryindex] = indexEntry;
            // insertmarker++;
            inserted = true;

            for (entryindex++; entryindex <= blockHeader.numEntries; entryindex++)
            {
                leafBlock.getEntry(&entry, entryindex-1);
                indices[entryindex] = entry;
            }
            break;
        }
    }

    // adding the last element in indices
    if (!inserted) indices[blockHeader.numEntries] = indexEntry;

    if (blockHeader.numEntries < MAX_KEYS_LEAF) {
        // (leaf block has not reached max limit)

        // increment blockHeader.numEntries and update the header of block
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        leafBlock.setHeader(&blockHeader);

        // iterate through all the entries of the array `indices` and populate the
        // entries of block with them using IndLeaf::setEntry().

        for (int indicesIt = 0; indicesIt < blockHeader.numEntries; indicesIt++)
            leafBlock.setEntry(&indices[indicesIt], indicesIt);

        return SUCCESS;
    }

    // If we reached here, the `indices` array has more than entries than can fit
    // in a single leaf index block. Therefore, we will need to split the entries
    // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitLeaf(leafBlockNum, indices);

    // if splitLeaf() returned E_DISKFULL
    //     return E_DISKFULL
    if (newRightBlk == E_DISKFULL) return E_DISKFULL;

    // if (/* the current leaf block was not the root */) // check pblock in header
    if (blockHeader.pblock != -1)
    {
        //TODO: insert the middle value from `indices` into the parent block using the
        //TODO: insertIntoInternal() function. (i.e the last value of the left block)
        //* the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)

        // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
        // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
        // the insertIntoInternalFunction as follows

        InternalEntry middleEntry;
        middleEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal, 
        middleEntry.lChild = leafBlockNum, middleEntry.rChild = newRightBlk;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);
        // // if (ret == E_DISKFULL) return E_DISKFULL;
    } 
    else 
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
        //               current block, new right block)

        if(
            createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, 
                            leafBlockNum, newRightBlk) == E_DISKFULL
        )
            return E_DISKFULL;
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
    return SUCCESS;
}


int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {
    // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
    // leaf index block that will be used as the right block in the splitting

    // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
    // the existing leaf block
    IndLeaf indleaf_1;
    IndLeaf indleaf_2(leafBlockNum);

    int rightBlkNum =indleaf_1.getBlockNum();
    int leftBlkNum = indleaf_2.getBlockNum();

    if (rightBlkNum==E_DISKFULL/* newly allocated block has blockNum E_DISKFULL */) {
        //(failed to obtain a new leaf index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    indleaf_1.getHeader(&rightBlkHeader);
    indleaf_2.getHeader(&leftBlkHeader);


    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
    // - pblock = pblock of leftBlk
    // - lblock = leftBlkNum
    // - rblock = rblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    rightBlkHeader.numEntries=(MAX_KEYS_LEAF+1)/2;
    rightBlkHeader.blockType=leftBlkHeader.blockType;
    rightBlkHeader.pblock=leftBlkHeader.pblock;
    rightBlkHeader.lblock=leftBlkNum;
    rightBlkHeader.rblock=leftBlkHeader.rblock;
    indleaf_1.setHeader(&rightBlkHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
    // - rblock = rightBlkNum
    // and update the header of leftBlk using BlockBuffer::setHeader() */
    leftBlkHeader.numEntries=32;
    leftBlkHeader.rblock=rightBlkNum;
    indleaf_2.setHeader(&leftBlkHeader);

    // set the first 32 entries of leftBlk = the first 32 entries of indices array
    // and set the first 32 entries of newRightBlk = the next 32 entries of
    // indices array using IndLeaf::setEntry().
    int j=0;
    for(int i=0;i<32;i++)
    {
        indleaf_2.setEntry(&indices[j],i);
        j++;
    }
    j== MIDDLE_INDEX_LEAF + 1;
    for(int i=0;i<32;i++)
    {
        indleaf_1.setEntry(&indices[j],i);
        j++;
    }
    return rightBlkNum;
}



int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], 
                                    int intBlockNum, InternalEntry intEntry) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // declare intBlk, an instance of IndInternal using constructor 2 for the block
    // corresponding to intBlockNum
    IndInternal internalBlock (intBlockNum);

    HeadInfo blockHeader;
    // load blockHeader with header of intBlk using BlockBuffer::getHeader().
    internalBlock.getHeader(&blockHeader);

    // declare internalEntries to store all existing entries + the new entry
    InternalEntry internalEntries[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array
    `internalEntries`. Insert `indexEntry` at appropriate position in the
    array maintaining the ascending order.
        - use IndInternal::getEntry() to get the entry
        - use compareAttrs() to compare two structs of type Attribute

    Update the lChild of the internalEntry immediately following the newly added
    entry to the rChild of the newly added entry.
    */

    // bool inserted = false;
    int insertedIndex = -1;
    for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
    {
        InternalEntry internalBlockEntry;
        internalBlock.getEntry(&internalBlockEntry, entryindex);

        if (compareAttrs(internalBlockEntry.attrVal, intEntry.attrVal, attrCatEntryBuffer.attrType) <= 0)
        {
            // the new value is lesser (or equal to), hence does not go here
            internalEntries[entryindex] = internalBlockEntry;
        }
        else 
        {
            // // setting the previous entry's rChild to the lChild of this entry
            // if (entryindex > 0)
            // {
            //     internalBlock.getEntry(&internalBlockEntry, entryindex-1);
            //     internalBlockEntry.rChild = intEntry.lChild;
            // }

            // setting the correct entry in `internalEntries`
            internalEntries[entryindex] = intEntry;
            insertedIndex = entryindex;

            // // setting the following entry's lChild to the rChild of this entry
            // internalBlock.getEntry(&internalBlockEntry, entryindex+1);
            // internalBlockEntry.lChild = intEntry.rChild;

            for (entryindex++; entryindex <= blockHeader.numEntries; entryindex++)
            {
                internalBlock.getEntry(&internalBlockEntry, entryindex-1);
                internalEntries[entryindex] = internalBlockEntry;
            }

            break;
        }
    }

    // inserting the last remaining entry, if any
    //// internalEntries[blockHeader.numEntries] = inserted ? lastEntry : intEntry;
    if (insertedIndex == -1) {
        internalEntries[blockHeader.numEntries] = intEntry;
        insertedIndex = blockHeader.numEntries;
    }

    // setting the previous entry's rChild to lChild of `intEntry`
    if (insertedIndex > 0)
    {
        // InternalEntry entry;
        // internalBlock.getEntry(&entry, insertedIndex-1);
        internalEntries[insertedIndex-1].rChild = intEntry.lChild;
    }

    // setting the following entry's lChild to rChild of `intEntry`
    if (insertedIndex < blockHeader.numEntries)
    {
        // InternalEntry entry;
        // internalBlock.getEntry(&entry, insertedIndex+1);
        internalEntries[insertedIndex+1].lChild = intEntry.rChild;
    }


    if (blockHeader.numEntries < MAX_KEYS_INTERNAL) {
        // (internal index block has not reached max limit)

        // increment blockheader.numEntries and update the header of intBlk
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        internalBlock.setHeader(&blockHeader);

        // iterate through all entries in internalEntries array and populate the
        // entries of intBlk with them using IndInternal::setEntry().
        for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
            internalBlock.setEntry(&internalEntries[entryindex], entryindex);

        return SUCCESS;
    }

    // If we reached here, the `internalEntries` array has more than entries than
    // can fit in a single internal index block. Therefore, we will need to split
    // the entries in `internalEntries` between two internal index blocks. We do
    // this using the splitInternal() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitInternal(intBlockNum, internalEntries);

    // if (/* splitInternal() returned E_DISKFULL */) 
    if (newRightBlk == E_DISKFULL)
    {
        // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree
        BPlusTree::bPlusDestroy(intEntry.rChild);

        return E_DISKFULL;
    }

    // if the current block was not the root  
    if (blockHeader.pblock != -1) // (check pblock in header)
    {  
        // insert the middle value from `internalEntries` into the parent block
        // using the insertIntoInternal() function (recursively).

        // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
        // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
        // and pass it as argument to the insertIntoInternalFunction as follows

        InternalEntry middleEntry;
        middleEntry.lChild = intBlockNum, middleEntry.rChild = newRightBlk;

        // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)
        middleEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);
        // // if (ret == E_DISKFULL) return E_DISKFULL;
    } 
    else 
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        return createNewRoot(relId, attrName, internalEntries[MIDDLE_INDEX_INTERNAL].attrVal, 
                                intBlockNum, newRightBlk);


        // createNewRoot(relId, attrName,
        //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
        //               current block, new right block)

        //// if (ret == E_DISKFULL) return E_DISKFULL;
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
    return SUCCESS;
}

int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {
    // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
    // internal index block that will be used as the right block in the splitting

    // declare leftBlk, an instance of IndInternal using constructor 2 to read from
    // the existing internal index block
    IndInternal indint_1;
    IndInternal indint_2(intBlockNum);

    int rightBlkNum = indint_1.getBlockNum();
    int leftBlkNum =indint_2.getBlockNum();

    // if (/* newly allocated block has blockNum E_DISKFULL */) 
    if(rightBlkNum==E_DISKFULL)
    {
        //(failed to obtain a new internal index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()
    indint_1.getHeader(&rightBlkHeader);
    indint_2.getHeader(&leftBlkHeader);
    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - pblock = pblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    rightBlkHeader.numEntries=50;
    rightBlkHeader.pblock=leftBlkHeader.pblock;
    indint_1.setHeader(&rightBlkHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - rblock = rightBlkNum
    // and update the header using BlockBuffer::setHeader()
    leftBlkHeader.numEntries=50;
    leftBlkHeader.rblock=rightBlkNum;
    indint_2.setHeader(&leftBlkHeader);

    /*
    - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
      array
    - set the first 50 entries of newRightBlk = entries from index 51 to 100
      of internalEntries array using IndInternal::setEntry().
      (index 50 will be moving to the parent internal index block)
    */
   int ite=0;
   for(ite=0;ite<50;ite++)
   {
        indint_2.setEntry(&internalEntries[ite],ite);
   }
    ite=MIDDLE_INDEX_INTERNAL+1;
    for(int i=0;i<MIDDLE_INDEX_INTERNAL;i++)
    {
        indint_1.setEntry(&internalEntries[ite],i);
        ite++;
    }
    int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild);
    /* block type of a child of any entry of the internalEntries array */
    //            (use StaticBuffer::getStaticBlockType())

    BlockBuffer blockbuffer (internalEntries[MIDDLE_INDEX_INTERNAL+1].lChild);

    HeadInfo blockHeader;
    blockbuffer.getHeader(&blockHeader);

    blockHeader.pblock = rightBlkNum;
    blockbuffer.setHeader(&blockHeader);

    for (int entryindex = 0; entryindex < MIDDLE_INDEX_INTERNAL; entryindex++)
    {
        // declare an instance of BlockBuffer to access the child block using
        // constructor 2
        BlockBuffer blockbuffer (internalEntries[entryindex + MIDDLE_INDEX_INTERNAL+1].rChild);

        // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
        // and BlockBuffer::setHeader().
        // HeadInfo blockHeader;
        blockbuffer.getHeader(&blockHeader);

        blockHeader.pblock = rightBlkNum;
        blockbuffer.setHeader(&blockHeader);
    }

    return rightBlkNum; 
}
// int x=0;
int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    // printf("The level reached=%d",x);
    // x++;
    AttrCatEntry attrcat;
    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcat);

    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newrootblk;

    int newRootBlkNum =newrootblk.getBlockNum();
    /* block number of newRootBlk */
    HeadInfo newhead;
    newrootblk.getHeader(&newhead);

    if (newRootBlkNum == E_DISKFULL) {
        // (failed to obtain an empty internal index block because the disk is full)

        // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree
        bPlusDestroy(rChild);

        return E_DISKFULL;
    }

    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    newhead.numEntries=1;
    newrootblk.setHeader(&newhead);

    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
    InternalEntry inten;
    if(attrcat.attrType==NUMBER)
    inten.attrVal.nVal=attrVal.nVal;
    else if(attrcat.attrType==STRING)
    strcpy(inten.attrVal.sVal,attrVal.sVal);
    inten.lChild=lChild;
    inten.rChild=rChild;
    newrootblk.setEntry(&inten,0);

    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()

    BlockBuffer leftChildBlock (lChild);
    BlockBuffer rightChildBlock (rChild);

    HeadInfo leftChildHeader, rightChildHeader;
    leftChildBlock.getHeader(&leftChildHeader);
    rightChildBlock.getHeader(&rightChildHeader);

    leftChildHeader.pblock = newRootBlkNum;
    rightChildHeader.pblock = newRootBlkNum;

    leftChildBlock.setHeader(&leftChildHeader);
    rightChildBlock.setHeader(&rightChildHeader);

    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().
    attrcat.rootBlock = newRootBlkNum;
    AttrCacheTable::setAttrCatEntry(relId, attrName,  &attrcat);

    return SUCCESS;
}

// int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry) {
//     AttrCatEntry attrcat;
//     AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcat);

//     IndLeaf indleaf(blockNum);

//     HeadInfo blockHeader;
//     indleaf.getHeader(&blockHeader);
//     /*
//     Iterate through all the entries in the block and copy them to the array indices.
//     Also insert `indexEntry` at appropriate position in the indices array maintaining
//     the ascending order.
//     - use IndLeaf::getEntry() to get the entry
//     - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
//     */
//     Index indices[blockHeader.numEntries + 1];
//     Index current_leafEntry;
//     int current_leafEntryIndex=0;
//     int flag=0;
//     int numEntries=blockHeader.numEntries;
//     int curr=0;
//     for(curr=0;curr<numEntries;curr++)
//     {
//         indleaf.getEntry(&current_leafEntry,curr);
//         if(flag==0)
//         {
//             if(compareAttrs(indexEntry.attrVal,current_leafEntry.attrVal,attrcat.attrType)<0)
//             {
//                 if(attrcat.attrType==NUMBER)
//                 {
//                     indices[current_leafEntryIndex].attrVal.nVal=indexEntry.attrVal.nVal;
//                 }
//                 else if(attrcat.attrType==STRING)
//                 {
//                     strcpy(indices[current_leafEntryIndex].attrVal.sVal,indexEntry.attrVal.sVal);
//                 }
//                 indices[current_leafEntryIndex].block=indexEntry.block;
//                 indices[current_leafEntryIndex].slot=indexEntry.slot;
//                 flag=1;
//                 current_leafEntryIndex++;
//             }
//         }
//             if(attrcat.attrType==NUMBER){
//                 indices[current_leafEntryIndex].attrVal.nVal=current_leafEntry.block;
//             }
//             else if(attrcat.attrType==STRING)
//             {
//                 strcpy(indices[current_leafEntryIndex].attrVal.sVal,current_leafEntry.attrVal.sVal);
//             }
//             indices[current_leafEntryIndex].block=current_leafEntry.block;
//             indices[current_leafEntryIndex].slot=current_leafEntry.slot;
//             current_leafEntryIndex++;
//     }
//     if(curr==current_leafEntryIndex){
//         if(attrcat.attrType==NUMBER)
//             {
//                 indices[current_leafEntryIndex].attrVal.nVal=indexEntry.attrVal.nVal;
//             }
//             else if(attrcat.attrType==STRING)
//             {
//                 strcpy(indices[current_leafEntryIndex].attrVal.sVal,indexEntry.attrVal.sVal);
//             }
//             indices[current_leafEntryIndex].block=indexEntry.block;
//             indices[current_leafEntryIndex].slot=indexEntry.slot;
//             current_leafEntryIndex++;
//     }

//     if (blockHeader.numEntries < MAX_KEYS_LEAF) {
//         // (leaf block has not reached max limit)

//         // increment blockHeader.numEntries and update the header of block
//         // using BlockBuffer::setHeader().
//         blockHeader.numEntries=blockHeader.numEntries+1;
//         indleaf.setHeader(&blockHeader);

//         // iterate through all the entries of the array `indices` and populate the
//         // entries of block with them using IndLeaf::setEntry().
//         for(int i=0;i<blockHeader.numEntries;i++)
//         {
//             indleaf.setEntry(&indices[i],i);
//         }
//         return SUCCESS;
//     }

//     // If we reached here, the `indices` array has more than entries than can fit
//     // in a single leaf index block. Therefore, we will need to split the entries
//     // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
//     // This function will return the blockNum of the newly allocated block or
//     // E_DISKFULL if there are no more blocks to be allocated.

//     int newRightBlk = splitLeaf(blockNum, indices);

//     // if splitLeaf() returned E_DISKFULL
//     //     return E_DISKFULL
//     if(newRightBlk==E_DISKFULL)
//     return E_DISKFULL;

//     // if (/* the current leaf block was not the root */) 
//     int l=0;
//     if(blockHeader.pblock!=-1)
//     {  // check pblock in header
//         // insert the middle value from `indices` into the parent block using the
//         // insertIntoInternal() function. (i.e the last value of the left block)
//         // the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)
//         // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
//         // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
//         // the insertIntoInternalFunction as follows
//         // insertIntoInternal(relId, attrName, parent of current block, new internal entry)

//         InternalEntry inf;
//         if(attrcat.attrType==NUMBER)
//         inf.attrVal.nVal=indices[MIDDLE_INDEX_LEAF].attrVal.nVal;
//         else if(attrcat.attrType==STRING)
//         strcpy(inf.attrVal.sVal,indices[MIDDLE_INDEX_LEAF].attrVal.sVal);
//         inf.lChild=blockNum;
//         inf.rChild=newRightBlk;
//         return insertIntoInternal(relId,attrName,blockHeader.pblock,inf);

//     } else {
//         // the current block was the root block and is now split. a new internal index
//         // block needs to be allocated and made the root of the tree.
//         // To do this, call the createNewRoot() function with the following arguments

//         l=createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,blockNum,newRightBlk);
//         if(l==E_DISKFULL)
//         return l;
//     }

//     // if either of the above calls returned an error (E_DISKFULL), then return that
    
//     return SUCCESS;
// }

// int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry intEntry) {
//     // get the attribute cache entry corresponding to attrName
//     // using AttrCacheTable::getAttrCatEntry().
//     AttrCatEntry attrcat;
//     AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcat);

//     IndInternal indblk(intBlockNum);
//     // declare intBlk, an instance of IndInternal using constructor 2 for the block
//     // corresponding to intBlockNum

//     HeadInfo blockHeader;
//     indblk.getHeader(&blockHeader);

//     // declare internalEntries to store all existing entries + the new entry
//     InternalEntry internalEntries[blockHeader.numEntries + 1];

//     int entries=blockHeader.numEntries;
//     int flag=0;
//     InternalEntry internalEntry;
//     int current_entryNumber=0;
//     int indices_iter=0;
//     for(indices_iter=0;indices_iter<entries;indices_iter++)
//     {
//         indblk.getEntry(&internalEntry,indices_iter);
//         if(flag==0){
//             if(compareAttrs(intEntry.attrVal,internalEntry.attrVal,attrcat.attrType)<=0)
//             {
//                 if(attrcat.attrType==NUMBER){
//                     internalEntries[current_entryNumber].attrVal.nVal=intEntry.attrVal.nVal;
//                 }
//                 else if(attrcat.attrType==STRING){
//                     strcpy(internalEntries[current_entryNumber].attrVal.sVal,intEntry.attrVal.sVal);
//                 }
//                 internalEntries[current_entryNumber].lChild=intEntry.lChild;
//                 internalEntries[current_entryNumber].rChild=intEntry.rChild;
//                 flag=1;
//                 current_entryNumber++;
//             }
//         }
//         if(attrcat.attrType==NUMBER){
//             internalEntries[current_entryNumber].attrVal.nVal=internalEntry.attrVal.nVal;
//         }else if(attrcat.attrType==STRING){
//             strcpy(internalEntries[current_entryNumber].attrVal.sVal,internalEntry.attrVal.sVal);
//         }
//         if(current_entryNumber-1>=0){
//             internalEntries[current_entryNumber].lChild=internalEntries[current_entryNumber-1].rChild;
//         }
//         else{
//             internalEntries[current_entryNumber].lChild=intEntry.lChild;
//         }
//         internalEntries[current_entryNumber].rChild=intEntry.rChild;
//         current_entryNumber++;
//     }

//     if(flag==0)
//     {
//         if(attrcat.attrType==NUMBER){
//                     internalEntries[current_entryNumber].attrVal.nVal=intEntry.attrVal.nVal;
//                 }
//                 else if(attrcat.attrType==STRING){
//                     strcpy(internalEntries[current_entryNumber].attrVal.sVal,intEntry.attrVal.sVal);
//                 }
//                 internalEntries[current_entryNumber].lChild=intEntry.lChild;
//                 internalEntries[current_entryNumber].rChild=intEntry.rChild;
//                 current_entryNumber++;
//     }

//     /*
//     Iterate through all the entries in the block and copy them to the array
//     `internalEntries`. Insert `indexEntry` at appropriate position in the
//     array maintaining the ascending order.
//         - use IndInternal::getEntry() to get the entry
//         - use compareAttrs() to compare two structs of type Attribute

//     Update the lChild of the internalEntry immediately following the newly added
//     entry to the rChild of the newly added entry.
//     */
   

//     if (blockHeader.numEntries < MAX_KEYS_INTERNAL) {
//         // (internal index block has not reached max limit)

//         // increment blockheader.numEntries and update the header of intBlk
//         // using BlockBuffer::setHeader().
//         blockHeader.numEntries+=1;
//         indblk.setHeader(&blockHeader);

//         // iterate through all entries in internalEntries array and populate the
//         // entries of intBlk with them using IndInternal::setEntry().
//         for(int j=0;j<blockHeader.numEntries;j++)
//         {
//             indblk.setEntry(&internalEntries[j],j);
//         }

//         return SUCCESS;
//     }

//     // If we reached here, the `internalEntries` array has more than entries than
//     // can fit in a single internal index block. Therefore, we will need to split
//     // the entries in `internalEntries` between two internal index blocks. We do
//     // this using the splitInternal() function.
//     // This function will return the blockNum of the newly allocated block or
//     // E_DISKFULL if there are no more blocks to be allocated.

//     int newRightBlk = splitInternal(intBlockNum, internalEntries);

//     // if (/* splitInternal() returned E_DISKFULL */) 
//     if(newRightBlk==E_DISKFULL)
//     {

//         // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
//         // This corresponds to the tree built up till now that has not yet been
//         // connected to the existing B+ Tree
//         bPlusDestroy(intEntry.rChild);
//         return E_DISKFULL;
//     }

//     // if (/* the current block was not the root */) 
//     int ret=0;
//     if(attrcat.rootBlock!=-1)
//     {  // (check pblock in header)
//         // insert the middle value from `internalEntries` into the parent block
//         // using the insertIntoInternal() function (recursively).
//         // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)
//         // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
//         // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
//         // and pass it as argument to the insertIntoInternalFunction as follows
//         // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
//         InternalEntry inten;
//         if(attrcat.attrType==NUMBER)
//         inten.attrVal.nVal=internalEntries[MIDDLE_INDEX_INTERNAL].attrVal.nVal;
//         else if(attrcat.attrType==STRING)
//         strcpy(inten.attrVal.sVal,internalEntries[MIDDLE_INDEX_INTERNAL].attrVal.sVal);
//         inten.lChild=internalEntries[MIDDLE_INDEX_INTERNAL].lChild;
//         inten.rChild=internalEntries[MIDDLE_INDEX_INTERNAL].rChild;
//         return insertIntoInternal(relId,attrName,blockHeader.pblock,inten);

//     } else {
//         // the current block was the root block and is now split. a new internal index
//         // block needs to be allocated and made the root of the tree.
//         // To do this, call the createNewRoot() function with the following arguments
//         // createNewRoot(relId,attrName,)
//         return createNewRoot(relId,attrName,internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,intBlockNum,newRightBlk);
//     }

//     // if either of the above calls returned an error (E_DISKFULL), then return that
//     // else return SUCCESS
//     // if(ret==E_DISKFULL)
//     // return E_DISKFULL;
//     // else
//      return SUCCESS;
// }