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
#include "ml_types.h"

ML_EXTERN_C_BEGIN

/*!
  \addtogroup Meshing2
  \brief APIs for the Meshing system.

  - The Meshing system is for generating a mesh representation of the real world.

  \attention EXPERIMENTAL

  \{
*/

/*! Request flags for the meshing system */
typedef enum MLMeshingFlags {
  /*! If set, will return a point cloud instead of a triangle mesh.  */
  MLMeshingFlags_PointCloud = 1 << 0,
  /*! If set, the system will compute the normals for the triangle vertices.  */
  MLMeshingFlags_ComputeNormals = 1 << 1,
  /*! If set, the system will compute the confidence values.  */
  MLMeshingFlags_ComputeConfidence = 1 << 2,
  /*! If set, the system will planarize the returned mesh (planar regions
      will be smoothed out).
  */
  MLMeshingFlags_Planarize = 1 << 3,
  /*! If set, the mesh skirt (overlapping area between two mesh blocks) will be removed.  */
  MLMeshingFlags_RemoveMeshSkirt = 1 << 4,
  /*! If set, winding order of indices will be be changed from clockwise to counter clockwise.
      This could be useful for face culling process in different engines.
  */
  MLMeshingFlags_IndexOrderCCW = 1 << 5,
  /*! Ensure enum is represented as 32 bits. */
  MLMeshingFlags_Ensure32Bits = 0x7FFFFFFF
} MLMeshingFlags;

/*! Level of detail of the block mesh. */
typedef enum MLMeshingLOD {
  /*! Minimum Level of Detail (LOD) for the mesh. */
  MLMeshingLOD_Minimum,
  /*! Medium Level of Detail (LOD) for the mesh. */
  MLMeshingLOD_Medium,
  /*! Maximum Level of Detail (LOD) for the mesh. */
  MLMeshingLOD_Maximum,
  /*! Ensure enum is represented as 32 bits. */
  MLMeshingLOD_Ensure32Bits = 0x7FFFFFFF
} MLMeshingLOD;

/*! Result of a mesh request. */
typedef enum MLMeshingResult {
  /*! Mesh request has succeeded. */
  MLMeshingResult_Success,
  /*! Mesh request has failed.  */
  MLMeshingResult_Failed,
  /*! Mesh request is pending.  */
  MLMeshingResult_Pending,
  /*! There are partial updates on the mesh request.  */
  MLMeshingResult_PartialUpdate,
  /*! Ensure enum is represented as 32 bits. */
  MLMeshingResult_Ensure32Bits = 0x7FFFFFFF
} MLMeshingResult;

/*! State of a block mesh. */
typedef enum MLMeshingMeshState {
  /*! Mesh has been created */
  MLMeshingMeshState_New,
  /*! Mesh has been updated. */
  MLMeshingMeshState_Updated,
  /*! Mesh has been deleted. */
  MLMeshingMeshState_Deleted,
  /*! Mesh is unchanged. */
  MLMeshingMeshState_Unchanged,
  /*! Ensure enum is represented as 32 bits. */
  MLMeshingMeshState_Ensure32Bits = 0x7FFFFFFF
} MLMeshingMeshState;

/*! Mesh Settings for the underlying system. */
typedef struct MLMeshingSettings {
  /*! Request flags that are a combination of MLMeshingFlags.  */
  uint32_t flags;
  /*! Perimeter (in meters) of holes you wish to have filled. */
  float fill_hole_length;
  /*! Any component that is disconnected from the main mesh and
      which has an area (in meters^2) less than this size will be removed.
  */
  float disconnected_component_area;
} MLMeshingSettings;

/*! Axis aligned bounding box for querying updated mesh info. */
typedef struct MLMeshingExtents {
  /*! The center of the bounding box. */
  MLVec3f center;
  /*! The rotation of the bounding box.*/
  MLQuaternionf rotation;
  /*! The size of the bounding box. */
  MLVec3f extents;
} MLMeshingExtents;

/*! Representation of a mesh block. */
typedef struct MLMeshingBlockInfo {
  /*! The coordinate frame UID to represent the block. */
  MLCoordinateFrameUID id;
  /*! The extents of the bounding box. */
  MLMeshingExtents extents;
  /*! The timestamp when block was updated. */
  uint64_t timestamp;
  /*! The state of the Mesh Block. */
  MLMeshingMeshState state;
} MLMeshingBlockInfo;

/*! Response structure for the mesh block info. */
typedef struct MLMeshingMeshInfo
{
  /*! The response timestamp to a earlier request. */
  uint64_t timestamp;
  /*! The number of responses in data pointer. */
  uint32_t data_count;
  /*! The meshinfo returned by the system. */
  MLMeshingBlockInfo *data;
} MLMeshingMeshInfo;

/*! Request structure to get the actual mesh for a block. */
typedef struct MLMeshingBlockRequest {
  /*! The UID to represent the block. */
  MLCoordinateFrameUID id;
  /*! The LOD level to request. */
  MLMeshingLOD level;
} MLMeshingBlockRequest;

/*! Request structure to get the actual mesh for a set of blocks. */
typedef struct MLMeshingMeshRequest {
  /*! The number of blocks requested. */
  int request_count;
  /*! Per block request. */
  MLMeshingBlockRequest *data;
} MLMeshingMeshRequest;

