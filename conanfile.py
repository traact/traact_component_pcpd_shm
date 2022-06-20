# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class TraactPackage(ConanFile):
    python_requires = "traact_run_env/1.0.0@traact/latest"
    python_requires_extend = "traact_run_env.TraactPackageCmake"

    name = "PCPD SHM client for traact"
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
        self.requires("pcpd_shm_client/0.0.2@artekmed/stable")
        if self.options.with_tests:
            self.requires("gtest/[>=1.11.0]")

    def configure(self):
        self.options['pcpd_shm_client'].with_python = False
        self.options['pcpd_shm_client'].with_visualization = False
        self.options['pcpd_shm_client'].with_apps = False
