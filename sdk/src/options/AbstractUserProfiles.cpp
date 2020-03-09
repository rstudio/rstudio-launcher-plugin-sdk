/*
 * AbstractUserProfiles.cpp
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

#include <options/AbstractUserProfiles.hpp>

#include <cassert>
#include <grp.h>
#include <map>
#include <string>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <Error.hpp>
#include <system/User.hpp>
#include <system/FilePath.hpp>
#include "../SafeConvert.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace options {

namespace
{

enum class UserProfileError
{
   SUCCESS = 0,
   CONF_PARSE_ERROR = 1,
   VALUE_NOT_FOUND_ERROR = 2,
   VALUE_PARSE_ERROR = 3,
   INVALID_VALUE_ERROR = 4,
};

Error userProfileError(
   UserProfileError in_code,
   const std::string& in_message,
   const Error& in_cause,
   const ErrorLocation& in_location)
{
   std::string message;
   switch(in_code)
   {
      case UserProfileError::SUCCESS:
         return Success();
      case UserProfileError::CONF_PARSE_ERROR:
      {
         message = "Failed to parse the profiles configuration file: ";
         break;
      }
      case UserProfileError::VALUE_NOT_FOUND_ERROR:
      {
         message = "The specified value could not be found for the requested user: ";
         break;
      }
      case UserProfileError::VALUE_PARSE_ERROR:
      {
         message = "The specified value could not be parsed as the requested type: ";
         break;
      }
      case UserProfileError::INVALID_VALUE_ERROR:
      {
         message = "Invalid value requested: ";
         break;
      }
      default:
         message = "Unknown error: ";
   }

   message.append(in_message);

   if (in_cause)
      return Error("UserProfilesError", static_cast<int>(in_code), in_message, in_cause, ERROR_LOCATION);

   return Error("UserProfilesError", static_cast<int>(in_code), in_message, ERROR_LOCATION);
}

Error userProfileError(
   UserProfileError in_code,
   const std::string& in_message,
   const ErrorLocation& in_location)
{
   return userProfileError(in_code, in_message, Success(), in_location);
}

template <typename T>
Error parseValue(const std::string& in_value, T& out_parsedValue)
{
   Optional<T> value = safe_convert::stringTo<T>(boost::trim_copy(in_value));
   if (!value)
      return userProfileError(
         UserProfileError::VALUE_PARSE_ERROR,
         "Could not convert \"" + in_value + "\" to type " + typeid(T).name(),
         ERROR_LOCATION);

   // Guaranteed to exist here.
   out_parsedValue = value.getValueOr(T());
   return Success();
}

template <>
Error parseValue(const std::string& in_value, std::string& out_parsedValue)
{
   out_parsedValue = boost::trim_copy(in_value);
   return Success();
}

template <typename U>
Error parseValue(const std::string& in_value, std::set<U>& out_parsedValues)
{
   const boost::regex splitExpr("\\s*,\\s*");
   boost::sregex_token_iterator itr(in_value.begin(), in_value.end(), splitExpr, -1);
   boost::sregex_token_iterator end;

   while (itr != end)
   {
      U value;
      Error error = parseValue(*itr, value);
      if (error)
      {
         error.addOrUpdateProperty("full-value", in_value);
         return error;
      }

      out_parsedValues.insert(value);
   }

   return Success();
}

template <typename U>
Error parseValue(const std::string& in_value, std::vector<U>& out_parsedValues)
{
   const boost::regex splitExpr("\\s*,\\s*");
   boost::sregex_token_iterator itr(in_value.begin(), in_value.end(), splitExpr, -1);
   boost::sregex_token_iterator end;

   while (itr != end)
   {
      U value;
      Error error = parseValue(*itr, value);
      if (error)
      {
         error.addOrUpdateProperty("full-value", in_value);
         return error;
      }

      out_parsedValues.push_back(value);
   }

   return Success();
}

template <typename U, typename V>
Error parseValue(const std::string& in_value, std::map<U, V>& out_parsedValues)
{
   const boost::regex splitExpr("\\s*;\\s*");
   boost::sregex_token_iterator itr(in_value.begin(), in_value.end(), splitExpr, -1);
   boost::sregex_token_iterator end;

   while (itr != end)
   {
      std::vector<std::string> keyValStrs;
      boost::split(keyValStrs, *itr, boost::is_any_of("="));

      if (keyValStrs.size() < 2)
      {
         Error error = userProfileError(
            UserProfileError::VALUE_PARSE_ERROR,
            "Map value \"" + *itr + R"(" does not contain "=" delimiter. Expected exactly 1)",
            ERROR_LOCATION);
         error.addOrUpdateProperty("full-value", in_value);
         return error;
      }
      else if (keyValStrs.size() > 2)
      {
         Error error = userProfileError(
            UserProfileError::VALUE_PARSE_ERROR,
            "Map value \"" + *itr + "\" contains " +
               std::to_string(keyValStrs.size() - 1) + " \"=\" delimiters. Expected exactly 1.",
            ERROR_LOCATION);
         error.addOrUpdateProperty("full-value", in_value);
         return error;
      }

      // Exactly 2 values in the vector if we get here.
      U key;
      Error error = parseValue(keyValStrs[0], key);
      if (error)
      {
         error.addOrUpdateProperty("full-value", in_value);
         return error;
      }

      V value;
      error = parseValue(keyValStrs[1], key);
      if (error)
      {
         error.addOrUpdateProperty("full-value", in_value);
         error.addOrUpdateProperty("key-value", keyValStrs[0]);
         return error;
      }

      out_parsedValues[key] = value;
   }

   return Success();
}


} // anonymous namespace

/**
 * @brief Enum which represents the level of specificity of a section.
 */
