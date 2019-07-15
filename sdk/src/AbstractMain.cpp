/*
 * AbstractMain.cpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "AbstractMain.hpp"

#include <iostream>

namespace rstudio{
namespace launcher_plugins
{

int AbstractMain::run(int, char**)
{
   std::cout << "Success" << std::endl;
}

} // namespace launcher_plugins
} // namespace rstudio
