#include "Algebra.h"

#include <cstring>

#include<bits/stdc++.h>


/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/

// will return if a string can be parsed as a floating point number
bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}



int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
    // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
    // return E_NOTPERMITTED;

    if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0)
    return E_NOTPERMITTED;

    // get the relation's rel-id using OpenRelTable::getRelId() method
    int relId = OpenRelTable::getRelId(relName);

    // if relation is not open in open relation table, return E_RELNOTOPEN
    if(relId<0 || relId>=12)
    return E_RELNOTOPEN;
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relcatentry;
    
    RelCacheTable::getRelCatEntry(relId,&relcatentry);

    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */
    if(relcatentry.numAttrs!=nAttrs)
    return E_NATTRMISMATCH;
    // let recordValues[numberOfAttributes] be an array of type union Attribute
    Attribute recordValues[nAttrs];

    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    // iterate through 0 to nAttrs-1: (let i be the iterator)
    for(int i=0;i<nAttrs;i++)
    {
        // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())
        AttrCatEntry attrCatBuf;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrCatBuf);
        int type=attrCatBuf.attrType;
        // let type = attrCatEntry.attrType;

        if (type==NUMBER)
        {
            // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if(isNumber(record[i]))
            {
                /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */
                   recordValues[i].nVal=std::atof(record[i]);
                  //  printf(" in algebra %d\n",(int)recordValues[i].nVal);
            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
            // copy record[i] to recordValues[i].sVal
            strcpy(recordValues[i].sVal,record[i]);
        }
    }

    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call
    int retVal=BlockAccess::insert(relId,recordValues);

    return retVal;
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
    // get the srcRel's rel-id (let it be srcRelid), using OpenRelTable::getRelId()
    int srcRelid;
    srcRelid=OpenRelTable::getRelId(srcRel);
    // if srcRel is not open in open relation table, return E_RELNOTOPEN
    // get the attr-cat entry for attr, using AttrCacheTable::getAttrCatEntry()
    // if getAttrcatEntry() call fails return E_ATTRNOTEXIST

    if(srcRelid<0 || srcRelid>11)
    {
      return E_RELNOTOPEN;
    }
                  
  AttrCatEntry attrCatEntry;
  int x=AttrCacheTable::getAttrCatEntry(srcRelid,attr,&attrCatEntry);
  if(x!=SUCCESS)
  return E_ATTRNOTEXIST;
  // get the attribute catalog entry for attr, using AttrCacheTable::getAttrcatEntry()
  //    return E_ATTRNOTEXIST if it returns the error
  /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
  int type = attrCatEntry.attrType;
  // printf("type= %d\n",type);
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {  
      // printf("no error\n");     // the isNumber() function is implemented below
      attrVal.nVal = std::atof(strVal);
    } else {
      // printf("error\n");
      return E_ATTRTYPEMISMATCH;
    }
  } else if (type == STRING) {
    // printf("its a string\n");
    strcpy(attrVal.sVal, strVal);
  }

    /*** Creating and opening the target relation ***/
    // Prepare arguments for createRel() in the following way:
    // get RelcatEntry of srcRel using RelCacheTable::getRelCatEntry()
    RelCatEntry relCatBuff;
    RelCacheTable::getRelCatEntry(srcRelid,&relCatBuff);
    // int src_nAttrs = /* the no. of attributes present in src relation */ ;
    int src_nAttrs=relCatBuff.numAttrs;

    /* let attr_names[src_nAttrs][ATTR_SIZE] be a 2D array of type char
        (will store the attribute names of rel). */
    // let attr_types[src_nAttrs] be an array of type int
    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];

    /*iterate through 0 to src_nAttrs-1 :
        get the i'th attribute's AttrCatEntry using AttrCacheTable::getAttrCatEntry()
        fill the attr_names, attr_types arrays that we declared with the entries
        of corresponding attributes
    */
    AttrCatEntry attrCatBuff;
    for(int i=0;i<src_nAttrs;i++)
    {
      AttrCacheTable::getAttrCatEntry(srcRelid,i,&attrCatBuff);
      strcpy(attr_names[i],attrCatBuff.attrName);
      attr_types[i]=attrCatBuff.attrType;
    }

    int ret_schema=Schema::createRel(targetRel,src_nAttrs,attr_names,attr_types);
    if(ret_schema!=SUCCESS)
    return ret_schema;
    /* Create the relation for target relation by calling Schema::createRel()
       by providing appropriate arguments */
    // if the createRel returns an error code, then return that value.
    int ret_open=OpenRelTable::openRel(targetRel);
    if(ret_open<0 || ret_open>11)
    {
      Schema::deleteRel(targetRel);
      return ret_open;
    }
    /* Open the newly created target relation by calling OpenRelTable::openRel()
       method and store the target relid */
    /* If opening fails, delete the target relation by calling Schema::deleteRel()
       and return the error value returned from openRel() */

    /*** Selecting and inserting records into the target relation ***/
    /* Before calling the search function, reset the search to start from the
       first using RelCacheTable::resetSearchIndex() */
      Attribute record[src_nAttrs];
      int targetRelId=OpenRelTable::getRelId(targetRel);
      RelCacheTable::resetSearchIndex(targetRelId);
    /*
        The BlockAccess::search() function can either do a linearSearch or
        a B+ tree search. Hence, reset the search index of the relation in the
        relation cache using RelCacheTable::resetSearchIndex().
        Also, reset the search index in the attribute cache for the select
        condition attribute with name given by the argument `attr`. Use
        AttrCacheTable::resetSearchIndex().
        Both these calls are necessary to ensure that search begins from the
        first record.
    */

    RelCacheTable::resetSearchIndex(srcRelid);
    AttrCacheTable::resetSearchIndex(srcRelid,attr);

    // read every record that satisfies the condition by repeatedly calling
    // BlockAccess::search() until there are no more records to be read

    while (BlockAccess::search(srcRelid,record,attr,attrVal,op)==SUCCESS) {

        // ret = BlockAccess::insert(targetRelId, record);
        int ret=BlockAccess::insert(targetRelId,record);
        // if (insert fails) {
        //     close the targetrel(by calling Schema::closeRel(targetrel))
        //     delete targetrel (by calling Schema::deleteRel(targetrel))
        //     return ret;
        // }
        if(ret!=SUCCESS)
        {
          Schema::closeRel(targetRel);
          Schema::deleteRel(targetRel);
          return ret;
        }
    }

    // Close the targetRel by calling closeRel() method of schema layer
    Schema::closeRel(targetRel);

    return SUCCESS;
}


