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
// ---------------------------------------------------------------------
// %BANNER_END%

#pragma once

#include "ml_api.h"
#include "ml_types.h"

ML_EXTERN_C_BEGIN

/*!
  \addtogroup Audio
  \brief APIs for the Audio Service.

  The Audio Service supports audio input (capture) and audio output (render).

  The audio capture capabilities of the Audio Service include capturing:
  (1) a mono mic signal focused on the user's voice (voice comm); (2) a stereo
  mic signal of the ambient environment (world capture); (3) a stereo mix
  of the audio output currently being rendered (virtual capture); and (4) a
  mix of voice comm and world capture (mixed capture). To initiate a capture
  input, the client allocates an 'input' using one of the create functions,
  and then starts the stream of audio input. The audio input streaming can be
  accomplished either by polling or callbacks (callbacks are recommended).

  The audio render capabilities of the Audio Service include rendering:
  (1) an audio file fully loaded into memory (loaded file); (2) an audio file
  streamed from flash memory to main memory in chunks (streamed file); and
  (3) a buffered stream of audio output (output stream). To initiate a render
  output, the client allocates a 'sound' using one of the create functions,
  and then calls 'start' to play the audio. For the first two rendering
  options, the file to be played can either be passed to the create function,
  or it can be pre-loaded as a 'resource' so that it can be loaded once and
  then used by multiple sounds that get created and destroyed. For audio
  output streams, the audio input streaming can be accomplished either by
  polling or callbacks (callbacks are recommended). For sounds where 3D
  audio spatialization is desired, the client must enable 'spatial sound'
  for that sound and then set its position (and if desired, other spatial
  sound params can be adjusted for more detailed control).

  \{
*/

/*!
  \enum MLAudioState
  Possible playback states for sounds and inputs.
*/
typedef enum MLAudioState {
  /*! Stopped, can be started. */
  MLAudioState_Stopped,
  /*! Playing, can be stopped or paused. */
  MLAudioState_Playing,
  /*! Paused, can be resumed or re-started. */
  MLAudioState_Paused,
  /*! Ensure enum is 32-bits. */
  MLAudioState_Ensure32Bits = 0x7FFFFFFF
} MLAudioState;

/*!
  \enum MLAudioEvent
  Possible audio events for sounds and inputs.
*/
typedef enum MLAudioEvent {
  /*! Sound at end and is stopping. */
  MLAudioEvent_End,
  /*! Sound at end and is looping. */
  MLAudioEvent_Loop,
  /*! Sound or input muted by policy manager. */
  MLAudioEvent_MutedBySystem,
  /*! Sound or input unmuted by policy manager. */
  MLAudioEvent_UnmutedBySystem,
  /*! Sound ducked by policy manager. */
  MLAudioEvent_DuckedBySystem,
  /*! Sound unducked by policy manager. */
  MLAudioEvent_UnduckedBySystem,
  /*! Resource used by sound has been destroyed. */
  MLAudioEvent_ResourceDestroyed,
  /*! Ensure enum is 32-bits. */
  MLAudioEvent_Ensure32Bits = 0x7FFFFFFF
} MLAudioEvent;

/*!
  \struct MLAudioSpatialSoundDistanceProperties
  Properties specifying the distance response of a spatial sound.
*/
typedef struct MLAudioSpatialSoundDistanceProperties {
  /*! Distance where sound is at full volume. */
  float min_distance;
  /*! Distance beyond which sound gets no quieter. */
  float max_distance;
  /*! Modification to real-world distance response. */
  float rolloff_factor;
} MLAudioSpatialSoundDistanceProperties;

/*!
  \struct MLAudioSpatialSoundRadiationProperties
  Properties specifying the directivity of a spatial sound.
*/
typedef struct MLAudioSpatialSoundRadiationProperties {
  /*! Inner cone angle (0-360); radiation unaffected. */
  float inner_angle;
  /*! Outer cone angle (0-360); directivity at maximum. */
  float outer_angle;
  /*! Volume scale (0-1) beyond outer cone for all freqs. */
  float outer_gain;
  /*! Volume scale (0-1) beyond outer cone for low freqs. */
  float outer_gain_lf;
  /*! Volume scale (0-1) beyond outer cone for mid freqs. */
  float outer_gain_mf;
  /*! Volume scale (0-1) beyond outer cone for high freqs. */
  float outer_gain_hf;
} MLAudioSpatialSoundRadiationProperties;

/*!
  \struct MLAudioSpatialSoundSendLevels
  Properties specifying send levels for a spatial sound.
*/
typedef struct MLAudioSpatialSoundSendLevels {
  /*! Volume scale (0-1) for all freqs. */
  float gain;
  /*! Volume scale (0-1) for low freqs. */
  float gain_lf;
  /*! Volume scale (0-1) for mid freqs. */
  float gain_mf;
  /*! Volume scale (0-1) for high freqs. */
  float gain_hf;
} MLAudioSpatialSoundSendLevels;

/*!
  \struct MLAudioSpatialSoundRoomProperties
  Properties specifying room acoustics.
*/
typedef struct MLAudioSpatialSoundRoomProperties {
  /*! Volume scale (0-1) for all room sound. */
  float gain;
  /*! Pre-delay (secs) for early reflections. */
  float reflections_delay;
  /*! Volume scale (0-8) for early reflections. */
  float reflections_gain;
  /*! Additional delay (secs) for late reverberation. */
  float reverb_delay;
  /*! Volume scale (0-8) for late reverberation. */
  float reverb_gain;
  /*! Decay time (secs) for late reverberation. */
  float reverb_decay_time;
  /*! Relative decay time for low freq reverb. */
  float reverb_decay_time_lf_ratio;
  /*! Relative decay time for high freq reverb. */
  float reverb_decay_time_hf_ratio;
} MLAudioSpatialSoundRoomProperties;

