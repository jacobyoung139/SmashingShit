using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SelectPlane : MonoBehaviour {

    #region Private Variables
    private GameObject _parentPlane;
    #endregion

    #region Public Variables
    public Material _gamePlaneMaterial;
    #endregion

    #region Unity Methods
    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}
    #endregion

    #region Event Handlers
    private void OnTriggerEnter(Collider collider)
    {
        if (collider.gameObject.CompareTag("ButtonPresser"))
        {
            GameObject gamePlane = Instantiate(_parentPlane, _parentPlane.transform.position, _parentPlane.transform.rotation);
            (gamePlane.GetComponent<MeshRenderer>() as MeshRenderer).material = _gamePlaneMaterial;
            DontDestroyOnLoad(gamePlane);
            Destroy(this.gameObject);
        }
    }
    #endregion

    #region Public Methods
    public void SetParentPlane(GameObject parentPlane)
    {
        _parentPlane = parentPlane;
    }
    #endregion
}
