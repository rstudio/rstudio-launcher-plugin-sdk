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
      Error error = getValueForUser("custom-type-field", in_user, valAsStr);
      if (error)
         return error;

      boost::trim(valAsStr);
      if (boost::iequals(valAsStr, "monday"))
         out_value = Weekday::MONDAY;
      else if (boost::iequals(valAsStr, "tuesday"))
         out_value = Weekday::TUESDAY;
      else if (boost::iequals(valAsStr, "wednesday"))
         out_value = Weekday::WEDNESDAY;
      else if (boost::iequals(valAsStr, "thursday"))
         out_value = Weekday::THURSDAY;
      else if (boost::iequals(valAsStr, "friday"))
         out_value = Weekday::FRIDAY;
      else if (boost::iequals(valAsStr, "saturday"))
         out_value = Weekday::SATURDAY;
      else if (boost::iequals(valAsStr, "sunday"))
         out_value = Weekday::SUNDAY;
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

void checkMap(
   const std::map<std::string, std::vector<int32_t> >& in_actual,
   const std::map<std::string, std::vector<int32_t> >& in_expected)
{
   CHECK(in_actual.size() == in_expected.size());
   for (auto itr = in_expected.begin(), end = in_expected.end(); itr != end; ++itr)
   {
      auto actualItr = in_actual.find(itr->first);
      CHECK(actualItr !=  in_actual.end());
      if (actualItr != in_actual.end())
         CHECK(std::equal(itr->second.begin(), itr->second.end(), actualItr->second.begin()));
   }
}

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
   CHECK(profiles.getBoolField(user));
   CHECK(profiles.getStrField(user) == "some string value");
   CHECK(profiles.getDoubleField(user) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), profiles.getSetField(user).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), profiles.getListField(user).begin()));

   checkMap(profiles.getMapField(user), expectedMap);


   CHECK((!profiles.getCustomField(user, day) && day == Weekday::TUESDAY));

   // Validate proper handling of mistakes
   profiles.getWrongTypeField();
   profiles.getUnsupportedField();
}

TEST_CASE("Parsing errors")
{
   TestUserProfiles badInt("badInt.profiles.conf");
   TestUserProfiles badList("badList.profiles.conf");
   TestUserProfiles badMap("badMap.profiles.conf");
   TestUserProfiles badGroup("badGroup.profiles.conf");

   CHECK(badInt.initialize());
   CHECK(badList.initialize());
   CHECK(badMap.initialize());
   CHECK(badGroup.initialize());
}

TEST_CASE("Complex case")
{
   system::User userOne;
   system::User userTwo;
   system::User userThree;
   system::User userFour;
   system::User userFive;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, userOne));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, userTwo));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_THREE, userThree));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FOUR, userFour));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FIVE, userFive));


   // Precedence of groups in the test file is two < one < three. Users who are in groups two and one should see values
   // from group one. User two should see user two specific values.

   // Groups:
   //    user one: group one
   //    user two: group one, group two, group three
   //    user three: group two
   //    user four: group two, group three
   //    user five: group one, group three

   TestUserProfiles userProfiles("complex.profiles.conf");
   REQUIRE_FALSE(userProfiles.initialize());

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

   std::map<std::string, std::vector<int32_t> > expectedMap, group1Map;
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

   group1Map["key1"].push_back(60);
   group1Map["key1"].push_back(897);
   group1Map["key1"].push_back(33);

   // Validate User One values.
   CHECK(userProfiles.getIntField(userOne) == -43);
   CHECK(userProfiles.getUIntField(userOne) == 10);
   CHECK_FALSE(userProfiles.getBoolField(userOne));
   CHECK(userProfiles.getStrField(userOne) == "Group One Users");
   CHECK(userProfiles.getDoubleField(userOne) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), userProfiles.getSetField(userOne).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), userProfiles.getListField(userOne).begin()));
   checkMap(userProfiles.getMapField(userOne), group1Map);
   CHECK((!userProfiles.getCustomField(userOne, day) && day == Weekday::SATURDAY));
   
   // Validate User Two values.
   day = Weekday::MONDAY;
   CHECK(userProfiles.getIntField(userTwo) == -43);
   CHECK(userProfiles.getUIntField(userTwo) == 10);
   CHECK(userProfiles.getBoolField(userTwo));
   CHECK(userProfiles.getStrField(userTwo) == "Test User Two");
   CHECK(userProfiles.getDoubleField(userTwo) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), userProfiles.getSetField(userTwo).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), userProfiles.getListField(userTwo).begin()));
   checkMap(userProfiles.getMapField(userTwo), group1Map);
   CHECK((!userProfiles.getCustomField(userTwo, day) && day == Weekday::FRIDAY));

   // Validate User Three values.
   day = Weekday::MONDAY;
   CHECK(userProfiles.getIntField(userThree) == -43);
   CHECK(userProfiles.getUIntField(userThree) == 3028);
   CHECK(userProfiles.getBoolField(userThree));
   CHECK(userProfiles.getStrField(userThree) == "Group Two Users");
   CHECK(userProfiles.getDoubleField(userThree) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), userProfiles.getSetField(userThree).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), userProfiles.getListField(userThree).begin()));
   checkMap(userProfiles.getMapField(userThree), expectedMap);
   CHECK((!userProfiles.getCustomField(userThree, day) && day == Weekday::WEDNESDAY));

   // Validate User Four values.
   day = Weekday::MONDAY;
   CHECK(userProfiles.getIntField(userFour) == -43);
   CHECK(userProfiles.getUIntField(userFour) == 3028);
   CHECK(userProfiles.getBoolField(userFour));
   CHECK(userProfiles.getStrField(userFour) == "Group Three Users");
   CHECK(userProfiles.getDoubleField(userFour) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), userProfiles.getSetField(userFour).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), userProfiles.getListField(userFour).begin()));
   checkMap(userProfiles.getMapField(userFour), expectedMap);
   CHECK((!userProfiles.getCustomField(userFour, day) && day == Weekday::FRIDAY));

   // Validate User Five values.
   day = Weekday::MONDAY;
   CHECK(userProfiles.getIntField(userFive) == -43);
   CHECK(userProfiles.getUIntField(userFive) == 10);
   CHECK_FALSE(userProfiles.getBoolField(userFive));
   CHECK(userProfiles.getStrField(userFive) == "Group Three Users");
   CHECK(userProfiles.getDoubleField(userFive) == 54.3);
   CHECK(std::equal(expectedSet.begin(), expectedSet.end(), userProfiles.getSetField(userFive).begin()));
   CHECK(std::equal(expectedList.begin(), expectedList.end(), userProfiles.getListField(userFive).begin()));
   checkMap(userProfiles.getMapField(userFive), group1Map);
   CHECK((!userProfiles.getCustomField(userFive, day) && day == Weekday::FRIDAY));

}

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio
