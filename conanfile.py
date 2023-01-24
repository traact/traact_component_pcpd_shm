# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class TraactPackage(ConanFile):
    python_requires = "traact_run_env/1.0.0@traact/latest"
    python_requires_extend = "traact_run_env.TraactPackageCmake"

    name = "traact_component_pcpd_shm"
    description = "Source and Sink components using pcpd shared memory"
    url = "https://github.com/traact/traact_component_pcpd_shm.git"
    license = "MIT"
    author = "Frieder Pankratz"

    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"

    exports_sources = "src/*", "CMakeLists.txt"

    def requirements(self):
        # add your dependencies
        self.traact_requires("traact_spatial", "latest")
        self.traact_requires("traact_vision", "latest")
        #self.requires("pcpd_shm_client/0.0.1@artekmed/stable")
        #self.requires("capnproto/0.8.0@camposs/stable")
        self.requires("pcpd_shm_client/0.0.2@artekmed/stable")
        self.requires("capnproto/0.9.1@camposs/stable")
        self.requires("zlib/1.2.13")
        if self.options.with_tests:
            self.requires("gtest/cci.20210126")

    def configure(self):
        self.options['pcpd_shm_client'].with_python = False
        self.options['pcpd_shm_client'].with_visualization = False
        self.options['pcpd_shm_client'].with_apps = False
        self.options['capnproto'].shared = True
