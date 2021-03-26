#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <AbstractMain.hpp>
#include <api/AbstractPluginApi.hpp>
#include <api/IJobSource.hpp>
#include <jobs/AbstractJobRepository.hpp>

#include <Error.hpp>

namespace py = pybind11;

namespace rstudio {
namespace launcher_plugins {

class AbstractMainTrampoline : public AbstractMain 
{
protected:
    std::shared_ptr<api::AbstractPluginApi> createLauncherPluginApi(
      std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator) const override 
    {
        PYBIND11_OVERRIDE_PURE(
            std::shared_ptr<api::AbstractPluginApi>,
            AbstractMain,
            createLauncherPluginApi,
            in_launcherCommunicator);
    }

   system::FilePath getConfigFile() const override
   {
        PYBIND11_OVERRIDE(
            system::FilePath,
            AbstractMain,
            getConfigFile);
   }

   std::string getPluginName() const override
   {
        PYBIND11_OVERRIDE_PURE(
            std::string,
            AbstractMain,
            getPluginName);
   }

   std::string getProgramId() const override
   {
        PYBIND11_OVERRIDE(
            std::string,
            AbstractMain,
            getProgramId);
   }

   Error initialize() override
   {
        PYBIND11_OVERRIDE_PURE(
            Error,
            AbstractMain,
            initialize);
   }
};

class AbstractMainPublisher : public AbstractMain {
public:
    using AbstractMain::AbstractMain;
    using AbstractMain::initialize;
    using AbstractMain::getPluginName;
};

} // namespace launcher_plugins
} // namespace rstudio

namespace rlps = rstudio::launcher_plugins;

PYBIND11_MODULE(rlpswrapper, m) {
    py::class_<rlps::Error>(m, "Error")
        .def(py::init<>())
        .def("getSummary", &rlps::Error::getSummary);

    py::class_<rlps::Success, rlps::Error>(m, "Success")
        .def(py::init<>());

    py::class_<rlps::AbstractMain, rlps::AbstractMainTrampoline>(m, "AbstractMain")
        .def(py::init<>())
        .def("run", static_cast<int (rlps::AbstractMain::*)(int, std::vector<std::string>)>(&rlps::AbstractMain::run))
        .def("initialize", &rlps::AbstractMainPublisher::initialize)
        .def("getPluginName", &rlps::AbstractMainPublisher::getPluginName);
}