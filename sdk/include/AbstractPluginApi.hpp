/*
 * AbstractPluginApi.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
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
   virtual ~AbstractPluginApi() = default;

   virtual Error initialize() = 0;
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
