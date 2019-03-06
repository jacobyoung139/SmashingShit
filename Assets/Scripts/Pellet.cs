using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Pellet : MonoBehaviour {

    private AudioSource oofSource;

    public float _lifeTimeSeconds;

    private float _timeSpawned;

    private bool hasOofed = false;

	// Use this for initialization
	void Start () {
        _timeSpawned = Time.time;
        oofSource = GetComponent<AudioSource>();
    }

    private void OnCollisionEnter(Collision other)
    {
        if (!hasOofed)
        {
            oofSource.transform.position = transform.position;
            oofSource.Play(0);
            hasOofed = true;
        }
    }

    // Update is called once per frame
    void Update () {
        if (Time.time - _timeSpawned > _lifeTimeSeconds) Destroy(this.gameObject);
	}
}
