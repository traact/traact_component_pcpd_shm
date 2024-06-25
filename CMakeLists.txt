cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME traact_pcpd)

project(${TARGET_NAME} VERSION "0.0.0" LANGUAGES CXX)

find_package(traact_base)
list(APPEND CMAKE_MODULE_PATH ${traact_base_INCLUDE_DIR})

include(traact_default_library_setup)

# add your source files here
set(COMPONENT_SRC src/ShmCompositeBufferSource.cpp src/SynchronizedBufferReader.cpp src/SynchronizedBufferReader.h src/utils.h src/ShmSensorCalibrationSource.cpp
        src/dds/DDSSink.cpp)


add_library(${TARGET_NAME} SHARED ${COMPONENT_SRC})

find_package(CapnProto CONFIG REQUIRED)
find_package(artekmed_schema_capnproto CONFIG REQUIRED)
find_package(traact_spatial CONFIG REQUIRED)
find_package(traact_vision CONFIG REQUIRED)
find_package(iceoryx CONFIG REQUIRED)
find_package(pcpd_shm_client CONFIG REQUIRED)
find_package(CapnProto CONFIG REQUIRED)

target_link_libraries(${TARGET_NAME} PRIVATE
        opencv::opencv
        pcpd_shm_client::pcpd_shm_client
        traact_core::traact_core
        traact_spatial::traact_spatial
        traact_vision::traact_vision
        CapnProto::capnp-rpc
        iceoryx::iceoryx
        artekmed_schema_capnproto::artekmed_schema_capnproto
        )

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(traact_default_library_final)