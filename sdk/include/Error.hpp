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

#include <boost/core/noncopyable.hpp>

#include <boost/current_function.hpp>
#include <boost/system/error_code.hpp>

#include <string>

#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {

class Error;
class Success;

/**
 * @brief A class which can be derived from in order to prevent child classes from being derived from further.
 */
class Error_Lock
{
   // Classes which may derive this class.
   friend class Error;
   friend class Success;

private:
   /**
    * @brief Private constructor to prevent further derivation of Error and Success.
    */
   Error_Lock() = default;
};

/**
 * @brief Class which represents the location of an error
 */
class ErrorLocation : boost::noncopyable
{
public:
   /**
    * @brief Default constructor.
    */
   ErrorLocation();

   /**
    * @brief Move constructor.
    *
    * @param in_other   The error location to move to this.
    */
   ErrorLocation(ErrorLocation&& in_other) noexcept;

   /**
    * @brief Constructor.
    *
    * param in_function     The function in which the error occurred.
    * @param in_file        The file in which the error occurred.
    * @param in_line        The line at which the error occurred.
    */
   ErrorLocation(const char* in_function, const char* in_file, long in_line);

   /**
    * @brief Checks whether the location is set.
    *
    * @return True if a location has been set; false otherwise.
    */
   bool hasLocation() const;

   /**
    * @brief Gets the function where the error occurred.
    *
    * @return The function where the error occurred.
    */
   const std::string& getFunction() const;

   /**
    * @brief Gets the file where the error occurred.
    *
    * @return The file where the error occurred.
    */
   const std::string& getFile() const;

   /**
    * @brief Gets the line where the error occurred.
    *
    * @return The line where the error occurred.
    */
   long getLine() const;

   /**
    * @brief Formats the error location as a string.
    *
    * @return The error location formatted as a string.
    */
   std::string asString() const;

private:
   // The private implementation of ErrorLocation.
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents an error.
 *
 * This class should not be derived from since it is returned by value throughout the SDK. Instead, create helper
 * functions for each "sub-class" of Error that would be desired.
 *
 * Errors are not copyable. To return by value, use std::move.
 */
class Error : virtual Error_Lock
{
public:
   /**
    * @brief Default constructor.
    */
   Error();

