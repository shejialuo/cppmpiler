#ifndef _OBJECT_OBJECT_HPP_
#define _OBJECT_OBJECT_HPP_

class Environment;

#include "ast.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using ObjectType = std::string;

/**
 * @brief Base class to represent the object
 *
 */
class Object {
public:
  virtual ObjectType type() = 0;
  virtual std::string inspect() = 0;
  virtual ~Object() = default;
};

/**
 * @brief Integer class represents int64_t
 *
 */
class Integer : public Object {
public:
  int64_t value;

  Integer() = default;
  Integer(int64_t v);

  ObjectType type() override;
  std::string inspect() override;
};

/**
 * @brief Boolean class represents bool
 *
 */
class Boolean : public Object {
public:
  bool value;

  Boolean() = default;
  Boolean(bool v);

  ObjectType type() override;
  std::string inspect() override;
};

/**
 * @brief ReturnValue class represents the return value
 *
 */
class ReturnValue : public Object {
public:
  std::shared_ptr<Object> value;

  ReturnValue() = default;

  ObjectType type() override;
  std::string inspect() override;
};

/**
 * @brief Function class represents the function
 *
 */
class Function : public Object {
public:
  std::vector<std::unique_ptr<Identifier>> parameters;
  std::unique_ptr<BlockStatement> body;
  std::shared_ptr<Environment> env;

  Function() = default;
  Function(std::vector<std::unique_ptr<Identifier>> &p,
           std::unique_ptr<BlockStatement> &&b,
           std::shared_ptr<Environment> e);

  ObjectType type() override;
  std::string inspect() override;
};

/**
 * @brief Error class represents the error
 *
 */
class Error : public Object {
public:
  std::string message;

  Error() = default;
  Error(const std::string &m);

  ObjectType type() override;
  std::string inspect() override;
};

/**
 * @brief Represents the string "foobar".
 *
 */
class String : public Object {
public:
  std::string value;

  String() = default;
  String(const std::string &);

  ObjectType type() override;
  std::string inspect() override;
};

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
  std::shared_ptr<Environment> outer;
  std::unordered_map<std::string, std::shared_ptr<Object>> store{};

public:
  Environment() = default;
  Environment(std::shared_ptr<Environment> o);
  Environment(const Environment &) = delete;
  Environment(Environment &&) = default;

  /**
   * @brief get the binding value
   *
   * @param name the identifier name
   * @return std::shared_ptr<Object>
   */
  std::shared_ptr<Object> get(const std::string &name);

  /**
   * @brief bind the identifier
   *
   * @param name the identifier name
   * @param val the new object value
   */
  void set(const std::string &name, std::shared_ptr<Object> val);
};

#endif  // _OBJECT_OBJECT_HPP_