enum class LevelType
{
   // NONE is used for comparisons. No section should have LevelType::NONE.
   NONE     = -1,
   // The least specific level - all users.
   ALL      = 0,
   // The middle level - any user in a specific group.
   GROUP    = 1,
   // The most specific level - a particular user.
   USER     = 2,
};

/**
 * @brief Represents a section (or level) of the ini file.
 */
struct Level
{
   /**
    * @brief Default constructor.
    */
   Level() : Type(LevelType::NONE) {}

   /** The type of the section. */
   LevelType Type;

   /** The name of the section. */
   std::string Name;
};

// Group Typedefs
typedef std::set<std::string> GroupMembers;
typedef std::map<std::string, GroupMembers> GroupLookupMap;

// Level Typedefs
typedef std::map<std::string, std::string> ValueMap;
typedef std::pair<Level, ValueMap> LevelValue;

// Impl Struct =========================================================================================================
struct AbstractUserProfiles::Impl
{
   /**
    * @brief Gets the most specific instance of a value for the given user.
    *
    * @param in_valueName       The name of the value to retrieve.
    * @param in_userName        The name of the user for whom to retrieve the value.
    * @param out_value          The value, if any was found.
    *
    * @return True if a value was found with the given name for the user; Error otherwise.
    */
   bool getValueForUser(const std::string& in_valueName, const std::string& in_userName, std::string& out_value) const;

   /**
    * @brief Checks whether the specified user is in the specified group.
    *
    * @param in_userName        The user.
    * @param in_groupName       The group.
    *
    * @return True if the user is in the group; false otherwise.
    */
   bool isInGroup(const std::string& in_userName, const std::string& in_groupName) const;

   /**
    * @brief Iterates all sections of the ini and applies the in_onValueFound function to any values that were found.
    *
    * @param in_valueName       The value to be validated.
    * @param in_onValueFound    The function to be invoked for each occurrence of in_valueName.
    *
    * @return Success if every invocation of in_onValueFound returns Success; the Error that occurred otherwise.
    */
   Error iterateValues(
      const std::string& in_valueName,
      const std::function<Error(const std::string&)>& in_onValueFound) const;

   /**
    * @brief Parses the sections from ini file data. Also gathers group information for any groups specified in the
    *        file.
    *
    * @param in_levelData       The string data from the ini file.
    * @param in_fieldNames      The names of valid fields.
    *
    * @return True if the ini file could be parsed and there were no unexpected fields or groups; Error otherwise.
    */
   Error parseLevels(const std::string& in_levelData, const std::set<std::string>& in_fieldNames);

