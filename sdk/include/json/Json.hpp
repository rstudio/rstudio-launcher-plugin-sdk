/*
 * Json.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_JSON_HPP
#define LAUNCHER_PLUGINS_JSON_HPP

#include "Error.hpp"
#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace json {

class Array;
class Object;

/**
 * @brief Enum which represents the type of a json value.
 */
enum class Type
{
   ARRAY,
   BOOL,
   DOUBLE,
   FLOAT,
   INT,
   INT64,
   OBJECT,
   STRING,
   UINT,
   UINT64,
   NULL_TYPE,
   UNKNOWN
};

/**
 * @brief Class which represents a json value.
 */
class Value
{
protected:
   // Private implementation of Value is defined first so it may be referred to.
   PRIVATE_IMPL(m_impl);

   typedef std::unique_ptr<Impl, ImplDeleter> ValueImplPtr;

   friend class Array;

public:
   /**
    * @brief Constructor.
    */
   Value();

   /**
    * @brief Constructor. Creates a JSON value from a Value::Impl object.
    *
    * @param in_valueImpl   The Value::Impl object to use for the creation of this JSON value.
    */
   explicit Value(ValueImplPtr in_valueImpl);

   /**
    * @brief Copy constructor.
    *
    * @param in_other   The value to copy.
    */
   Value(const Value& in_other);

   /**
    * @brief Move constructor.
    *
    * @param in_other   The value to move from.
    */
   Value(Value&& in_other) noexcept;

   /**
    * @brief Conversion constructors.
    *
    * @param in_value   The literal value to set this JSON Value to.
    */
   explicit Value(bool in_value);
   explicit Value(double in_value);
   explicit Value(float in_value);
   explicit Value(int in_value);
   explicit Value(int64_t in_value);
   explicit Value(const char* in_value);
   explicit Value(const std::string& in_value);
   explicit Value(unsigned int in_value);
   explicit Value(uint64_t in_value);

   /**
    * @brief Virtual destructor.
    */
   virtual ~Value() = default;

   /**
    * @brief Assignment operator from Value.
    *
    * @param in_other   The value to copy to this value.
    *
    * @return A reference to this value.
    */
   Value& operator=(const Value& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other   The value to move to this value.
    *
    * @return A reference to this value.
    */
   Value& operator=(Value&& in_other) noexcept;

   /**
    * @brief Assignment operators.
    *
    * @param in_value   The literal value to set this JSON Value to.
    *
    * @return A reference to this value.
    */
   Value& operator=(bool in_value);
   Value& operator=(double in_value);
   Value& operator=(float in_value);
   Value& operator=(int in_value);
   Value& operator=(int64_t in_value);
   Value& operator=(const char* in_value);
   Value& operator=(const std::string& in_value);
   Value& operator=(unsigned int in_value);
   Value& operator=(uint64_t in_value);

   /**
    * @brief Equality operator.
    *
    * @param in_other   The value to compare this value to.
    *
    * @return True if the two values are the same; false otherwise.
    */
   bool operator==(const Value& in_other) const;

   /**
    * @brief Gets the value as a JSON array. If the call to getType() does not return Type::ARRAY, this method is
    *        invalid.
    *
    * @return The value as a JSON array.
    */
   Array getArray() const;

   /**
    * @brief Gets the value as a bool. If the call to getType() does not return Type::BOOL, this method is invalid.
    *
    * @return The value as a bool.
    */
   bool getBool() const;

   /**
    * @brief Gets the value as a double. If the call to getType() does not return Type::DOUBLE, this method is invalid.
    *
    * @return The value as a double.
    */
   double getDouble() const;

   /**
    * @brief Gets the value as a float. If the call to getType() does not return Type::FLOAT, this method is invalid.
    *
    * @return The value as a float.
    */
   float getFloat() const;

   /**
    * @brief Gets the value as an int. If the call to getType() does not return Type::INT, this method is invalid.
    *
    * @return The value as an int.
    */
   int getInt() const;

   /**
    * @brief Gets the value as an int64. If the call to getType() does not return Type::INT64, this method is invalid.
    *
    * @return The value as an int64.
    */
   int64_t getInt64() const;

   /**
    * @brief Gets the value as a JSON object. IF the call to getType() does not return Type::OBJECT, this method is
    *        invalid.
    *
    * @return The value as a JSON object.
    */
   Object getObject() const;

   /**
    * @brief Gets the value as a string. If the call to getType() does not return Type::STRING, this method is invalid.
    *
    * @return The value as a string.
    */
   std::string getString() const;

   /**
    * @brief Gets the type of this value.
    *
    * @return The type of this value.
    */
   Type getType() const;

   /**
    * @brief Gets the value as an unsigned int. If the call to getType() does not return Type::UINT, this method is
    *        invalid.
    *
    * @return The value as an unsigned int.
    */
   unsigned int getUInt() const;

   /**
    * @brief Gets the value as an uint64. If the call to getType() does not return Type::UINT64, this method is invalid.
    *
    * @return The value as an uint64.
    */
   uint64_t getUInt64() const;

   /**
    * @brief Checks whether the value is null or not.
    *
    * @return True if the value is null; false otherwise.
    */
   bool isNull() const;

   /**
    * @brief Parses the json string into this value.
    *
    * @param in_jsonStr     The json string to parse.
    *
    * @return Success on successful parse; error otherwise (e.g. TODO)
    */
   Error parse(const char* in_jsonStr);
   Error parse(const std::string& in_jsonStr);
};

/**
 * @brief Class which represents a specific type of JSON Value: a JSON object.
 */
class Object : Value
{
public:
   /**
    * @brief Class which represents a single member of a JSON object.
    */
   class Member
   {
   public:
      /**
       * @brief Default Constructor.
       */
       Member() = default;

