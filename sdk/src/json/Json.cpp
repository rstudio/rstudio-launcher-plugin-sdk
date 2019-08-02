/*
 * Json.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "json/Json.hpp"

#include "rapidjson/document.h"

namespace rstudio {
namespace launcher_plugins {
namespace json {

typedef rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> JsonDocument;
typedef rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator> JsonValue;

namespace {

static rapidjson::CrtAllocator s_allocator;

} // anonymous namespace

// Value ===============================================================================================================
struct Value::Impl
{
   Impl() : Document(&s_allocator) { };

   Impl(const Impl& in_other) :
      Document(&s_allocator)
   {
      Document.CopyFrom(in_other.Document, s_allocator);
   }

   explicit Impl(const JsonDocument& in_jsonDocument) :
      Impl()
   {
      Document.CopyFrom(in_jsonDocument, s_allocator);
   }

   explicit Impl(const JsonValue& in_jsonValue) :
      Impl()
   {
      Document.CopyFrom(in_jsonValue, s_allocator);
   }

   JsonDocument Document;
};

PRIVATE_IMPL_DELETER_IMPL(Value)

Value::Value() :
   m_impl(new Impl())
{
}

Value::Value(ValueImplPtr in_valueImpl) :
   m_impl(std::move(in_valueImpl))
{
}

Value::Value(const Value& in_other) :
   m_impl(new Impl(*in_other.m_impl))
{
}

Value::Value(Value&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
   // Make sure we're never in a situation where m_impl is null. Most of the time this shouldn't be a problem since you
   // you shouldn't be moving something you intend to continue using later, but it's better to be on the safe side.
   in_other.m_impl.reset(new Impl());
}

Value::Value(bool in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(double in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(float in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(int in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(int64_t in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(const char* in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(const std::string& in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(unsigned int in_value) :
   Value()
{
   *this = in_value;
}

Value::Value(uint64_t in_value) :
   Value()
{
   *this = in_value;
}

Value& Value::operator=(const Value& in_other)
{
   // Don't bother copying if these objects are the same object.
   if (this != &in_other)
      m_impl.reset(new Impl(*in_other.m_impl));
   return *this;
}

Value& Value::operator=(Value&& in_other) noexcept
{
   // Don't try to move this object into itself.
   if (this != &in_other)
   {
      m_impl = std::move(in_other.m_impl);
      // Make sure we're never in a situation where m_impl is null.
      in_other.m_impl.reset(new Impl());
   }

   return *this;
}

Value& Value::operator=(bool in_value)
{
   m_impl->Document.SetBool(in_value);
   return *this;
}

Value& Value::operator=(double in_value)
{
   m_impl->Document.SetDouble(in_value);
   return *this;
}

Value& Value::operator=(float in_value)
{
   m_impl->Document.SetFloat(in_value);
   return *this;
}

Value& Value::operator=(int in_value)
{
   m_impl->Document.SetInt(in_value);
}

Value& Value::operator=(int64_t in_value)
{
   m_impl->Document.SetInt64(in_value);
   return *this;
}

Value& Value::operator=(const char* in_value)
{
   m_impl->Document.SetString(in_value, s_allocator);
   return *this;
}

Value& Value::operator=(const std::string& in_value)
{
   m_impl->Document.SetString(in_value.c_str(), s_allocator);
   return *this;
}

Value& Value::operator=(unsigned int in_value)
{
   m_impl->Document.SetUint(in_value);
   return *this;
}

Value& Value::operator=(uint64_t in_value)
{
   m_impl->Document.SetUint64(in_value);
   return *this;
}

bool Value::operator==(const Value& in_other) const
{
   // Don't bother with deep comparison if shallow comparison will do.
   if (this == &in_other)
      return true;

   return m_impl->Document == in_other.m_impl->Document;
}

Array Value::getArray() const
{
   assert(getType() == Type::ARRAY);
   return Array(*this);
}

bool Value::getBool() const
{
   assert(getType() == Type::BOOL);
   return m_impl->Document.GetBool();
}

double Value::getDouble() const
{
   assert(getType() == Type::DOUBLE);
   return m_impl->Document.GetDouble();
}

float Value::getFloat() const
{
   assert(getType() == Type::FLOAT);
   return m_impl->Document.GetFloat();
}

int Value::getInt() const
{
   assert(getType() == Type::INT);
   return m_impl->Document.GetInt();
}

int64_t Value::getInt64() const
{
   assert(getType() == Type::INT64);
   return m_impl->Document.GetInt64();
}

Object Value::getObject() const
{
   assert(getType() == Type::OBJECT);
   return Object(*this);
}

std::string Value::getString() const
{
   assert(getType() == Type::STRING);
   return std::string(m_impl->Document.GetString(), m_impl->Document.GetStringLength());
}

Type Value::getType() const
{
   switch(m_impl->Document.GetType())
   {
      case rapidjson::kArrayType:
         return Type::ARRAY;
      case rapidjson::kTrueType:
      case rapidjson::kFalseType:
         return Type::BOOL;
      case rapidjson::kNumberType:
      {
         if (m_impl->Document.IsDouble())
            return Type::DOUBLE;
         if (m_impl->Document.IsFloat())
            return Type::FLOAT;
         if (m_impl->Document.IsInt())
            return Type::INT;
         if (m_impl->Document.IsInt64())
            return Type::INT64;
         if (m_impl->Document.IsUint())
            return Type::UINT;
         if (m_impl->Document.IsUint64())
            return Type::UINT64;

         return Type::UNKNOWN;
      }
      case rapidjson::kObjectType:
         return Type::OBJECT;
      case rapidjson::kStringType:
         return Type::STRING;
      case rapidjson::kNullType:
         return Type::NULL_TYPE;
      default:
         return Type::UNKNOWN;
   }
}

unsigned int Value::getUInt() const
{
   assert(getType() == Type::UINT);
   return m_impl->Document.GetUint();
}

uint64_t Value::getUInt64() const
{
   assert(getType() == Type::UINT64);
   return m_impl->Document.GetUint64();
}

bool Value::isNull() const
{
   return m_impl->Document.IsNull();
}

Error Value::parse(const char* in_jsonStr)
{
   rapidjson::ParseResult result = m_impl->Document.Parse(in_jsonStr);

   if (result.IsError())
   {
      std::string message = "An error occurred while parsing json. Offset: " + std::to_string(result.Offset());
      return Error(result.Code(), message, ERROR_LOCATION);
   }

   return Success();
}

Error Value::parse(const std::string& in_jsonStr)
{
   return parse(in_jsonStr.c_str());
}

// Object Member =======================================================================================================
Object::Member::Member(std::string in_name, Value in_value) :
   m_name(std::move(in_name)),
   m_value(std::move(in_value))
{
}

const std::string& Object::Member::getName() const
{
   return m_name;
}

const Value& Object::Member::getValue() const
{
   return m_value;
}

// Object Iterator =====================================================================================================
Object::Iterator::Iterator(const Object* in_parent, std::ptrdiff_t in_startPos) :
   m_parent(in_parent),
   m_pos(in_startPos)
{
}

Object::Iterator& Object::Iterator::operator=(const Object::Iterator& in_other)
{
   m_parent = in_other.m_parent;
   m_pos = in_other.m_pos;
}

Object::Iterator& Object::Iterator::operator++()
{
   if (static_cast<rapidjson::SizeType>(m_pos) < m_parent->m_impl->Document.MemberCount())
      ++m_pos;
   return *this;
}

Object::Iterator Object::Iterator::operator++(int) // NOLINT
{
   Iterator copied(*this);
   ++(*this);
   return copied;
}

Object::Iterator& Object::Iterator::operator--()
{
   if (m_pos > 0)
      --m_pos;
   return *this;
}

Object::Iterator Object::Iterator::operator--(int) // NOLINT
{
   Iterator copied(*this);
   --(*this);
   return copied;
}

bool Object::Iterator::operator==(const Object::Iterator& in_other) const
{
   return (m_parent == in_other.m_parent) && (m_pos == in_other.m_pos);
}

Object::Iterator::reference Object::Iterator::operator*() const
{
   if (m_pos > m_parent->m_impl->Document.MemberCount())
      return Object::Member();

   auto itr = m_parent->m_impl->Document.MemberBegin() + m_pos;
   return Member(
      std::string(itr->name.GetString(), itr->name.GetStringLength()),
      Value(ValueImplPtr(new Value::Impl(itr->value))));
}

// Object ==============================================================================================================
Object::Object() :
   Value()
{
   m_impl->Document.SetObject();
}

Object& Object::operator=(const Object& in_other)
{
   m_impl.reset(new Impl(*in_other.m_impl));
   return *this;
}

Object& Object::operator=(Object&& in_other) noexcept
{
   m_impl = std::move(in_other.m_impl);
   in_other.m_impl.reset(new Impl());
   return *this;
}

Value Object::operator[](const char* in_name)
{
   JsonDocument& doc = m_impl->Document;
   if (!doc.HasMember(in_name))
   {
      doc.AddMember(JsonValue(in_name, s_allocator), JsonDocument(), s_allocator);
   }

   return Value(ValueImplPtr(new Impl(doc[in_name])));
}

Value Object::operator[](const std::string& in_name)
{
   return (*this)[in_name.c_str()];
}

Object::Iterator Object::find(const char* in_name) const
{
   auto itr = m_impl->Document.FindMember(in_name);
   if (itr == m_impl->Document.MemberEnd())
      return end();

   return Object::Iterator(this, itr - m_impl->Document.MemberBegin());
}

Object::Iterator Object::find(const std::string& in_name) const
{
   return find(in_name.c_str());
}

Object::Iterator Object::begin() const
{
   return Object::Iterator(this);
}

Object::Iterator Object::end() const
{
   return Object::Iterator(this, getSize());
}

Object::ReverseIterator Object::rbegin() const
{
   return Object::ReverseIterator(end());
}

Object::ReverseIterator Object::rend() const
{
   return Object::ReverseIterator(begin());
}

void Object::clear()
{
   m_impl->Document.SetObject();
}

bool Object::erase(const char* in_name)
{
   return m_impl->Document.EraseMember(in_name);
}

bool Object::erase(const std::string& in_name)
{
   return erase(in_name.c_str());
}

Object::Iterator Object::erase(const Object::Iterator& in_itr)
{
   auto internalItr = m_impl->Document.MemberBegin() + in_itr.m_pos;
   std::ptrdiff_t newPos = m_impl->Document.EraseMember(internalItr) - m_impl->Document.MemberBegin();
   return Object::Iterator(this, newPos);
}

size_t Object::getSize() const
{
   return m_impl->Document.MemberCount();
}

bool Object::hasMember(const char* in_name)
{
   return m_impl->Document.HasMember(in_name);
}

bool Object::hasMember(const std::string& in_name)
{
   return hasMember(in_name.c_str());
}

void Object::insert(const Member& in_member)
{
   (*this)[in_member.getName()] = in_member.getValue();
}

bool Object::isEmpty() const
{
   return m_impl->Document.ObjectEmpty();
}

Object::Object(const Value& in_value) :
   Value(in_value)
{
   assert(m_impl->Document.IsObject());
}

// Array Iterator ======================================================================================================
Array::Iterator::Iterator(const Array* in_parent, std::ptrdiff_t in_startPos) :
   m_parent(in_parent),
   m_pos(in_startPos)
{
}

Array::Iterator& Array::Iterator::operator=(const Array::Iterator& in_other)
{
   m_parent = in_other.m_parent;
   m_pos = in_other.m_pos;
}

Array::Iterator& Array::Iterator::operator++()
{
   if (m_pos < m_parent->m_impl->Document.Size())
      ++m_pos;

   return *this;
}

Array::Iterator& Array::Iterator::operator--()
{
   if (m_pos > 0)
      --m_pos;

   return *this;
}

Array::Iterator Array::Iterator::operator++(int) // NOLINT
{
   Array::Iterator copied(*this);
   ++(*this);
   return copied;
}

Array::Iterator Array::Iterator::operator--(int) // NOLINT
{
   Array::Iterator copied(*this);
   --(*this);
   return copied;
}

bool Array::Iterator::operator==(const Array::Iterator& in_other) const
{
   return (m_parent == in_other.m_parent) && (m_pos == in_other.m_pos);
}

Array::Iterator::reference Array::Iterator::operator*() const
{
   if (m_pos >= m_parent->m_impl->Document.Size())
      return Value();

   auto internalItr = m_parent->m_impl->Document.Begin() + m_pos;
   return Value(ValueImplPtr(new Impl(*internalItr)));
}

// Array ===============================================================================================================
Array::Array() :
   Value()
{
   m_impl->Document.SetArray();
}

Array& Array::operator=(const Array& in_other)
{
   m_impl.reset(new Impl(*in_other.m_impl));
   return *this;
}

Array& Array::operator=(Array&& in_other) noexcept
{
   m_impl = std::move(in_other.m_impl);
   in_other.m_impl.reset(new Impl());
   return *this;
}

Value Array::operator[](size_t in_index) const
{
   JsonValue& internalValue = m_impl->Document[in_index];
   return Value(ValueImplPtr(new Impl(internalValue)));
}

Array::Iterator Array::begin() const
{
   return Array::Iterator(this);
}

Array::Iterator Array::end() const
{
   return Array::Iterator(this, m_impl->Document.Size());
}

Array::ReverseIterator Array::rbegin() const
{
   return Array::ReverseIterator(end());
}

Array::ReverseIterator Array::rend() const
{
   return Array::ReverseIterator(begin());
}

void Array::clear()
{
   m_impl->Document.Clear();
}

Array::Iterator Array::erase(const Array::Iterator& in_itr)
{
   if (getSize() == 0)
      return Array::Iterator(this);

   auto internalItr = m_impl->Document.Begin() + in_itr.m_pos;
   std::ptrdiff_t newPos = m_impl->Document.Erase(internalItr) - m_impl->Document.Begin();
   return Array::Iterator(this, newPos);
}

Array::Iterator Array::erase(const Array::Iterator& in_first, const Array::Iterator& in_last)
{
   if (getSize() == 0)
      return Array::Iterator(this);

   auto internalFirst = m_impl->Document.Begin() + in_first.m_pos;
   auto internalLast = m_impl->Document.End() + in_last.m_pos;

   std::ptrdiff_t newPos = m_impl->Document.Erase(internalFirst, internalLast) - m_impl->Document.Begin();
   return Array::Iterator(this, newPos);
}

Value Array::getBack() const
{
   return (*this)[getSize() - 1];
}

Value Array::getFront() const
{
   return (*this)[0];
}

Value Array::getValueAt(size_t in_index) const
{
   return (*this)[in_index];
}

size_t Array::getSize() const
{
   return m_impl->Document.Size();
}

bool Array::isEmpty() const
{
   return m_impl->Document.Empty();
}

void Array::pushBack(Value in_value)
{
   m_impl->Document.PushBack(in_value.m_impl->Document, s_allocator);
}

Array::Array(const Value& in_value) :
   Value(in_value)
{
}

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio
