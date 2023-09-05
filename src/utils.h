/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_COMPONENT_SHM_SRC_UTILS_H_
#define TRAACT_COMPONENT_SHM_SRC_UTILS_H_

#include <traact/vision.h>
#include <artekmed/core.capnp.h>
#include <pcpd_datatypes/rigid_transform.h>
#include <pcpd_datatypes/intrinsic_parameters.h>

namespace traact {

    static constexpr vision::PixelFormat Artekmed2Traact(artekmed::schema::PixelFormat format){
        switch (format) {
            case artekmed::schema::PixelFormat::UNKNOWN:return vision::PixelFormat::UNKNOWN_PIXELFORMAT;
            case artekmed::schema::PixelFormat::LUMINANCE:return vision::PixelFormat::LUMINANCE;
            case artekmed::schema::PixelFormat::RGB:return vision::PixelFormat::RGB;
            case artekmed::schema::PixelFormat::BGR:return vision::PixelFormat::BGR;
            case artekmed::schema::PixelFormat::RGBA:return vision::PixelFormat::RGBA;
            case artekmed::schema::PixelFormat::BGRA:return vision::PixelFormat::BGRA;
            case artekmed::schema::PixelFormat::YUV422:return vision::PixelFormat::YUV422;
            case artekmed::schema::PixelFormat::YUV411:return vision::PixelFormat::YUV411;
            case artekmed::schema::PixelFormat::RAW:return vision::PixelFormat::RAW;
            case artekmed::schema::PixelFormat::DEPTH:return vision::PixelFormat::DEPTH;
            case artekmed::schema::PixelFormat::FLOAT:return vision::PixelFormat::FLOAT;
            case artekmed::schema::PixelFormat::MJPEG:return vision::PixelFormat::MJPEG;
            case artekmed::schema::PixelFormat::ZDEPTH:return vision::PixelFormat::UNKNOWN_PIXELFORMAT;
            default:return vision::PixelFormat::UNKNOWN_PIXELFORMAT;
        }
    }

    static spatial::Pose6D Artekmed2Traact(const pcpd::datatypes::RigidTransform& transform){
        return spatial::Translation3D(transform.translation.cast<Scalar>()) * transform.rotation.cast<Scalar>();
    }
static vision::CameraCalibration Artekmed2Traact(const pcpd::datatypes::IntrinsicParameters& intrinsic){
        vision::CameraCalibration result;
        result.width = intrinsic.width;
        result.height = intrinsic.height;
        result.cx = intrinsic.c_x;
        result.cy = intrinsic.c_y;
        result.fx = intrinsic.fov_x;
        result.fy = intrinsic.fov_y;
        result.skew = 0;

    result.radial_distortion.resize(intrinsic.radial_distortion.rows());
    for (int i = 0; i < intrinsic.radial_distortion.rows(); ++i) {
        result.radial_distortion[i] = intrinsic.radial_distortion[i];
    }
    result.tangential_distortion.resize(intrinsic.tangential_distortion.rows());
    for (int i = 0; i < intrinsic.tangential_distortion.rows(); ++i) {
        result.tangential_distortion[i] = intrinsic.tangential_distortion[i];
    }

    return result;
}
}

#endif //TRAACT_COMPONENT_SHM_SRC_UTILS_H_
