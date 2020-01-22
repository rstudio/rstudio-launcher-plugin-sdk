/*
 * JsonUtils.hpp
 *
 * Copyright (C) 2020 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_JSON_UTILS
#define LAUNCHER_PLUGINS_JSON_UTILS

#include "Json.hpp"

#include <Error.hpp>
#include <sstream>

namespace rstudio {
namespace launcher_plugins {
namespace json {

/**
 * @file
 * Utility functions for working with JSON classes.
 */

/**
 * @enum JsonReadError
 * @brief Errors which may occur while reading values from JSON objects.
 */
enum class JsonReadError
{
   SUCCESS = 0,
   MISSING_MEMBER = 1,
   INVALID_TYPE = 2,
};

/**
 * @brief Creates a JSON read error.
 *
 * @param in_errorCode          The code of the error to create.
 * @param in_message            The message of the error.
 * @param in_errorLocation      The location at which the error occurred.
 *
 * @return The newly created JSON read error.
 */
Error jsonReadError(JsonReadError in_errorCode, const std::string& in_message, const ErrorLocation& in_errorLocation);

/**
 * @brief Checks whether the supplied error is a "missing memeber" error.
 *
 * @param in_error      The error to check.
 *
 * @return True if the error is a missing member error; False otherwise.
 */
bool isMissingMemberError(const Error& in_error);


/**
 * @brief Reads a member from an object.
 *
 * @tparam T            The type of the member.
 *
 * @param in_object     The object from which the member should be read.
 * @param in_name       The name of the member to read.
 * @param out_value     The value of the member, if no error occurs.
 *
 * @return Success if the member could be found and is of type T; Error otherwise.
 */
template <typename T>
Error readObject(const Object& in_object, const std::string& in_name, T& out_value)
{
   Object::Iterator itr = in_object.find(in_name);
   if (itr == in_object.end())
      return jsonReadError(
         JsonReadError::MISSING_MEMBER,
         "Member " + in_name + " does not exist in the specified JSON object.",
         ERROR_LOCATION);

   if (!isType<T>((*itr).getValue()))
   {
      std::ostringstream msgStream;
      msgStream << "Member " << in_name << " has type " << (*itr).getValue().getType() <<
         " which is not compatible with requested type " << typeid(T).name() << ".";
      return jsonReadError(
         JsonReadError::INVALID_TYPE,
         msgStream.str(),
         ERROR_LOCATION);
   }

   out_value = (*itr).getValue().getValue<T>();
   return Success();
}

/**
 * @brief Reads an array member from an object.
 *
 * @tparam T            The type of values of the array member.
 *
 * @param in_object     The object from which the member should be read.
 * @param in_name       The name of the member to read.
 * @param out_values    The values of the array member, if no error occurs.
 *
 * @return Success if the member could be found and its values are of type T; Error otherwise.
 */
template <typename T>
Error readObject(const Object& in_object, const std::string& in_name, std::vector<T>& out_values)
{
   Object::Iterator itr = in_object.find(in_name);
   if (itr == in_object.end())
      return jsonReadError(
         JsonReadError::MISSING_MEMBER,
         "Member " + in_name + " does not exist in the specified JSON object.",
         ERROR_LOCATION);

   if (!(*itr).getValue().isArray())
      return jsonReadError(JsonReadError::INVALID_TYPE,
         "Member " + in_name + " is not an array.",
         ERROR_LOCATION);


   Array array = (*itr).getValue().getArray();
   for (size_t i = 0, n = array.getSize(); i < n; ++i)
   {
      const Value& value = array[i];
      if (!isType<T>(value))
      {
         std::ostringstream msgStream;
         msgStream << "Element " << i << " of member " + in_name << " is of type " <<  value.getType() <<
            " which is not compatible with the requested type " << typeid(T).name() << ".";
         return jsonReadError(
            JsonReadError::INVALID_TYPE,
            msgStream.str(),
            ERROR_LOCATION);
      }

      out_values.push_back(value.getValue<T>());
   }

   return Success();
}

/**
 * @brief Reads multiple members from an object.
 *
 * @tparam T            The type of the first member to read.
 * @tparam Args         The template parameter pack for the remaining members.
 *
 * @param in_object     The object from which to read the members.
 * @param in_name       The name of the first member to be read.
 * @param out_value     The value of the first member to be read, if no error occurs.
 * @param io_args       The parameter pack of the remaining members to be read.
 *
 * @return Success if all the members exist and have valid types; Error otherwise.
 */
template <typename T, typename... Args>
Error readObject(const Object& in_object, const std::string& in_name, T& out_value, Args... io_args)
{
   Error error = readObject(in_object, in_name, out_value);
   if (error)
      return error;

   return readObject(in_object, io_args...);
}

/**
 * @brief Reads multiple members from an object.
 *
 * @tparam T            The type of the values of the first array member to read.
 * @tparam Args         The template parameter pack for the remaining members.
 *
 * @param in_object     The object from which to read the members.
 * @param in_name       The name of the first member to be read.
 * @param out_values    The values of the first array member to be read, if no error occurs.
 * @param io_args       The parameter pack of the remaining members to be read.
 *
 * @return Success if all the members exist and have valid types; Error otherwise.
 */
template <typename T, typename... Args>
Error readObject(const Object& in_object, const std::string& in_name, std::vector<T>& out_values, Args... io_args)
{
   Error error = readObject(in_object, in_name, out_values);
   if (error)
      return error;

   return readObject(in_object, io_args...);
}

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio

#endif
