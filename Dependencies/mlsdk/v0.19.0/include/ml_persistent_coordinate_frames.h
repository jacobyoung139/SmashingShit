// %BANNER BEGIN%
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
// ---------------------------------------------------------------------
// %BANNER_END%

#pragma once

#include "ml_api.h"
#include "ml_types.h"
#include "ml_coordinate_frame_uid.h"
#include "ml_passable_world.h"

ML_EXTERN_C_BEGIN

/*!
  \defgroup PersistentCoordinateFrames Persistent Coordinate Frames
  \addtogroup PersistentCoordinateFrames
  \brief APIs for the Persistent Coordinate Frame Tracker system.

  - Make content persist at locations in the real world.

  \attention EXPERIMENTAL

  \{
*/

/*!
  \brief  Creates a Persistent Coordinate Frame Tracker.
  \param[out] out_tracker_handle pointer to a MLHandle.
  \retval MLResult_Ok Returned a valid MLHandle.
  \retval MLResult_PrivilegeDenied Privileges not met. Check app manifest.
  \retval MLResult_UnspecifiedFailure Unspecified failure.
  \retval MLResult_InvalidParam out param is null.
  \priv   PwFoundObjRead
*/
ML_API MLResult ML_CALL MLPersistentCoordinateFrameTrackerCreate(MLHandle *out_tracker_handle);

/*!
  \brief  Returns the count of currently available Persistent Coordinate Frames.
  \param[in]  tracker_handle Valid MLHandle to a Persistent Coordinate Frame Tracker.
  \param[out]  out_count number of currently available Persistent Coordinate Frames.
  \retval MLResult_Ok Returned a valid MLHandle.
  \retval MLResult_PrivilegeDenied Privileges not met. Check app manifest.
  \retval MLResult_UnspecifiedFailure Unspecified failure.
  \retval MLResult_InvalidParam out param is null.
  \retval MLPassableWorldResult_LowMapQuality Map quality too low for content persistence. Continue building the map.
  \retval MLPassableWorldResult_UnableToLocalize Currently unable to localize into any map. Continue building the map.
  \priv   PwFoundObjRead
*/
ML_API MLResult ML_CALL MLPersistentCoordinateFrameGetCount(MLHandle tracker_handle, uint32_t *out_count);

/*!
  \brief Returns all the Persistent Coordinate Frame currently available.
  \param[in] tracker_handle Valid MLHandle to a Persistent Coordinate Frame Tracker
  \param[in] buffer_size Size allocated for out_cfuids. Will only write buffer_size/sizeof(MLCoorindateFrameUID) cfuids.
  \param[out] out_cfuids ptr to be used for writing the Persistent Coordinate Frame's MLCoordinateFrameUID.
  \retval MLResult_Ok Returned a valid MLHandle.
  \retval MLResult_AllocFailed Size allocated is not sufficient. Use MLPersistentCoordinateFrameGetCount to get the count. Size
          should be sizeof(MLCoordinateFrameUID) * count.
  \retval MLResult_PrivilegeDenied Privileges not met. Check app manifest.
  \retval MLResult_InvalidParam out param is null.
  \retval MLResult_UnspecifiedFailure Unspecified failure.
  \retval MLPassableWorldResult_LowMapQuality Map quality too low for content persistence. Continue building the map.
  \retval MLPassableWorldResult_UnableToLocalize Currently unable to localize into any map. Continue building the map.
  \priv PwFoundObjRead
*/
ML_API MLResult ML_CALL MLPersistentCoordinateFrameGetAll(MLHandle tracker_handle, uint32_t buffer_size, MLCoordinateFrameUID **out_cfuids);

/*!
  \brief Returns the closest Persistent Coordinate Frame to the target point passed in.
  \param[in] tracker_handle Valid MLHandle to a Persistent Coordinate Frames Tracker.
  \param[in] target XYZ of the destination that the nearest Persistent Coordinate Frame is requested for.
  \param[out] out_cfuid ptr to be used to write the MLCoordinateFrameUID for the nearest Persistent Coordinate Frame to the target destination.
  \retval MLResult_Ok Returned a valid MLHandle.
  \retval MLResult_PrivilegeDenied Privileges not met. Check app manifest.
  \retval MLResult_UnspecifiedFailure Unspecified failure.
  \retval MLResult_InvalidParam out param is null.
  \retval MLPassableWorldResult_LowMapQuality Map quality too low for content persistence. Continue building the map.
  \retval MLPassableWorldResult_UnableToLocalize Currently unable to localize into any map. Continue building the map.
  \priv PwFoundObjRead
*/
ML_API MLResult ML_CALL MLPersistentCoordinateFrameGetClosest(MLHandle tracker_handle, const MLVec3f *target, MLCoordinateFrameUID *out_cfuid);

/*!
  \brief Destroys a Persistent Coordinate Frame Tracker.
  \param tracker_handle to the PersistentCoordinateFrameTracker to be destroyed.
  \retval MLResult_Ok Returned a valid MLHandle.
  \retval MLResult_InvalidParam tracker handle is not known.
  \retval MLResult_UnspecifiedFailure Unspecified failure.
  \priv none
*/
ML_API MLResult ML_CALL MLPersistentCoordinateFrameTrackerDestroy(MLHandle tracker_handle);

/*!
  \brief Returns an ascii string representation for each result code.

  This call returns strings for all of the global MLResult and
  MLPassableWorldResult codes.

  \param[in] result_code MLResult type to be converted to string.
  \retval ascii string containing readable version of the result code.
  \priv none
*/
ML_API const char* ML_CALL MLPersistentCoordinateFrameGetResultString(MLResult result_code);

/*! \} */
ML_EXTERN_C_END
