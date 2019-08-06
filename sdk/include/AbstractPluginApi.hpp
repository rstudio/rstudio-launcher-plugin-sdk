/*
 * AbstractPluginApi.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_PLUGIN_API_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_PLUGIN_API_HPP

#include <boost/noncopyable.hpp>

#include "Error.hpp"

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Base class for the Launcher Plugin API.
 */
class AbstractPluginApi : public boost::noncopyable
{
public:
   /**
    * @brief Virtual destructor.
    */
   virtual ~AbstractPluginApi() = default;

   /**
    * @brief This method should initialize any components needed to communicate with the job scheduling tool, including
    *        custom options (TODO: other examples).
    *
    * @return Success if all components needed by this Plugin were successfully initialized; Error otherwise.
    */
   virtual Error initialize() = 0;
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
