// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
//
// Copyright (c) 2017 Magic Leap, Inc. (COMPANY) All Rights Reserved.
// Magic Leap, Inc. Confidential and Proprietary
//
// NOTICE:  All information contained herein is, and remains the property
// of COMPANY. The intellectual and technical concepts contained herein
// are proprietary to COMPANY and may be covered by U.S. and Foreign
// Patents, patents in process, and are protected by trade secret or
// copyright law.  Dissemination of this information or reproduction of
// this material is strictly forbidden unless prior written permission is
// obtained from COMPANY.  Access to the source code contained herein is
// hereby forbidden to anyone except current COMPANY employees, managers
// or contractors who have executed Confidentiality and Non-disclosure
// agreements explicitly covering such access.
//
// The copyright notice above does not evidence any actual or intended
// publication or disclosure  of  this source code, which includes
// information that is confidential and/or proprietary, and is a trade
// secret, of  COMPANY.   ANY REPRODUCTION, MODIFICATION, DISTRIBUTION,
// PUBLIC  PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS
// SOURCE CODE  WITHOUT THE EXPRESS WRITTEN CONSENT OF COMPANY IS
// STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND
// INTERNATIONAL TREATIES.  THE RECEIPT OR POSSESSION OF  THIS SOURCE
// CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS
// TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE,
// USE, OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.
//
// %COPYRIGHT_END%
// --------------------------------------------------------------------*/
// %BANNER_END%

#pragma once

#include "ml_api.h"

ML_EXTERN_C_BEGIN

/*!
  \addtogroup MusicService Music Service
  \{
*/

/*! MusicService errors. */
typedef enum MLMusicServiceError {
  MLMusicServiceError_None = 0,
  MLMusicServiceError_Connectivity,
  MLMusicServiceError_Timeout,
  MLMusicServiceError_GeneralPlayback,
  MLMusicServiceError_Privilege,
  MLMusicServiceError_ServiceSpecific,
  MLMusicServiceError_NoMemory,
  MLMusicServiceError_Unspecified,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServiceError_Ensure32Bits = 0x7FFFFFFF
} MLMusicServiceError;

/*! MusicService info. */
typedef enum MLMusicServiceStatus {
  MLMusicServiceStatus_ContextChanged = 0,
  MLMusicServiceStatus_Created,
  MLMusicServiceStatus_LoggedIn,
  MLMusicServiceStatus_LoggedOut,
  MLMusicServiceStatus_NextTrack,
  MLMusicServiceStatus_PrevTrack,
  MLMusicServiceStatus_TrackChanged,
  MLMusicServiceStatus_Unknown,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServiceStatus_Ensure32Bits = 0x7FFFFFFF
} MLMusicServiceStatus;

/*! MusicService playback options. */
typedef enum MLMusicServicePlaybackOption {
  MLMusicServicePlaybackOptions_Playback = 0,
  MLMusicServicePlaybackOptions_Repeat,
  MLMusicServicePlaybackOptions_Shuffle,
  MLMusicServicePlaybackOptions_Unknown,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServicePlaybackOptions_Ensure32Bits = 0x7FFFFFFF
} MLMusicServicePlaybackOptions;

/*! MusicService playback states. */
typedef enum MLMusicServicePlaybackState {
  MLMusicServicePlaybackState_Playing = 0,
  MLMusicServicePlaybackState_Paused,
  MLMusicServicePlaybackState_Stopped,
  MLMusicServicePlaybackState_Error,
  MLMusicServicePlaybackState_Unknown,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServicePlaybackState_Ensure32Bits = 0x7FFFFFFF
} MLMusicServicePlaybackState;

/*! MusicService repeat settings. */
typedef enum MLMusicServiceRepeatState {
  MLMusicServiceRepeatState_Off = 0,
  MLMusicServiceRepeatState_Song,
  MLMusicServiceRepeatState_Album,
  MLMusicServiceRepeatState_Unknown,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServiceRepeatState_Ensure32Bits = 0x7FFFFFFF
} MLMusicServiceRepeatState;

/*! MusicService shuffle settings. */
typedef enum MLMusicServiceShuffleState {
  MLMusicServiceShuffleState_On = 0,
  MLMusicServiceShuffleState_Off,
  MLMusicServiceShuffleState_Unknown,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServiceShuffleState_Ensure32Bits = 0x7FFFFFFF
} MLMusicServiceShuffleState;

/*! Enumerations used to get the Metadata information of a track. */
typedef enum MLMusicServiceTrackType {
  MLMusicServiceTrackType_Previous = 0,
  MLMusicServiceTrackType_Current,
  MLMusicServiceTrackType_Next,
  /*! Ensure enum is represented as 32 bits. */
  MLMusicServiceTrackType_Ensure32Bits = 0x7FFFFFFF
} MLMusicServiceTrackType;

/*! MusicService Metadata.*/
typedef struct MLMusicServiceMetadata {
  /*! Track name/title. */
  const char *track_title;
  /*! Album name. */
  const char *album_name;
  /*! Album URL. */
  const char *album_url;
  /*! Album cover URL. */
  const char *album_cover_url;
  /*! Artist name. */
  const char *artist_name;
  /*! Artist URL. */
  const char *artist_url;
  /*! Length/Duration of the track in seconds. */
  uint32_t   length;
} MLMusicServiceMetadata;

/*! \} */

ML_EXTERN_C_END
