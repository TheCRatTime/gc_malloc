/*
 * Copyright (C) 2026 TheCRatTime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GC_MALLOC_GC_MALLOC_H_
#define GC_MALLOC_GC_MALLOC_H_

#include <stddef.h>

#define GC_NOT_MARKED 0
#define GC_MARKED     1
#define GC_MARKED_BAD 2

#define GC_MAGIC_TOTAL 4
#define GC_MAGIC1 0xCC
#define GC_MAGIC2 0xFF
#define GC_MAGIC3 0xDD
#define GC_MAGIC4 0xAA

/* GC unsigned char alias */
typedef unsigned char gc_u8;

/* GC destructor function */
typedef void (*gc_destructor)(void*);

/* gc_object struct */
typedef struct gc_object {
  struct gc_object* next;
  struct gc_object* next_root;
  gc_destructor destructor;
  char magic[GC_MAGIC_TOTAL];
  gc_u8 marked;
} gc_object;

/* GC state struct */
typedef struct gc_state {
  gc_object* roots;
  gc_object* all_ptrs;
} gc_state;

/* Global GC state */
extern gc_state g_gc_state;

/* Allocate memory.
   Return value is malloc() function like. */
void* gcmalloc(size_t size);

/* Collect garbage in program */
void run_gc(void);

/* Register destructor that will be called
   when PTR is free */
void gc_reg_destructor(void* ptr, gc_destructor dsr);

#define GC_INIT_FRAME(name) \
  gc_object* name = g_gc_state.roots

#define GC_DEINIT_FRAME(name) \
  g_gc_state.roots = name

#define GC_INIT() \
  gc_object* gc__end_roots = g_gc_state.roots

#define GC_DEINIT() \
  g_gc_state.roots = gc__end_roots

#define GCreturn \
  GC_DEINIT(); return

#endif /* GC_MALLOC_GC_MALLOC_H_ */

#ifdef GC_MALLOC_SOURCE

#include <stdlib.h>

#ifdef GC_THOUGHTS
# define DEBUG
#endif

#ifdef DEBUG
# include <stdio.h>
# define GC_MSG(msg)          printf(msg)
# define GC_MSG_PTR(msg, ptr) printf(msg, ptr)
#else
# define GC_MSG(msg)          (void)msg
# define GC_MSG_PTR(msg, ptr) (void)ptr
#endif

#ifdef GC_THOUGHTS
#  define GC_THOUGHT(msg)     printf(msg)
# else
#  define GC_THOUGHT(msg)     (void)msg
# endif

gc_state g_gc_state = {NULL, NULL};

#define gc_get_raw(obj) (void*)((char*)(obj) + sizeof(gc_object))

#define gc_mark(obj) do { \
  if (obj) { \
    obj->marked = GC_MARKED; \
  } \
} while (0)

#define gc_unmark(obj) do { \
  if ((obj)) { \
    (obj)->marked = GC_NOT_MARKED; \
  } \
} while (0)

static int gc_invalid(gc_object* obj) {
  if (!obj ||
      obj->magic[0] != GC_MAGIC1 ||
      obj->magic[1] != GC_MAGIC2 ||
      obj->magic[2] != GC_MAGIC3 ||
      obj->magic[3] != GC_MAGIC4 ||
      obj->marked >= GC_MARKED_BAD) {
    return 1;
  }

  return 0;
}

static void gc_init_magic(gc_object* obj) {
  if (obj) {
    obj->magic[0] = GC_MAGIC1;
    obj->magic[1] = GC_MAGIC2;
    obj->magic[2] = GC_MAGIC3;
    obj->magic[3] = GC_MAGIC4;
  }
}

static void gc_register_root(gc_object* obj) {
  if (gc_invalid(obj)) {
    return;
  }

  obj->next_root = g_gc_state.roots;
  g_gc_state.roots = obj;
}

static void gc_init_object(gc_object* obj) {
  if (obj) {
    gc_init_magic(obj);
    gc_unmark(obj);
    obj->destructor = NULL;
  }
}

void* gcmalloc(size_t size) {
  void* ret = NULL;
  gc_object* as_obj = NULL;
  
  if (size == 0) {
    return NULL;
  }

  ret = malloc(sizeof(gc_object) + size);
  if (!ret) {
    return NULL;
  }

  as_obj = (gc_object*)ret;
  gc_init_object(as_obj);

  as_obj->next = g_gc_state.all_ptrs;
  g_gc_state.all_ptrs = as_obj;

  gc_register_root(as_obj);

  GC_MSG_PTR("GCMalloc: new pointer: %p\n", gc_get_raw(as_obj));
  GC_THOUGHT("GCMalloc: a new garbage?\n");

  return gc_get_raw(as_obj);
}

static void gc_sweep_stage(void) {
  gc_object** cur = &g_gc_state.all_ptrs;

  while (*cur != NULL) {
    if ((*cur)->marked != GC_MARKED) {
      gc_object* saved = *cur;

      GC_MSG_PTR("GC: pointer %p ", gc_get_raw(saved));
      
#ifdef GC_THOUGHTS
      GC_THOUGHT("comes to hell\n");
#else
      GC_MSG("haven't roots\n");
#endif

      *cur = (*cur)->next;

      if (saved->destructor) {
        saved->destructor(gc_get_raw(saved));
      }

      free(saved);
      continue;
    }

    gc_unmark(*cur);

    cur = &(*cur)->next;
  }
}

static void gc_mark_stage(void) {
  gc_object* cur = NULL;
  cur = g_gc_state.roots;
  
  for (; cur != NULL; cur = cur->next_root) {
    gc_mark(cur);
  }
}

void run_gc(void) {
  /* =-=-=-=-=-= MARK & SWEEP GC =-=-=-=-=-= */
  GC_MSG("GC: running...\n");

  /* 1. Marking living objects */
  gc_mark_stage();

  /* 2. Deleting dead objects. */
  gc_sweep_stage();
  
  GC_MSG("GC: end of GC log\n");
  GC_THOUGHT("GC: garbage not stops.\n");
}

void gc_reg_destructor(void* ptr, gc_destructor dsr) {
  gc_object* obj = NULL;
  
  if (!ptr || !dsr) {
    return;
  }
  
  obj = (gc_object*)(void*)((char*)ptr - sizeof(gc_object));

  if (gc_invalid(obj)) {
    return;
  }

  obj->destructor = dsr;
}

#endif /* GC_MALLOC_SOURCE */
