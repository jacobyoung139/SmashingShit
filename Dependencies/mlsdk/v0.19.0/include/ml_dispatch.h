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
#include "ml_fileinfo.h"

ML_EXTERN_C_BEGIN

/*!
  \addtogroup Dispatch
  \brief This interface lets an application query the platform to handle things
  that the app itself cannot or wants someone else to handle.
  For example, if an application comes across a schema tag that it doesn't know
  what to do with, it can query the platform to see if some other application might.
  This can be useful for handling http://, https:// or other custom Schema://<arg1>/<arg2>
  Apart from handling schema tags in URIs, this interface can also be used
  to query the platform to handle a type of file based on file-extension or mime-type.

  \{
*/

/*! Defines the prefix for MLDispatchResult codes. */
enum {
  MLResultAPIPrefix_Dispatch = MLRESULT_PREFIX(0xBBE0)
};

typedef enum MLDispatchResult {
  MLDispatchResult_CannotStartApp = MLResultAPIPrefix_Dispatch,
  MLDispatchResult_InvalidPacket,
  MLDispatchResult_NoAppFound,
  MLDispatchResult_AppPickerDialogFailure,
  MLDispatchResult_Ensure32Bits = 0x7FFFFFFF
} MLDispatchResult;

/*!
  \brief #MLDispatchPacket type can be used with this interface.
  It can be used to pass a URI string or #MLFileInfo objects
  to the platform.
*/
typedef struct MLDispatchPacket MLDispatchPacket;

