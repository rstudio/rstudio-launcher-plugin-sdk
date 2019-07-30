/*
 * QuickStartPluginApi.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
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

