#include "NewDelete.hpp"
#include "core/Debug.hpp"

static Allocator *g_CPPAllocator;

void SetCppAllocator(Allocator *cppAllocator) { g_CPPAllocator = cppAllocator; }

void *operator new(size_t size) throw() {
  if (!g_CPPAllocator)
    return nullptr;

  return g_CPPAllocator->Allocate(size);
}

void *operator new[](size_t size) throw() {
  if (!g_CPPAllocator)
    return nullptr;

  return g_CPPAllocator->Allocate(size);
}

void operator delete(void *p) {
  if (g_CPPAllocator)
    g_CPPAllocator->Free(p);
}

void operator delete[](void *p) {
  if (g_CPPAllocator)
    g_CPPAllocator->Free(p);
}