int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {
  // ,int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId<0 || srcRelId>11)
    {
      return E_RELNOTOPEN;
    }
    RelCatEntry relCatBuff;
    RelCacheTable::getRelCatEntry(srcRelId,&relCatBuff);

    int numAttrs=relCatBuff.numAttrs;
    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];
   for(int i=0;i<numAttrs;i++)
   {
      AttrCatEntry attrCatBuff;
      AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrCatBuff);
      strcpy(attrNames[i],attrCatBuff.attrName);
      attrTypes[i]=attrCatBuff.attrType;
   }


    /*** Creating and opening the target relation ***/

    // Create a relation for target relation by calling Schema::createRel()

    // if the createRel returns an error code, then return that value.
    int x=Schema::createRel(targetRel,numAttrs,attrNames,attrTypes);
    if(x!=SUCCESS)
    {
      return x;
    }
    OpenRelTable::openRel(targetRel);
    int targetRelID=OpenRelTable::getRelId(targetRel);
    if(targetRelID<0 || targetRelID>11)
    {
      Schema::deleteRel(targetRel);
      return targetRelID;
    }
    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid

    // If opening fails, delete the target relation by calling Schema::deleteRel() of
    // return the error value returned from openRel().


    /*** Inserting projected records into the target relation ***/
  
    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(targetRelID);
    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];

    // while (/* BlockAccess::project(srcRelId, record) returns SUCCESS */)
    while(BlockAccess::project(srcRelId,record)==SUCCESS)
    {
        // record will contain the next record

        // ret = BlockAccess::insert(targetRelId, proj_record);
        int ret=BlockAccess::insert(targetRelID,record);

        if (ret!=SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close the targetRel by calling 
    Schema::closeRel(targetRel);
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    // int srcRelId = /*srcRel's rel-id (use OpenRelTable::getRelId() function)*/
    int srcRelId=OpenRelTable::getRelId(srcRel);
    if(srcRelId<0 || srcRelId>11)
    {
      return E_RELNOTOPEN;
    }
    RelCatEntry relcatbuff;
    RelCacheTable::getRelCatEntry(srcRelId,&relcatbuff);
    int numAttr_src=relcatbuff.numAttrs;
    
    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];
    /*** Checking if attributes of target are present in the source relation
         and storing its offsets and types ***/
    
    /*iterate through 0 to tar_nAttrs-1 :
        - get the attribute catalog entry of the attribute with name tar_attrs[i].
        - if the attribute is not found return E_ATTRNOTEXIST
        - fill the attr_offset, attr_types arrays of target relation from the
          corresponding attribute catalog entries of source relation
    */
    for(int i=0;i<tar_nAttrs;i++)
    {
      AttrCatEntry attrCatBuff;
     int x= AttrCacheTable::getAttrCatEntry(srcRelId,tar_Attrs[i],&attrCatBuff);
     if(x!=SUCCESS)
     return x;
     attr_offset[i]=attrCatBuff.offset;
     attr_types[i]=attrCatBuff.attrType;
      // AttrCacheTable::getAttrCatEntry()
    }
    /*** Creating and opening the target relation ***/

    int x=Schema::createRel(targetRel,tar_nAttrs,tar_Attrs,attr_types);
    if(x!=SUCCESS)
    return x;
    OpenRelTable::openRel(targetRel);
    int tarRelId=OpenRelTable::getRelId(targetRel);
  if(tarRelId<0 || tarRelId>11)
  {
    Schema::deleteRel(targetRel);
    return tarRelId;
  }
    /*** Inserting projected records into the target relation ***/
    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(srcRelId);
  RelCacheTable::resetSearchIndex(tarRelId);
    Attribute record[numAttr_src];
    // while (/* BlockAccess::project(srcRelId, record) returns SUCCESS */) 
    while(BlockAccess::project(srcRelId,record)==SUCCESS)
    {
        // the variable `record` will contain the next record
        Attribute proj_record[tar_nAttrs];
        //iterate through 0 to tar_attrs-1:
        //    proj_record[attr_iter] = record[attr_offset[attr_iter]]
        for(int i=0;i<tar_nAttrs;i++)
        {
          proj_record[i]=record[attr_offset[i]];
        }
        int ret=BlockAccess::insert(tarRelId,proj_record);
        // ret = BlockAccess::insert(targetRelId, proj_record);
        if (ret!=SUCCESS) {
            // close the targetrel by calling Schema::closeRel()
            // delete targetrel by calling Schema::deleteRel()
            // return ret;
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close the targetRel by calling Schema::closeRel()
    Schema::closeRel(targetRel);

    return SUCCESS; 
}

int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE]) {

    // get the srcRelation1's rel-id using OpenRelTable::getRelId() method
    int src1_relid=OpenRelTable::getRelId(srcRelation1);
    int src2_relid=OpenRelTable::getRelId(srcRelation2);

    // get the srcRelation2's rel-id using OpenRelTable::getRelId() method

    // if either of the two source relations is not open
    //     return E_RELNOTOPEN
    if(src1_relid<0 || src1_relid>=12 || src2_relid<0 || src2_relid>=12)
    return E_RELNOTOPEN;

    AttrCatEntry attrCatEntry1, attrCatEntry2;
    int x1=AttrCacheTable::getAttrCatEntry(src1_relid,attribute1,&attrCatEntry1);
    int x2=AttrCacheTable::getAttrCatEntry(src2_relid,attribute2,&attrCatEntry2);
    // get the attribute catalog entries for the following from the attribute cache
    // (using AttrCacheTable::getAttrCatEntry())
    // - attrCatEntry1 = attribute1 of srcRelation1
    // - attrCatEntry2 = attribute2 of srcRelation2

    // if attribute1 is not present in srcRelation1 or attribute2 is not
    // present in srcRelation2 (getAttrCatEntry() returned E_ATTRNOTEXIST)
    //     return E_ATTRNOTEXIST.
    // if(strcmp(attrCatEntry1.attrName,attribute1)!=0 || strcmp(attrCatEntry2.attrName,attribute2)!=0)
    // return E_ATTRNOTEXIST;
    if(x1!=SUCCESS)
    return x1;
    if(x2!=SUCCESS) return x2;

    // if attribute1 and attribute2 are of different types return E_ATTRTYPEMISMATCH
    if(attrCatEntry1.attrType!=attrCatEntry2.attrType)
    return E_ATTRTYPEMISMATCH;


    // iterate through all the attributes in both the source relations and check if
    // there are any other pair of attributes other than join attributes
    // (i.e. attribute1 and attribute2) with duplicate names in srcRelation1 and
    // srcRelation2 (use AttrCacheTable::getAttrCatEntry())
    // If yes, return E_DUPLICATEATTR

    RelCatEntry relcat1,relcat2;
    RelCacheTable::getRelCatEntry(src1_relid,&relcat1);
    RelCacheTable::getRelCatEntry(src2_relid,&relcat2);

    int f=0,g=0;
    for(int i=0;i<relcat1.numAttrs;i++)
    {
      AttrCatEntry attrcat;
      AttrCacheTable::getAttrCatEntry(src1_relid,i,&attrcat);
      if(strcmp(attrcat.attrName,attribute1)==0)
      f++;
    }
    for(int i=0;i<relcat2.numAttrs;i++)
    {
      AttrCatEntry attrcat;
      AttrCacheTable::getAttrCatEntry(src2_relid,i,&attrcat);
      if(strcmp(attrcat.attrName,attribute2)==0)
      g++;
    }

    if(f>1 || g>1)
    return E_DUPLICATEATTR;

    

    // get the relation catalog entries for the relations from the relation cache
    // (use RelCacheTable::getRelCatEntry() function)

    int numOfAttributes1 = relcat1.numAttrs;
    int numOfAttributes2 = relcat2.numAttrs;

    // if rel2 does not have an index on attr2
    //     create it using BPlusTree:bPlusCreate()
    //     if call fails, return the appropriate error code
    //     (if your implementation is correct, the only error code that will
    //      be returned here is E_DISKFULL)
    if(attrCatEntry2.rootBlock==-1)
    {
      int ret=BPlusTree::bPlusCreate(src2_relid,attribute2);
      // if(ret!=SUCCESS || ret!=E_DISKFULL)
      // printf("Somthing happened msitake from BPlustree code!!!");
      if(ret!=SUCCESS) return ret;
    }
    int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;
    // Note: The target relation has number of attributes one less than
    // nAttrs1+nAttrs2 (Why?)

    // declare the following arrays to store the details of the target relation
    char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
    int targetRelAttrTypes[numOfAttributesInTarget];
    int ind=0;
    for(int i=0;i<numOfAttributes1;i++)
    {
      AttrCatEntry attrcat;
      AttrCacheTable::getAttrCatEntry(src1_relid,i,&attrcat);
      strcpy(targetRelAttrNames[ind],attrcat.attrName);
      targetRelAttrTypes[ind]=attrcat.attrType;
      ind++;
    }
    for(int i=0;i<numOfAttributes2;i++)
    {
      AttrCatEntry attrcat;
      AttrCacheTable::getAttrCatEntry(src2_relid,i,&attrcat);
      if(strcmp(attrcat.attrName,attribute2)!=0)
      {
      strcpy(targetRelAttrNames[ind],attrcat.attrName);
      targetRelAttrTypes[ind]=attrcat.attrType;
      ind++;
      }
    }

    int ret=Schema::createRel(targetRelation,numOfAttributesInTarget,targetRelAttrNames,targetRelAttrTypes);
    if(ret!=SUCCESS)
    return ret;

    // iterate through all the attributes in both the source relations and
    // update targetRelAttrNames[],targetRelAttrTypes[] arrays excluding attribute2
    // in srcRelation2 (use AttrCacheTable::getAttrCatEntry())

    // create the target relation using the Schema::createRel() function

    // if createRel() returns an error, return that error

    // Open the targetRelation using OpenRelTable::openRel()
    int x=OpenRelTable::openRel(targetRelation);

    // if openRel() fails (No free entries left in the Open Relation Table)
    if(x<0 || x>=12)
    {
        // delete target relation by calling Schema::deleteRel()
        // return the error code
        Schema::deleteRel(targetRelation);
        return x;
    }

    Attribute record1[numOfAttributes1];
    Attribute record2[numOfAttributes2];
    Attribute targetRecord[numOfAttributesInTarget];
    // this loop is to get every record of the srcRelation1 one by one
    RelCacheTable::resetSearchIndex(src1_relid);
    while (BlockAccess::project(src1_relid, record1) == SUCCESS) {
        // reset the search index of `srcRelation2` in the relation cache
        // using RelCacheTable::resetSearchIndex()
        RelCacheTable::resetSearchIndex(src2_relid);

        // reset the search index of `attribute2` in the attribute cache
        // using AttrCacheTable::resetSearchIndex()
        AttrCacheTable::resetSearchIndex(src2_relid,attribute2);

        // this loop is to get every record of the srcRelation2 which satisfies
        //the following condition:
        // record1.attribute1 = record2.attribute2 (i.e. Equi-Join condition)
    ind=0;
        while (BlockAccess::search(
            src2_relid, record2, attribute2, record1[attrCatEntry1.offset], EQ
        ) == SUCCESS ) {

            // copy srcRelation1's and srcRelation2's attribute values(except
            // for attribute2 in rel2) from record1 and record2 to targetRecord

            for(int i=0;i<numOfAttributes1;i++)
            {
              targetRecord[i]=record1[i];
            }
            for(int i=0;i<numOfAttributes2;i++)
            {
              AttrCatEntry attrcat;
              AttrCacheTable::getAttrCatEntry(src2_relid,attribute2,&attrcat);
              if(strcmp(attrcat.attrName,attribute2)!=0)
              {
                targetRecord[ind]=record2[i];
                ind++;
              }
            }
            // insert the current record into the target relation by calling
            // BlockAccess::insert()
            int ret_1=BlockAccess::insert(x,targetRecord);

            // if(/* insert fails (insert should fail only due to DISK being FULL) */) 
            if(ret_1!=SUCCESS)
            {

                // close the target relation by calling OpenRelTable::closeRel()
                OpenRelTable::closeRel(x);
                Schema::deleteRel(targetRelation);
                // delete targetRelation (by calling Schema::deleteRel())
                return E_DISKFULL;
            }
        }
    }

    // close the target relation by calling OpenRelTable::closeRel()
    OpenRelTable::closeRel(x);
    return SUCCESS;
}

