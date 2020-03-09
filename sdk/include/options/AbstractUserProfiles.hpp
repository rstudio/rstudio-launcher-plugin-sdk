/*
 * AbstractUserProfiles.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_USER_PROFILES_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_USER_PROFILES_HPP

#include <Noncopyable.hpp>

#include <functional>

#include <PImpl.hpp>
#include <map>
#include <set>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace system {

class User;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace options {

/**
 * @brief Base class which reads an ini-based user profiles file.
 */
class AbstractUserProfiles : Noncopyable
{
public:
   /**
    * @brief Default virtual destructor for inheritance.
    */
   virtual ~AbstractUserProfiles() = default;

   /**
    * @brief Initializes the user profiles. Must be called before attempting to retrieve configuration values.
    *
    * @return Success if the user profiles file could be opened for read and parsed; Error otherwise.
    */
   Error initialize();

protected:
   /**
    * @brief Default constructor.
    *
    * This constructor should only be used if the inheriting class overrides `getConfigurationFileName`.
    *
    * If this constructor is used, the user profiles file will be `/etc/rstudio/<getConfigurationFileName()>.conf`.
    */
   AbstractUserProfiles();

   /**
    * @brief Constructor.
    *
    * If this constructor is used, the user profiles file will be `/etc/rstudio/launcher.<in_pluginName>.profiles.conf`.
    *
    * @param in_pluginName      The lower-case name of the plugin, to be used to set the configuration file name.
    */
   explicit AbstractUserProfiles(const std::string& in_pluginName);
   /**
    * @brief Gets the value with the specified name for the given user, based on the profiles configuration file.
    *
    * This template method is precompiled for all supported types. Valid types:
    *   std::string
    *   int32_t
    *   uint32_t
    *   int64_t
    *   uint64_t
    *   float
    *   double
    *   bool
    *   std::set<U>, where U is one of the types above
    *   std::vector<U>, where U is one of the types above (except std::set)
    *   std::map<U, V> where U and V are any two of the types above
    *
    * If additional types are required, this method should be invoked using T=std::string and the caller may parse the
    * string value as desired.
    *
    * @tparam T     The type of the value, as defined above.
    *
    * @param in_valueName   The name of the value to retrieve for the specified user.
    * @param in_user        The user for which to retrieve the value.
    * @param out_value      The requested value, if no error occurred.
    *
    * @return Success if the requested value could be found for the given user and could be parsed to type T; Error
    *         otherwise.
    */
   template <typename T>
   Error getValueForUser(const std::string& in_valueName, const system::User& in_user, T& out_value) const;

   /**
    * @brief Checks whether the error indicates that the configuration value was not found.
    * @param in_error
    * @return
    */
   static bool isValueNotFoundError(const Error& in_error);

private:
   /**
    * @brief Gets the name, without extension, of the user profiles configuration file. The default implementation will
    *        return "launcher.<in_pluginName>.profiles".
    *
    * NOTE: The inheriting class should only rely on the default implementation of this method if it uses the
    * AbstractUserProfiles(const std::string& in_pluginName) constructor.
    *
    * @return The name, without extension, of the user profiles configuration file/
    */
   virtual std::string getConfigurationFileName() const;

   /**
    * @brief Gets the set of fields which may be set under any section of the user profiles configuration file.
    *
    * These values are used to validate the configuration file when it is parsed.
    *
    * @return The set of fields which may be set under any section of the user profiles configuration file.
    */
   virtual std::set<std::string> getValidFieldNames() const = 0;

   PRIVATE_IMPL(m_impl);
};

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

#endif
