#include "Frontend.h"

#include <cstring>
#include <iostream>

int Frontend::create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  int ret=Schema::createIndex(relname,attrname);
  return ret;
}

int Frontend::drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  int ret=Schema::dropIndex(relname,attrname);
  return ret;
}

int Frontend::open_table(char relname[ATTR_SIZE]) {
  // Schema::openRel
  return Schema::openRel(relname);
  return SUCCESS;
}

int Frontend::close_table(char relname[ATTR_SIZE]) {
  // Schema::closeRel
  return Schema::closeRel(relname);
  return SUCCESS;
}

int Frontend::alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE]) {
  return Schema::renameRel(relname_from, relname_to);
}

int Frontend::alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[ATTR_SIZE],
                                        char attrname_to[ATTR_SIZE]) {
  return Schema::renameAttr(relname, attrname_from, attrname_to);
}

int Frontend::create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE], int type_attrs[]) {
  return Schema::createRel(relname, no_attrs, attributes, type_attrs);
}

int Frontend::drop_table(char relname[ATTR_SIZE]) {
  // return SUCCESS;
  return Schema::deleteRel(relname);
}

int Frontend::insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE]) {
  return Algebra::insert(relname, attr_count, attr_values);
}

int Frontend::select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE]) {
  // Algebra::project
  Algebra::project(relname_source,relname_target);
  return SUCCESS;
}

int Frontend::select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                         int attr_count, char attr_list[][ATTR_SIZE]) {
  // Algebra::project
  // int project(char srcRel[16], char targetRel[16], int tar_nAttrs, char tar_Attrs[][16])
  return Algebra::project(relname_source,relname_target,attr_count,attr_list);
}

int Frontend::select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                      char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]) {
  // Algebra::select
  return Algebra::select(relname_source,relname_target,attribute,op,value);;
}

int Frontend::select_attrlist_from_table_where(
    char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
    int attr_count, char attr_list[][ATTR_SIZE],
    char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]) {
      char temp[16];
      strcpy(temp,TEMP);
      int x=Algebra::select(relname_source,temp,attribute,op,value);
    // Call select() method of the Algebra Layer with correct arguments to
    // create a temporary target relation with name ".temp" (use constant TEMP)
    if(x!=SUCCESS)
    return x;
    int id=OpenRelTable::openRel(temp);
    if(id<0 || id>11)
    {
      Schema::deleteRel(temp);
      return id;
    }
    x=Algebra::project(temp,relname_target,attr_count,attr_list);
    OpenRelTable::closeRel(id);
    Schema::deleteRel(temp);
    if(x!=SUCCESS)
    return x;
    return SUCCESS;
}

int Frontend::select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                     char relname_target[ATTR_SIZE],
                                     char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE]) {
  // Algebra::join
  int ret=Algebra::join(relname_source_one,relname_source_two,relname_target,join_attr_one,join_attr_two);
  return ret;
  // join(char *srcRelOne, char *srcRelTwo, char *targetRel, char *attrOne, char *attrTwo)
  // return SUCCESS;
}

int Frontend::select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                              char relname_target[ATTR_SIZE],
                                              char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE],
                                              int attr_count, char attr_list[][ATTR_SIZE]) {
  // Algebra::join + project

  char temp[16];
      strcpy(temp,TEMP);
      int x=Algebra::join(relname_source_one,relname_source_two,temp,join_attr_one,join_attr_two);
    // Call select() method of the Algebra Layer with correct arguments to
    // create a temporary target relation with name ".temp" (use constant TEMP)
    if(x!=SUCCESS)
    return x;
    int id=OpenRelTable::openRel(temp);
    if(id<0 || id>11)
    {
      Schema::deleteRel(temp);
      return id;
    }
    x=Algebra::project(temp,relname_target,attr_count,attr_list);
    OpenRelTable::closeRel(id);
    Schema::deleteRel(temp);
    if(x!=SUCCESS)
    return x;
    return SUCCESS;
}

int Frontend::custom_function(int argc, char argv[][ATTR_SIZE]) {
  // argc gives the size of the argv array
  // argv stores every token delimited by space and comma

  // implement whatever you desire

  return SUCCESS;
}