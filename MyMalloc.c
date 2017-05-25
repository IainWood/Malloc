/*
 * CS252: MyMalloc Project
 *
 * The current implementation gets memory from the OS
 * every time memory is requested and never frees memory.
 *
 * You will implement the allocator as indicated in the handout,
 * as well as the deallocator.
 * 
 * You will also need to add the necessary locking mechanisms to 
 * support multi-threaded programs.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include "MyMalloc.c"

static pthread_mutex_t mutex;
const int arenaSize = 2097152;
void increaseMallocCalls() { _mallocCalls++; }
void increasedReallocCalls() { _reallocCalls++; }
void increaseCallocCalls() { _callocCalls++; }
void increaseFreeCalls() { _freeCalls++; }

extern void atExitHandlerInC(){
  atExitHandler();
}

/*
 * Initial setup of allocator. First chunk is retrieved from the OS,
 * and the fence posts and freeList are initialized.
 */
void initialize(){
  //Environment var VERBOSE prints stats at end and turns on debugging
  //Default is on
  _verbose = 1;
  const char *envverbose = getenv("MALLOCVERBOSE");
  if(envverbose && !strcmp(envverbose, "NO")){
    verbose = 0;
  }
  
  pthread_mutex_init(&mutex, NULL);
  void *_mem = getMemoryFromOS(arenaSize);
  
  //In verbose mode register also printing statistics at end
  atexit(atExitHandlerInC);
  
  //establish fence posts
  ObjectHeader * fencePostHead = (ObjectHeader *)_mem;
  fencePostHead->_allocated = 1;
  fencePostHead->_objectSize = 0;
  
  //Set up the sentinel as the start of the freelist
  _freeList = &_freeListSentinel;
  
  //Initailize the list to point to the _mem
  temp = (char *)_mem + sizeof(ObjectHeader);
  ObjectHeader *currentHeader = (ObjectHeader *)temp;
  currentHeader->_objectSize = arenaSize - (2*sizeof(ObjectHeader)); // ~2MB
  currentHeader->_leftObjectSize = 0;
  currentHeader->_allocated = 0;
  currentHeader->_listNext = _freeList;
  currentHeader->_listPrev = _freeList;
  _freeList->_listNext = currentHeader;
  _freeList->_listPrev = currentHeader;
  
  //Set the start of the allocated memory
  _memStart = (char *)currentHeader;
  
  _initialized = 1;
}

