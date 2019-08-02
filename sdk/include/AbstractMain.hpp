/*
 * AbstractMain.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_MAIN_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_MAIN_HPP

#include <boost/noncopyable.hpp>

#include <memory>
#include <system/FilePath.hpp>

#include "AbstractPluginApi.hpp"

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Base class for the Plugin Main class, which runs the plugin.
 */
class AbstractMain : public boost::noncopyable
{
public:
   /**
    * @brief Default Constructor.
    */
   AbstractMain() = default;

   /**
    * @brief Default destructor.
    */
   virtual ~AbstractMain() = default;

   /**
    * @brief Runs the plugin.
    *
    * @param in_argCount    The number of arguments in in_argList.
    * @param in_argList     The argument list to the program.
    *
    * @return 0 on a successful exit. A non-zero error code, otherwise.
    */
   int run(int in_argCount, char** in_argList);

private:
   /**
    * @brief Creates the Launcher Plugin API.
    *
    * @return The Plugin specific Launcher Plugin API.
    */
   virtual std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const = 0;

   /**
    * @brief Gets the configuration file for this program. The default is /etc/rstudio/launcher.<pluginName>.conf.
    *
    * @return The configuration file for this program.
    */
   virtual system::FilePath getConfigFile() const;

   /**
    * @brief Gets the name of this pluign.
    *
    * @return The name of this pluign.
    */
   virtual std::string getPluginName() const = 0;

   /**
    * @brief Gets the unique program ID for this plugin. The default ID is rstudio-<pluginName>-launcher.
    *
    * @return The unique program ID for this plugin.
    */
    virtual std::string getProgramId() const;
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
