// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
//
// Copyright (c) 2017 Magic Leap, Inc. (COMPANY) All Rights Reserved.
// Magic Leap, Inc. Confidential and Proprietary
//
// NOTICE: All information contained herein is, and remains the property
// of COMPANY. The intellectual and technical concepts contained herein
// are proprietary to COMPANY and may be covered by U.S. and Foreign
// Patents, patents in process, and are protected by trade secret or
// copyright law. Dissemination of this information or reproduction of
// this material is strictly forbidden unless prior written permission is
// obtained from COMPANY. Access to the source code contained herein is
// hereby forbidden to anyone except current COMPANY employees, managers
// or contractors who have executed Confidentiality and Non-disclosure
// agreements explicitly covering such access.
//
// The copyright notice above does not evidence any actual or intended
// publication or disclosure of this source code, which includes
// information that is confidential and/or proprietary, and is a trade
// secret, of COMPANY. ANY REPRODUCTION, MODIFICATION, DISTRIBUTION,
// PUBLIC PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE OF THIS
// SOURCE CODE WITHOUT THE EXPRESS WRITTEN CONSENT OF COMPANY IS
// STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND
// INTERNATIONAL TREATIES. THE RECEIPT OR POSSESSION OF THIS SOURCE
// CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS
// TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE,
// USE, OR SELL ANYTHING THAT IT MAY DESCRIBE, IN WHOLE OR IN PART.
//
// %COPYRIGHT_END%
// --------------------------------------------------------------------
// %BANNER_END%

#pragma once

#include "ml_api.h"
#include "ml_types.h"

ML_EXTERN_C_BEGIN

/*!
  \addtogroup HandTracking
  \brief APIs for the Hand Tracking system.

  \{
*/

/*! Available hand types. */
typedef enum MLHandType {
  MLHandType_Left,
  MLHandType_Right,
  MLHandType_Count,
  /*! Reserve for future expansions. */
  MLHandType_Ensure32Bits = 0x7FFFFFFF
} MLHandType;

enum {
  /*! Maximum number of key points per gesture. */
  MLHandTrackingStaticData_MaxKeyPoints = 24
};

/*! Static key pose types which are available when both hands are separated. */
typedef enum MLHandTrackingKeyPose {
  /*! Index finger */
  MLHandTrackingKeyPose_Finger,
  /*! A closed fist. */
  MLHandTrackingKeyPose_Fist,
  /*! A pinch. */
  MLHandTrackingKeyPose_Pinch,
  /*! A closed fist with the thumb pointed up. */
  MLHandTrackingKeyPose_Thumb,
  /*! An L shape */
  MLHandTrackingKeyPose_L,
  /*! An open hand with the back of the hand facing the user. */
  MLHandTrackingKeyPose_OpenHandBack,
  /*! A pinch with all fingers, except the index finger and the thumb, extended out.  */
  MLHandTrackingKeyPose_Ok,
  /*! A rounded 'C' alphabet shape. */
  MLHandTrackingKeyPose_C,
  /*! No pose was recognized. */
  MLHandTrackingKeyPose_NoPose,
  /*! No hand was detected.  Should be the last pose. */
  MLHandTrackingKeyPose_NoHand,
  /*! Number of static poses. */
  MLHandTrackingKeyPose_Count,
  /*! Ensure enum is represented as 32 bits. */
  MLHandTrackingKeyPose_Ensure32Bits = 0x7FFFFFFF
} MLHandTrackingKeyPose;

/*! Keypoint data structure. */
typedef struct MLKeyPointState {
  /*! Keypoint coordinate frame. */
  MLCoordinateFrameUID frame_id;
  /*! Keypoint is valid or not. */
  bool is_valid;
} MLKeyPointState;

/*! Thumb data structure. */
typedef struct MLThumbState {
  /*! Tip of finger. */
  MLKeyPointState tip;
  /*! Inter-phalengial. */
  MLKeyPointState ip;
  /*! Meta-carpal phalengial. */
  MLKeyPointState mcp;
  /*! Carpals-Meta-carpal. */
  MLKeyPointState cmc;
} MLThumbState;

/*! Finger data structure. */
typedef struct MLFingerState {
  /*! Tip of finger. */
  MLKeyPointState tip;
  /*! Distal-inter-phalengial. */
  MLKeyPointState dip;
  /*! Proximal-inter-phalengial. */
  MLKeyPointState pip;
  /*! Meta-carpal phalengial. */
  MLKeyPointState mcp;
} MLFingerState;

