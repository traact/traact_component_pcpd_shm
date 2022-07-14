/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
// based on multistream_ndi_sender example

#include <iceoryx_posh/runtime/posh_runtime.hpp>
#include <traact/traact.h>
#include <traact/vision.h>
#include <thread>
#include "SynchronizedBufferReader.h"
#include <pcpd_shm_client/shm_reader/synchronized_stream_reader.h>
#include "utils.h"

namespace traact::component {

class ShmCompositeBufferSource : public Component {
 public:
    using OutPortImage = buffer::PortConfig<traact::vision::ImageHeader, 0>;

    ~ShmCompositeBufferSource() {


    }

    static traact::pattern::Pattern::Ptr GetPattern() {
        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>("artekmed::ShmCompositeBufferSource",
                                                       Concurrency::SERIAL,
                                                       ComponentType::ASYNC_SOURCE);

        pattern->addStringParameter("stream", "camera_x5")
            .beginPortGroup("color_image")
            .addProducerPort<OutPortImage>("output")
            .addStringParameter("channel", "camera01_colorimage")
            .endPortGroup()
            .beginPortGroup("ir_image")
            .addProducerPort<OutPortImage>("output")
            .addStringParameter("channel", "camera01_infraredimage")
            .endPortGroup();

        return
            pattern;
    }

    explicit ShmCompositeBufferSource(std::string name) : Component(std::move(name)) {}

    virtual void configureInstance(const pattern::instance::PatternInstance &pattern_instance) override {
        shm_app_name_ = "traact_shm_" + pattern_instance.instance_id;

        pattern_instance.setValueFromParameter("stream", stream_name_);

        color_image_group = pattern_instance.getPortGroupInfo("color_image");
        for (int i = 0; i < color_image_group.size; ++i) {
            std::string channel;
            pattern_instance.setValueFromParameter(color_image_group.port_group_index, i, "channel", channel);
            color_channel_names.emplace(channel, i);
        }

        ir_image_group_ = pattern_instance.getPortGroupInfo("ir_image");
        for (int i = 0; i < ir_image_group_.size; ++i) {
            std::string channel;
            pattern_instance.setValueFromParameter(ir_image_group_.port_group_index, i, "channel", channel);
            ir_channel_names.emplace(channel, i);
        }


    }


    bool configure(const pattern::instance::PatternInstance &pattern_instance,
                   buffer::ComponentBufferConfig *data) override {

        return true;
    }
    bool start() override {
        SPDLOG_INFO("ShmCompositeBufferSource start");
        running_.store(true, std::memory_order_release);
        stopped_future_ = stopped_promise_.get_future();




        thread_ = std::make_unique<std::thread>([this](){
            try{
                initShm();

                if (!reader_->connect()) {
                    SPDLOG_ERROR("error connecting to shm segment: {0}", stream_name_);
                    return;
                }
                while (running_.load(std::memory_order_acquire) && reader_->isConnected()){
                    reader_->try_receive(kDefaultTimeout.count());
                }
                if (!reader_->disconnect()) {
                    SPDLOG_ERROR("error disconnecting from shm segment: {0}", stream_name_);

                }
            } catch(std::exception& exception) {
                SPDLOG_ERROR(exception.what());
            }

            stopped_promise_.set_value();
        });

        SPDLOG_DEBUG("ShmCompositeBufferSource done");
        return true;
    }
    bool stop() override {
        should_stop_ = true;
        running_.store(false, std::memory_order_release);
        while(stopped_future_.wait_for(kDataflowStopTimeout) == std::future_status::timeout){
            SPDLOG_WARN("timeout waiting for shm source to stop");
        }
        try {
            if (thread_->joinable()) {
                thread_->join();
            }
            reader_.reset();
        } catch (std::exception &e) {
            SPDLOG_ERROR(e.what());
        }
        return true;
    }
    bool teardown() override {

        return true;
    }

 private:
    std::atomic_bool running_{false};
    bool should_stop_{false};
    std::unique_ptr<std::thread> thread_;
    std::promise<void> stopped_promise_;
    std::future<void> stopped_future_;

    std::string shm_app_name_;
    iox::RuntimeName_t iox_app_name_;
    std::string stream_name_;
    std::unique_ptr<SynchronizedBufferReader> reader_;

    PortGroupInfo color_image_group;
    std::map<std::string, size_t> color_channel_names;
    PortGroupInfo ir_image_group_;
    std::map<std::string, size_t> ir_channel_names;

    bool configure_stream_receiver(const std::map<std::string, pcpd::shm::StreamMetaData> &md){
        SPDLOG_DEBUG("ShmCompositeBufferSource config start");

        for (const auto& [name, stream_data]: md) {
            SPDLOG_INFO("got stream {0}", name);
        }

        for (const auto& name_index : color_channel_names) {
            auto is_present = md.find(name_index.first);
            if(is_present == md.end()){
                SPDLOG_ERROR("channel {0} is missing", name_index.first);
                return false;
            }
        }
        for (const auto& name_index : ir_channel_names) {
            auto is_present = md.find(name_index.first);
            if(is_present == md.end()){
                SPDLOG_ERROR("channel {0} is missing", name_index.first);
                return false;
            }
        }
        SPDLOG_DEBUG("ShmCompositeBufferSource config done");
        return true;
    };