      /**
       * @brief Constructor.
       *
       * @param in_name     The name of the member.
       * @param in_value    The value of the member.
       */
      Member(std::string in_name, Value in_value);

      /**
       * @brief Gets the name of the member.
       *
       * @return The name of the member.
       */
      const std::string& getName() const;

      /**
       * @brief Gets the value of the member.
       *
       * @return The value of the member.
       */
      const Value& getValue() const;

   private:
      // The name of the member.
      std::string m_name;

      // The value of the member.
      Value m_value;
   };

   /**
    * @brief Class which allows iterating over the members of a JSON object.
    */
   class Iterator: public std::iterator<std::bidirectional_iterator_tag,   // iterator_category
                                        Member,                            // value_type
                                        std::ptrdiff_t,                    // difference_type
                                        const Member*,                     // pointer
                                        Member>                            // reference
   {
   public:
      /**
       * @brief Constructor.
       *
       * @param in_parent       The parent object which will be iterated.
       * @param in_startPos     The starting position of the iterator. Default: the first member.
       */
      explicit Iterator(const Object* in_parent, std::ptrdiff_t in_startPos = 0);

      /**
       * @brief Copy constructor.
       *
       * @param in_other    The iterator to copy.
       */
      Iterator(const Iterator& in_other) = default;

      /**
       * @brief Assignment operator.
       *
       * @param in_other    The iterator to copy.
       *
       * @return A reference to this iterator.
       */
      Iterator& operator=(const Iterator& in_other);

      /**
       * @brief Pre-increment operator.
       *
       * @return A reference to this operator, incremented by one position.
       */
      Iterator& operator++();

      /**
       * @brief Pre-decrement operator.
       *
       * @return A reference to this operator, decremented by one position.
       */
      Iterator& operator--();

      /**
       * @brief Post-increment operator.
       *
       * @return A copy of this operator prior to this increment.
       */
      Iterator operator++(int);

      /**
       * @brief Post-decrement operator.
       *
       * @return A copy of this operator prior to this decrement.
       */
      Iterator operator--(int);

      /**
       * @brief Equality operator.
       *
       * @return True if this iterator is the same as in_other; false otherwise.
       */
      bool operator==(const Iterator& in_other) const;

      /**
       * @brief Dereference operator.
       *
       * @return A reference to the value this iterator is currently pointing at.
       */
      reference operator*() const;

   private:
      // The parent object that is being iterated.
      const Object* m_parent;

      // The current position.
      std::ptrdiff_t m_pos;

      // Let the parent class manipulate its iterators.
      friend class Object;
   };

   // Reverse iterator for a JSON object.
   typedef std::reverse_iterator<Iterator> ReverseIterator;

   /**
    * @brief Constructs an empty JSON object.
    */
   Object();

   /**
    * @brief Copy constructor.
    *
    * @param in_other   The JSON object to copy from.
    */
   Object(const Object& in_other) = default;

   /**
    * @brief Move constructor.
    *
    * @param in_other   The JSON object to move to this Object.
    */
   Object(Object&& in_other) = default;

