#ifndef _VM_FRAME_HPP_
#define _VM_FRAME_HPP_

#include "code.hpp"
#include "object.hpp"

#include <memory>

class Frame {
public:
  std::shared_ptr<CompiledFunction> fn;
  int ip;

  Frame() = default;
  Frame(const Frame &f) = default;
  Frame(Frame &&f) = default;
  Frame(std::shared_ptr<CompiledFunction> &fn_, int ip_) : fn{fn_}, ip{ip_} {}

  Instructions &instructions();
};

#endif  // _VM_FRAME_HPP_