/*!
  \brief Create empty dispatch packet.

  The MLFileInfo array in the #MLDispatchPacket returned by this function
  will be NULL as will the uri. In order to allocate memory for these
  members, caller will have to explicitly call #MLDispatchAllocateFileInfoList
  and #MLDispatchAllocateUri respectively.

  The caller will need to free this structure by calling
  #MLDispatchReleasePacket.

  \param[out] out_packet a pointer to #MLDispatchPacket structure on success and NULL on failure.
  \retval MLResult_Ok if dispatch packet was allocated successfully
  \retval MLResult_AllocFailed if failed to allocate dispatch packet
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchAllocateEmptyPacket(MLDispatchPacket **out_packet);

/*!
  \brief Release the #MLDispatchPacket that is allocated by #MLDispatchAllocateEmptyPacket
  and all its resources. The pointer to the #MLDispatchPacket struct will point to NULL after this call.
  \param[in] packet Pointer to #MLDispatchPacket struct pointer.
  \param[in] release_members If \c true, function will attempt to release/free
             #MLFileInfo array and uri members from the MLDispatchPacket.
  \param[in] close_fds If \c true, function will attempt to close the fds in #MLFileInfo,
             If \c false, caller will have to close fds.
  \retval MLResult_Ok if memory allocated to dispatch packet is released successfully
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchReleasePacket(MLDispatchPacket **packet, bool release_members, bool close_fds);

/*!
  \brief Allocate an empty #MLFileInfo array in the #MLDispatchPacket for given length.

  The caller can release/free by calling MLDispatchReleaseFileInfo() or by calling
  MLDispatchReleasePacket()

  \param[in,out] packet Pointer to #MLDispatchPacket whose #MLFileInfo* member will be
                 allocated file_info_list_length entries.
  \param[in] file_info_list_length Maximum length of the file info array to be allocated.

  \retval MLResult_Ok if #MLFileInfo array is successfully allocated file_info_list_length
          entries.
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_AllocFailed if allocation of #MLFileInfo array failed
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchAllocateFileInfoList(MLDispatchPacket *packet, uint64_t file_info_list_length);

/*!
  \brief This API retrieves length of the #MLFileInfo array in the given #MLDispatchPacket.

  This function can return length of 0 which implies there is no file info
  available.

  \param[in] packet Pointer to #MLDispatchPacket whose #MLFileInfo* member's list length
             is calculated and stored in out_file_info_list_length
  \param[out] out_file_info_list_length length of #MLFileInfo array
  \retval MLResult_Ok if operation is successful
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchGetFileInfoListLength(const MLDispatchPacket *packet, uint64_t *out_file_info_list_length);

/*!
  \brief Get the #MLFileInfo at the given index.

  The #MLFileInfo array should have been allocated by calling MLDispatchAllocateFileInfoList().
  \param[in] packet Pointer to #MLDispatchPacket whose #MLFileInfo array for given index will be returned.
  \param[in] index Index of the #MLFileInfo array.
  \param[out] out_file_info Pointer to #MLFileInfo* in packet at index.
  \retval MLResult_Ok if the #MLFileInfo object at the parameter index can be retrieved
          successfully
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchGetFileInfoByIndex(const MLDispatchPacket *packet, int64_t index, MLFileInfo **out_file_info);

/*!
  \brief Populate #MLFileInfo array in the #MLDispatchPacket for current index.

  The caller will have to call MLDispatchAllocateFileInfoList() before calling this
  function. After obtaining the length of the list through MLDispatchGetFileInfoListLength(),
  the caller should get each #MLFileInfo structures in the array through
  MLDispatchGetFileInfoByIndex(). After setting the fields of #MLFileInfo using mutator
  functions, this function should be called to add the #MLFileInfo just set to the
  dispatch packet.

  \param[in,out] packet Pointer to #MLDispatchPacket whose #MLFileInfo* member will be populated.
  \param[in] finfo #MLFileInfo structure that will be added to the #MLDispatchPacket.
  \retval MLResult_Ok if parameter finfo was successfully added to #MLFileInfo array in the packet
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_AllocFailed if failed to allocate memory for one of the fields in #MLFileInfo
          or there are no available slots in the #MLFileInfo array in the packet
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchAddFileInfo(MLDispatchPacket *packet, const MLFileInfo *finfo);

/*!
  \brief Release the pointer to #MLFileInfo array that is allocated by MLDispatchAllocateFileInfoList().

  The caller will have to call MLDispatchAllocateFileInfoList() before calling this
  function. The #MLFileInfo pointer in #MLDispatchPacket will point to NULL after
  this call.

  \param[in] packet Pointer to MLDispatchPacket.
  \param[in] close_fds If \c true, function will attempt to close the fds in #MLFileInfo,
             if \c false, caller will have to close fds.
  \retval MLResult_Ok if memory allocated to #MLFileInfo array was released successfully
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchReleaseFileInfoList(MLDispatchPacket *packet, bool close_fds);

/*!
  \brief Allocate and assign URI in the #MLDispatchPacket.

  The caller can release/free by calling MLDispatchReleaseUri() or by calling
  MLDispatchReleasePacket().

  \param[in,out] packet Pointer to #MLDispatchPacket whose uri member will be allocated and populated.
  \param[in] uri Value assigned to #MLDispatchPacket's uri member.
  \retval MLResult_Ok if operation is succesfful
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_AllocFailed if failed to allocate uri
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchSetUri(MLDispatchPacket *packet, const char *uri);

/*!
  \brief Release uri that is allocated by MLDispatchAllocateUri().

  The caller will have to call MLDispatchSetUri() before calling this
  function. The char pointer uri in #MLDispatchPacket will point to NULL after
  this call.

  \param[in] packet Pointer to #MLDispatchPacket struct pointer.
  \retval MLResult_Ok if operation is successful
  \retval MLResult_InvalidParam if a function parameter is not valid
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchReleaseUri(MLDispatchPacket *packet);

/*!
  \brief Try to open the application that supports a given mime type or schema type.

  If the caller does not specify a mime-type or schema type in the dispatch packet,
  dispatch service will try to open an application which supports the file extension
  specified in the file name.

  \param[in] packet Pointer to #MLDispatchPacket structure.
  \retval MLResult_Ok if an application was found that can open a given mime or schema type,
  \retval MLDispatchResult Dispatch specific error occurred.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \priv none
*/
ML_API MLResult ML_CALL MLDispatchTryOpenApplication(const MLDispatchPacket *packet);

/*!
  \brief Returns an ascii string for MLDispatchResult and MLResultGlobal codes.
  \param[in] result_code The input MLResult enum from MLDispatch functions.
  \return ascii string containing readable version of result code
  \priv none
*/
ML_API const char* ML_CALL MLDispatchGetResultString(MLResult result_code);

/*! \} */

ML_EXTERN_C_END
