using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ReticleParticle : MonoBehaviour {

    public float _maxDistance;

    private Vector3 _initPosition;

	// Use this for initialization
	void Start () {
        _initPosition = transform.position;
	}

    private void OnTriggerEnter(Collider other)
    {
        Destroy(this.gameObject);
    }

    // Update is called once per frame
    void Update () {
        // Only let the particle travel a certain distance before despawning
        Vector3 posDiff = transform.position - _initPosition;
        float distanceTravelled = posDiff.magnitude;
        if (distanceTravelled > _maxDistance) Destroy(this.gameObject);
	}
}
