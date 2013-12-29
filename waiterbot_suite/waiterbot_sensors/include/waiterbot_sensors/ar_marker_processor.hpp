/*
 * ar_marker_processor.hpp
 *
 *  LICENSE : BSD - https://raw.github.com/yujinrobot/kobuki-x/license/LICENSE
 *
 *  Created on: Apr 6, 2013
 *      Author: jorge
 *  Modified on: Dec, 2013 
 *      Jihoon Lee          
 *
 */

#ifndef AR_MARKER_PROCESSOR_HPP_
#define AR_MARKER_PROCESSOR_HPP_

#include <ros/ros.h>

#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>

#include <yocs_math_toolkit/common.hpp>
#include <yocs_math_toolkit/geometry.hpp>
#include "waiterbot_sensors/yaml_parser_ar_markers.hpp"

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <ar_track_alvar/AlvarMarkers.h>

namespace waiterbot
{
  typedef std::list<geometry_msgs::PoseStamped> ObsList;

  namespace ARMarkerProcessorDefaultParams {
    const std::string GLOBAL_MARKER_FILENAME = "filename";
    const std::string GLOBAL_FRAME           = "map";
    const std::string ODOM_FRAME             = "odom";
    const std::string BASE_FRAME             = "base_footprint";
    const double      TF_BROADCAST_FREQ      = 0.0;
    const double      GLOBAL_POSE_CONF       = 0.8;
    const double      DOCKING_BASE_CONF      = 0.5;
    const double      MAX_VALID_D_INC        = 0.8;
    const double      MAX_VALID_H_INC        = 4.0;
    const double      MAX_TRACKING_TIME      = 2.0;
    const double      MIN_PENALIZED_DIST     = 1.4;
    const double      MAX_RELIABLE_DIST      = 2.8;
    const double      MIN_PENALIZED_HEAD     = 1.0;
    const double      MAX_RELIABLE_HEAD      = 1.4;

    const double      AR_TRACKER_FREQ        = 10.0;

    const uint32_t    MARKERS_COUNT          = 32;
  }

  class TrackedMarker
  {
    public:
      TrackedMarker()
      {
        distance      = 0.0;
        heading       = 0.0;
        confidence    = 0.0;
        conf_distance = 0.0;
        conf_heading  = 0.0;
        persistence   = 0.0;
        stability     = 0.0;
      }
                             
      ~TrackedMarker()
      {
        obs_list_.clear();
      }
                             
      ObsList obs_list_;
      double distance;
      double heading;
      double confidence;
      double conf_distance;
      double conf_heading;
      double persistence;
      double stability;
  };

  class ARMarkerProcessor
  {
    public:

      ARMarkerProcessor();
      virtual ~ARMarkerProcessor();

      bool init();
      void spin();




      /**
       * Return spotted markers satisfying the constraints specified by the parameters
       * @param younger_than    Elapsed time between now and markers timestamp must be below this limit.
       * @param min_confidence
       * @param exclude_globals
       * @param spotted_markers
       * @return
       */
      bool spotted(double younger_than, double min_confidence, bool exclude_globals, ar_track_alvar::AlvarMarkers& spotted_markers);
      bool closest(double younger_than, double min_confidence, bool exclude_globals, ar_track_alvar::AlvarMarker& closest_marker);

      bool spotDockMarker(uint32_t base_marker_id);
    protected:
      // load the poses of AR Markers which used for global localization
      void loadGlobalMarkers();

      // raw list of ar markers from ar_alvar_track pkg 
      void arPoseMarkersCB(const ar_track_alvar::AlvarMarkers::ConstPtr& msg);
        void maintainTrackedMarker(TrackedMarker& marker,const ar_track_alvar::AlvarMarker& msgMarker, const int obs_list_max_size, const double max_valid_d_inc, const double max_valid_h_inc);
        void processGlobalMarker(const TrackedMarker& marker, const ar_track_alvar::AlvarMarker& msgMarker,const ar_track_alvar::AlvarMarker& global_marker);

      //////////////////// tf related
      void broadcastMarkersTF();
      bool getMarkerTf(const std::string& ref_frame, uint32_t marker_id, const ros::Time& timestamp, tf::StampedTransform& tf);


      /////////////////// utility functions
      bool spotted(double younger_than, const ar_track_alvar::AlvarMarkers& including, const ar_track_alvar::AlvarMarkers& excluding, ar_track_alvar::AlvarMarkers& spotted_markers);
      bool closest(const ar_track_alvar::AlvarMarkers& including, const ar_track_alvar::AlvarMarkers& excluding, ar_track_alvar::AlvarMarker& closest_marker);

      // check if the given id ar_marker is in the list. If yes, return the full ar marker data
      bool included(const uint32_t id, const ar_track_alvar::AlvarMarkers& v, ar_track_alvar::AlvarMarker* e = NULL);

      // Check if the given id is in the list of ar markers
      bool excluded(const uint32_t id, const ar_track_alvar::AlvarMarkers& v);

    private:
      std::string global_marker_filename_;

      // Confidence evaluation attributes
      double global_pose_conf_;
      double docking_base_conf_;
      double min_penalized_dist_;
      double max_reliable_dist_;
      double min_penalized_head_;
      double max_reliable_head_;
      double max_tracking_time_;  /**< Maximum time tacking a marker to ensure that it's a stable observation */
      double max_valid_d_inc_;    /**< Maximum valid distance increment per second to consider stable tracking */
      double max_valid_h_inc_;    /**< Maximum valid heading increment per second to consider stable tracking */
      double ar_tracker_freq_;    /**< AR tracker frequency; unless changed with setTrackerFreq, it must be the
                                       same value configured on ar_track_alvar node */

      bool global_marker_localization_;

      // Other attributes
      std::string global_frame_;
      std::string odom_frame_;
      std::string base_frame_;

      std::vector<TrackedMarker> tracked_markers_;

      ar_track_alvar::AlvarMarker  docking_marker_;  /**< AR markers described in the semantic map */
      ar_track_alvar::AlvarMarkers global_markers_, global_markers_mirrors_;  /**< AR markers described in the semantic map */
      ar_track_alvar::AlvarMarkers spotted_markers_;

      ros::Subscriber    sub_ar_markers_;

      tf::TransformListener    tf_listener_;
      tf::TransformBroadcaster tf_brcaster_;
      double                   tf_broadcast_freq_;  /**< Allows enabling tf broadcasting; mostly for debug */


  };

} /* namespace waiterbot */

#endif /* AR_MARKERS_HPP_ */