   /**
    * @brief Populates the Groups member with unix group information from the list of requested group names.
    *
    * @param in_groupNames      The list of group names for which to retrieve group information.
    *
    * @return Success if all the group information could be populated; Error otherwise.
    */
   Error populateGroups(const std::set<std::string>& in_groupNames);

   /** Cached unix group information so we don't need to get it each time. */
   GroupLookupMap Groups;

   /** The parsed sections of the ini file. */
   std::vector<LevelValue> LevelValues;

   /** The configuration file. */
   system::FilePath ConfigurationFile;
};

bool AbstractUserProfiles::Impl::getValueForUser(
   const std::string& in_valueName,
   const std::string& in_userName,
   std::string& out_value) const
{
   LevelType specificity = LevelType::NONE;
   const std::string* value = nullptr;

   // Within the same category (e.g. if a user belongs to multiple groups) the last matching entry will be applied.
   for (const LevelValue& levelValue : LevelValues)
   {
      // If the level is less specific than the most recently found value, skip this entry.
      if (levelValue.first.Type < specificity)
         continue;

      // If the level is a group level and the user isn't in the group, skip this entry.
      if ((levelValue.first.Type == LevelType::GROUP) && !isInGroup(in_userName, levelValue.first.Name))
         continue;

      // If the level is a user level and it's not for this user, skip this entry.
      if ((levelValue.first.Type == LevelType::USER) && (levelValue.first.Name != in_userName))
         continue;

      // Otherwise this level applies to the user. Try to find the value
      const auto itr = levelValue.second.find(in_valueName);
      if (itr != levelValue.second.end())
         value = &(itr->second);
   }

   if (value == nullptr)
      return false;

   out_value = *value;
   return true;
}

bool AbstractUserProfiles::Impl::isInGroup(const std::string& in_userName, const std::string& in_groupName) const
{
   auto itr = Groups.find(in_groupName);

   // Some how there's a group in the conf file that wasn't populated and there wasn't an error earlier? Should be
   // impossible, but return false for posterity.
   assert(itr != Groups.end());
   if (itr == Groups.end())
      return false;

   return itr->second.find(in_userName) == itr->second.end();
}

Error AbstractUserProfiles::Impl::iterateValues(
   const std::string& in_valueName,
   const std::function<Error (const std::string&)>& in_onValueFound) const
{
   for (const LevelValue& levelValue: LevelValues)
   {
      const auto itr = levelValue.second.find(in_valueName);
      if (itr != levelValue.second.end())
      {
         Error error = in_onValueFound(itr->second);
         if (error)
         {
            error.addProperty("section-name", levelValue.first.Name);
            return error;
         }
      }
   }

   return Success();
}

Error AbstractUserProfiles::Impl::parseLevels(
   const std::string& in_levelData,
   const std::set<std::string>& in_fieldNames)
{
   using namespace boost::property_tree;

   ptree profileTree;
   try
   {
      ini_parser::read_ini(in_levelData, profileTree);
   }
   catch (const ini_parser_error& e)
   {
      Error error = userProfileError(
         UserProfileError::CONF_PARSE_ERROR,
         "User Profiles configuration file is not in ini format.",
         ERROR_LOCATION);
      error.addProperty("description", e.message());
      error.addProperty("line", std::to_string(e.line()));

      return error;
   }

   std::set<std::string> groups;
   for (const ptree::value_type& sectionNode: profileTree)
   {
      Level section;
      if (sectionNode.first == "*")
      {
         section.Type = LevelType::ALL;
      }
      else if (boost::starts_with(sectionNode.first, "@"))
      {
         if (sectionNode.first.size() <= 1)
            return userProfileError(
               UserProfileError::CONF_PARSE_ERROR,
               "Invalid profile section: " + sectionNode.first,
               ERROR_LOCATION);

         section.Type = LevelType::GROUP;
         section.Name = sectionNode.first.substr(1); // Trim the @

         groups.insert(section.Name);
      }
      else if (!sectionNode.first.empty())
      {
         section.Type = LevelType::USER;
         section.Name = sectionNode.first;
      }
      else
         return userProfileError(
            UserProfileError::CONF_PARSE_ERROR,
            "Empty profile section",
            ERROR_LOCATION);

      ValueMap values;
      const auto end = in_fieldNames.end();
      for (const ptree::value_type& sectionValue: sectionNode.second)
      {
         if (in_fieldNames.find(sectionValue.first) == end)
            return userProfileError(
               UserProfileError::CONF_PARSE_ERROR,
               "Unknown value (" + sectionValue.first + ") in section [" + sectionNode.first + "]",
               ERROR_LOCATION);

         values[sectionValue.first] = sectionValue.second.get_value<std::string>();
      }

      LevelValues.emplace_back(section, values);
   }

   return populateGroups(groups);
}

