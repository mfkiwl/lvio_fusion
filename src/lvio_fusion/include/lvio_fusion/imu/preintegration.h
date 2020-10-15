#ifndef lvio_fusion_PREINTEGRATION_H
#define lvio_fusion_PREINTEGRATION_H

#include "lvio_fusion/common.h"
#include "lvio_fusion/imu/imu.hpp"

namespace lvio_fusion
{

class Frame;

namespace imu
{

extern int O_T, O_R, O_V, O_BA, O_BG, O_PR, O_PT;
extern Vector3d g;

class Preintegration
{
public:
    typedef std::shared_ptr<Preintegration> Ptr;

    static Preintegration::Ptr Create(const Vector3d &_acc_0, const Vector3d &_gyr_0, const Vector3d &_v0, const Vector3d &_linearized_ba, const Vector3d &_linearized_bg, const Imu::Ptr imu)
    {
        Preintegration::Ptr new_preintegration(new Preintegration(_acc_0, _gyr_0, _v0, _linearized_ba, _linearized_bg, imu));
        return new_preintegration;
    }

/*    void Append(double dt, const Vector3d &acc, const Vector3d &gyr)
    {
        dt_buf.push_back(dt);
        acc_buf.push_back(acc);
        gyr_buf.push_back(gyr);
        Propagate(dt, acc, gyr);
    }*/
    void Appendimu(imuPoint imuMeasure)
    {
        imuData_buf.push_back(imuMeasure);
    }

    void PreintegrateIMU(double last_frame_time,double current_frame_time);
   void IntegrateNewMeasurement(const Vector3d &acceleration, const Vector3d  &angVel, const float &dt);


  /*  void Repropagate(const Vector3d &_linearized_ba, const Vector3d &_linearized_bg);

    void MidPointIntegration(double _dt,
                             const Vector3d &_acc_0, const Vector3d &_gyr_0,
                             const Vector3d &_acc_1, const Vector3d &_gyr_1,
                             const Vector3d &delta_p, const Quaterniond &delta_q, const Vector3d &delta_v,
                             const Vector3d &linearized_ba, const Vector3d &linearized_bg,
                             Vector3d &result_delta_p, Quaterniond &result_delta_q, Vector3d &result_delta_v,
                             Vector3d &result_linearized_ba, Vector3d &result_linearized_bg, bool update_jacobian);

    void Propagate(double _dt, const Vector3d &_acc_1, const Vector3d &_gyr_1);

    Matrix<double, 15, 1> Evaluate(const Vector3d &Pi, const Quaterniond &Qi, const Vector3d &Vi, const Vector3d &Bai, const Vector3d &Bgi,
                                   const Vector3d &Pj, const Quaterniond &Qj, const Vector3d &Vj, const Vector3d &Baj, const Vector3d &Bgj);
*/
    double dt;
    Vector3d acc0, gyr0;
    Vector3d acc1, gyr1;
    Vector3d linearized_acc, linearized_gyr;
    Vector3d linearized_ba, linearized_bg;
    Matrix<double, 15, 15> jacobian, covariance;
    Matrix<double, 15, 15> step_jacobian;
    Matrix<double, 15, 18> step_V;
    Matrix<double, 18, 18> noise;
    double sum_dt;
    Vector3d delta_p;
    Quaterniond delta_q;
    Vector3d delta_v;
    Vector3d v0;

   std::vector<imuPoint> imuData_buf;

    std::vector<double> dt_buf;
    std::vector<Vector3d> acc_buf;
    std::vector<Vector3d> gyr_buf;

private:
    Preintegration() = default;

    Preintegration(const Vector3d &_acc_0, const Vector3d &_gyr_0, const Vector3d &_v0,
                   const Vector3d &_linearized_ba, const Vector3d &_linearized_bg, const Imu::Ptr imu)
        : acc0{_acc_0}, gyr0{_gyr_0}, v0(_v0), linearized_acc{_acc_0}, linearized_gyr{_gyr_0},
          linearized_ba{_linearized_ba}, linearized_bg{_linearized_bg},
          jacobian{Matrix<double, 15, 15>::Identity()}, covariance{Matrix<double, 15, 15>::Zero()},
          sum_dt{0.0}, delta_p{Vector3d::Zero()}, delta_q{Quaterniond::Identity()}, delta_v{Vector3d::Zero()}
    {
        noise = Matrix<double, 18, 18>::Zero();
        noise.block<3, 3>(0, 0) = (imu->ACC_N * imu->ACC_N) * Matrix3d::Identity();
        noise.block<3, 3>(3, 3) = (imu->GYR_N * imu->GYR_N) * Matrix3d::Identity();
        noise.block<3, 3>(6, 6) = (imu->ACC_N * imu->ACC_N) * Matrix3d::Identity();
        noise.block<3, 3>(9, 9) = (imu->GYR_N * imu->GYR_N) * Matrix3d::Identity();
        noise.block<3, 3>(12, 12) = (imu->ACC_W * imu->ACC_W) * Matrix3d::Identity();
        noise.block<3, 3>(15, 15) = (imu->GYR_W * imu->GYR_W) * Matrix3d::Identity();
    }

    struct integrable
    {
        integrable(const Vector3d &a_, const Vector3d &w_ , const float &t_):a(a_),w(w_),t(t_){}
        Vector3d a;
        Vector3d w;
        float t;
    };

    std::vector<integrable> mvMeasurements;


    float dT;
    cv::Mat C;   //cov
    cv::Mat Info;
    cv::Mat Nga, NgaWalk;

    // Values for the original bias (when integration was computed)
    Bias b;
    cv::Mat dR, dV, dP;
    cv::Mat JRg, JVg, JVa, JPg, JPa; 
    cv::Mat avgA;
    cv::Mat avgW;


};

typedef std::map<double, Preintegration::Ptr> PreIntegrations;
} // namespace imu

} // namespace lvio_fusion

#endif // lvio_fusion_PREINTEGRATION_H