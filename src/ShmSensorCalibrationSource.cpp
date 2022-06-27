/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
// based on multistream_ndi_sender example

#include <iceoryx_posh/runtime/posh_runtime.hpp>
#include <traact/traact.h>
#include <traact/vision.h>
#include <pcpd_shm_client/datatypes/stream_headers.h>
#include <pcpd_shm_client/datatypes/stream_headers.h>
#include <pcpd_shm_client/datatypes/stream_metadata.h>
#include <pcpd_shm_client/shm_reader/sensor_calibration_reader.h>
#include "utils.h"

namespace traact::component {

class ShmSensorCalibrationSource : public Component {
 public:
    using OutPortWorldToCamera = buffer::PortConfig<spatial::Pose6DHeader , 0>;
    using OutPortColorCalibration = buffer::PortConfig<vision::CameraCalibrationHeader, 1>;
    using OutPortIrCalibration = buffer::PortConfig<vision::CameraCalibrationHeader, 2>;
    using OutPortColorToIrCamera = buffer::PortConfig<spatial::Pose6DHeader , 3>;
    using OutPortIrCameraToAcc = buffer::PortConfig<spatial::Pose6DHeader , 4>;
    using OutPortIrCameraToGyro = buffer::PortConfig<spatial::Pose6DHeader , 5>;

    static pattern::Pattern::Ptr GetPattern() {
        pattern::Pattern::Ptr
            pattern =
            std::make_shared<pattern::Pattern>("artekmed::ShmSensorCalibrationSource",
                                                       Concurrency::SERIAL,
                                                       ComponentType::SYNC_SOURCE);

        pattern->addStringParameter("stream", "camera01")
        .addProducerPort<OutPortWorldToCamera>("output")
            .addProducerPort<OutPortColorCalibration>("output_color_calibration")
            .addProducerPort<OutPortIrCalibration>("output_ir_calibration")
            .addProducerPort<OutPortColorToIrCamera>("output_color_to_ir")
            .addProducerPort<OutPortIrCameraToAcc>("output_ir_to_acc")
            .addProducerPort<OutPortIrCameraToGyro>("output_ir_to_gyro");

        return
            pattern;
    }

    explicit ShmSensorCalibrationSource(std::string name) : Component(std::move(name)) {}

    virtual void configureInstance(const pattern::instance::PatternInstance &pattern_instance) override {
        shm_app_name_ = "traact_shm_" + pattern_instance.instance_id;
        pattern_instance.setValueFromParameter("stream", stream_name_);

    }

    bool start() override {

        const iox::RuntimeName_t APP_NAME(iox::cxx::TruncateToCapacity, shm_app_name_);
        iox::runtime::PoshRuntime::initRuntime(APP_NAME);
        auto calib_reader = pcpd::shm::SensorCalibrationReader(should_stop_, "traact_shm", stream_name_);
        if (!calib_reader.connect()) {
            SPDLOG_ERROR("Error connecting to shm-provider for camera calibration: {0}", stream_name_);
            return true;
        }
        auto artekmed_calibration = calib_reader.getDeviceCalibration();
        world_to_ir_camera_transform_ = Artekmed2Traact(artekmed_calibration.camera_pose);
        color_camera_calibration_ = Artekmed2Traact(artekmed_calibration.color_parameters);
        ir_camera_calibration_ = Artekmed2Traact(artekmed_calibration.depth_parameters);
        color_to_ir_transform_ = Artekmed2Traact(artekmed_calibration.color2depth_transform);
        ir_to_gyro_transform_  = Artekmed2Traact(artekmed_calibration.depth2gyro_transform);
        ir_to_acc_transform_  = Artekmed2Traact(artekmed_calibration.depth2acc_transform);

        calib_reader.disconnect();

        return true;
    }
    bool stop() override {
        should_stop_ = true;
        running_.store(false, std::memory_order_release);
        return true;
    }

    virtual bool processTimePoint(buffer::ComponentBuffer &data) override {
        data.getOutput<OutPortWorldToCamera>() = world_to_ir_camera_transform_;
        data.getOutput<OutPortColorCalibration>() = color_camera_calibration_;
        data.getOutput<OutPortIrCalibration>() = ir_camera_calibration_;

        data.getOutput<OutPortColorToIrCamera>() = color_to_ir_transform_;
        data.getOutput<OutPortIrCameraToAcc>() = ir_to_acc_transform_;
        data.getOutput<OutPortIrCameraToGyro>() = ir_to_gyro_transform_;
        return true;
    }

 private:
    std::atomic_bool running_{false};
    bool should_stop_{false};

    std::string shm_app_name_;
    std::string stream_name_;

    spatial::Pose6D world_to_ir_camera_transform_;
    vision::CameraCalibration color_camera_calibration_;
    vision::CameraCalibration ir_camera_calibration_;

    spatial::Pose6D color_to_ir_transform_;
    spatial::Pose6D ir_to_gyro_transform_;
    spatial::Pose6D ir_to_acc_transform_;







};

CREATE_TRAACT_COMPONENT_FACTORY(ShmSensorCalibrationSource)

}

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(traact::component::ShmSensorCalibrationSource)
END_TRAACT_PLUGIN_REGISTRATION
