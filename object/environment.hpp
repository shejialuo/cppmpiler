#ifndef _OBJECT_ENVIRONMENT_HPP
#define _OBJECT_ENVIRONMENT_HPP

#include "object.hpp"

#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Environment is used to indicate the current frame
 * environment. It is a scope and all the values associated
 * with this scope is recorded in it. When we meet a new
 * scope such as functions, we should spawn a new scope,
 * which means that we should create a new environment class.
 *
 */
class Environment {
private:
  std::unordered_map<std::string, std::unique_ptr<Object>> store{};

public:
  Environment();
  Environment(const Environment &) = delete;
  Environment(Environment &&) = default;

  /**
   * @brief get the binding value
   *
   * @param name the identifier name
   * @return std::unique_ptr<Object>
   */
  std::unique_ptr<Object> get(const std::string &name);

  /**
   * @brief bind the identifier
   *
   * @param name the identifier name
   * @param val the new object value
   */
  void set(const std::string &name, std::unique_ptr<Object> &&val);
};

#endif  // _OBJECT_ENVIRONMENT_HPP
