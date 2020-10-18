#ifndef MAP_H
#define MAP_H

#include "lvio_fusion/common.h"
#include "lvio_fusion/frame.h"
#include "lvio_fusion/navsat/navsat.h"
#include "lvio_fusion/visual/landmark.h"

namespace lvio_fusion
{

class Map
{
public:
    typedef std::shared_ptr<Map> Ptr;

    Map() {}

    visual::Landmarks &GetAllLandmarks()
    {
        std::unique_lock<std::mutex> lock(mutex_data_);
        return landmarks_;
    }

    Frames &GetAllKeyFrames()
    {
        std::unique_lock<std::mutex> lock(mutex_data_);
        return keyframes_;
    }

    Frames GetKeyFrames(double start, double end = 0, int num = 0);

    void InsertKeyFrame(Frame::Ptr frame);

    void InsertLandmark(visual::Landmark::Ptr landmark);

    void RemoveLandmark(visual::Landmark::Ptr landmark);

    SE3d ComputePose(double time);

    void Reset()
    {
        landmarks_.clear();
        keyframes_.clear();
    }

    NavsatMap::Ptr navsat_map;
    double time_local_map = 0;
    double time_mapping = 0;

private:
    std::mutex mutex_data_;
    visual::Landmarks landmarks_;
    Frames keyframes_;
};
} // namespace lvio_fusion

#endif // MAP_H