/*! Final structure for a block mesh. */
typedef struct MLMeshingBlockMesh {
  /*! The result of the meshing. */
  MLMeshingResult result;
  /*! The coordinate FrameID of the block. */
  MLCoordinateFrameUID id;
  /*! The LOD level of the meshing request. */
  MLMeshingLOD level;
  /*! The settings with which meshing took place. */
  uint32_t flags;
  /*! The number of indices in index buffer. */
  uint16_t index_count;
  /*! The number of vertices in vertex/normal/confidence buffer. */
  uint32_t vertex_count;
  /*! Pointer to the vertex buffer. */
  MLVec3f *vertex;
  /*! Pointer to the index buffer. */
  uint16_t *index;
  /*! Pointer to normals buffer. */
  MLVec3f *normal;
  /*! Pointer to confidence buffer. */
  float *confidence;
} MLMeshingBlockMesh;

typedef struct MLMeshingMesh {
  /*! The result of the meshing. Can have partial updates*/
  MLMeshingResult result;
  /*! The timestamp when data was generated */
  uint64_t timestamp;
  /*! Number of meshes available in data */
  uint32_t data_count;
  /*! The mesh data */
  MLMeshingBlockMesh *data;
} MLMeshingMesh;

/*!
  \brief Create the meshing client

  Note that this will be the only function in the meshing API that will return MLResult_PrivilegeDenied.
  Trying to call the other functions with an invalid MLHandle will result in MLResult_InvalidParam.

  \param out_client_handle The handle to the created client
  \param settings The initial settings to be used for meshing.
  \retval MLResult_Ok Meshing Client was created successfully.
  \retval MLResult_PrivilegeDenied Client was not created due to insufficient privilege.
  \retval MLResult_InvalidParam Meshing Client was not created due to an invalid parameter.
  \retval MLResult_UnspecifiedFailure Meshing Client was not created due to an unknown error
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingCreateClient(MLHandle *out_client_handle, const MLMeshingSettings *settings);

/*!
  \brief Free the client resources.

  \param client_handle The client to destroy
  \retval MLResult_Ok Meshing Client was destroyed successfully.
  \retval MLResult_InvalidParam Meshing Client was not destroyed due to an invalid parameter
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingDestroyClient(MLHandle *client_handle);

/*!
  \brief Initialize the meshing settings with system defaults

  \param out_settings The initial settings to be used for meshing.
  \retval MLResult_Ok Mesh Settings were initialized successfully.
  \retval MLResult_InvalidParam Mesh Settings were not initialized due to an invalid parameter.
  \priv none
*/
ML_API MLResult ML_CALL MLMeshingInitSettings(MLMeshingSettings *out_settings);

/*!
  \brief Update the meshing settings at runtime

  \param client_handle The handle to the created client
  \param settings The updated settings to be used for meshing.
  \retval MLResult_Ok Mesh Settings were updated successfully.
  \retval MLResult_InvalidParam Mesh Settings were not updated due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingUpdateSettings(const MLHandle client_handle, const MLMeshingSettings *settings);

/*!
  \brief Request the Mesh Info which includes CFUIDs and bounding extents of the blocks

  \param client_handle The handle to the created client
  \param extents The region of interest for meshing.
  \param out_request_handle The handle for the current request. Needs to be passed to query the result of the request
  \retval MLResult_Ok Mesh Info was requested successfully.
  \retval MLResult_InvalidParam Mesh info was not requested due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingRequestMeshInfo(const MLHandle client_handle, const MLMeshingExtents *extents, MLHandle *out_request_handle);

/*!
  \brief Get the Result of a previous MeshInfo request

  \param client_handle The handle to the created client
  \param request_handle The handle populated in a prev MLMeshingGetMeshInfo.
  \param out_info The final result which will be populated only if the result is successful
  \retval MLResult_Ok Mesh Info was populated successfully.
  \retval MLResult_Peding Mesh Info is pending update.
  \retval MLResult_InvalidParam Mesh Settings were not updated due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingGetMeshInfoResult(const MLHandle client_handle, const MLHandle request_handle, MLMeshingMeshInfo *out_info);

/*!
  \brief Request the Mesh for all CFUIDs populated in request

  \param client_handle The handle to the created client
  \param request The request for meshes of interest.
  \param out_request_handle The handle for the current request. Needs to be passed to query the result of the request
  \retval MLResult_Ok Meshes were requested successfully.
  \retval MLResult_InvalidParam Meshes were not requested due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingRequestMesh(const MLHandle client_handle, const MLMeshingMeshRequest *request, MLHandle *out_request_handle);

/*!
  \brief Get the Result of a previous Mesh request

  \param client_handle The handle to the created client
  \param request_handle The handle populated in a prev MLMeshingGetMesh.
  \param out_mesh The final result which will be populated only if the result is successful
  \retval MLResult_Ok Meshes was populated successfully.
  \retval MLResult_Peding Meshes pending update.
  \retval MLResult_InvalidParam Meshes were not updated due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingGetMeshResult(const MLHandle client_handle, const MLHandle request_handle, MLMeshingMesh *out_mesh);

/*!
  \brief Free resources created by the meshing APIS. Needs to be called whenever MLMeshingGetMeshInfoResult,
         MLMeshingGetMeshResult return a success.

  \param client_handle The handle to the created client
  \param request_handle The handle populated in a prev request.
  \retval MLResult_Ok Resources were freed successfully.
  \retval MLResult_InvalidParam Resources were not freed due to an invalid parameter.
  \priv WorldReconstruction
*/
ML_API MLResult ML_CALL MLMeshingFreeResource(const MLHandle client_handle, MLHandle *request_handle);

/*! \} */

ML_EXTERN_C_END
