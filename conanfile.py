# /usr/bin/python3
import os
from conan import ConanFile
from conan.tools.build import can_run

class TraactPackage(ConanFile):
    python_requires = "traact_base/0.0.0@traact/latest"
    python_requires_extend = "traact_base.TraactPackageCmake"

    name = "traact_component_pcpd_shm"
    version = "0.0.0"
    description = "Source and Sink components using pcpd shared memory"
    url = "https://github.com/traact/traact_component_pcpd_shm.git"
    license = "MIT"
    author = "Frieder Pankratz"

    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"

    exports_sources = "src/*", "CMakeLists.txt"

    def requirements(self):
        self.requires("traact_spatial/0.0.0@traact/latest")
        self.requires("traact_vision/0.0.0@traact/latest")

        #self.requires("pcpd_shm_client/0.0.1@artekmed/stable")
        #self.requires("capnproto/0.8.0@camposs/stable")
        self.requires("pcpd_shm_client/0.0.2@artekmed/stable")
        self.requires("capnproto/0.10.3")
        self.requires("gtest/1.14.0", override=True)
        
        

    def configure(self):
        self.options['pcpd_shm_client'].with_python = False
        self.options['pcpd_shm_client'].with_visualization = False
        self.options['pcpd_shm_client'].with_apps = False
        self.options['capnproto'].shared = True
        self.options['iceoryx/*'].with_introspection = True

    def _after_package_info(self):
        self.cpp_info.libs = ["traact_component_pcpd_shm"]