Error AbstractUserProfiles::Impl::populateGroups(const std::set<std::string>& in_groupNames)
{
   // Get the buffer size - but set a conservative value if the system returns -1.
   long int bufferSize = ::sysconf(_SC_GETGR_R_SIZE_MAX);
   if (bufferSize == -1)
      bufferSize = 4096;

   // Don't re-allocate and resize the buffer for every iteration.
   std::vector<char> buffer;
   for (const std::string& groupName: in_groupNames)
   {
      buffer.clear();

      struct group grp, *grpResult;
      int retCode = 0;

      do
      {
         buffer.resize(bufferSize);
         retCode = ::getgrnam_r(groupName.c_str(), &grp, buffer.data(), buffer.size(), &grpResult);

         // If the buffer was too small, increase the size and try again.
         if (retCode == ERANGE)
            bufferSize *= 2;

      } while (retCode == ERANGE);

      if (grpResult == nullptr)
      {
         // If there's no group result but no error, the group could not be found. Convert to EACCES.
         if (retCode == 0)
            retCode = EACCES;

         Error error = systemError(retCode, "Unable to retrieve group information." ,ERROR_LOCATION);
         error.addProperty("group-value", groupName);
         return error;
      }

      // Success! Create an entry in the look-up map, and iterate through the members to add them to the set.
      char** usersItr = grp.gr_mem;
      while(*usersItr)
      {
         Groups[groupName].insert(*(usersItr++));
      }
   }

   return Success();
}

// AbstractUserProfiles ================================================================================================
Error AbstractUserProfiles::initialize()
{
   std::shared_ptr<std::istream> inputStream;
   Error error = getConfigurationFile().openForRead(inputStream);
   if (error)
   {
      error = userProfileError(
         UserProfileError::CONF_PARSE_ERROR,
         "Could not open configuration file for read.",
         error,
         ERROR_LOCATION);
      error.addProperty("file", getConfigurationFile().getAbsolutePath());
      return error;
   }

   // Read the whole file into a string stream.
   std::ostringstream oStrStream;
   try
   {
      // Ensure an exception will be thrown if the failbit or badbit is set.
      inputStream->exceptions(std::istream::failbit | std::istream::badbit);

      boost::iostreams::copy(*inputStream, oStrStream);
   }
   catch (std::exception& e)
   {
      error = userProfileError(UserProfileError::CONF_PARSE_ERROR, "Failed to read configuration file.", ERROR_LOCATION);
      error.addProperty("description", e.what());
      error.addProperty("file", getConfigurationFile().getAbsolutePath());
      return error;
   }

   error = m_impl->parseLevels(oStrStream.str(), getValidFieldNames());
   if (error)
      return error;

   return validateValues();
}

AbstractUserProfiles::AbstractUserProfiles() :
   m_impl(new Impl())
{
}

AbstractUserProfiles::AbstractUserProfiles(const std::string& in_pluginName) :
   m_impl(new Impl())
{
   m_impl->ConfigurationFile =
      system::FilePath("/etc/rstudio/").completeChildPath("launcher." + in_pluginName + ".profiles.conf");
}

template<typename T>
Error AbstractUserProfiles::getValueForUser(
   const std::string& in_valueName,
   const system::User& in_user,
   T& out_value) const
{
   const std::set<std::string>& validValueNames = getValidFieldNames();
   if (validValueNames.find(in_valueName) == validValueNames.end())
      return userProfileError(
         UserProfileError::INVALID_VALUE_ERROR,
         "The requested value \"" + in_valueName + "\" is not supported.",
         ERROR_LOCATION);

   std::string strValue;
   if (!m_impl->getValueForUser(in_valueName, in_user.getUsername(), strValue))
      return userProfileError(
         UserProfileError::VALUE_NOT_FOUND_ERROR,
         "The value \"" + in_valueName + "\" could not be found for the user \"" + in_user.getUsername() + "\".",
         ERROR_LOCATION);

   return parseValue(in_valueName, out_value);
}