/*!
  \struct MLAudioSpatialSoundControlFrequencies
  Specification of lf, mf and hf frequencies used throughout.
*/
typedef struct MLAudioSpatialSoundControlFrequencies {
  /*! Frequency (Hz) for low freq control. */
  float control_lf;
  /*! Frequency (Hz) for mid freq control. */
  float control_mf;
  /*! Frequency (Hz) for high freq control. */
  float control_hf;
} MLAudioSpatialSoundControlFrequencies;

/*!
  \enum MLAudioOutputDevice
  The currently active output device.
*/
typedef enum MLAudioOutputDevice {
  /*! Built-in speakers in the wearable. */
  MLAudioOutputDevice_Wearable,
  /*! 3.5mm jack on the beltpack. */
  MLAudioOutputDevice_AnalogJack,
  /*! Ensure enum is 32-bits. */
  MLAudioOutputDevice_Ensure32Bits = 0x7FFFFFFF
} MLAudioOutputDevice;

/*!
  \enum MLAudioSampleFormat
  Possible sample formats for input and output streams.
*/
typedef enum MLAudioSampleFormat {
  /*! Samples are integer. */
  MLAudioSampleFormat_Int,
  /*! Samples are float - NOT IMPLEMENTED. */
  MLAudioSampleFormat_Float,
  /*! Ensure enum is 32-bits. */
  MLAudioSampleFormat_Ensure32Bits = 0x7FFFFFFF
} MLAudioSampleFormat;

/*!
  \struct MLAudioBufferFormat
  Buffer format settings for input and output streams.
*/
typedef struct MLAudioBufferFormat {
  /*! Number of channels. */
  uint32_t channel_count;
  /*! Sample rate. */
  uint32_t samples_per_second;
  /*! Number of bits per sample. */
  uint32_t bits_per_sample;
  /*! Number of bits used per sample. */
  uint32_t valid_bits_per_sample;
  /*! Integer or float (float not implemented). */
  MLAudioSampleFormat sample_format;
  /*! Reserved for future use. */
  uint32_t reserved;
} MLAudioBufferFormat;

/*!
  \struct MLAudioBuffer
  An audio buffer for passing data from input streams or to output streams.
*/
typedef struct MLAudioBuffer {
  /*! Pointer to audio data. */
  uint8_t *ptr;
  /*! Size of audio data. */
  uint32_t size;
} MLAudioBuffer;

enum {
  MLResultAPIPrefix_Audio = MLRESULT_PREFIX(0x9e11)
};

/*!
  \enum MLAudioResult
  Return values for Audio Service API calls.
*/
typedef enum MLAudioResult {
  /*! Function not implemented. */
  MLAudioResult_NotImplemented = MLResultAPIPrefix_Audio,
  /*! Not a valid MLHandle for a sound or input. */
  MLAudioResult_HandleNotFound,
  /*! Sample rate not supported. */
  MLAudioResult_InvalidSampleRate,
  /*! Bits per sample not supported. */
  MLAudioResult_InvalidBitsPerSample,
  /*! Valid bits per sample not supported. */
  MLAudioResult_InvalidValidBits,
  /*! Sample format not supported. */
  MLAudioResult_InvalidSampleFormat,
  /*! Channel count not supported. */
  MLAudioResult_InvalidChannelCount,
  /*! Buffer size too small. */
  MLAudioResult_InvalidBufferSize,
  /*! Buffer not ready for read or write. */
  MLAudioResult_BufferNotReady,
  /*! Specified file not found. */
  MLAudioResult_FileNotFound,
  /*! Specified file has unsupported format. */
  MLAudioResult_FileNotRecognized,
  /*! Specified resource is not on the list. */
  MLAudioResult_ResourceNotFound,
  /*! Data was unloaded or file was closed. */
  MLAudioResult_ResourceDiscarded,
  /*! Ensure enum is 32-bits. */
  MLAudioResult_Ensure32Bits = 0x7FFFFFFF
} MLAudioResult;

/*!
  \brief Returns an ascii string for each result code.

  This call returns strings for all of the MLResult and
  MLAudioResult codes.

  \param[in] result_code MLResult to select the result code

  \retval ascii string containing readable version of result code
*/
ML_API const char* ML_CALL MLAudioGetResultString(MLResult result_code);

/*!
  \brief MLAudioEventCallback
  Callback to notify when an audio event has occurred.

  \note The callback will be called on a separate thread.
*/
typedef void (*MLAudioEventCallback)(MLHandle handle, MLAudioEvent event, void *callback_context);

/*!
  \brief MLAudioBufferCallback
  Callback to notify when an audio buffer is available for reading or writing.

  \note The callback will be called on a separate thread.
*/
typedef void (*MLAudioBufferCallback)(MLHandle handle, void *callback_context);

/*!
  \brief MLAudioResourceDiscardedCallback
  Callback to notify when audio resource data is being discarded to make room
  for other resource data. Resources that are not being referenced by a sound
  will be discarded before resources that are being referenced by a sound.
  Resources that are being played by a sound will not be discarded.

  \note The callback will be called on a separate thread.
  \note The audio data should not be refreshed until needed again.
*/
typedef void (*MLAudioResourceDiscardedCallback)(MLHandle resource, void *callback_context);

/*!
  \brief MLAudioMasterVolumeChangedCallback
  Callback to notify when master volume has been changed.

  \note The callback will be called on a separate thread.
*/
typedef void (*MLAudioMasterVolumeChangedCallback)(float new_volume, void *callback_context);

/*!
  \brief MLAudioMicMuteCallback
  Callback to notify when global mic mute has been changed.

  \note The callback will be called on a separate thread.
*/
typedef void (*MLAudioMicMuteCallback)(bool is_muted, void *callback_context);