   /**
    * @brief Assignment operator.
    *
    * @param in_other   The JSON object to copy from.
    *
    * @return A reference to this JSON object.
    */
   Object& operator=(const Object& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other   The JSON object to move from.
    *
    * @return A reference to this JSON object.
    */
   Object& operator=(Object&& in_other) noexcept;

   /**
    * @brief Accessor operator. Gets the value a member of this JSON object by name. If no such object exists, an empty
    *        JSON value will be returned.
    *
    * @param in_name    The name of the member to access.
    *
    * @return The value of the member with the specified name, if it exists; empty JSON value otherwise.
    */
   Value operator[](const char* in_name);
   Value operator[](const std::string& in_name);

   /**
    * @brief Finds a JSON member by name.
    *
    * @param in_name    The name of the member to find.
    *
    * @return If such a member exists, an iterator pointing to that member; the end iterator otherwise.
    */
   Iterator find(const char* in_name) const;
   Iterator find(const std::string& in_name) const;

   /**
    * @brief Gets an iterator pointing to the first member of this object.
    *
    * @return An iterator pointing to the first member of this object.
    */
   Iterator begin() const;

   /**
    * @brief Gets an iterator after the last member of this object.
    *
    * @return An iterator after the last member of this object.
    */
   Iterator end() const;

   /**
    * @brief Gets an iterator pointing to the last member of this object, which iterates in the reverse direction.
    *
    * @return A reverse iterator pointing to the last member of this object.
    */
   ReverseIterator rbegin() const;

   /**
    * @brief Gets an iterator before the first member of this object, which can be compared with an other
    *        ReverseIterator to determine when reverse iteration has ended.
    *
    * @return An iterator before the first member of this object.
    */
   ReverseIterator rend() const;

   /**
    * @brief Clears the JSON object.
    */
   void clear();

   /**
    * @brief Erases a member by name.
    *
    * @param in_name    The name of the member to erase.
    *
    * @return True if a member was erased; false otherwise.
    */
   bool erase(const char* in_name);
   bool erase(const std::string& in_name);

   /**
    * @brief Erases the member specified by the provided iterator.
    *
    * @param in_itr     The iterator pointing to the member to erase.
    *
    * @return An iterator pointing to the member immediately after the erased member.
    */
   Iterator erase(const Iterator& in_itr);

   /**
    * @brief Gets the number of members in the JSON object.
    *
    * @return The number of members in the JSON object.
    */
   size_t getSize() const;

   /**
    * @brief Checks whether this object has a member with the specified name.
    *
    * @param in_name    The name of the member for which to check.
    *
    * @return True if a member with the specified name exists; false otherwise.
    */
   bool hasMember(const char* in_name);
   bool hasMember(const std::string& in_name);

   /**
    * @brief Inserts the specified member into this JSON object. If an object with the same name already exists, it will be
    *        overriden.
    *
    * @param in_member      The member to insert into this JSON object.
    */
   void insert(const Member& in_member);

   /**
    * @brief Checks whether the JSON object is empty.
    *
    * @return True if the JSON object has no members; false otherwise.
    */
   bool isEmpty() const;

private:
   /**
    * @brief Constructs a JSON object from a JSON value. The specified value should return Type::OBJECT from getType().
    *
    * @param in_value   A JSON value which returns Type::OBJECT from getType();
    */
   explicit Object(const Value& in_value);

   friend class Value;
   friend class Iterator;
};

class Array : public Value
{
   /**
    * @brief Class which allows iterating over the members of a JSON object.
    */
   class Iterator: public std::iterator<std::bidirectional_iterator_tag,   // iterator_category
                                        Value,                             // value_type
                                        std::ptrdiff_t,                    // difference_type
                                        const Value*,                      // pointer
                                        Value>                             // reference
   {
   public:
      /**
       * @brief Constructor.
       *
       * @param in_parent       The parent array which will be iterated.
       * @param in_startPos     The starting position of the iterator. Default: the first member.
       */
      explicit Iterator(const Array* in_parent, std::ptrdiff_t in_startPos = 0);

      /**
       * @brief Copy constructor.
       *
       * @param in_other    The iterator to copy.
       */
      Iterator(const Iterator& in_other) = default;

      /**
       * @brief Assignment operator.
       *
       * @param in_other    The iterator to copy.
       *
       * @return A reference to this iterator.
       */
      Iterator& operator=(const Iterator& in_other);

      /**
       * @brief Pre-increment operator.
       *
       * @return A reference to this operator, incremented by one position.
       */
      Iterator& operator++();

      /**
       * @brief Pre-decrement operator.
       *
       * @return A reference to this operator, decremented by one position.
       */
      Iterator& operator--();

      /**
       * @brief Post-increment operator.
       *
       * @return A copy of this operator prior to this increment.
       */
      Iterator operator++(int);

      /**
       * @brief Post-decrement operator.
       *
       * @return A copy of this operator prior to this decrement.
       */
      Iterator operator--(int);

      /**
       * @brief Equality operator.
       *
       * @return True if this iterator is the same as in_other; false otherwise.
       */
      bool operator==(const Iterator& in_other) const;

      /**
       * @brief Dereference operator.
       *
       * @return A reference to the value this iterator is currently pointing at.
       */
      reference operator*() const;

   private:
      // The parent array that is being iterated.
      const Array* m_parent;

      // The current position.
      std::ptrdiff_t m_pos;

      // Allow the array class to manipulate its own iterators.
      friend class Array;
   };

