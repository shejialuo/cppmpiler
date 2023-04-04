#ifndef _OBJECT_BUILTINS_HPP_
#define _OBJECT_BUILTINS_HPP_

#include "object.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class Builtins {
private:
  static std::unordered_map<std::string, std::shared_ptr<Builtin>> builtins;
  static std::vector<std::string> builtinNames;

public:
  /**
   * @brief The built function len to calculate the string length
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>;
   */
  static std::shared_ptr<Object> len(std::vector<std::shared_ptr<Object>> &arguments);

  /**
   * @brief The built function to get the first element of the array
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> first(std::vector<std::shared_ptr<Object>> &arguments);

  /**
   * @brief The built function to get the last element of the array
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> last(std::vector<std::shared_ptr<Object>> &arguments);

  /**
   * @brief The built function to drop the first element of the array
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> rest(std::vector<std::shared_ptr<Object>> &arguments);

  /**
   * @brief Push a new element to the a and get the b.
   *
   * @param arguments the arguments
   * @return std::shared_ptr<Object>
   */
  static std::shared_ptr<Object> push(std::vector<std::shared_ptr<Object>> &arguments);

  static inline std::unordered_map<std::string, std::shared_ptr<Builtin>> &getBuiltins() { return builtins; }
  static inline std::vector<std::string> &getBuiltinNames() { return builtinNames; }
  static inline std::shared_ptr<Builtin> getBuiltinByIndex(int i) { return builtins[builtinNames[i]]; }
};

#endif  // _OBJECT_BUILTINS_HPP_
