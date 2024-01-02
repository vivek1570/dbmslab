  #include "StaticBuffer.h"
#include<bits/stdc++.h>

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];
// RELCAT_BLOCK
StaticBuffer::StaticBuffer() {

  // initialise all blocks as fre
  // unsigned char temp[4][BLOCK_SIZE];
  // int x=0;
  // for(int i=0;i<4;i++)
  // {
  //   Disk::readBlock(temp[i],i);
  //   memcpy(blockAllocMap+x,temp[i],2048);
  //   x=x+2048;
  // }
  for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];
		Disk::readBlock(buffer, blockIndex);

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++)
			StaticBuffer::blockAllocMap[blockAllocMapSlot] = buffer[slot];
	}
  for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    metainfo[bufferIndex].free= true;
    metainfo[bufferIndex].dirty=false;
    metainfo[bufferIndex].timeStamp=-1;
    metainfo[bufferIndex].blockNum=-1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {
  //  unsigned char temp[4][BLOCK_SIZE];
  // int x=0;
  // for(int i=0;i<4;i++)
  // {
  //   memcpy(temp[i],blockAllocMap+x,2048);
  //   Disk::writeBlock(temp[i],i);
  //   x=x+2048;
  // }
  for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++) 
			buffer[slot] = blockAllocMap[blockAllocMapSlot];

		Disk::writeBlock(buffer, blockIndex);
	}
  for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
    if(metainfo[bufferIndex].free==false && metainfo[bufferIndex].dirty==true)
    {
      Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
    }
  }
}
int StaticBuffer::getFreeBuffer(int blockNum){
  
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
     if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
    // increase the timeStamp in metaInfo of all occupied buffers.
    for(int i=0;i<BUFFER_CAPACITY;i++)
    {
      if(metainfo[i].free==false)
      {
        metainfo[i].timeStamp++;
      }
    }
    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum=-1;
   
    // iterate through metainfo and check if there is any buffer free

    // if a free buffer is available, set bufferNum = index of that free buffer.
   for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
     if(metainfo[bufferIndex].free)
     {
      bufferNum=bufferIndex;
      break;
     }
  }
    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer
    int ind=-1;
  if(bufferNum==-1)
  {
    int x=0;
    for(int i=0;i<BUFFER_CAPACITY;i++)
    {
      if(metainfo[i].timeStamp>=x)
      {
        ind=i;
        x=metainfo[i].timeStamp;
      }
    }
     if(metainfo[ind].dirty==true)
    {
      Disk::writeBlock(blocks[ind],metainfo[ind].blockNum);
    }
  bufferNum=ind;
  }
    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    // return the bufferNum.
  metainfo[bufferNum].free=false;
  metainfo[bufferNum].blockNum=blockNum;
  metainfo[bufferNum].dirty=false;
  metainfo[bufferNum].timeStamp=0;
  return bufferNum;
}
/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  
  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++/*bufferIndex = 0 to BUFFER_CAPACITY-1*/) {
     if(metainfo[bufferIndex].blockNum==blockNum)
     {
      return bufferIndex;
      break;
     }
  }
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
  int bufferNum=StaticBuffer::getBufferNum(blockNum);
    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
  
    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
  if(bufferNum==E_BLOCKNOTINBUFFER)
  {
    return E_BLOCKNOTINBUFFER;
  }
else if(bufferNum==E_OUTOFBOUND)
return E_OUTOFBOUND;
else{
  metainfo[bufferNum].dirty=true;
}
    return SUCCESS;
}

int StaticBuffer::getStaticBlockType(int blockNum){
    if(blockNum<0 || blockNum>=DISK_BLOCKS)
    return E_OUTOFBOUND;
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    int x=(int)blockAllocMap[blockNum];
    return x;
    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
}