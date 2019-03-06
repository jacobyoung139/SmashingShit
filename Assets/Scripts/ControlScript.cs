using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR.MagicLeap;

public class ControlScript : MonoBehaviour {

    private MLInputController _controller;
    private bool _triggerDown = false;


	// Use this for initialization
	void Start () {
        MLInput.Start();
        _controller = MLInput.GetController(MLInput.Hand.Left);
	}

    // Called when the app is closed
    private void OnDestroy() {
        MLInput.Stop();
    }

    // Checks the current state of the controller and performs the
    // relevant actions
    void CheckControl() {
        if (_controller.TriggerValue > 0.2f && !_triggerDown) {
            _triggerDown = true;
            //TODO: Shoot slingshot
        } else if (_triggerDown) {
            _triggerDown = false;
        }
    }

    // Update is called once per frame
    void Update () {
        // Check current state of controller
        CheckControl();
	}
}
