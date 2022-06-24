cmake_minimum_required(VERSION 3.16)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CONAN_TRAACT_RUN_ENV_ROOT}/cmake)

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION})

include(traact_default_library_setup)

find_package(CapnProto CONFIG REQUIRED)

find_package(artekmed_schema CONFIG REQUIRED)

# add your source files here
set(COMPONENT_SRC src/ShmCompositeBufferSource.cpp src/SynchronizedBufferReader.cpp src/SynchronizedBufferReader.h src/utils.h src/ShmSensorCalibrationSource.cpp)

add_library(${CONAN_PACKAGE_NAME} SHARED ${COMPONENT_SRC})

#conan_target_link_libraries(${CONAN_PACKAGE_NAME})
target_link_libraries(${CONAN_PACKAGE_NAME} PRIVATE
        CONAN_PKG::opencv
        CONAN_PKG::pcpd_shm_client
        CONAN_PKG::traact_core
        CONAN_PKG::traact_spatial
        CONAN_PKG::traact_vision
        CapnProto::capnp-rpc
        CONAN_PKG::iceoryx
        )

include(traact_default_library_final)