/*!
  \brief Creates a new audio resource that references a loaded audio file.

  Creates a new audio resource that references the audio file specified by the
  'file' argument, which it tries to load during resource creation. An optional
  MLAudioResourceDiscardedCallback will inform the client if the data later gets
  discarded due to memory limits (or if it fails to load during creation). After creation
  the resource can be played by passing its id to MLAudioCreateSoundWithLoadedResource.

  \param[in] file file descriptor specifying the file to load
  \param[in] dynamic_decode not currently implemented
  \param[in] callback MLAudioResourceDiscardedCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback
  \param[out] out_resource MLHandle used in subsequent calls for this resource

  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_AllocFailed if memory allocation fails.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_OK if resource creation was successful

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateLoadedResource(int file, bool dynamic_decode, MLAudioResourceDiscardedCallback callback, void *callback_context, MLHandle *out_resource);

/*!
  \brief Creates a new audio resource that references a streamed audio file.

  Creates a new audio resource that references the audio file specified by the 'file'
  argument. An optional MLAudioResourceDiscardedCallback will inform the client if the
  file's file descriptor later gets discarded due to file descriptor limits (or if a file
  descriptor cannot be generated during resource creation). After creation the
  resource can be played by passing its id to MLAudioCreateSoundWithStreamedResource.

  \param[in] file file descriptor specifying the file to load
  \param[in] callback MLAudioResourceDiscardedCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback
  \param[out] out_resource MLHandle used in subsequent calls for this resource

  \retval MLResult_OK if resource creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateStreamedResource(int file, MLAudioResourceDiscardedCallback callback, void *callback_context, MLHandle *out_resource);

/*!
  \brief Checks whether an audio resource has been discarded due to memory limits.

  This call can be used instead of the MLAudioResourceDiscardedCallback to determine whether
  an audio resource has been discarded. Resources that are not being referenced by a sound
  will be discarded before resources that are being referenced by a sound. Resources that
  are being played by a sound will not be discarded. MLAudioCheckResource can be called
  just before the resource is needed, or at some other appropriate time for accessing data.
  However it should not be called constantly on a polling basis to instantly refresh
  resources that get discarded, since that would defeat the purpose of the memory limit.

  \param[in] resource MLHandle used to identify the resource
  \param[out] out_is_ready indicates whether the resource is ready or not

  \retval MLResult_OK if resource checking was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCheckResource(MLHandle resource, bool *out_is_ready);

/*!
  \brief Refreshes an audio resource whose data was discarded due to memory limits.

  This call allows the client to 'refresh' a resource whose data has been discarded. This
  should be done just prior to the next time the data is needed, or some other convenient
  time when data access is appropriate, but not right after the data is discarded, as that
  would defeat the purpose of the memory limit.

  \param[in] resource MLHandle used to identify the resource
  \param[in] file file descriptor specifying the audio data to refresh

  \retval MLResult_OK if resource refreshing was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioRefreshResource(MLHandle resource, int file);

/*!
  \brief Destroys a previously created audio resource. If there are sounds that
  are currently playing the resource, they will be stopped, and an event callback
  will be issued for MLAudioEvent_ResourceDestroyed.

  \param[in] resource MLHandle used to identify the resource

  \retval MLResult_OK if resource destruction was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioDestroyResource(MLHandle resource);

/*!
  \brief Creates a new sound output that plays a loaded audio resource.

  Creates a sound output to play a loaded audio file referenced as a resource.
  Loaded resources are created and loaded by calling MLAudioCreateLoadedResource.
  After sound creation the sound can be started by calling MLAudioStartSound. Supported
  file formats are all file types supported by the Multimedia Service, mono or stereo,
  sampled with a frequency in the range from 16 kHz - 48 kHz, and represented as
  16-bit integers.

  \param[in] resource MLHandle specifying the resource to play
  \param[in] auto_destroy if true, the sound output is destroyed when done playing
  \param[out] out_handle MLHandle used in subsequent calls for this sound output

  \retval MLResult_OK if sound creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateSoundWithLoadedResource(MLHandle resource, bool auto_destroy, MLHandle *out_handle);

/*!
  \brief Creates a new sound output that plays a streamed audio resource.

  Creates a sound output to play a streamed audio file referenced as a resource.
  Streamed resources are created by calling MLAudioCreateStreamedResource. After sound
  creation the sound can be started by calling MLAudioStartSound.  Supported file formats
  are all file types supported by the Multimedia Service, mono or stereo, sampled
  with a frequency in the range from 16 kHz - 48 kHz, and represented as 16-bit
  integers.

  \param[in] resource MLHandle specifying the resource to play
  \param[in] auto_destroy if true, the sound output is destroyed when done playing
  \param[out] out_handle MLHandle used in subsequent calls for this sound output

  \retval MLResult_OK if sound creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateSoundWithStreamedResource(MLHandle resource, bool auto_destroy, MLHandle *out_handle);

/*!
  \brief Creates a new sound output that plays a loaded file.

  Creates a sound output and loads the audio file specified by the 'file'
  argument into memory. After creation the sound can be started by calling
  MLAudioStartSound. Supported file formats are all file types supported by
  the Multimedia Service, mono or stereo, sampled with a frequency in the range
  from 16 kHz - 48 kHz, and represented as 16-bit integers.

  \param[in] file file descriptor specifying the file to play
  \param[in] auto_destroy if true, the sound output is destroyed when done playing
  \param[in] dynamic_decode not currently implemented
  \param[out] out_handle MLHandle used in subsequent calls for this sound output

  \retval MLResult_OK if sound creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateSoundWithLoadedFile(int file, bool auto_destroy, bool dynamic_decode, MLHandle *out_handle);

/*!
  \brief Creates a new sound output that plays a streamed file.

  Creates a sound output and prepares to stream the audio file specified by the
  'file' argument. After creation the sound can be started by calling
  MLAudioStartSound. Supported file formats are all file types supported by the
  Multimedia Service, mono or stereo, sampled with a frequency in the range from
  16 kHz - 48 kHz, and represented as 16-bit integers.

  \param[in] file file descriptor specifying the file to play
  \param[in] auto_destroy if true, the sound output is destroyed when done playing
  \param[out] out_handle MLHandle used in subsequent calls for this sound output

  \retval MLResult_OK if sound creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateSoundWithStreamedFile(int file, bool auto_destroy, MLHandle *out_handle);

/*!
  \brief Creates a new sound output that plays a buffered audio stream.

  Creates a sound output and allocates a double-buffer for streaming
  audio data into it. To stream  data into the buffers call
  MLAudioGetOutputStreamBuffer to get an empty buffer, then write the
  data into the buffer, then call MLAudioReleaseOutputStreamBuffer to
  indicate that the buffer is ready to play. This can be done either on
  a polling basis or within a callback set via the 'callback' argument. The
  'format' argument specifies the format of which the following are supported:
  mono or stereo, 16 bit integer, a sampling rate in the range of 16k - 48k.
  The 'buffer_size' argument will use the recommended size if 0 is passed in.
  After creation, one or both buffers can optionally be pre-filled, and the
  sound can be started by calling MLAudioStartSound.

  \param[in] format MLAudioBufferFormat specifying the audio format of the stream
  \param[in] buffer_size requested size in bytes for each of the two stream buffers
  \param[in] callback MLAudioBufferCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback
  \param[out] out_handle MLHandle used in subsequent calls for this sound output

  \retval MLResult_OK if sound creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioCreateSoundWithOutputStream(const MLAudioBufferFormat *format, uint32_t buffer_size, MLAudioBufferCallback callback, void *callback_context, MLHandle *out_handle);

/*!
  \brief Destroys a previously created sound output.

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if sound destruction was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioDestroySound(MLHandle handle);

/*!
  \brief Starts playback for a sound output.

  When starting playback for a file (loaded, streamed,
  system sound), playback starts at the beginning of the
  file (even after a pause). When starting playback for an
  output stream, the "frames played" counter gets reset
  to zero (and if the output stream was previously paused
  the streaming buffers get reset as well).

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if starting was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioStartSound(MLHandle handle);

/*!
  \brief Stops playback for a sound output.

  Stops playback for a sound output that's playing or paused.

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if stopping was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioStopSound(MLHandle handle);

/*!
  \brief Pauses playback for a sound output.

  Pauses playback for a sound output that's playing. If the
  sound output is not playing, the pause will fail.

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if pausing was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioPauseSound(MLHandle handle);

/*!
  \brief Resumes playback for a sound output.

  Resumes playback for a sound output that's paused. If the
  sound output is not paused, the resume will fail.

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if resuming was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioResumeSound(MLHandle handle);

/*!
  \brief Indicates the play state of a sound output.

  The MLAudioState indicates whether the sound output is
  stopped, playing or paused.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_state indicates the current play state

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSoundState(MLHandle handle, MLAudioState *out_state);

/*!
  \brief Returns the audio format of a sound output.

  The returned information includes the channel count, sample
  rate and bit depth of the sound's audio data. For Streamed Files,
  the format is not available until about 100+ milliseconds after
  playback of the Streamed File is started.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_format MLAudioBufferFormat struct containing the format info

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSoundFormat(MLHandle handle, MLAudioBufferFormat *out_format);

/*!
  \brief Registers a callback for events on a sound output.

  Events that can result in a callback being called include:
  stopping at the end of a file; looping at the end of a file;
  a sound output having its volume lowered or restored by the
  audio policy manager; a sound output being muted or unmuted
  by the audio policy manager. The 'callback_context' argument
  is passed back to the callback as its first argument. The
  particular event type (MLAudioEvent) that caused the callback
  is passed to the callback as its second argument.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] callback MLAudioEventCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundEventCallback(MLHandle handle, MLAudioEventCallback callback, void *callback_context);

/*!
  \brief Enables/disables 3D audio processing for a sound output.

  The 3D audio processing for a sound output can be turned on
  or off by this function (default is off). When 3D processing
  is enabled, the 3D position should be set by calling
  MLAudioSetSpatialSoundPosition. When 3D processing is
  disabled, stereo sounds will play their left and right
  channels directly to the left and right speakers or
  earphones, and mono sounds will be panned center.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] is_enabled determines whether 3D processing is on or off

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundEnable(MLHandle handle, bool is_enabled);

/*!
  \brief Indicates whether 3D audio processing is enabled for a sound output.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_is_enabled indicates whether 3D processing is on or off

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundEnable(MLHandle handle, bool *out_is_enabled);

/*!
  \brief Sets the 3D position for one channel of a sound output.

  When 3D audio processing is enabled for a sound output (see
  MLAudioSetSpatialSoundEnable) this funtions sets the
  perceived position within the user's environment. Multi-channel
  sounds require the 3D position to be set individually for each
  channel by calling this function once for each channel. For mono
  sounds use channel = 0. For stereo sounds use channel = 0 for
 • left and channel = 1 for right. Positive x is right. Positive y
  is up. Positive z is back.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose position is being set
  \param[in] position MLVec3f struct to set the position

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundPosition(MLHandle handle, uint32_t channel, const MLVec3f *position);

/*!
  \brief Returns the 3D position for one channel of a sound output.

  Multi-channel sounds require the 3D position to be read individually
  for each channel by calling this function once for each channel. For
  mono sounds use channel = 0. For stereo sounds use channel = 0
  for left and channel = 1 for right. Positive x is right. Positive y
  is up. Positive z is back.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose position is being read
  \param[out] out_position MLVec3f struct to return the position

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundPosition(MLHandle handle, uint32_t channel, MLVec3f *out_position);

/*!
  \brief Sets the radiation direction for one channel of a sound output.

  For sounds that have been set to radiate in a directional way
  by MLAudioSetSpatialSoundRadiationProperties, this function
  sets the direction of the radiation. Multi-channel sounds require
  the radiation direction to be set individually for each channel by
  calling this function once for each channel. For mono sounds
  use channel = 0. For stereo sounds use channel = 0 for left and
 • channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose direction is being set
  \param[in] direction MLQuaternionf struct to set the direction

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundDirection(MLHandle handle, uint32_t channel, const MLQuaternionf *direction);

/*!
  \brief Returns the radiation direction for one channel of a sound output.

  Multi-channel sounds require the radiation direction to be read
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds
  use channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose direction is being read
  \param[out] out_direction MLQuaternionf struct to return the direction

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundDirection(MLHandle handle, uint32_t channel, MLQuaternionf *out_direction);

/*!
  \brief Sets the distance properties for one channel of a sound output.

  When 3D audio processing is enabled for a sound output (see
  MLAudioSetSpatialSoundEnable) this function sets the way in
  which distance affects sound volume. Multi-channel sounds
  require the distance properties to be set individually for each
  channel by calling this function once for each channel. For mono
  sounds use channel = 0. For stereo sounds use channel = 0 for
  left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose distance properties are being set
  \param[in] properties MLAudioSpatialSoundDistanceProperties struct to set the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundDistanceProperties(MLHandle handle, uint32_t channel, const MLAudioSpatialSoundDistanceProperties *properties);

/*!
  \brief Returns the distance properties for one channel of a sound output.

  Multi-channel sounds require the distance properties to be read
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose distance properties are being read
  \param[out] out_properties MLAudioSpatialSoundDistanceProperties struct to return the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundDistanceProperties(MLHandle handle, uint32_t channel, MLAudioSpatialSoundDistanceProperties *out_properties);

/*!
  \brief Sets the radiation properties for one channel of a sound output.

  When 3D audio processing is enabled for a sound output (see
  MLAudioSetSpatialSoundEnable) this function sets the degree
  of directionality in the sound projection. The default setting is for
  sounds to be omnidirectional (equal projection in all directions).
  Multi-channel sounds require the radiation properties to be set
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose radiation properties are being set
  \param[in] properties MLAudioSpatialSoundRadiationProperties struct to set the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundRadiationProperties(MLHandle handle, uint32_t channel, const MLAudioSpatialSoundRadiationProperties *properties);

/*!
  \brief Returns the radiation properties for one channel of a sound output.

  Multi-channel sounds require the radiation properties to be read
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose radiation properties are being set
  \param[out] out_properties MLAudioSpatialSoundRadiationProperties struct to return the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundRadiationProperties(MLHandle handle, uint32_t channel, MLAudioSpatialSoundRadiationProperties *out_properties);

/*!
  \brief Sets the direct send levels for one channel of a sound output.

  When 3D audio processing is enabled for a sound output (see
  MLAudioSetSpatialSoundEnable) this function sets the send
  levels for the direct component of the sound, i.e., the audio mix
  for the the part of the sound not affected by room acoustics.
  Multi-channel sounds require the direct send levels to be set
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose direct send levels are being set
  \param[in] levels MLAudioSpatialSoundSendLevels struct to set the levels

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundDirectSendLevels(MLHandle handle, uint32_t channel, const MLAudioSpatialSoundSendLevels *levels);

/*!
  \brief Returns the direct send levels for one channel of a sound output.

  Multi-channel sounds require the direct send levels to be read
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose direct send levels are being read
  \param[out] out_levels MLAudioSpatialSoundSendLevels struct to return the levels

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundDirectSendLevels(MLHandle handle, uint32_t channel, MLAudioSpatialSoundSendLevels *out_levels);

/*!
  \brief Sets the room send levels for one channel of a sound output.

  When 3D audio processing is enabled for a sound output (see
  MLAudioSetSpatialSoundEnable) this function sets the send
  levels for the room component of the sound, i.e., the audio mix
  for the the part of the sound that's affected by room acoustics.
  Multi-channel sounds require the room send levels to be set
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose room send levels are being set
  \param[in] levels MLAudioSpatialSoundSendLevels struct to set the levels

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundRoomSendLevels(MLHandle handle, uint32_t channel, const MLAudioSpatialSoundSendLevels *levels);

/*!
  \brief Returns the room send levels for one channel of a sound output.

  Multi-channel sounds require the room send levels to be read
  individually for each channel by calling this function once for each
  channel. For mono sounds use channel = 0. For stereo sounds use
  channel = 0 for left and channel = 1 for right.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] channel selects the channel whose room send levels are being read
  \param[out] out_levels MLAudioSpatialSoundSendLevels struct to return the levels

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundRoomSendLevels(MLHandle handle, uint32_t channel, MLAudioSpatialSoundSendLevels *out_levels);

/*!
  \brief Sets the room acoustics shared by all spatial sounds.

  For sound outputs that use 3D audio processing this function sets
  the acoustic properties of the "room" that all spatial sounds share.

  \note For the time being (until a future update), this global acoustic
  setting applies to all LuminRT and native applications and services.
  You should only use this call if you are an immersive app that needs
  specific acoustical properties. In that case, you should be sure to
  "get" the current settings first, and restore them when you exit.

  \param[in] properties MLAudioSpatialSoundRoomProperties struct to set the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundRoomProperties(const MLAudioSpatialSoundRoomProperties *properties);

/*!
  \brief Returns the room acoustics shared by all spatial sounds.

  For sound outputs that use 3D audio processing this function reads
  the acoustic properties of the "room" that all spatial sounds share.

  \note For the time being (until a future update), this global acoustic
  setting applies to all LuminRT and native applications and services.
  You should only use this call if you are an immersive app that needs
  specific acoustical properties. In that case, you should be sure to
  "get" the current settings first, and restore them when you exit.

  \param[out] out_properties MLAudioSpatialSoundRoomProperties struct to return the properties

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundRoomProperties(MLAudioSpatialSoundRoomProperties *out_properties);

/*!
  \brief Sets the control frequencies used for frequency-based settings.

  For sound outputs that use 3D audio processing this function sets
  the control frequencies (lf, mf, hf) that all spatial sounds share.

  \note For the time being (until a future update), this global acoustic
  setting applies to all LuminRT and native applications and services.
  You should only use this call if you are an immersive app that needs
  specific acoustical properties. In that case, you should be sure to
  "get" the current settings first, and restore them when you exit.

  \param[in] frequencies MLAudioSpatialSoundControlFrequencies struct to set the frequencies

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSpatialSoundControlFrequencies(const MLAudioSpatialSoundControlFrequencies *frequencies);

/*!
  \brief Returns the control frequencies used for frequency-based settings.

  For sound outputs that use 3D audio processing this function reads
  the control frequencies (lf, mf, hf) that all spatial sounds share.

  \note For the time being (until a future update), this global acoustic
  setting applies to all LuminRT and native applications and services.
  You should only use this call if you are an immersive app that needs
  specific acoustical properties. In that case, you should be sure to
  "get" the current settings first, and restore them when you exit.

  \param[out] out_frequencies MLAudioSpatialSoundControlFrequencies struct to return the
  frequencies

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSpatialSoundControlFrequencies(MLAudioSpatialSoundControlFrequencies *out_frequencies);

/*!
  \brief Sets the volume of a sound output using a linear scale.

  The range of the volume is 0 to 8, with 0 for silence,
  1 for unity gain, and 8 for 8x gain.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] volume the volume that the sound output will be set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundVolumeLinear(MLHandle handle, float volume);

/*!
  \brief Returns the volume of a sound output using a linear scale.

  The range of the volume is 0 to 8, with 0 for silence,
  1 for unity gain, and 8 for 8x gain.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_volume the volume that the sound output is set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSoundVolumeLinear(MLHandle handle, float *out_volume);

/*!
  \brief Sets the volume of a sound output using a decibel scale.

  The range of the volume is -100 db to +18 dB, with -100 dB for
  silence, 0 dB for unity gain, and +18 dB for 8x gain.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] volume the volume that the sound output will be set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundVolumeDb(MLHandle handle, float volume);

/*!
  \brief Returns the volume of a sound output using a decibel scale.

  The range of the volume is -100 db to +18 dB, with -100 dB for
  silence, 0 dB for unity gain, and +18 dB for 8x gain.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_volume the volume that the sound output is set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSoundVolumeDb(MLHandle handle, float *out_volume);

/*!
  \brief Sets the pitch of a sound output.

  The range of the pitch is 0.5 to 2.0, with 0.5 being one octave down
  and 2.0 being one octave up (i.e., the pitch is a frequency multiple).
  A pitch of 1.0 is the default and means no change.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] pitch the pitch that the sound output will be set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundPitch(MLHandle handle, float pitch);

/*!
  \brief Returns the pitch of a sound output.

  The range of the pitch is 0.5 to 2.0, with 0.5 being one octave down
  and 2.0 being one octave up (i.e., the pitch is a frequency multiple).
  A pitch of 1.0 is the default and means no change.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_pitch the pitch that the sound output is set to

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetSoundPitch(MLHandle handle, float *out_pitch);

/*!
  \brief Mutes or un-mutes a sound output.

  This mute control is entirely under the control of the owner
  of the sound output, and is separate from any muting done by
  the audio policy manager.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] is_muted determines whether the sound output is muted or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundMute(MLHandle handle, bool is_muted);

/*!
  \brief Returns whether a sound output is muted or not.

  This mute control is entirely under the control of the owner
  of the sound output, and will not indicate whether the sound
  output has been muted by the audio policy manager. To find out
  about the latter, use MLAudioSetSoundEventCallback to register
  a callback that will provide "policy mute" events.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_is_muted indicates whether the sound output is muted or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioIsSoundMuted(MLHandle handle, bool *out_is_muted);

/*!
  \brief Sets whether a sound output is looping or not.

  Looping is available for sound outputs that play files (loaded,
  streamed, system sounds). The loop goes from the very beginning
  to the very end of the file. Looping does not apply to sound
  outputs that are output streams.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] is_looping determines whether the sound output is looping or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetSoundLooping(MLHandle handle, bool is_looping);

/*!
  \brief Returns whether a sound output is looping or not.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_is_looping indicates whether the sound output is looping or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioIsSoundLooping(MLHandle handle, bool *out_is_looping);

/*!
  \brief Sets the playback position for a streamed file.

  This function will cause a streaming file to jump to a new
  location. The playback position is specified in milliseconds.
  This function can be used while a streaming file is playing,
  paused or stopped.

  \param[in] handle MLHandle used to identify the sound output
  \param[in] offset_in_msec sets the new playback position

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetStreamedFileOffset(MLHandle handle, uint32_t offset_in_msec);

/*!
  \brief Returns the playback position for a streamed file.

  This function can be used while a streaming file is playing,
  paused or stopped.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_offset_in_msec indicates the current playback position

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetStreamedFileOffset(MLHandle handle, uint32_t *out_offset_in_msec);

/*!
  \brief Returns whether sound output is being sent to the speakers or earphones.

  This is a global configuration that is not specific to individual sound outputs.

  \param[out] out_current_device indicates whether output goes to wearable or analog jack

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetOutputDevice(MLAudioOutputDevice *out_current_device);

/*!
  \brief Returns the preferred (i.e. best performing) settings for streaming.

  \param[in] channel_count number of audio channels (e.g. 2 for stereo)
  \param[in] samples_per_second sample rate for the audio stream
  \param[in] max_pitch_change top pitch (1.0 - 2.0) for the audio stream
  \param[out] out_format recommended settings for MLAudioBufferFormat
  \param[out] out_recommended_size recommended size for the streaming buffers
  \param[out] out_min_size minimum allowable size for the streaming buffers

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetOutputStreamDefaults(uint32_t channel_count, uint32_t samples_per_second, float max_pitch_change, MLAudioBufferFormat *out_format, uint32_t *out_recommended_size, uint32_t *out_min_size);

/*!
  \brief Returns the latency for an output stream.

  This function returns the latency between the time a buffer
  is passed to the stream via MLAudioReleaseOutputStreamBuffer
  and the time its audio is heard at the speakers or earphones.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_latency_in_msec indicates the latency in milliseconds

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetOutputStreamLatency(MLHandle handle, float *out_latency_in_msec);

/*!
  \brief Returns the number of audio frames played by an output stream.

  This function returns the number of frames that have been sent
  down to the audio mixer (-not- the number that have been passed
  to the stream buffers) since the last time MLAudioStartSound
  was called for this stream. Note that the perceived sound
  may be slightly behind this value due to the stream latency
  (which can be queried by calling MLAudioGetOutputStreamLatency).
  Pausing and resuming does not affect the frame count.

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_frames_played the number of audio frames played

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetOutputStreamFramesPlayed(MLHandle handle, uint64_t *out_frames_played);

/*!
  \brief Returns an empty buffer for writing more stream data.

  This function returns a buffer when one of the two streaming buffers
  is empty and needs more data. It is possible to call this function in
  a polling fashion. However for greater efficiency it is recommended
  to provide a callback to the 'create' function (in which case this
  function may be called from within the callback).

  \param[in] handle MLHandle used to identify the sound output
  \param[out] out_buf MLAudioBuffer containing the buf to write into

  \retval MLResult_OK if buffer available and false if not, or if fail
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetOutputStreamBuffer(MLHandle handle, MLAudioBuffer *out_buf);

/*!
  \brief Releases a buffer once it has been filled.

  After receiving an empty buffer from MLAudioGetOutputStreamBuffer
  and writing audio data into that buffer, call this function to
  indicate that the buffer has been filled and can now be played.

  \param[in] handle MLHandle used to identify the sound output

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioReleaseOutputStreamBuffer(MLHandle handle);

/*!
  \brief Returns the master volume for the audio system.

  The range of the volume is 0-10, with 0 being silent and 10 being full volume.

  \param[out] out_volume the current setting of master volume

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetMasterVolume(float *out_volume);

/*!
  \brief Registers a callback for changes to the master volume.

  The callback will be called whenever the master volume
  gets changed (even if changed by the owner of the callback).
  The 'callback_context' argument is passed back to the callback
  as its first argument, and the new master volume setting is
  passed to the callback as its second argument.

  \param[in] callback MLAudioMasterVolumeChangedCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioSetMasterVolumeCallback(MLAudioMasterVolumeChangedCallback callback, void *callback_context);

/*!
  \brief Creates a new sound input that provides a "voice communications" stream.

  Creates a sound input and allocates a double-buffer for streaming audio
  data from it. The "voice communications" stream provides the user's voice
  processed for communications, with other sounds rejected. It is an
  inherently mono stream, so if stereo is requested the mono data will
  be duplicated in both channels. To stream  data from the buffers call
  MLAudioGetInputStreamBuffer to get a full buffer, then read the data
  from the buffer, then call MLAudioReleaseInputStreamBuffer to indicate
  that the buffer is ready to re-use. This can be done either on a polling
  basis or within a callback set via the 'callback' argument.  The 'format'
  argument specifies the format of which the following are supported:
  a single mono channel, 16 bit integer, a sampling rate of 16k. The
  'buffer_size' argument will use the recommended size if 0 is passed in.
  After creation, the input can be started by calling MLAudioStartInput.

  \param[in] format MLAudioBufferFormat specifying the audio format of the stream
  \param[in] buffer_size requested size in bytes for each of the two stream buffers
  \param[in] callback MLAudioBufferCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback
  \param[out] out_handle MLHandle used in subsequent calls for this sound input

  \retval MLResult_OK if input creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if AudioCaptureMic privilege is not allowed.

  \priv AudioCaptureMic
*/
ML_API MLResult ML_CALL MLAudioCreateInputFromVoiceComm(const MLAudioBufferFormat *format, uint32_t buffer_size, MLAudioBufferCallback callback, void *callback_context, MLHandle *out_handle);

