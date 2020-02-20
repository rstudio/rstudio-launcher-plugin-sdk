/*
 * QuickStartPluginApi.cpp
 * 
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include "QuickStartPluginApi.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

Error QuickStartPluginApi::initialize()
{
   // TODO #?: Initialize everything your plugin API needs to work. For example:
   //    - ensure your custom options are registered.
   //    - open (or test) communication with the job management system.
   //    - add any custom logging destinations to the logger.
   return Success();
}

}
} // namespace launcher_plugins
} // namespace rstudio