// int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], 
//             char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], 
//             char attribute2[ATTR_SIZE]) 
// {
//     // get the srcRelation1's rel-id using OpenRelTable::getRelId() method
//     int srcRelId1 = OpenRelTable::getRelId(srcRelation1);

//     // get the srcRelation2's rel-id using OpenRelTable::getRelId() method
//     int srcRelId2 = OpenRelTable::getRelId(srcRelation2);

//     // if either of the two source relations is not open
//     //     return E_RELNOTOPEN

//     if (srcRelId1 == E_RELNOTOPEN || srcRelId2 == E_RELNOTOPEN)
//         return E_RELNOTOPEN;

//     AttrCatEntry attrCatEntry1, attrCatEntry2;
//     // get the attribute catalog entries for the following from the attribute cache
//     // (using AttrCacheTable::getAttrCatEntry())
//     // - attrCatEntry1 = attribute1 of srcRelation1
//     // - attrCatEntry2 = attribute2 of srcRelation2
    
//     // if attribute1 is not present in srcRelation1 or attribute2 is not
//     // present in srcRelation2 (getAttrCatEntry() returned E_ATTRNOTEXIST)
//     //     return E_ATTRNOTEXIST.

//     int ret = AttrCacheTable::getAttrCatEntry(srcRelId1, attribute1, &attrCatEntry1);
//     if (ret != SUCCESS) return E_ATTRNOTEXIST;

