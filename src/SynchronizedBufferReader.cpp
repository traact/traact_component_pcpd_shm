/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "SynchronizedBufferReader.h"

namespace traact {
SynchronizedBufferReader::SynchronizedBufferReader(bool &should_stop,
                                                   const std::string &node_name,
                                                   const std::string &service_name,
                                                   ConfigureCallback configure_callback,
                                                   HandleCallback handle_callback) : BaseSynchronizedBufferReader(
    should_stop,
    node_name,
    service_name), configure_callback_(std::move(configure_callback)), handle_callback_(handle_callback) {}
bool SynchronizedBufferReader::configure_stream_receiver(const std::map<std::string, pcpd::shm::StreamMetaData> &md) {
    if(configure_callback_){
        return configure_callback_(md);
    } else {
        return BaseSynchronizedBufferReader::configure_stream_receiver(md);
    }

}
void SynchronizedBufferReader::handle_raw_frame(const uint64_t timestamp,
                                                const artekmed::network::ShmBufferDescriptor::Reader reader) {
    if(handle_callback_){
        handle_callback_(timestamp, reader);
    }
}
} // traact