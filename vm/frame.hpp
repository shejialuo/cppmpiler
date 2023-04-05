#ifndef _VM_FRAME_HPP_
#define _VM_FRAME_HPP_

#include "code.hpp"
#include "object.hpp"

#include <memory>

class Frame {
public:
  std::shared_ptr<Closure> closure;
  int ip;
  int basePointer;

  Frame() = default;
  Frame(const Frame &f) = default;
  Frame(Frame &&f) = default;
  Frame(std::shared_ptr<Closure> &closure_, int basePointer_) : closure{closure_}, ip{-1}, basePointer{basePointer_} {}

  Instructions &instructions();
};

#endif  // _VM_FRAME_HPP_
