/* gc_test.c - test GC garbage collection.
   for checks using dbg_alloc:
       https://github.com/TheCRatTime/dbg_alloc/
*/

#define DBG_ALLOC_SOURCE
#define GC_MALLOC_SOURCE
#define WRAP_REAL_MALLOC
#define DEBUG

#include <dbg_alloc.h>
#include <gcmalloc.h>

#include <stdio.h>

static void PtrHandler(void* ptr) {
  printf("GC really freed %p\n", ptr);
}

typedef struct hard_struct {
  struct hard_struct* cycle_dep;
} hard_struct;

static void test_func(void) {
  hard_struct* ptr1 = NULL;
  hard_struct* ptr2 = NULL;
  hard_struct* garbage = NULL; /* unused */

  GC_INIT();

  ptr1 = (hard_struct*)gcmalloc(sizeof(hard_struct));
  ptr2 = (hard_struct*)gcmalloc(sizeof(hard_struct)); /* No registering to roots */
  garbage = (hard_struct*)gcmalloc(sizeof(hard_struct)); /* unused */

  if (!ptr1 || !ptr2 || !garbage) {
    /* GC automatically free objects that was allocated. */
    GCreturn;
  }

  /* If you want, you can set destructor, for example: */
  gc_reg_destructor(ptr1, PtrHandler);
  gc_reg_destructor(ptr2, PtrHandler);
  gc_reg_destructor(garbage, PtrHandler);

  run_gc();
  
  GC_DEINIT();
}

int main(void) {
  InitAllocator(4096);
  
  printf("TestFunc()\n");
  
  test_func();
  
  printf("RunGC()\n");

  run_gc();

  AllocatorLog();
  KillAllocator();
  
  return 0;
}