/*!
  \brief Creates a new sound input that provides a "world capture" stream.

  Creates a sound input and allocates a double-buffer for streaming audio
  data from it. The "world capture" stream provides a stereo stream
  capturing the user's surroundings. To stream  data from the buffers call
  MLAudioGetInputStreamBuffer to get a full buffer, then read the data
  from the buffer, then call MLAudioReleaseInputStreamBuffer to indicate
  that the buffer is ready to re-use. This can be done either on a polling
  basis or within a callback set via the 'callback' argument. The 'format'
  argument specifies the format of which the following are supported:
  two channels (stereo), 16 bit integer, a sampling rate of 48k. The
  'buffer_size' argument will use the recommended size if 0 is passed in.
  After creation, the input can be started by calling MLAudioStartInput.

  \param[in] format MLAudioBufferFormat specifying the audio format of the stream
  \param[in] buffer_size requested size in bytes for each of the two stream buffers
  \param[in] callback MLAudioBufferCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback
  \param[out] out_handle MLHandle used in subsequent calls for this sound input

  \retval MLResult_OK if input creation was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if AudioCaptureMic privilege is denied.

  \priv AudioCaptureMic
*/
ML_API MLResult ML_CALL MLAudioCreateInputFromWorldCapture(const MLAudioBufferFormat *format, uint32_t buffer_size, MLAudioBufferCallback callback, void *callback_context, MLHandle *out_handle);

