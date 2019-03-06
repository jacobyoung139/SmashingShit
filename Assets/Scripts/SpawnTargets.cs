using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SpawnTargets : MonoBehaviour {

    public Material _targetPlaneMaterial;

	// Use this for initialization
	void Start () {
        GameObject[] targetSurfaces = GameObject.FindGameObjectsWithTag("GameSurfacePlane");
        for (int i = 0; i < targetSurfaces.Length; ++i)
        {
            TargetSpawn spawnScript = targetSurfaces[i].GetComponent<TargetSpawn>();
            targetSurfaces[i].GetComponent<MeshRenderer>().material = _targetPlaneMaterial;
            spawnScript.SpawnTargets();
        }
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
