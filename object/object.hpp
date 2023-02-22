#ifndef _OBJECT_OBJECT_HPP_
#define _OBJECT_OBJECT_HPP_

#include <cstdint>
#include <memory>
#include <string>

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
 * @brief  ReturnValue class represents the return value
 *
 */
class ReturnValue : public Object {
public:
  std::unique_ptr<Object> value;

  ReturnValue() = default;

  ObjectType type() override;
  std::string inspect() override;
};

class Error : public Object {
public:
  std::string message;

  Error() = default;
  Error(const std::string &m);

  ObjectType type() override;
  std::string inspect() override;
};

#endif  // _OBJECT_OBJECT_HPP_
