using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR.MagicLeap;

public class SurfaceRenderScript : MonoBehaviour {

    #region Public Variables
    public float _planeDistanceThreshold;
    public Transform BBoxTransform;
    public Vector3 BBoxExtents;
    public GameObject PlanePrefab;
    [BitMask(typeof(MLWorldPlanesQueryFlags))]
    public MLWorldPlanesQueryFlags QueryFlags;
    #endregion

    #region Private Variables
    private float timeout = 5f;
    private float timeSinceLastRequest = 0f;
    private MLWorldPlanesQueryParams _queryParams = new MLWorldPlanesQueryParams();
    private List<GameObject> _planeCache = new List<GameObject>();
    private List<GameObject> _selectionButtons = new List<GameObject>();
    #endregion

    #region Unity Methods
    private void Start()
    {
        MLWorldPlanes.Start();
    }

    private void OnDestroy()
    {
        MLWorldPlanes.Stop();
    }

    private void Update()
    {
        // Plane extraction
        timeSinceLastRequest += Time.deltaTime;
        if (timeSinceLastRequest > timeout)
        {
            timeSinceLastRequest = 0f;
            RequestPlanes();
        }
    }
    #endregion

    #region Private Methods

    void RequestPlanes()
    {
        _queryParams.Flags = QueryFlags;
        _queryParams.MaxResults = 100;
        _queryParams.BoundsCenter = BBoxTransform.position;
        _queryParams.BoundsRotation = BBoxTransform.rotation;
        _queryParams.BoundsExtents = BBoxExtents;

        MLWorldPlanes.GetPlanes(_queryParams, HandleOnReceivedPlanes);
    }

    private void HandleOnReceivedPlanes(MLResult result, MLWorldPlane[] planes, MLWorldPlaneBoundaries[] boundaries)
    {
        // Destroy old planes
        for (int i = _planeCache.Count - 1; i >= 0; --i)
        {
            Destroy(_planeCache[i]);
            Destroy(_selectionButtons[i]);
            _planeCache.Remove(_planeCache[i]);
            _selectionButtons.Remove(_selectionButtons[i]);
        }

        // Find all new planes
        GameObject newPlane;
        for (int i = 0; i < planes.Length; ++i)
        {
            if (Mathf.Abs(planes[i].Rotation.eulerAngles.x - 90f) < 20)
            {
                newPlane = Instantiate(PlanePrefab);
                newPlane.transform.position = planes[i].Center;
                newPlane.transform.rotation = planes[i].Rotation;
                newPlane.transform.localScale = new Vector3(planes[i].Width, planes[i].Height, 1f);

                bool keepPlane = true;
                for (int j = 0; j < _planeCache.Count; ++j)
                {
                    if ((newPlane.transform.position - _planeCache[j].transform.position).magnitude < _planeDistanceThreshold)
                    {
                        keepPlane = false;
                    }
                }
                if (keepPlane)
                {
                    _planeCache.Add(newPlane);
                    _selectionButtons.Add(newPlane.GetComponent<TargetSpawn>().SpawnSelectionButton());
                }
                else
                {
                    Destroy(newPlane);
                }
                
            }
        }
    }
    #endregion
}