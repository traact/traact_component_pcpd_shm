/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_COMPONENT_SHM_SRC_SYNCHRONIZEDBUFFERREADER_H_
#define TRAACT_COMPONENT_SHM_SRC_SYNCHRONIZEDBUFFERREADER_H_

#include <pcpd_shm_client/shm_reader/synchronized_stream_reader.h>

namespace traact {

class SynchronizedBufferReader : public pcpd::shm::BaseSynchronizedBufferReader {
 public:
    using ConfigureCallback = std::function<bool(const std::map<std::string, pcpd::shm::StreamMetaData>)>;
    using HandleCallback = std::function<void(const uint64_t,
                                              const artekmed::network::ShmBufferDescriptor::Reader)>;
    SynchronizedBufferReader(bool &should_stop, const std::string &node_name, const std::string &service_name, ConfigureCallback configure_callback,
                             HandleCallback handle_callback);
    virtual ~SynchronizedBufferReader() = default;
 protected:
    bool configure_stream_receiver(const std::map<std::string, pcpd::shm::StreamMetaData> &md) override;
    void handle_raw_frame(const uint64_t timestamp,
                                  const artekmed::network::ShmBufferDescriptor::Reader reader) override;

    ConfigureCallback configure_callback_;
    HandleCallback handle_callback_;

};

} // traact

#endif //TRAACT_COMPONENT_SHM_SRC_SYNCHRONIZEDBUFFERREADER_H_
