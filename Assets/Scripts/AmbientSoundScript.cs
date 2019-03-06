using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AmbientSoundScript : MonoBehaviour {
   
    private float _timeElapsed = 0f;

	// Use this for initialization
	void Start () {
        _timeElapsed += Time.deltaTime;
        this.transform.position = this.GetComponentInParent<Transform>().transform.position 
            + new Vector3(Mathf.Sin(_timeElapsed), Mathf.Sin(_timeElapsed * 0.674f), Mathf.Sin(_timeElapsed * 1.1f));
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
