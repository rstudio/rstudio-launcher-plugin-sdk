/*
 * UserProfileTests.cpp
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

#include <TestMain.hpp>

#include <boost/algorithm/string.hpp>

#include <Error.hpp>
#include <options/AbstractUserProfiles.hpp>
#include <system/FilePath.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace options {

constexpr char const* USER_ONE   = "rlpstestusrone";
constexpr char const* USER_TWO   = "rlpstestusrtwo";
constexpr char const* USER_THREE = "rlpstestusrthree";
constexpr char const* USER_FOUR  = "rlpstestusrfour";
constexpr char const* USER_FIVE  = "rlpstestusrfive";

enum class Weekday
{
   MONDAY,
   TUESDAY,
   WEDNESDAY,
   THURSDAY,
   FRIDAY,
   SATURDAY,
   SUNDAY
};

class TestUserProfiles: public AbstractUserProfiles
{
public:
   explicit TestUserProfiles(const std::string& in_fileName)
   {
      m_confFile = system::FilePath::safeCurrentPath(
         system::FilePath()).completeChildPath("profile-files").completeChildPath(in_fileName);

      m_validFieldNames.insert("int-field");
      m_validFieldNames.insert("uint-field");
      m_validFieldNames.insert("bool-field");
      m_validFieldNames.insert("str-field");
      m_validFieldNames.insert("double-field");
      m_validFieldNames.insert("str-set-field");
      m_validFieldNames.insert("float-list-field");
      m_validFieldNames.insert("str-int-list-map-field");
      m_validFieldNames.insert("custom-type-field");
   }

   int64_t getIntField(const system::User& in_user) const
   {
      // Default value
      int64_t value = 0;
      Error error = getValueForUser("int-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   uint32_t getUIntField(const system::User& in_user) const
   {
      // Default value
      uint32_t value = 0;
      Error error = getValueForUser("uint-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   bool getBoolField(const system::User& in_user) const
   {
      bool value = false;
      Error error = getValueForUser("bool-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   std::string getStrField(const system::User& in_user) const
   {
      std::string value;
      Error error = getValueForUser("str-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   double getDoubleField(const system::User& in_user) const
   {
      double value = 0.0f;
      Error error = getValueForUser("double-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   std::set<std::string> getSetField(const system::User& in_user) const
   {
      std::set<std::string> value;
      Error error = getValueForUser("str-set-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   std::vector<float> getListField(const system::User& in_user) const
   {
      std::vector<float> value;
      Error error = getValueForUser("float-list-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   std::map<std::string, std::vector<int32_t> > getMapField(const system::User& in_user) const
   {
      std::map<std::string, std::vector<int32_t> > value;
      Error error = getValueForUser("str-int-list-map-field", in_user, value);
      CHECK((!error || isValueNotFoundError(error)));
      return value;
   }

   Error getCustomField(const system::User& in_user, Weekday& out_value) const
   {
      std::string valAsStr;
      Error error = getValueForUser("custom-value", in_user, valAsStr);
      if (error)
         return error;

      boost::trim(valAsStr);
      if (boost::iequals(valAsStr, "monday"))
         out_value = Weekday::MONDAY;
      else if (boost::iequals(valAsStr, "tuesday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "wednesday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "thursday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "friday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "saturday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "sunday"))
         out_value = Weekday::TUESDAY;
      else
      {
         // This should be caught by validation.
         assert(false);
         return Error(
            "InvalidWeekday",
            1,
            "The value " + valAsStr + " is not a valid day of the week.",
            ERROR_LOCATION);
      }

      return Success();
   }

   void getWrongTypeField()
   {
      system::User user;
      Error error = system::User::getUserFromIdentifier(USER_ONE, user);
      REQUIRE_FALSE(error);

      std::map<std::set<int>, std::vector<bool>> value;
      error = getValueForUser("bool-field", user, value);
      CHECK((error && error.getName() == "UserProfilesError" && error.getCode() == 3));
   }

   void getUnsupportedField()
   {
      system::User user;
      Error error = system::User::getUserFromIdentifier(USER_ONE, user);
      REQUIRE_FALSE(error);

      std::map<std::vector<uint64_t>, std::vector<std::string>> value;
      error = getValueForUser("not-registered-field", user, value);
      CHECK((error && error.getName() == "UserProfilesError" && error.getCode() == 4));
   }

private:
   system::FilePath m_confFile;

   std::set<std::string> m_validFieldNames;

   const system::FilePath& getConfigurationFile() const override
   {
      return m_confFile;
   }

   const std::set<std::string>& getValidFieldNames() const override
   {
      return m_validFieldNames;
   }

   Error validateValues() const override
   {
      Error error = validateValue<int64_t>("int-field");
      if (error)
         return error;

      error = validateValue<uint32_t>("uint-field");
      if (error)
         return error;

      error = validateValue<bool>("bool-field");
      if (error)
         return error;

      error = validateValue<std::string>("str-field");
      if (error)
         return error;

      error = validateValue<double>("double-field");
      if (error)
         return error;

      error = validateValue<std::set<std::string> >("str-set-field");
      if (error)
         return error;

      error = validateValue<std::vector<float> >("float-list-field");
      if (error)
         return error;

      error = validateValue<std::map<std::string, std::vector<int32_t> > >("str-int-list-map-field");
      if (error)
         return error;

      return validateValue(
         "custom-type-field",
         [](const std::string& in_value) -> Error
         {
            std::string trimmed = boost::trim_copy(in_value);

            if (boost::iequals(trimmed, "monday") ||
               boost::iequals(trimmed, "tuesday") ||
               boost::iequals(trimmed, "wednesday") ||
               boost::iequals(trimmed, "thursday") ||
               boost::iequals(trimmed, "friday") ||
               boost::iequals(trimmed, "saturday") ||
               boost::iequals(trimmed, "sunday"))
               return Success();

            return Error(
               "InvalidWeekday",
               1,
               "The value " + in_value + " is not a valid day of the week.",
               ERROR_LOCATION);
         });
   }
};

TEST_CASE("Simple case")
{
   TestUserProfiles profiles("simple.profiles.conf");

   system::User user;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, user));

   Weekday day = Weekday::MONDAY;

   std::set<std::string> expectedSet;
   expectedSet.insert("value1");
   expectedSet.insert("value2");
   expectedSet.insert("value3");
   expectedSet.insert("value with spaces");

   std::vector<float> expectedList;
   expectedList.push_back(25.5f);
   expectedList.push_back(38.4f);
   expectedList.push_back(607.25f);

   std::map<std::string, std::vector<int32_t> > expectedMap;
   expectedMap["key1"].push_back(1);
   expectedMap["key1"].push_back(2);
   expectedMap["key1"].push_back(3);
   expectedMap["key1"].push_back(4);
   expectedMap["key2"].push_back(5);
   expectedMap["key2"].push_back(4);
   expectedMap["key2"].push_back(3);
   expectedMap["key3"].push_back(10);
   expectedMap["key3"].push_back(35);
   expectedMap["key3"].push_back(15);

   REQUIRE_FALSE(profiles.initialize());
   CHECK(profiles.getIntField(user) == -43);
   CHECK(profiles.getUIntField(user) == 3028);
   CHECK(profiles.getBoolField(user) == true);
   CHECK(profiles.getStrField(user) == "some string value");
   CHECK(profiles.getDoubleField(user) == 54.3f);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), profiles.getSetField(user).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), profiles.getListField(user).begin()));

   auto actualMap = profiles.getMapField(user);
   CHECK(actualMap.size() == expectedMap.size());
   for (auto itr = expectedMap.begin(), end = expectedMap.end(); itr != end; ++itr)
   {
      auto actualItr = actualMap.find(itr->first);
      CHECK(actualItr !=  actualMap.end());
      if (actualItr != actualMap.end())
         CHECK(std::equal(itr->second.begin(), itr->second.end(), actualItr->second.begin()));
   }

   CHECK((!profiles.getCustomField(user, day) && day == Weekday::TUESDAY));

   // Validate proper handling of mistakes
   profiles.getWrongTypeField();
   profiles.getUnsupportedField();
}

TEST_CASE("Parsing errors")
{

}

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio
