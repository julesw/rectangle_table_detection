/** \file rviz_publish.h
 * File defining utilities for publishing visualization messages for rviz, related with table and planes.
 * \author Jules Waldhart
 */
#ifndef _RVIZ_PUBLISH_H_
#define _RVIZ_PUBLISH_H_
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <rectangular_table_detection/PlaneArray.h>

/**
 * publish an rviz shape for displaying the detected table on rviz
 **/
void publish_rviz_shape(ros::Publisher* marker_pub, float dim_x, float dim_y, float dim_z=0.1, bool remove_shape = false);


/**
 * publish an array of markers representing normals of the planes described in array message planes_def.
 */
void publish_rviz_plane_norms( const ros::Publisher& marker_array_pub, const rectangular_table_detection::PlaneArray& planes_def, const std::string& frame_id);

#endif // _RVIZ_PUBLISH_H_
