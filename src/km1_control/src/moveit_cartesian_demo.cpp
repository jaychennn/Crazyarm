/***********************************************************************
Copyright 2019 Wuhan PS-Micro Technology Co., Itd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***********************************************************************/

#include <ros/ros.h>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/robot_trajectory/robot_trajectory.h>
#include <moveit/trajectory_processing/iterative_time_parameterization.h>
#include <cmath>
#define PI 3.14159265358979323846
int main(int argc, char **argv)
{
	ros::init(argc, argv, "moveit_cartesian_demo");
	ros::AsyncSpinner spinner(1);
	spinner.start();

    moveit::planning_interface::MoveGroupInterface arm("manipulator");

    //获取终端link的名称
    std::string end_effector_link = arm.getEndEffectorLink();

    //设置目标位置所使用的参考坐标系
    std::string reference_frame = "base_link";
    arm.setPoseReferenceFrame(reference_frame);

    //当运动规划失败后，允许重新规划
    arm.allowReplanning(true);

    //设置位置(单位：米)和姿态（单位：弧度）的允许误差
    arm.setGoalPositionTolerance(0.001);
    arm.setGoalOrientationTolerance(0.01);

    //设置允许的最大速度和加速度
    arm.setMaxAccelerationScalingFactor(0.2);
    arm.setMaxVelocityScalingFactor(0.2);

    // 控制机械臂先回到初始化位置
    arm.setNamedTarget("prepared");
    arm.move();
    sleep(1);

    // 获取当前位姿数据最为机械臂运动的起始位姿
    geometry_msgs::Pose start_pose = arm.getCurrentPose(end_effector_link).pose;

	std::vector<geometry_msgs::Pose> waypoints;

    //将初始位姿加入路点列表
	// waypoints.push_back(start_pose); //加了反而会报错ABORTED
    const int num_points = 36;
    double radius = 0.05;
    // double step = M_PI
    double stepsize = PI/num_points;
    start_pose.position.y -= radius;
    waypoints.push_back(start_pose);
	for (int i = 2; i < num_points; i++){
        // start_pose.position.x += radius * (sin(10*i/180*M_PI)-sin(10*(i-1)/180*M_PI));
        // start_pose.position.y -= radius * (cos(10*i/180*M_PI)-cos(10*(i-1)/180*M_PI));
        start_pose.position.x += radius * (sin(i*stepsize)-sin((i-1)*stepsize));
        start_pose.position.y -= radius * (cos(i*stepsize)-cos((i-1)*stepsize));
        waypoints.push_back(start_pose);
    }
    // start_pose.position.x += 0.05;
    // // start_pose.position.z -= 0.05;
	// waypoints.push_back(start_pose);
    // geometry_msgs::Pose start_pose = arm.getCurrentPose(end_effector_link).pose;

    // std::vector<geometry_msgs::Pose> waypoints;

    // 计算圆上的点
    // const double radius = 0.02;
    // const int num_points = 100; // 圆上的点数
    // geometry_msgs::Pose pose = start_pose;
    // for (int i = 1; i < num_points; ++i)
    // {
    //     double angle = 2 * M_PI * i / num_points; // 角度
    //     pose.position.x += radius * cos(angle);
    //     pose.position.z += radius * sin(angle);
    //     waypoints.push_back(pose);
    // }
    // start_pose.position.x += radius * cos(30/180*M_PI);
    // start_pose.position.z += radius * sin(90/180*M_PI);
    // waypoints.push_back(start_pose);
    // start_pose.position.z -= 0.05;
	// waypoints.push_back(start_pose);
    
    // start_pose.position.x -= 0.01;
    // waypoints.push_back(start_pose);
    // start_pose.position.y += 0.1;
	// waypoints.push_back(start_pose);

	// 笛卡尔空间下的路径规划
	moveit_msgs::RobotTrajectory trajectory;
	// const double jump_threshold = 0.0;
	const double eef_step = 0.01;
	double fraction = 0.0;
    int maxtries = 100;   //最大尝试规划次数
    int attempts = 0;     //已经尝试规划次数

    while(fraction < 1.0 && attempts < maxtries)
    {
        fraction = arm.computeCartesianPath(waypoints, eef_step, trajectory);
        attempts++;
        
        if(attempts % 10 == 0)
            ROS_INFO("Still trying after %d attempts...", attempts);
    }
    
    if(fraction == 1)
    {   
        ROS_INFO("Path computed successfully. Moving the arm.");

	    // 生成机械臂的运动规划数据
	    moveit::planning_interface::MoveGroupInterface::Plan plan;
	    plan.trajectory_ = trajectory;
    
	    // 执行运动
	    arm.execute(plan);
        sleep(1);
    }
    else
    {
        ROS_INFO("Path planning failed with only %0.6f success after %d attempts.", fraction, maxtries);
    }

    // 控制机械臂先回到初始化位置
    arm.setNamedTarget("prepared");
    arm.move();
    sleep(1);

	ros::shutdown(); 
	return 0;
}
