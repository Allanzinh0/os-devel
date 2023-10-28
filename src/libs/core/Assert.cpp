#include "Assert.hpp"

#include "Debug.hpp"
#include <core/arch/i686/IO.hpp>

bool Assert_(const char *condition, const char *filename, int line,
             const char *func) {
  Debug::Critical("Assert", "Assertion failed: %s", condition);
  Debug::Critical("Assert", " > In file %s line %d function %s", filename, line,
                  func);

  arch::i686::Panic();

  return true;
}
