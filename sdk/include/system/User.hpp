/*
 * User.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_USER_HPP
#define LAUNCHER_PLUGINS_USER_HPP

#include <boost/noncopyable.hpp>

#include <string>

#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Class which represents a system user.
 */
class User : boost::noncopyable
{
public:
   /**
    * @brief Default Constructor.
    *
    * Creates a user object which represents all users. It should be used for Launcher requests that apply to all users
    * (e.g. get jobs for all users). Username == "*".
    */
   User();

   /**
    * @brief Creates a user by username.
    *
    * @param in_username    The name of the user.
    */
   explicit User(std::string in_username);

   /**
    * @brief Returns whether this object represents all users or not. See the default constructor for more details.
    * @return
    */
   bool isAllUsers() const;

   const std::string& getUsername() const;

private:
   PRIVATE_IMPL(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
