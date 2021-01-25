#ifndef lvio_fusion_LOOP_DETECTOR_H
#define lvio_fusion_LOOP_DETECTOR_H

#include "lvio_fusion/adapt/problem.h"
#include "lvio_fusion/backend.h"
#include "lvio_fusion/common.h"
#include "lvio_fusion/frame.h"
#include "lvio_fusion/frontend.h"
#include "lvio_fusion/lidar/association.h"
#include "lvio_fusion/lidar/mapping.h"
#include "lvio_fusion/loop/loop.h"
#include "lvio_fusion/loop/pose_graph.h"
#include "lvio_fusion/visual/matcher.h"

namespace lvio_fusion
{

class Relocator
{
public:
    typedef std::shared_ptr<Relocator> Ptr;

    Relocator(int mode);

    void SetMapping(Mapping::Ptr mapping) { mapping_ = mapping; }

    void SetBackend(Backend::Ptr backend) { backend_ = backend; }

private:
    enum Mode
    {
        None = 0,
        Visual = 1,
        Lidar = 2,
        VisualAndLidar = 3
    };

    void DetectorLoop();

    bool DetectLoop(Frame::Ptr frame, Frame::Ptr &old_frame);

    bool Relocate(Frame::Ptr frame, Frame::Ptr old_frame);

    bool RelocateByImage(Frame::Ptr frame, Frame::Ptr old_frame);

    bool RelocateByPoints(Frame::Ptr frame, Frame::Ptr old_frame);

    void BuildProblem(Frames &active_kfs, adapt::Problem &problem);

    void BuildProblemWithRelocated(Frames &active_kfs, adapt::Problem &problem);

    void CorrectLoop(double old_time, double start_time, double end_time);

    Mapping::Ptr mapping_;
    Backend::Ptr backend_;
    ORBMatcher matcher_;

    std::thread thread_;
    Mode mode_;
};

} // namespace lvio_fusion

#endif // lvio_fusion_LOOP_DETECTOR_H