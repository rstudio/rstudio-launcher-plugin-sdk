/*
 * SingularityPluginApi.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "SingularityPluginApi.hpp"

#include "SingularityOptions.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

Error SingularityPluginApi::initialize()
{
   // Make sure SingularityOptions have been initialized.
   SingularityOptions::getInstance().initialize();

   return Success();
}

} // namespace singularity
} // namespace launcher_plugins
} // namespace rstudio