//     ret = AttrCacheTable::getAttrCatEntry(srcRelId2, attribute2, &attrCatEntry2);
//     if (ret != SUCCESS) return E_ATTRNOTEXIST;

//     // if attribute1 and attribute2 are of different types return E_ATTRTYPEMISMATCH
//     if (attrCatEntry1.attrType != attrCatEntry2.attrType)
//         return E_ATTRTYPEMISMATCH;

//     // TODO: iterate through all the attributes in both the source relations and check if
//     // TODO: there are any other pair of attributes other than join attributes
//     // (i.e. attribute1 and attribute2) with duplicate names in srcRelation1 and
//     // srcRelation2 (use AttrCacheTable::getAttrCatEntry())
//     // If yes, return E_DUPLICATEATTR

//     // get the relation catalog entries for the relations from the relation cache
//     // (use RelCacheTable::getRelCatEntry() function)
//     RelCatEntry relCatEntryBuf1, relCatEntryBuf2;
//     RelCacheTable::getRelCatEntry(srcRelId1, &relCatEntryBuf1);
//     RelCacheTable::getRelCatEntry(srcRelId2, &relCatEntryBuf2);

//     int numOfAttributes1 = relCatEntryBuf1.numAttrs;
//     int numOfAttributes2 = relCatEntryBuf2.numAttrs;

