/*
 * SingularityPluginApi.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef SINGULARITY_SINGULARITY_PLUGIN_API_HPP
#define SINGULARITY_SINGULARITY_PLUGIN_API_HPP

#include <AbstractPluginApi.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

/**
 * @brief Launcher Plugin API for the Singularity Plugin.
 */
class SingularityPluginApi : public AbstractPluginApi
{
public:
   Error initialize() override;
};

} // namespace singularity
} // namespace launcher_plugins
} // namespace rstudio

#endif