/*!
  \brief Destroys a previously created sound input.

  \param[in] handle MLHandle used to identify the sound input

  \retval MLResult_OK if input destruction was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioDestroyInput(MLHandle handle);

/*!
  \brief Starts capture for a sound input.

  \param[in] handle MLHandle used to identify the sound input

  \retval MLResult_OK if starting was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioStartInput(MLHandle handle);

/*!
  \brief Stops capture for a sound input.

  \param[in] handle MLHandle used to identify the sound input

  \retval MLResult_OK if stopping was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioStopInput(MLHandle handle);

/*!
  \brief Indicates the play state of a sound input.

  The MLAudioState indicates whether the sound input is
  stopped or playing.

  \param[in] handle MLHandle used to identify the sound input
  \param[out] out_state indicates the current play state

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioGetInputState(MLHandle handle, MLAudioState *out_state);

/*!
  \brief Registers a callback for events on a sound input.

  Currently only one event can result in a callback being called:
  a sound input being muted or unmuted by the audio policy manager.
  The 'callback_context' argument is passed back to the callback
  as its first argument. The particular event type (MLAudioEvent)
  that caused the callback is passed to the callback as its second
  argument.

  \param[in] handle MLHandle used to identify the sound input
  \param[in] callback MLAudioEventCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioSetInputEventCallback(MLHandle handle, MLAudioEventCallback callback, void *callback_context);

/*!
  \brief Returns the preferred (i.e. best performing) settings for streaming.

  \param[in] channel_count number of audio channels (e.g. 2 for stereo)
  \param[in] samples_per_second sample rate for the audio stream
  \param[out] out_format recommended settings for MLAudioBufferFormat
  \param[out] out_recommended_size recommended size for the streaming buffers
  \param[out] out_min_size minimum allowable size for the streaming buffers

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.

  \priv none
*/
ML_API MLResult ML_CALL MLAudioGetInputStreamDefaults(uint32_t channel_count, uint32_t samples_per_second, MLAudioBufferFormat *out_format, uint32_t *out_recommended_size, uint32_t *out_min_size);

