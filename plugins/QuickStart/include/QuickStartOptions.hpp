/*
 * QuickStartOptions.hpp
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

#ifndef LAUNCHER_PLUGINS_QUICK_START_OPTIONS_HPP
#define LAUNCHER_PLUGINS_QUICK_START_OPTIONS_HPP

#include <Noncopyable.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

/**
 * @brief Class which defines options for the QuickStart Launcher Plugin.
 */
class QuickStartOptions : public Noncopyable
{
public:
   /**
    * @brief Gets the single instance of QuickStartOptions for the plugin.
    *
    * @return The single instance of QuickStartOptions for the plugin.
    */
   static QuickStartOptions& getInstance();

   /**
    * @brief Gets the value of the sample option.
    *
    * @return The value of the sample option.
    */
   bool getSampleOption() const;

   /**
    * @brief Method which initializes QuickStartOptions. This method should be called exactly once, before the options
    *        file is read.
    *
    * This is where QuickStart Options are registered with the Options object.
    */
   void initialize();

private:
   /**
    * @brief Private default constructor to prevent multiple instantiation.
    */
   QuickStartOptions() = default;

   /** The sample option */
   bool m_sampleOption;

};

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio

#endif
