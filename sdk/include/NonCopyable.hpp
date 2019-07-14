/*
 * NonCopyable.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_NON_COPYABLE_HPP
#define LAUNCHER_PLUGINS_NON_COPYABLE_HPP

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Base class for classes which should not be copied.
 */
class NonCopyable
{
public:
   /**
    * @brief Default Constructor.
    */
   NonCopyable() = default;

   /**
    * @brief Default Destructor.
    */
   virtual ~NonCopyable() = default;

private:
   /**
    * @brief Deleted Copy Constructor.
    */
   NonCopyable(const NonCopyable&) = delete;

   /**
    * @brief Deleted Assignment Operator.
    */
   NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace launcher_plugins
} // namesapce rstudio

#endif
