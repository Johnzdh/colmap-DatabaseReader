#ifndef SRC_TYPES_UTILS_
#define SRC_TYPES_UTILS_

#include <cstdint>
#include <Eigen/Core>
#include <opencv2/core/eigen.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

// Unique identifier for cameras.
typedef uint32_t camera_t;

// Unique identifier for images.
typedef uint32_t image_t;

// Each image pair gets a unique ID, see `Database::ImagePairToPairId`.
typedef uint64_t image_pair_t;

// Index per image, i.e. determines maximum number of 2D points per image.
typedef uint32_t point2D_t;

// Unique identifier per added 3D point. Since we add many 3D points,
// delete them, and possibly re-add them again, the maximum number of allowed
// unique indices should be large.
typedef uint64_t point3D_t;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> FeatureKeypointsBlob;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> FeatureDescriptorsBlob;


namespace cv{
    typedef cv::Mat cvDes;
    typedef std::vector<cv::KeyPoint> cvKps;
}

#endif //SRC_TYPES_UTILS_