using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BoxColliderScript : MonoBehaviour {

    private AudioSource _collisionSound;
    private float _minimumCollisionVelocity = 10f;

	// Use this for initialization
	void Start () {
        _collisionSound = GetComponent<AudioSource>();
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    private void OnCollisionEnter(Collision collision)
    {
        if (collision.relativeVelocity.magnitude > _minimumCollisionVelocity)
        {
            _collisionSound.transform.position = this.transform.position;
            _collisionSound.Play(0);
        }
    }
}