bool AbstractUserProfiles::isValueNotFoundError(const Error& in_error)
{
   return (in_error == userProfileError(UserProfileError::VALUE_NOT_FOUND_ERROR, "", ErrorLocation()));
}

template <typename T>
Error AbstractUserProfiles::validateValue(const std::string& in_valueName) const
{
   return validateValue(
      in_valueName,
      [](const std::string& in_value)
      {
         T parsedVal;
         return parseValue(in_value, parsedVal);
      });
}

Error AbstractUserProfiles::validateValue(
   const std::string& in_valueName,
   const CustomValueValidator& in_validator) const
{
   Error error = m_impl->iterateValues(in_valueName, in_validator);
   if (error)
      return userProfileError(
         UserProfileError::CONF_PARSE_ERROR,
         "Invalid value(s) in " + getConfigurationFile().getAbsolutePath(),
         error,
         ERROR_LOCATION);

   return Success();
}

const system::FilePath& AbstractUserProfiles::getConfigurationFile() const
{
   return m_impl->ConfigurationFile;
}

// Template Instantiations =============================================================================================
#define INSTANTIATE_GET_VALUE_TEMPLATE(in_type)                               \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<in_type>(                         \
   const std::string&,                                                        \
   const system::User&,                                                       \
   in_type&) const;                                                           \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<std::set<in_type> >(              \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::set<in_type>&) const;                                                 \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<std::vector<in_type> >(           \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::vector<in_type>&) const;                                              \

#define INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(in_kType, in_vType)               \
/* U -> V map */                                                              \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<std::map<in_kType, in_vType> >(   \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<in_kType, in_vType>&) const;                                      \
/* set<U> -> V map */                                                         \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::set<in_kType>, in_vType> >(                                  \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::set<in_kType>, in_vType>&) const;                            \
/* U -> set<V> map */                                                         \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<in_kType, std::set<in_vType> > >(                                 \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<in_kType, std::set<in_vType> >&) const;                           \
/* set<U> -> set<V> map */                                                    \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::set<in_kType>, std::set<in_vType> > >(                       \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::set<in_kType>, std::set<in_vType> >&) const;                 \
/* vector<U> -> V map */                                                      \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::vector<in_kType>, in_vType> >(                               \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::vector<in_kType>, in_vType>&) const;                         \
/* U -> vector<V> map */                                                      \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<in_kType, std::vector<in_vType> > >(                              \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<in_kType, std::vector<in_vType> >&) const;                        \
/* vector<U> -> vector<V> map */                                              \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::vector<in_kType>, std::vector<in_vType> > >(                 \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::vector<in_kType>, std::vector<in_vType> >&) const;           \
/* set<U> -> vector<V> map */                                                 \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::set<in_kType>, std::vector<in_vType> > >(                    \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::set<in_kType>, std::vector<in_vType> >&) const;              \
/* vector<U> -> set<V> map */                                                 \
template <>                                                                   \
Error AbstractUserProfiles::getValueForUser<                                  \
   std::map<std::vector<in_kType>, std::set<in_vType> > >(                    \
   const std::string&,                                                        \
   const system::User&,                                                       \
   std::map<std::vector<in_kType>, std::set<in_vType> >&) const;              \


INSTANTIATE_GET_VALUE_TEMPLATE(bool)
INSTANTIATE_GET_VALUE_TEMPLATE(int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE(uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE(int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE(uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE(double)
INSTANTIATE_GET_VALUE_TEMPLATE(float)
INSTANTIATE_GET_VALUE_TEMPLATE(std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(bool, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int32_t, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint32_t, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(int64_t, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(uint64_t, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(double, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(float, std::string)

INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, bool)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, int32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, uint32_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, int64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, uint64_t)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, double)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, float)
INSTANTIATE_GET_VALUE_TEMPLATE_MAPS(std::string, std::string)

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio
