#include "visualization.h"
#include <pcl_conversions/pcl_conversions.h>

using namespace Eigen;

ros::Publisher path_pub;
ros::Publisher navsat_pub;
ros::Publisher points_cloud_pub;
ros::Publisher points_cloud_pub1;
nav_msgs::Path path, navsat_path;

void register_pub(ros::NodeHandle &n)
{
    path_pub = n.advertise<nav_msgs::Path>("path", 1000);
    navsat_pub = n.advertise<nav_msgs::Path>("navsat_path", 1000);
    points_cloud_pub = n.advertise<sensor_msgs::PointCloud2>("point_cloud", 1000);
    points_cloud_pub1 = n.advertise<sensor_msgs::PointCloud2>("point_cloud1", 1000);
}

void pub_odometry(Estimator::Ptr estimator, double time)
{
    if (estimator->frontend->status == FrontendStatus::TRACKING_GOOD)
    {
        path.poses.clear();
        for (auto frame : estimator->map->GetAllKeyFrames())
        {
            auto position = frame.second->pose.inverse().translation();
            geometry_msgs::PoseStamped pose_stamped;
            pose_stamped.header.stamp = ros::Time(frame.first);
            pose_stamped.header.frame_id = "world";
            pose_stamped.pose.position.x = position.x();
            pose_stamped.pose.position.y = position.y();
            pose_stamped.pose.position.z = position.z();
            path.poses.push_back(pose_stamped);
        }
        path.header.stamp = ros::Time(time);
        path.header.frame_id = "world";
        path_pub.publish(path);
    }
    sensor_msgs::PointCloud2 ros_cloud;
    PointCloudRGB pcl_cloud;
    for (auto frame : estimator->map->GetAllKeyFrames())
    {
        auto position = frame.second->pose.inverse().translation();
        PointRGB p;
        p.x = position.x();
        p.y = position.y();
        p.z = position.z();
        p.rgba = 0x00FF00FF;
        pcl_cloud.push_back(p);
    }
    pcl::toROSMsg(pcl_cloud, ros_cloud);
    ros_cloud.header.stamp = ros::Time(time);
    ros_cloud.header.frame_id = "world";
    points_cloud_pub1.publish(ros_cloud);
}

void pub_navsat(Estimator::Ptr estimator, double time)
{
    if (estimator->map->navsat_map->initialized)
    {
        if (navsat_path.poses.size() == 0)
        {
            for (auto pair : estimator->map->navsat_map->GetAllPoints())
            {
                NavsatPoint point = pair.second;
                geometry_msgs::PoseStamped pose_stamped;
                pose_stamped.header.stamp = ros::Time(point.time);
                pose_stamped.header.frame_id = "navsat";
                pose_stamped.pose.position.x = point.position.x();
                pose_stamped.pose.position.y = point.position.y();
                pose_stamped.pose.position.z = point.position.z();
                navsat_path.poses.push_back(pose_stamped);
            }
        }
        else
        {
            geometry_msgs::PoseStamped pose_stamped;
            NavsatPoint point = (--estimator->map->navsat_map->GetAllPoints().end())->second;
            pose_stamped.pose.position.x = point.position.x();
            pose_stamped.pose.position.y = point.position.y();
            pose_stamped.pose.position.z = point.position.z();
            navsat_path.poses.push_back(pose_stamped);
        }
        navsat_path.header.stamp = ros::Time(time);
        navsat_path.header.frame_id = "navsat";
        navsat_pub.publish(navsat_path);
    }
}

void pub_tf(Estimator::Ptr estimator, double time)
{
    static tf::TransformBroadcaster br;
    tf::Transform transform;
    tf::Quaternion tf_q;
    tf::Vector3 tf_t;
    // base_link
    if (estimator->frontend->status == FrontendStatus::TRACKING_GOOD)
    {
        SE3d pose = estimator->frontend->current_frame->pose;
        Quaterniond pose_q = pose.unit_quaternion();
        Vector3d pose_t = pose.translation();
        tf_q.setValue(pose_q.w(), pose_q.x(), pose_q.y(), pose_q.z());
        tf_t.setValue(pose_t.x(), pose_t.y(), pose_t.z());
        transform.setOrigin(tf_t);
        transform.setRotation(tf_q);
        br.sendTransform(tf::StampedTransform(transform, ros::Time(time), "world", "base_link"));
    }
    // navsat
    if (estimator->map->navsat_map != nullptr && estimator->map->navsat_map->initialized)
    {
        double *R = estimator->map->navsat_map->R;
        double *t = estimator->map->navsat_map->t;
        tf_q.setValue(R[1], R[2], R[3], R[0]);
        tf_t.setValue(t[0], t[1], t[2]);
        transform.setOrigin(tf_t);
        transform.setRotation(tf_q);
        br.sendTransform(tf::StampedTransform(transform, ros::Time(time), "world", "navsat"));
    }
}

void pub_point_cloud(Estimator::Ptr estimator, double time)
{
    sensor_msgs::PointCloud2 ros_cloud;
    PointCloudRGB pcl_cloud;
    auto landmarks = estimator->map->GetAllMapPoints();
    for (auto mappoint : landmarks)
    {
        PointRGB p;
        Vector3d pos = mappoint.second->position;
        p.x = pos.x();
        p.y = pos.y();
        p.z = pos.z();
        //NOTE: semantic map
        LabelType label = mappoint.second->label;
        switch (label)
        {
        case LabelType::Car:
            p.rgba = 0xFF0000FF;
            break;
        case LabelType::Person:
            p.rgba = 0x0000FFFF;
            break;
        case LabelType::Truck:
            p.rgba = 0xFF0000FF;
            break;
        default:
            p.rgba = 0x00FF00FF;
        }
        pcl_cloud.push_back(p);
    }
    pcl::toROSMsg(pcl_cloud, ros_cloud);
    ros_cloud.header.stamp = ros::Time(time);
    ros_cloud.header.frame_id = "world";
    points_cloud_pub.publish(ros_cloud);
}
