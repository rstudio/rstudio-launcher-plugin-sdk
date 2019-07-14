/*
 * AbstractMain.hpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_ABSTRACT_MAIN_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_MAIN_HPP

#include "NonCopyable.hpp"

#include <memory>

#include "AbstractPluginApi.hpp"

namespace rstudio {
namespace launcher_plugins {

/**
 * @brief Base class responsible for creating the launcher plugin API.
 */
class AbstractMain : public NonCopyable
{
public:
   AbstractMain() = default;
   virtual ~AbstractMain() = default;

   int run(int argc, char** argv);

private:
   virtual std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const = 0;
};

} // namespace launcher_plugins
} // namespace rstudio

#endif