/*! Wrist data structure. */
typedef struct MLWristState {
  /*! Wrist center. */
  MLKeyPointState center;
  /*! Ulnar-sided wrist. */
  MLKeyPointState ulnar;
  /*! Radial-sided wrist. */
  MLKeyPointState radial;
} MLWristState;

/*! Static information for one hand. */
typedef struct MLHandTrackingStaticHandState {
  /*! The keypoints on thumb. */
  MLThumbState thumb;
  /*! The keypoints on index finger. */
  MLFingerState index;
  /*! The keypoints on middle finger. */
  MLFingerState middle;
  /*! The keypoints on ring finger. */
  MLFingerState ring;
  /*! The keypoints on pinky finger. */
  MLFingerState pinky;
  /*! The keypoints on the wrist. */
  MLWristState wrist;
  /*! Hand center. */
  MLKeyPointState hand_center;
} MLHandTrackingStaticHandState;

/*!
  \brief Static information about a hand tracker. Populate this structure with
  MLHandTrackingGetStaticData().
*/
typedef struct MLHandTrackingStaticData {
    MLKeyPointState left_frame[MLHandTrackingStaticData_MaxKeyPoints];
    MLHandTrackingStaticHandState left;

    MLKeyPointState right_frame[MLHandTrackingStaticData_MaxKeyPoints];
    MLHandTrackingStaticHandState right;
} MLHandTrackingStaticData;

/*! Hand state. */
typedef struct MLHandTrackingHandState {
  /*! The static keypose currently being performed by the single hand. */
  MLHandTrackingKeyPose keypose;
  /*!
    \brief The confidence level of a hand is present in the scene.

    Value is between [0, 1], 0 is low, 1 is high degree of confidence.
  */
  float hand_confidence;
  /*! The confidence for all poses. */
  float keypose_confidence[MLHandTrackingKeyPose_Count];
  /*! The filtered confidence for all poses. */
  float keypose_confidence_filtered[MLHandTrackingKeyPose_Count];
  /*! Mask indicates if a keypoint is present or not. */
  bool keypoints_mask[MLHandTrackingStaticData_MaxKeyPoints];
  /*! Normalized position of hand center within depth-sensor view. Each dimension is between [-1, 1]. */
  MLVec3f hand_center_normalized;
} MLHandTrackingHandState;

/*! Data which is received when querying hand tracker from MLHandTrackingGetData(). */
typedef struct MLHandTrackingData {
  /*! Hand tracker state of the left hand. */
  MLHandTrackingHandState left_hand_state;
  /*! Hand tracker state of the right hand. */
  MLHandTrackingHandState right_hand_state;
} MLHandTrackingData;

/*! Configured level for keypoints filtering of keypoints and hand centers. */
typedef enum MLKeypointFilterLevel {
  /*! Default value, no filtering is done, the points are raw */
  MLKeypointFilterLevel_0,
  /*! Some smoothing at the cost of latency */
  MLKeypointFilterLevel_1,
  /*! Predictive smoothing, at higher cost of latency */
  MLKeypointFilterLevel_2,
  /*! Ensure enum is represented as 32 bits. */
  MLKeypointFilterLevel_Ensure32Bits = 0x7FFFFFFF
} MLKeypointFilterLevel;

/*! Configured level of filtering for static poses. */
typedef enum MLPoseFilterLevel {
  /*! Default value, No filtering, the poses are raw */
  MLPoseFilterLevel_0,
  /*! Some robustness to flicker at some cost of latency */
  MLPoseFilterLevel_1,
  /*! More robust to flicker at higher latency cost */
  MLPoseFilterLevel_2,
  /*! Ensure enum is represented as 32 bits. */
  MLPoseFilterLevel_Ensure32Bits = 0x7FFFFFFF
} MLPoseFilterLevel;

/*!
  \brief Configuration of the hand tracking system. This is used to activate or
  deactivate the poses the system will look for.
*/
typedef struct MLHandTrackingConfiguration {
  /*!
    \brief Configuration for the static poses. True will enable the pose to be
    tracked by the system, false will disable it. Note that the size of keypose_config
    is set to MLHandTrackingKeyPose_Count-1. Disabling NoHand is not allowed.
    If a disabled pose is performed, the most probable enabled pose will be reported.
  */
  bool keypose_config[MLHandTrackingKeyPose_Count - 1];

  /*!
    \brief True activates hand tracking. False deactivates the handtracking pipeline
    entirely and no recognition will take place.
  */
  bool handtracking_pipeline_enabled;

 /*! keypoint filter levels */
  MLKeypointFilterLevel keypoints_filter_level;

 /*! pose filter levels */
  MLPoseFilterLevel pose_filter_level;
} MLHandTrackingConfiguration;

