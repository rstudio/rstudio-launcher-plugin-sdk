/*
 * User.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "system/User.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

struct User::Impl
{
   explicit Impl(std::string in_name) : Name(std::move(in_name)) {};

   std::string Name;
};

PRIVATE_IMPL_DELETER_IMPL(User)

User::User() :
   m_impl(new Impl("*"))
{
}

User::User(const User& in_other) :
   m_impl(new Impl(in_other.m_impl->Name))
{

}

User::User(std::string in_username) :
   m_impl(new Impl(std::move(in_username)))
{
}

bool User::isAllUsers() const
{
   return m_impl->Name == "*";
}

const std::string& User::getUsername() const
{
   return m_impl->Name;
}

User& User::operator=(const User& in_other)
{
   m_impl->Name = in_other.m_impl->Name;
   return *this;
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

