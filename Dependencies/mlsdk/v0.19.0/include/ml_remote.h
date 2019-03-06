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

ML_EXTERN_C_BEGIN

/*!
  \addtogroup Remote
  \brief APIs for the ML Remote on Host OS Platforms.

  \attention These APIs are meant to enable a better integration to ML Remote for
             engines running on Host OS Platforms. The library is not available on
             device.
  \{
*/

/*!
  \brief Checks to see if ML Remote server is running.
  \param[out] out_is_configured Pointer to a bool indicating whether the remote
              server is running and is configured properly
  \retval MLResult_Ok If query was successful.
  \retval MLResult_InvalidParam is_configured parameter is not valid (null).
  \retval MLResult_Timeout The ML Remote server could not be reached.
  \retval MLResult_UnspecifiedFailure There was an unknown error submitting the query.
  \priv none
*/

ML_API MLResult ML_CALL MLRemoteIsServerConfigured(bool *out_is_configured);

#ifdef VK_VERSION_1_0
/*!
  \brief Returns a list of required VkInstance extensions.

  If out_required_extension_properties is null then the number of required extension is returned in out_extension_property_count.
  Otherwise, out_extension_property_count must point to a variable set to the number of elements in the out_required_extension_properties
  array, and on return the variable is overwritten with the number of strings actually written to out_required_extension_properties.

  \param[out] out_required_extension_properties Either null or a pointer to an array of VkExtensionProperties.
  \param[out] out_extension_property_count Is a pointer to an integer related to the number of extensions required or queried.
  \retval MLResult_Ok If query was successful.
  \retval MLResult_InvalidParam If input parameter is invalid.
  \retval MLResult_UnspecifiedFailure There was an unknown error submitting the query.
  \priv none
*/

ML_API MLResult ML_CALL MLRemoteEnumerateRequiredVkInstanceExtensions(VkExtensionProperties *out_required_extension_properties, uint32_t *out_extension_property_count);

/*!
  \brief Returns a list of required VkDevice extensions.

  If out_required_extension_properties is null then the number of required extension is returned in out_extension_property_count.
  Otherwise, out_extension_property_count must point to a variable set to the number of elements in the out_required_extension_properties
  array, and on return the variable is overwritten with the number of strings actually written to out_required_extension_properties.

  \param[out] out_required_extension_properties Either null or a pointer to an array of VkExtensionProperties.
  \param[out] out_extension_property_count Is a pointer to an integer related to the number of extensions required or queried.
  \retval MLResult_Ok If query was successful.
  \retval MLResult_InvalidParam If input parameter is invalid.
  \retval MLResult_UnspecifiedFailure There was an unknown error submitting the query.
  \priv none
*/
ML_API MLResult ML_CALL MLRemoteEnumerateRequiredVkDeviceExtensions(VkExtensionProperties *out_required_extension_properties, uint32_t *out_extension_property_count);
#endif

/*! \} */

ML_EXTERN_C_END
