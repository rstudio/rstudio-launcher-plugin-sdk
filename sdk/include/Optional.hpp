/*
 * Optional.hpp
 *
 * Copyright (C) 2020 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant to the terms of a commercial license agreement
 * with RStudio, then this program is licensed to you under the following terms:
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

#ifndef LAUNCHER_PLUGINS_OPTIONAL_HPP
#define LAUNCHER_PLUGINS_OPTIONAL_HPP

#include <memory>

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Container class which represents a value that may or may not be set.
 *
 * @tparam T    The type of the optional value.
 */
template <typename T>
class Optional
{
public:
   /**
    * @brief Default constructor.
    */
   Optional() = default;

   /**
    * @brief Constructor.
    *
    * @param in_value       The value to set on this optional. The optional takes ownership of this value.
    */
   explicit Optional(std::unique_ptr<T> in_value) :
      m_value(std::move(in_value))
   {
   }

   /**
    * @brief Constructor.
    *
    * @param in_value       The value to set on this optional.
    */
   explicit Optional(const T& in_value) :
      m_value(new T(in_value))
   {
   }

   /**
    * @brief Copy constructor.
    *
    * @param in_other       The optional value to copy.
    */
   Optional(const Optional& in_other) :
      m_value(new T(*in_other.m_value))
   {
   }

   /**
    * @brief Move constructor.
    *
    * @param in_other       The optional value to move into this optional value.
    */
   Optional(Optional&& in_other) noexcept
   {
      m_value.swap(in_other.m_value);
   }

   /**
    * @brief Boolean operator. Intentionally not explicit to allow for boolean checks on optional values.
    *
    * @return True if this optional has a value; false otherwise.
    */
   operator bool()
   {
      return hasValue();
   }

   /**
    * @brief Not boolean operator.
    *
    * @return True if this optional does not have a value; false otherwise.
    */
   bool operator!()
   {
      return !hasValue();
   }

   /**
    * @brief Assignment operator.
    *
    * @param in_other   The optional value to copy to this optional.
    *
    * @return A reference to this optional.
    */
   Optional& operator=(const Optional& in_other)
   {
      if (this == &in_other)
         return *this;

      m_value.reset(new T(*in_other.m_value));
      return *this;
   }

   /**
    * @brief Assignment operator.
    *
    * @param in_value   The value to assign to this optional. The optional will take ownership of the value.
    *
    * @return A reference to this optional.
    */
   Optional& operator=(std::unique_ptr<T> in_value)
   {
      m_value.swap(in_value);
      return *this;
   }

   /**
    * @brief Assginment operator.
    *
    * @param in_value   The value to assign to this optional.
    *
    * @return A reference to this optional.
    */
   Optional& operator=(const T& in_value)
   {
      m_value.reset(new T(in_value));
      return *this;
   }

   /**
    * @brief Gets the value of this optional, or the provided default value if this optional has no value.
    *
    * @param in_default     The default value to use if this optional has no value.
    *
    * @return The value of this optional, or the provided default value if this optional has no value.
    */
   const T& getValueOr(const T& in_default) const
   {
      if (hasValue())
         return *m_value;

      return in_default;
   }

   /**
    * @brief Gets the value of this optional, or the provided default value if this optional has no value.
    *
    * @param in_default     The default value to use if this optional has no value.
    *
    * @return The value of this optional, or the provided default value if this optional has no value.
    */
   T& getValueOr(T& in_default)
   {
      return const_cast<T&>(const_cast<const Optional*>(this)->getValueOr(in_default));
   }

   /**
    * @brief Checks whether this optional has a value.
    *
    * @return True if this optional has a value; false otherwise.
    */
   bool hasValue() const
   {
      return m_value != nullptr;
   }

private:
   /** The value of this optional. */
   std::unique_ptr<T> m_value;
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