//     for (int attrindex1 = 0; attrindex1 < numOfAttributes1; attrindex1++)
//     {
//         AttrCatEntry attrCatEntryTemp1;
//         AttrCacheTable::getAttrCatEntry(srcRelId1, attrindex1, &attrCatEntryTemp1);

//         if (strcmp(attrCatEntryTemp1.attrName, attribute1) == 0) continue;
        
//         for (int attrindex2 = 0; attrindex2 < numOfAttributes2; attrindex2++)
//         {
//             AttrCatEntry attrCatEntryTemp2;
//             AttrCacheTable::getAttrCatEntry(srcRelId2, attrindex2, &attrCatEntryTemp2);

//             if (strcmp (attrCatEntryTemp2.attrName, attribute2) == 0) continue;

//             if (strcmp (attrCatEntryTemp1.attrName, attrCatEntryTemp2.attrName) == 0)
//                 return E_DUPLICATEATTR;
//         }
//     }

//     // if rel2 does not have an index on attr2
//     //     create it using BPlusTree:bPlusCreate()
//     //     if call fails, return the appropriate error code
//     //     (if your implementation is correct, the only error code that will
//     //      be returned here is E_DISKFULL)

//     int rootBlock = attrCatEntry2.rootBlock;
//     if (rootBlock == -1)
//     {
//         ret = BPlusTree::bPlusCreate(srcRelId2, attribute2);
//         if (ret == E_DISKFULL) return E_DISKFULL;

//         rootBlock = attrCatEntry2.rootBlock;
//     }

//     int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;

//     //* Note: The target relation has number of attributes one less than
//     //* nAttrs1+nAttrs2 (Why?) -> Because one attribute on which EquiJoin is 
//     //* performed should not be present twice!

//     // declare the following arrays to store the details of the target relation
//     char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
//     int targetRelAttrTypes[numOfAttributesInTarget];

//     // iterate through all the attributes in both the source relations and
//     // update targetRelAttrNames[],targetRelAttrTypes[] arrays excluding attribute2
//     // in srcRelation2 (use AttrCacheTable::getAttrCatEntry())

//     for (int attrindex = 0; attrindex < numOfAttributes1; attrindex++)
//     {
//         AttrCatEntry attrcatentry; 
//         AttrCacheTable::getAttrCatEntry(srcRelId1, attrindex ,&attrcatentry);

//         strcpy(targetRelAttrNames[attrindex], attrcatentry.attrName);
//         targetRelAttrTypes[attrindex] = attrcatentry.attrType;
//     }

//     for (int attrindex = 0, flag = 0; attrindex < numOfAttributes2; attrindex++)
//     {
//         AttrCatEntry attrcatentry; 
//         AttrCacheTable::getAttrCatEntry(srcRelId2, attrindex ,&attrcatentry);

