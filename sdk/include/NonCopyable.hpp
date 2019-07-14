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

class NonCopyable
{
public:
   NonCopyable() = default;
   virtual ~NonCopyable() = default;

private:
   NonCopyable(const NonCopyable&) = delete;
   NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace launcher_plugins
} // namesapce rstudio

#endif
