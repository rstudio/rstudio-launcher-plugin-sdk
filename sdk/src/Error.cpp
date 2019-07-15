/*
 * Error.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "Error.hpp"

namespace rstudio {
namespace launcher_plugins {

Error::Error(int in_errorCode, std::string in_errorDescription, std::string in_errorMessage) :
   m_errorCode(in_errorCode),
   m_errorDescription(std::move(in_errorDescription)),
   m_errorMessage(std::move(in_errorMessage))
{

}

Error::operator bool()
{
   return 0 == m_errorCode;
}

} // namespace launcher_plugins
} // namespace rstudio