    void initShm() {
        auto configure = [this](auto md) {
            return configure_stream_receiver(md);
        };
        auto handle = [this](auto timestamp, auto reader){
            handle_raw_frame(timestamp, reader);
        };
        SPDLOG_DEBUG("ShmCompositeBufferSource init shm runtime");
        iox_app_name_ = iox::RuntimeName_t(iox::cxx::TruncateToCapacity, shm_app_name_);
        iox::runtime::PoshRuntime::initRuntime(iox_app_name_);
        reader_ = std::make_unique<SynchronizedBufferReader>(should_stop_, "traact_shm", stream_name_, configure, handle);
    }

    bool receivePort(const std::string &port_name) {
        auto is_color = color_channel_names.find(port_name);
        if(is_color != color_channel_names.end()){
            return true;
        }
        auto is_ir = ir_channel_names.find(port_name);
        if(is_ir != ir_channel_names.end()){
            return true;
        }
        return false;
    }

    void handle_raw_frame(const uint64_t timestamp,
                          const artekmed::network::ShmBufferDescriptor::Reader reader){

        auto numports = reader.getNumPorts();
        auto ports = reader.getPorts();
        auto global_timestamp = reader.getTimestamp();
        auto traact_timestamp = AsTimestamp(global_timestamp);
        auto buffer_future = request_callback_(traact_timestamp);
        buffer_future.wait();
        auto buffer = buffer_future.get();
        int last_successful_port{-1};
        int current_port{-1};
        try{

            if (!buffer) {
                SPDLOG_DEBUG("Could not get source buffer for ts {0}", global_timestamp);
                return;
            }


            for (int i = 0; i < numports; ++i) {
                current_port = i;
                auto port = ports[i];
                std::string port_name = port.getName().cStr();

                if(receivePort(port_name)) {
                    auto port_data = port.getData();
                    auto meta_data = port_data.getMetadata();
                    auto hdr = meta_data.getHeader();
                    auto data = static_cast<const void *>(port_data.getData().begin());
                    if (hdr.isImage()) {
                        // call handler
                        handle_image(port_name, AsTimestamp(timestamp), hdr, const_cast<void *>(data), buffer);
                    }
                }


                last_successful_port = i;
            }
        } catch(std::exception& error){
            SPDLOG_ERROR("current port: {2} last successful shm port index: {0} of {1}", last_successful_port, numports, current_port);
            SPDLOG_ERROR(error.what());
        }

        if (buffer) {
            buffer->commit(true);
        }



    };

    void handle_image(const std::string& port_name, Timestamp timestamp, const artekmed::schema::StreamHeader::Reader& hdr, void* data, buffer::SourceComponentBuffer* source_buffer ){

        auto pixel_format = hdr.getImage();
        // wrap buffer in cv::Mat
        int dtype;
        if (pixel_format == artekmed::schema::PixelFormat::DEPTH) {
            switch (hdr.getBitsPerElement()) {
                case 16:
                    dtype = CV_16UC1;
                    break;
                case 32:
                default:
                    spdlog::error("{0}: DatatypeConversion[ImageHeader-cv::Mat]: incompatible depth format",
                                  port_name);
                    return;
            }
        } else if (pixel_format == artekmed::schema::PixelFormat::RGBA) {
            dtype = CV_8UC4;
        } else if (pixel_format == artekmed::schema::PixelFormat::BGRA) {
            dtype = CV_8UC4;
        } else if (pixel_format == artekmed::schema::PixelFormat::RGB) {
            dtype = CV_8UC3;
        } else if (pixel_format == artekmed::schema::PixelFormat::BGR) {
            dtype = CV_8UC3;
        } else if (pixel_format == artekmed::schema::PixelFormat::LUMINANCE) {
            switch (hdr.getBitsPerElement()) {
                case 16:
                    dtype = CV_16UC1;
                    break;
                case 8:
                    dtype = CV_8UC1;
                    break;
                default:
                    SPDLOG_ERROR("{0}: DatatypeConversion[ImageHeader-cv::Mat]: incompatible depth format",
                                  port_name);
                    return;
            }
        }
        else {
            SPDLOG_ERROR("{0}: DatatypeConversion[ImageHeader-cv::Mat]: incompatible pixel format",
                          port_name);
            return;
        }

        cv::Mat image(cv::Size(hdr.getDimX(), hdr.getDimY()), dtype, data,
                       cv::Mat::AUTO_STEP);


        auto is_color = color_channel_names.find(port_name);
        if(is_color != color_channel_names.end()){
            auto& output_image = source_buffer->getOutput<OutPortImage>(color_image_group.port_group_index, is_color->second);
            image.copyTo(output_image.value());
            auto& output_header = source_buffer->getOutputHeader<OutPortImage>(color_image_group.port_group_index, is_color->second);
            output_header.setFrom(image);
        }

        auto is_ir = ir_channel_names.find(port_name);
        if(is_ir != ir_channel_names.end()){
            auto& output_image = source_buffer->getOutput<OutPortImage>(ir_image_group_.port_group_index, is_ir->second);
            image.copyTo(output_image.value());
            auto& output_header = source_buffer->getOutputHeader<OutPortImage>(ir_image_group_.port_group_index, is_ir->second);
            output_header.setFrom(image);
        }

    }

};

CREATE_TRAACT_COMPONENT_FACTORY(ShmCompositeBufferSource)

}

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(traact::component::ShmCompositeBufferSource)
END_TRAACT_PLUGIN_REGISTRATION
