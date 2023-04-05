#include "frame.hpp"

Instructions &Frame::instructions() { return closure->fn->instructions; }
