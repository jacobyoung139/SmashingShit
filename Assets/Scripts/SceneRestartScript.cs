using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneRestartScript : MonoBehaviour {

    private float resetTime = 30f;
    private float timeElapsed = 0f;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        timeElapsed += Time.deltaTime;
        if (timeElapsed > resetTime)
        {
            SceneManager.LoadSceneAsync("SampleScene");
        }
	}
}
