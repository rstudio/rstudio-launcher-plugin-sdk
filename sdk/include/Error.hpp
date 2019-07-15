/*
 * Error.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_ERROR_HPP
#define LAUNCHER_PLUGINS_ERROR_HPP

#include <string>

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Class which represents an error.
 */
class Error
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_errorCode            The error code. (e.g. 1)
    * @param in_errorDescription     A contextual description of the error code. (e.g. "RequestNotSupported")
    * @param in_errorMessage         The detailed error message. (e.g. "The JobNetworkRequest is not supported by this plugin.")
    */
   explicit Error(
      int in_errorCode = 0,
      std::string in_errorDescription = "",
      std::string in_errorMessage = "");

   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~Error() = default;

   /**
    * @brief Overload operator bool to allow Errors to be treated as boolean values.
    *
    * @return True if there is an error; false otherwise.
    */
   explicit operator bool();

private:
   // The error code.
   int m_errorCode;

   // The error description.
   std::string m_errorDescription;

   // The error message.
   std::string m_errorMessage;
};

/**
 * @brief Class which represents a successful operation (i.e. no error).
 */
class Success: public Error
{
   /**
    * @brief Constructor.
    */
   Success() { };
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