/*!
  \brief Returns the latency for a sound input.

  This function returns the latency between the time a sound
  occurs in the real world and the time it becomes available
  via MLAudioGetInputStreamBuffer.

  \param[in] handle MLHandle used to identify the sound input
  \param[out] out_latency_in_msec indicates the latency in milliseconds

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioGetInputStreamLatency(MLHandle handle, float *out_latency_in_msec);

/*!
  \brief Returns a full buffer containing captured audio data.

  This function returns a buffer when one of the two streaming buffers
  is full and needs to be read. It is possible to call this function in
  a polling fashion. However for greater efficiency it is recommended
  to provide a callback to the 'create' function (in which case this
  function may be called from within the callback).

  \param[in] handle MLHandle used to identify the sound input
  \param[out] out_buf MLAudioBuffer containing the buf to read from

  \retval MLResult_OK if buffer available and false if not, or if fail
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioGetInputStreamBuffer(MLHandle handle, MLAudioBuffer *out_buf);

/*!
  \brief Releases a buffer once it has been read.

  After receiving a full buffer from MLAudioGetInputStreamBuffer and
  reading the audio data from that buffer, call this function to
  indicate that the buffer has been read and can now be re-used.

  \param[in] handle MLHandle used to identify the sound input

  \retval MLResult_OK if call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if original privilege is denied.

  \priv requires same priv as was used to create handle
*/
ML_API MLResult ML_CALL MLAudioReleaseInputStreamBuffer(MLHandle handle);

