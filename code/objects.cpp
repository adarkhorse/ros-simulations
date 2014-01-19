/******************************************************
*    objects.cpp                                      *
*    Purpose: Object simulation code for ROS and Rviz *
*                                                     *
*    @author Nishant Sharma                           *
*    @version 1.0 19/01/14                            *
******************************************************/

#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <task_part_sim/simCom.h>
#include <task_part_sim/simDet.h>
#include <task_part_sim/robObj.h>
#include <task_part_sim/objLocate.h>
using namespace std;

  visualization_msgs::Marker nest, points;

    ros::Publisher objLocate;// = n.advertise<task_part_sim::objLocate>("objectLocation", 10);
    task_part_sim::objLocate objLocation;

int startFlag=0,totRob,X,Y,statObj;

struct objectList{
int id;
float x;
float y;
}tempObj;

vector<objectList> objLoc;

void node_init()
{
    nest.header.frame_id = "/simulation";
    nest.header.stamp = ros::Time::now();
    nest.ns = "ObjectAndNest";
    nest.action = visualization_msgs::Marker::ADD;
    nest.pose.orientation.w = 1.0;
    nest.id = 0;
    nest.type = visualization_msgs::Marker::CUBE;
    nest.scale.z = 0.05;
    nest.color.g = 1.0f;
    nest.color.r = 1.0f;
    nest.color.b = 1.0f;
    nest.color.a = 1.0;
    nest.lifetime = ros::Duration();
    nest.pose.position.z = 0;

    points.header.frame_id = "/simulation";
    points.header.stamp = ros::Time::now();
    points.ns = "ObjectAndNest";
    points.action = visualization_msgs::Marker::ADD;
    points.pose.orientation.w = 1.0;
    points.id = 1;
    points.type = visualization_msgs::Marker::POINTS;
    // POINTS markers use x and y scale for width/height respectively
    points.scale.x = 0.5;
    points.scale.y = 0.5;
    // Points are green
    points.color.g = 1.0f;
    points.color.a = 1.0;

}

void simulationDetails(const task_part_sim::simDet::ConstPtr& msg)
{
    X=msg->goalX;
    X+=(msg->numObjects/2)*(-4);
    Y=msg->goalY+msg->source;
    totRob=msg->numRobots;
    int i;
    for(i=0;i<msg->numObjects;i++)
    {
        tempObj.id=i;
        tempObj.x=X;
        tempObj.y=Y;
        objLoc.push_back(tempObj);
        X+=4;
    }

    nest.scale.x = msg->goalW;
    nest.scale.y = msg->goalL;

    nest.pose.position.x = msg->goalX;
    nest.pose.position.y = msg->goalY;
    cout<<"Details Received";
}

void simulationCommand(const task_part_sim::simCom::ConstPtr& msg)
{

    if(msg->command==0)
    {
        exit(1);
    }
    else
    {
        startFlag=1;
        cout<<"Node Started";
    }
}

void robObject(const task_part_sim::robObj::ConstPtr& msg)
{
    if(msg->flag==-1)
    {
        objLocation.id = msg->id;
        objLocation.X = INT_MAX;
        objLocation.Y = INT_MAX;
        objLocation.flag = msg->flag;
        objLocate.publish(objLocation);
        objLoc.erase(objLoc.begin()+msg->id);
        cout<<"Object Deleted"<<msg->id<<" \n";
    }
    if(msg->flag==1)
    {
        tempObj.x = objLocation.X = msg->X;
        tempObj.y = objLocation.Y = msg->Y;
        objLocation.flag = msg->flag;
        objLoc.push_back(tempObj);
        objLocation.id = objLoc.back().id=objLoc.size()-1;
        objLocate.publish(objLocation);
        cout<<"Object Added"<<objLocation.id<<" \n";
    }
    if(msg->flag==0)
    {
        if(msg->id>=statObj)
        {
            objLoc[msg->id].x = msg->X;
            objLoc[msg->id].y = msg->Y;
            cout<<"Object Updated"<<msg->id<<" \n";
        }
    }
}

int main( int argc, char** argv )
{
    ros::init(argc, argv, "objects");
          ros::NodeHandle n;
    ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 10);
    objLocate = n.advertise<task_part_sim::objLocate>("objectLocation", 10);
    node_init();
    int i=0;

    geometry_msgs::Point p;
    ros::Subscriber simDet = n.subscribe("simulationDetails", 5000, simulationDetails);
    ros::Subscriber simCom = n.subscribe("simulationCommand", 10, simulationCommand);
    ros::Subscriber robObj = n.subscribe("robObject", 1000, robObject);
    while(startFlag==0) ros::spinOnce();
    while (ros::ok())
    {
        ros::spinOnce();
        points.points.clear();
        for(i=0;i<objLoc.size();i++)
        {
            objLocation.id = objLoc[i].id = i;
            p.x = objLocation.X=objLoc[i].x;
            p.y = objLocation.Y=objLoc[i].y;
            objLocation.flag=0;
            objLocate.publish(objLocation);
            //cout<<"Object" << i << "Published \n";
            p.z=0;
            points.points.push_back(p);

        }
        marker_pub.publish(points);
        marker_pub.publish(nest);
        ros::Duration(0.2).sleep();
    }
}