//         if (strcmp(attribute2, attrcatentry.attrName) == 0)
//         {
//             flag = 1;
//             continue;
//         }

//         strcpy(targetRelAttrNames[numOfAttributes1 + attrindex-flag], attrcatentry.attrName);
//         targetRelAttrTypes[numOfAttributes1 + attrindex-flag] = attrcatentry.attrType;
//     }    

//     // create the target relation using the Schema::createRel() function
//     ret = Schema::createRel(targetRelation, numOfAttributesInTarget, 
//                                 targetRelAttrNames, targetRelAttrTypes);

//     // if createRel() returns an error, return that error
//     if (ret != SUCCESS) return ret;

//     // Open the targetRelation using OpenRelTable::openRel()
//     int targetRelId = OpenRelTable::openRel(targetRelation);

//     // if openRel() fails (No free entries left in the Open Relation Table)
//     if (targetRelId < 0)
//     {
//         // delete target relation by calling Schema::deleteRel()
//         Schema::deleteRel(targetRelation);

//         // return the error code
//         return targetRelId;
//     }

//     Attribute record1[numOfAttributes1];
//     Attribute record2[numOfAttributes2];
//     Attribute targetRecord[numOfAttributesInTarget];

//     // this loop is to get every record of the srcRelation1 one by one
//     RelCacheTable::resetSearchIndex(srcRelId1);

//     while (BlockAccess::project(srcRelId1, record1) == SUCCESS) 
//     {
//         // reset the search index of `srcRelation2` in the relation cache
//         // using RelCacheTable::resetSearchIndex()
//         RelCacheTable::resetSearchIndex(srcRelId2);

//         // reset the search index of `attribute2` in the attribute cache
//         // using AttrCacheTable::resetSearchIndex()
//         AttrCacheTable::resetSearchIndex(srcRelId2, attribute2);

//         // this loop is to get every record of the srcRelation2 which satisfies
//         //the following condition:
//         // record1.attribute1 = record2.attribute2 (i.e. Equi-Join condition)
//         while (BlockAccess::search(srcRelId2, record2, attribute2, 
//                                     record1[attrCatEntry1.offset], EQ) == SUCCESS) 
//         {
//             // copy srcRelation1's and srcRelation2's attribute values(except
//             // for attribute2 in rel2) from record1 and record2 to targetRecord
//             for (int attrindex = 0; attrindex < numOfAttributes1; attrindex++)
//                 targetRecord[attrindex] = record1[attrindex];

//             for (int attrindex = 0, flag = 0; attrindex < numOfAttributes2; attrindex++)
//             {
//                 if (attrindex == attrCatEntry2.offset)
//                 {
//                     flag = 1;
//                     continue;
//                 }
//                 targetRecord[attrindex + numOfAttributes1-flag] = record2[attrindex];
//             }

//             // insert the current record into the target relation by calling
//             // BlockAccess::insert()
//             ret = BlockAccess::insert(targetRelId, targetRecord);

//             // if insert fails (insert should fail only due to DISK being FULL)
//             if (ret == E_DISKFULL)
//             {
//                 // close the target relation by calling OpenRelTable::closeRel()
//                 ret = OpenRelTable::closeRel(targetRelId);

//                 // delete targetRelation (by calling Schema::deleteRel())
//                 ret = Schema::deleteRel(targetRelation);

//                 return E_DISKFULL;
//             }
//         }
//     }

//     // close the target relation by calling OpenRelTable::closeRel()
//     return SUCCESS;
// }