/*!
  \brief Mutes or un-mutes all microphone capture.

  This mute control applies to all microphone capture
  in general (excluding "recognizer voice" which is
  always active). When mic capture is muted or unmuted by
  one app, it is muted or unmuted for all apps. Note:
  this setting is separate from any muting done by
  the audio policy manager (such as when the "reality
  button" is pressed).

  \param[in] is_muted determines whether mic capture is muted or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if AudioCaptureMic privilege is denied.

  \priv AudioCaptureMic
*/
ML_API MLResult ML_CALL MLAudioSetMicMute(bool is_muted);

/*!
  \brief Returns whether all microphone capture is muted or not.

  This function will not indicate whether the microphone capture
  has been muted by the audio policy manager. To find out
  about the latter, use MLAudioSetInputEventCallback to register
  a callback that will provide "policy mute" events.

  \param[out] out_is_muted indicates whether the mic capture is muted or not

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if AudioCaptureMic privilege is denied.

  \priv AudioCaptureMic
*/
ML_API MLResult ML_CALL MLAudioIsMicMuted(bool *out_is_muted);

/*!
  \brief Registers a callback for changes to the global mic mute.

  The callback will be called whenever the global mic mute
  gets changed (even if changed by the owner of the callback).
  The 'callback_context' argument is passed back to the callback
  as its first argument, and the new mic mute setting is passed
  to the callback as its second argument.

  \param[in] callback MLAudioMicMuteCallback function pointer
  \param[in] callback_context generic data pointer passed back to callback

  \retval MLResult_OK if the call was successful
  \retval MLResult_UnspecifiedFailure if internal error occurs.
  \retval MLResult_InvalidParam if input parameter is invalid.
  \retval MLResult_PrivilegeDenied if AudioCaptureMic privilege is denied.

  \priv AudioCaptureMic
*/
ML_API MLResult ML_CALL MLAudioSetMicMuteCallback(MLAudioMicMuteCallback callback, void *callback_context);

/*!
  \deprecated This function has been deprecated, and scheduled for removal.
*/
ML_DEPRECATED ML_API MLResult ML_CALL MLAudioCreateInputFromVirtualCapture(const MLAudioBufferFormat *format, uint32_t buffer_size, MLAudioBufferCallback callback, void *callback_context, MLHandle *out_handle);

/*!
  \deprecated This function has been deprecated, and scheduled for removal.
*/
ML_DEPRECATED ML_API MLResult ML_CALL MLAudioCreateInputFromMixedCapture(const MLAudioBufferFormat *format, uint32_t buffer_size, MLAudioBufferCallback callback, void *callback_context, MLHandle *out_handle);

/*! \}*/

ML_EXTERN_C_END