   // Reverse iterator for a JSON object.
   typedef std::reverse_iterator<Iterator> ReverseIterator;

   /**
    * @brief Constructs an empty JSON array.
    */
   Array();

   /**
    * @brief Copy constructor.
    *
    * @param in_other   The JSON array to copy from.
    */
   Array(const Array& in_other) = default;

   /**
    * @brief Move constructor.
    *
    * @param in_other   The JSON array to move to this Object.
    */
   Array(Array&& in_other) = default;

   /**
    * @brief Assignment operator.
    *
    * @param in_other   The JSON array to copy from.
    *
    * @return A reference to this JSON array.
    */
   Array& operator=(const Array& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other   The JSON Array to move from.
    *
    * @return A reference to this JSON Array.
    */
   Array& operator=(Array&& in_other) noexcept;

   /**
    * @brief Accessor operator. Gets the JSON value at the specified position in the array.
    *
    * @param in_name    The name of the member to access.
    *
    * @throws std::out_of_range     If in_index is greater than or equal to the value returned by getSize().
    *
    * @return The value of the member with the specified name, if it exists; empty JSON value otherwise.
    */
   Value operator[](size_t in_index) const;

   /**
    * @brief Gets an iterator pointing to the first member of this array.
    *
    * @return An iterator pointing to the first member of this array.
    */
   Iterator begin() const;

   /**
    * @brief Gets an iterator after the last member of this array.
    *
    * @return An iterator after the last member of this array.
    */
   Iterator end() const;

   /**
    * @brief Gets an iterator pointing to the last member of this array, which iterates in the reverse direction.
    *
    * @return A reverse iterator pointing to the last member of this array.
    */
   ReverseIterator rbegin() const;

   /**
    * @brief Gets an iterator before the first member of this array, which can be compared with an other
    *        ReverseIterator to determine when reverse iteration has ended.
    *
    * @return An iterator before the first member of this array.
    */
   ReverseIterator rend() const;

   /**
    * @brief Clears the JSON array.
    */
   void clear();

   /**
    * @brief Erases the member specified by the provided iterator.
    *
    * @param in_itr     The iterator pointing to the member to erase.
    *
    * @return An iterator pointing to the member immediately after the erased member.
    */
   Iterator erase(const Iterator& in_itr);

   /**
    * @brief Erases the member specified by the provided iterator.
    *
    * @param in_itr     The iterator pointing to the member to erase.
    *
    * @return An iterator pointing to the member immediately after the erased member.
    */
   Iterator erase(const Iterator& in_first, const Iterator& in_last);

   /**
    * @brief Gets the value at the back of the JSON array.
    *
    * @return The value at the back of the JSON array or an empty value, if the array is emtpy.
    */
   Value getBack() const;

   /**
    * @brief Gets the value at the front of the JSON array.
    *
    * @return The value at the front of the JSON array or an empty value, if the array is emtpy.
    */
   Value getFront() const;

   /**
    * @brief Gets the value at the specified index of the JSON array.
    *
    * @param in_index   The index of the value to retrieve.
    *
    * @return The value at the specified index or an empty value if the index is out of bounds.
    */
   Value getValueAt(size_t in_index) const;

   /**
    * @brief Gets the number of values in the JSON array.
    *
    * @return The number of values in the JSON array.
    */
   size_t getSize() const;

   /**
    * @brief Checks whether the JSON array is empty.
    *
    * @return True if the JSON array has no members; false otherwise.
    */
   bool isEmpty() const;

   /**
    * @brief Pushes the value onto the end of the JSON array.
    *
    * @param in_value   The value to push onto the end of the JSON array.
    */
   void pushBack(Value in_value);

private:
   /**
    * @brief Constructs a JSON array from a JSON value. The specified value should return Type::ARRAY from getType().
    *
    * @param in_value   A JSON value which returns Type::ARRAY from getType();
    */
   explicit Array(const Value& in_value);

   friend class Value;
};

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio

#endif
