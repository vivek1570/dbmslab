#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>
#include <bits/stdc++.h>

// the declarations for these functions can be found in "BlockBuffer.h"

// the declarations for these functions can be found in "BlockBuffer.h"


/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;

    // if attrType == STRING
    //     diff = strcmp(attr1.sval, attr2.sval)

    // else
    //     diff = attr1.nval - attr2.nval
    if(attrType==STRING)
    {
      diff= strcmp(attr1.sVal, attr2.sVal);
    }
    else diff=attr1.nVal - attr2.nVal;
    /*
    if diff > 0 then return 1
    if diff < 0 then return -1
    if diff = 0 then return 0
    */
   if(diff>0) return 1;
   if(diff<0) return -1;
   return 0;
}
BlockBuffer::BlockBuffer(char blockType){
    
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
    int y=3;
    if(blockType=='R')
    y=0;
    if(blockType=='I')y=1;
    if(blockType=='L')y=2;
    int x=BlockBuffer::getFreeBlock(y);
    this->blockNum=x;
} 

RecBuffer::RecBuffer() : BlockBuffer('R'){}
BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  // printf("hello in block buffer int\n");
  this->blockNum=blockNum;
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}
int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr=NULL;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // ... (the rest of the logic is as in stage 2)

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr+16, 4);
  memcpy(&head->numAttrs, bufferPtr+20, 4);
  memcpy(&head->rblock, bufferPtr+12, 4);
  memcpy(&head->lblock, bufferPtr+8, 4);
  memcpy(&head->pblock, bufferPtr+4, 4);
  return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  // ...
  struct HeadInfo head;

  this->getHeader(&head);
  unsigned char *bufferPtr=new unsigned char;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  int attrCount=head.numAttrs;
  int slotCount=head.numSlots;
  
  int recordSize=attrCount*ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr+HEADER_SIZE+ slotCount +(recordSize*slotNum);

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
  // ... (the rest of the logic is as in stage 2
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  // printf("int buffer ptr 1\n");
  // printf("buffer num in load and block= %d\n",bufferNum);

if(bufferNum!=E_BLOCKNOTINBUFFER)
{
    for(int i=0;i<BUFFER_CAPACITY;i++)
    {
      if(StaticBuffer::metainfo[i].free==false)
      {
        StaticBuffer::metainfo[i].timeStamp++;
      }
    }
    StaticBuffer::metainfo[bufferNum].timeStamp=0;
}
else{
  // printf("int buffer ptr 2\n");

  bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
  if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }
    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
}

  // printf("buffer num=%d\n",bufferNum);
  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];
  return SUCCESS;
}





int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr=new unsigned char;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  this->getHeader(&head);

  // get the header of the block using getHeader() function

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // slotMap=slotMapInBuffer+slotCount
  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
memcpy(slotMap,slotMapInBuffer,slotCount);
  return SUCCESS;
}
int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr=new unsigned char;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
  
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

  int ret = loadBlockAndGetBufferPtr(&bufferPtr);

  // printf("ret %d\n",ret);
  // printf("the block number into setrecord=%d\n",this->blockNum);
  if (ret != SUCCESS) {
    return ret;
  }

    /* get the header of the block using the getHeader() function */
   struct HeadInfo head;
    this->getHeader(&head); 
    // printf("after header\n");
  // printf("in blockbuffer num=%d\n",head.numEntries);
    // get number of attributes in the block.
    // get the number of slots in the block.
  int num_slots=head.numSlots;
  if(slotNum<0 && slotNum>num_slots)
  return E_OUTOFBOUND;
    // if input slotNum is not in the permitted range return E_OUTOFBOUND.

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
  int recordSize=head.numAttrs*ATTR_SIZE;

  unsigned char *slotPointer = bufferPtr+HEADER_SIZE+ num_slots +(recordSize*slotNum);
  memcpy(slotPointer,rec, recordSize);

  int g=StaticBuffer::setDirtyBit(this->blockNum);
    // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */
    return g;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr=new unsigned char;

    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  // printf("ret %d\n",ret);
  if (ret != SUCCESS) {
    return ret;
  }
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
    bufferHeader->blockType=head->blockType;
    bufferHeader->lblock=head->lblock;
    bufferHeader->numAttrs=head->numAttrs;
    bufferHeader->numEntries=head->numEntries;
    bufferHeader->numSlots=head->numSlots;
    bufferHeader->pblock=head->pblock;
    bufferHeader->rblock=head->rblock;

    int x=StaticBuffer::setDirtyBit(this->blockNum);

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code
  if(x!=SUCCESS)
  return x;
  return SUCCESS;
    // return SUCCESS;
}
int BlockBuffer::setBlockType(int blockType){ 
    unsigned char *bufferPtr=new unsigned char;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
     int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  // printf("ret %d\n",ret);
  if (ret != SUCCESS) {
    return ret;
  }
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    // struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;
  // int32_t* blockType=(int32_t*)bufferPtr;
  *((int32_t *)bufferPtr) = blockType;

    StaticBuffer::blockAllocMap[this->blockNum]=blockType;
    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    int x=StaticBuffer::setDirtyBit(this->blockNum);
    if(x!=SUCCESS)
    return x;
    return SUCCESS;
    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call

    // return SUCCESS

}

