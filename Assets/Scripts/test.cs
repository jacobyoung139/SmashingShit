using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class test : MonoBehaviour {

    public GameObject cube;

	// Use this for initialization
	void Start () {
	}
	
	// Update is called once per frame
	void Update () {
        cube.transform.Rotate(new Vector3(40.0f, 32.0f, 15.0f) * Time.deltaTime);
	}
}
