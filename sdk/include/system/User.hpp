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

#include <string>

#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Class which represents a system user.
 */
class User
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
    * @brief Copy constructor.
    *
    * @param in_other   The user to copy.
    */
    User(const User& in_other);

   /**
    * @brief Creates a user by username.
    *
    * @param in_username    The name of the user.
    */
   explicit User(std::string in_username);

   /**
    * @brief Returns whether this object represents all users or not. See the default constructor for more details.
    *
    * @return True if this object represents all users; false otherwise.
    */
   bool isAllUsers() const;

   /**
    * @brief Returns the name of this user.
    *
    * @return The name of this user ("*" for all users).
    */
   const std::string& getUsername() const;

   /**
    * @brief Overloaded assignment operator.
    *
    * @param in_other   The user to copy to this one.
    *
    * @return This user.
    */
   User& operator=(const User& in_other);

private:
   // The private implementation of User.
   PRIVATE_IMPL(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
