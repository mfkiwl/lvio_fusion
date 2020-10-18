#ifndef lvio_fusion_VISUAL_ERROR_H
#define lvio_fusion_VISUAL_ERROR_H

#include "lvio_fusion/ceres/base.hpp"
#include "lvio_fusion/visual/camera.hpp"

namespace lvio_fusion
{

template <typename T>
inline void Reprojection(const T *pw, const T *Tcw, Camera::Ptr camera, T *result)
{
    T pc[3], pc_[3];
    ceres::SE3TransformPoint(Tcw, pw, pc_);
    T extrinsic[7];
    ceres::Cast(camera->extrinsic.data(), SE3d::num_parameters, extrinsic);
    ceres::SE3TransformPoint(extrinsic, pc_, pc);
    T xp = pc[0] / pc[2];
    T yp = pc[1] / pc[2];
    result[0] = camera->fx * xp + camera->cx;
    result[1] = camera->fy * yp + camera->cy;
}

class PoseOnlyReprojectionError
{
public:
    PoseOnlyReprojectionError(Vector2d ob, Vector3d pw, Camera::Ptr camera, double weight = 1.0)
        : ob_(ob), pw_(pw), camera_(camera) {}

    template <typename T>
    bool operator()(const T *Tcw, T *residuals) const
    {
        T p_p[2];
        T pw[3] = {T(pw_.x()), T(pw_.y()), T(pw_.z())};
        T ob[2] = {T(ob_.x()), T(ob_.y())};
        Reprojection(pw, Tcw, camera_, p_p);
        residuals[0] = T(weight_) * T(sqrt_info(0, 0)) * (p_p[0] - ob[0]);
        residuals[1] = T(weight_) * T(sqrt_info(1, 1)) * (p_p[1] - ob[1]);
        return true;
    }

    static ceres::CostFunction *Create(Vector2d ob, Vector3d pw, Camera::Ptr camera, double weight = 1.0)
    {
        return (new ceres::AutoDiffCostFunction<PoseOnlyReprojectionError, 2, 7>(
            new PoseOnlyReprojectionError(ob, pw, camera, weight)));
    }

    static Matrix2d sqrt_info;

private:
    Vector2d ob_;
    Vector3d pw_;
    Camera::Ptr camera_;
    double weight_;
};

template <typename T>
inline void Projection(const T *pc, const T *Tcw, T *result)
{
    T Tcw_inverse[7];
    ceres::SE3Inverse(Tcw, Tcw_inverse);
    ceres::SE3TransformPoint(Tcw_inverse, pc, result);
}

class TwoFrameReprojectionError
{
public:
    TwoFrameReprojectionError(Vector3d pr, Vector2d ob, Camera::Ptr camera, double weight = 1.0)
        : pr_(pr), ob_(ob), camera_(camera) {}

    template <typename T>
    bool operator()(const T *Tcw1, const T *Tcw2, T *residuals) const
    {
        T p_p[2], pw[3];
        T pr[3] = {T(pr_.x()), T(pr_.y()), T(pr_.z())};
        T ob2[2] = {T(ob_.x()), T(ob_.y())};
        Projection(pr, Tcw1, pw);
        Reprojection(pw, Tcw2, camera_, p_p);
        residuals[0] = T(weight_) * T(sqrt_info(0, 0)) * (p_p[0] - ob2[0]);
        residuals[1] = T(weight_) * T(sqrt_info(1, 1)) * (p_p[1] - ob2[1]);
        return true;
    }

    static ceres::CostFunction *Create(Vector3d pr, Vector2d ob, Camera::Ptr camera, double weight = 1.0)
    {
        return (new ceres::AutoDiffCostFunction<TwoFrameReprojectionError, 2, 7, 7>(
            new TwoFrameReprojectionError(pr, ob, camera, weight)));
    }

    static Matrix2d sqrt_info;

private:
    Vector3d pr_;
    Vector2d ob_;
    Camera::Ptr camera_;
    double weight_;
};

} // namespace lvio_fusion

#endif // lvio_fusion_VISUAL_ERROR_H