int BlockBuffer::getFreeBlock(int blockType){
    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int blk=-1;
    int f=1;
    for(int i=0;i<DISK_BLOCKS && f;i++)
    {
      if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK)
      {
        blk=i;
        f=0;
      }
    }
    if(blk==-1)
    return E_DISKFULL;

    // if no block is free, return E_DISKFULL.
    this->blockNum=blk;
    // set the object's blockNum to the block number of the free block.

    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
  if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }
    //  Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  //   Disk::readBlock(bufferPtr, bufferNum);
    
    struct HeadInfo *head=new struct HeadInfo;
        // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    head->pblock=-1;
    head->lblock=-1;
    head->rblock=-1;
    head->numEntries=0;
    head->numAttrs=0;
    head->numSlots=0;
    setHeader(head);

    // update the block type of the block to the input block type using setBlockType().
    setBlockType(blockType);
    return blk;
    // return block number of the free block.
}


// call parent non-default constructor with 'R' denoting record block.

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr=new unsigned char;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int x=loadBlockAndGetBufferPtr(&bufferPtr);
    if(x!=SUCCESS)
    return x;
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    struct HeadInfo head;

  getHeader(&head);
    // get the header of the block using the getHeader() function

    int numSlots = head.numSlots;
     unsigned char *slotPointer = bufferPtr+HEADER_SIZE;
    
  memcpy(slotPointer,slotMap, numSlots);
  // printf("in buffer\n");
  // printf("num slots=%d\n",numSlots);
  //  for(int i=0;i<numSlots;i++)
  //    {
  //     printf("%d ",(int)slotPointer[i]);
  //    }
    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
   int ret= StaticBuffer::setDirtyBit(this->blockNum);
   if(ret!=SUCCESS)
   return ret;
    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call

    return SUCCESS;
}

int BlockBuffer::getBlockNum(){

    // return getFreeBlock('R');
    // printf("hello\n");
    return this->blockNum;
    // return getFreeBlock('R');
}

void BlockBuffer::releaseBlock(){

    // if blockNum is INVALID_BLOCK (-1), or it is invalidated already, do nothing
    
    if(this->blockNum!=-1)
    {
      int buffer_num=StaticBuffer::getBufferNum(this->blockNum);
      if(buffer_num!=E_BLOCKNOTINBUFFER)
      {
        StaticBuffer::metainfo[buffer_num].free=true;
        StaticBuffer::blockAllocMap[this->blockNum]=UNUSED_BLK;
        this->blockNum=INVALID_BLOCKNUM;
      }
    }

    

    // else
        /* get the buffer number of the buffer assigned to the block
           using StaticBuffer::getBufferNum().
           (this function return E_BLOCKNOTINBUFFER if the block is not
           currently loaded in the buffer)
            */

        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.

        // free the block in disk by setting the data type of the entry
        // corresponding to the block number in StaticBuffer::blockAllocMap
        // to UNUSED_BLK.

        // set the object's blockNum to INVALID_BLOCK (-1)
}

IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType){}

IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum){}

IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum){}

IndLeaf::IndLeaf() : IndBuffer('L'){} 

IndInternal::IndInternal() : IndBuffer('I'){}

IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum){}
// call the corresponding parent constructor

int IndInternal::getEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
    if(indexNum<0 || indexNum>MAX_KEYS_INTERNAL)
    {
      return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
       int x=loadBlockAndGetBufferPtr(&bufferPtr);
    if(x!=SUCCESS)
    return x;
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.

    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    /*
    - copy the entries from the indexNum`th entry to *internalEntry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
    memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
    memcpy(&(internalEntry->rChild), entryPtr + 20, 4);

    return SUCCESS;
}

int IndLeaf::getEntry(void *ptr, int indexNum) {

    // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
    //     return E_OUTOFBOUND.
   if(indexNum<0 || indexNum>MAX_KEYS_INTERNAL)
    {
      return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
     int x=loadBlockAndGetBufferPtr(&bufferPtr);
    if(x!=SUCCESS)
    return x;

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.

    // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);

    return SUCCESS;
}

int IndLeaf::setEntry(void *ptr, int indexNum) {

    // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
    //     return E_OUTOFBOUND.
    if(indexNum<0 || indexNum>=MAX_KEYS_LEAF)
    return E_OUTOFBOUND;

    unsigned char *bufferPtr;
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS)
    return ret;

    // copy the Index at ptr to indexNum'th entry in the buffer using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy(entryPtr, (struct Index *)ptr, LEAF_ENTRY_SIZE);

    // update dirty bit using setDirtyBit()
    int x=StaticBuffer::setDirtyBit(indexNum);
    if(x!=SUCCESS)
    return x;
    // if setDirtyBit failed, return the value returned by the call

    return SUCCESS;
}

int IndInternal::setEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
    if(indexNum<0 || indexNum>=MAX_KEYS_INTERNAL)
    return E_OUTOFBOUND;

    unsigned char *bufferPtr;
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS)
    return ret;


    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    /*
    - copy the entries from *internalEntry to the indexNum`th entry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */

    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(entryPtr, &(internalEntry->lChild), 4);
    memcpy(entryPtr + 4, &(internalEntry->attrVal), ATTR_SIZE);
    memcpy(entryPtr + 20, &(internalEntry->rChild), 4);


    // update dirty bit using setDirtyBit()
    // if setDirtyBit failed, return the value returned by the call
    int x=StaticBuffer::setDirtyBit(indexNum);
    if(x!=SUCCESS)
    return x;

    return SUCCESS;
}