/*!
  \brief    Creates a hand tracker.
  \param[out] out_handle A handle which can be used with MLHandTrackingGetData to get
            information about the hand, or #ML_INVALID_HANDLE if the tracker
            could not be created.
  \retval   MLResult_Ok The tracker was created successfully.
  \retval   MLResult_UnspecifiedFailure It failed to create the tracker.
  \retval   MLResult_InvalidParam out_handle is null.
  \retval   MLResult_PrivilegeDenied The application lacks privilege.
  \priv     LowLatencyLightwear
*/
ML_API MLResult ML_CALL MLHandTrackingCreate(MLHandle *out_handle);

/*!
  \brief    Destroys a hand tracker.
  \param[in] hand_tracker A handle to a Hand Tracker created by MLHandTrackingCreate().
  \retval   MLResult_Ok It successfully destroyed the tracker.
  \retval   MLResult_UnspecifiedFailure It failed to destroy the tracker.
  \priv     LowLatencyLightwear
*/
ML_API MLResult ML_CALL MLHandTrackingDestroy(MLHandle hand_tracker);

/*!
  \brief    Queries the current state of the hand tracker.
  \param[in]  hand_tracker A handle to a Hand Tracker created by MLHandTrackingCreate().
  \param[out] out_data Pointer to a variable that receives information about the tracked hands.
  \retval   MLResult_Ok The hand information was available and the information in out_data is valid.
  \retval   MLResult_UnspecifiedFailure It failed to get the hand information.
  \retval   MLResult_InvalidParam out_data is null.
  \priv     LowLatencyLightwear

*/
ML_API MLResult ML_CALL MLHandTrackingGetData(MLHandle hand_tracker, MLHandTrackingData *out_data);

/*!
  \brief    Gets static information about hand tracking.
  \param[in]  hand_tracker A handle to a Hand Tracker created by MLHandTrackingCreate().
  \param[out] out_data Pointer to a variable that receives static data about the hand tracker.
  \retval   MLResult_Ok The hand information was available and the information in out_data is valid.
  \retval   MLResult_UnspecifiedFailure It failed to get the hand information.
  \retval   MLResult_InvalidParam out_data is null.
  \retval   MLResult_PrivilegeDenied The application lacks privilege.
  \priv     LowLatencyLightwear, GesturesSubscribe
*/
ML_API MLResult ML_CALL MLHandTrackingGetStaticData(MLHandle hand_tracker, MLHandTrackingStaticData *out_data);

/*!
  \brief    Sets the configuration of the hand tracking system.
  \param[in]  hand_tracker A handle to a Hand Tracker created by MLHandTrackingCreate().
  \param[out] config Pointer to a variable that contains the configuration.
  \retval   MLResult_Ok It successfully updated the hand configuration.
  \retval   MLResult_UnspecifiedFailure It failed to update the hand configuration.
  \retval   MLResult_InvalidParam config is null.
  \retval   MLResult_PrivilegeDenied The application lacks privilege.
  \priv     GesturesConfig
*/
ML_API MLResult ML_CALL MLHandTrackingSetConfiguration(MLHandle hand_tracker, const MLHandTrackingConfiguration *config);

/*!
  \brief    Gets the current configuration of the hand tracking system.
  \param[in]  hand_tracker A handle to a Hand Tracker created by MLHandTrackingCreate().
  \param[out] out_config Pointer to a variable that receives information about the current configuration.
  \retval   MLResult_Ok The information in out_config is valid.
  \retval   MLResult_UnspecifiedFailure It failed to get the current configuration.
  \retval   MLResult_InvalidParam out_config is null.
  \retval   MLResult_PrivilegeDenied The application lacks privilege.
  \priv     GesturesConfig, GesturesSubscribe
*/
ML_API MLResult ML_CALL MLHandTrackingGetConfiguration(MLHandle hand_tracker, MLHandTrackingConfiguration *out_config);

/*! \} */

ML_EXTERN_C_END