   /**
    * @brief Constructor.
    *
    * @param in_ec              The boost error code.
    * @param in_errorLocation   The location of the error.
    */
   Error(const boost::system::error_code& in_ec, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_ec              The boost error code.
    * @param in_cause           The error which caused this error.
    * @param in_errorLocation   The location of the error.
    */
   Error(const boost::system::error_code& in_ec, Error in_cause, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_ec              The boost error code.
    * @param in_errorMessage    The detailed error message. (e.g. "The JobNetworkRequest is not supported by this plugin.")
    * @param in_errorLocation   The location of the error.
    */
   Error(const boost::system::error_code& in_ec, std::string in_errorMessage, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_ec              The boost error code.
    * @param in_errorMessage    The detailed error message. (e.g. "The JobNetworkRequest is not supported by this plugin.")
    * @param in_cause           The error which caused this error.
    * @param in_errorLocation   The location of the error.
    */
   Error(const boost::system::error_code& in_ec, std::string in_errorMessage, Error in_cause, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_errorCode        The error code.
    * @param in_name             A contextual name of the error code. (e.g. "RequestNotSupported")
    * @param in_errorLocation    The location of the error.
    */
   Error(int in_errorCode, std::string in_name, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_errorCode        The error code.
    * @param in_name             A contextual name of the error code. (e.g. "RequestNotSupported")
    * @param in_cause            The error which caused this error.
    * @param in_errorLocation    The location of the error.
    */
   Error(int in_errorCode, std::string in_name, Error in_cause, ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_errorCode            The error code. (e.g. 1)
    * @param in_errorName            A contextual name of the error code. (e.g. "RequestNotSupported")
    * @param in_errorMessage         The detailed error message. (e.g. "The JobNetworkRequest is not supported by this plugin.")
    * @param in_errorLocation        The location of the error.
    */
   Error(
      int in_errorCode,
      std::string in_errorName,
      std::string in_errorMessage,
      ErrorLocation in_errorLocation);

   /**
    * @brief Constructor.
    *
    * @param in_errorCode            The error code. (e.g. 1)
    * @param in_errorName            A contextual name of the error code. (e.g. "RequestNotSupported")
    * @param in_errorMessage         The detailed error message. (e.g. "The JobNetworkRequest is not supported by this plugin.")
    * @param in_cause                The error which caused this error.
    * @param in_errorLocation        The location of the error.
    */
   Error(
      int in_errorCode,
      std::string in_errorName,
      std::string in_errorMessage,
      Error in_cause,
      ErrorLocation in_errorLocation);

   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~Error() = default;

   /**
    * @brief Overloaded operator bool to allow Errors to be treated as boolean values.
    *
    * @return True if there is an error; false otherwise.
    */
   operator bool() const;

   /**
    * @brief Formats the error as a string.
    *
    * @return The error formatted as a string.
    */
   std::string asString() const;

   /**
    * @brief Formats a summary of the error as a string.
    *
    * @return A summary of the error.
    */
   std::string getSummary() const;

   /**
    * @brief Gets the error code.
    *
    * @return The error code.
    */
   int getErrorCode() const;

   /**
    * @brief Gets the name of the error.
    *
    * @return The name of the error.
    */
   const std::string& getName() const;

   /**
    * @brief Gets the error message.
    *
    * @return The error message.
    */
   const std::string& getMessage() const;

   /**
    * @brief Gets the cause of the error.
    *
    * @return The cause of the error.
    */
   const Error& getCause() const;

   /**
    * @brief Gets the location where the error occurred.
    *
    * @return The location where the error occurred.
    */
   const ErrorLocation& getLocation() const;


private:
   // The private implementation of Error. This is safe because the public interface of Error is all const.
   PRIVATE_IMPL_SHARED(m_impl);
};

/**
 * @brief Class which represents a successful operation (i.e. no error).
 */
class Success: public Error
{
public:
   /**
    * @brief Constructor.
    */
   Success() : Error() { };
};

/**
 * @brief Function which creates a system error.
 *
 * @param in_errorCode        The error code.
 * @param in_errorLocation    The location of the error.
 *
 * @return A system error.
 */
Error systemError(int in_errorCode, ErrorLocation in_location);

/**
 * @brief Function which creates a system error..
 *
 * @param in_errorCode            The error code. (e.g. 1)
 * @param in_cause                The error which caused this error.
 * @param in_errorLocation        The location of the error.
 *
 * @return A system error.
 */
Error systemError(int in_errorCode, Error in_cause, ErrorLocation in_location);

/**
 * @brief Function which creates a system error..
 *
 * @param in_errorCode            The error code. (e.g. 1)
 * @param in_errorMessage         The detailed error message. (e.g. "Failed to open socket while attempting to connect to Kubernetes.")
 * @param in_errorLocation        The location of the error.
 *
 * @return A system error.
 */
Error systemError(int in_errorCode, std::string in_errorMessage, ErrorLocation in_location);

/**
 * @brief Function which creates a system error..
 *
 * @param in_errorCode            The error code. (e.g. 1)
 * @param in_errorMessage         The detailed error message. (e.g. "Failed to open socket while attempting to connect to Kubernetes.")
 * @param in_cause                The error which caused this error.
 * @param in_errorLocation        The location of the error.
 *
 * @return A system error.
 */
Error systemError(int in_errorCode, std::string in_errorMessage, Error in_cause, ErrorLocation in_location);

#define ERROR_LOCATION rstudio::launcher_plugins::ErrorLocation( \
   BOOST_CURRENT_FUNCTION,__FILE__,__LINE__)

} // namespace launcher_plugins
} // namespace rstudio

#endif
