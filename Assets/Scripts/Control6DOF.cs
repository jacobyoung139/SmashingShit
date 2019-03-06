using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR.MagicLeap;

public class Control6DOF : MonoBehaviour {

    #region Private Variables
    private MLInputController _controller;
    private AudioSource _fireSound;
    private bool _triggerDown = false;
    private Animator _animator;
    private GameObject _spawnZone;
    #endregion

    #region Public Variables
    public GameObject _pellet;
    public GameObject _reticleParticle;
    public float _pelletVelocity;
    #endregion

    #region Unity Methods

    // Use this for initialization
    void Start () {
        MLInput.Start();
        _controller = MLInput.GetController(MLInput.Hand.Left);
        _fireSound = GetComponent<AudioSource>();
        _animator = GetComponentInChildren<Animator>();
        _spawnZone = GameObject.FindGameObjectWithTag("SpawnZone");
    }

    private void OnDestroy() {
        MLInput.Stop();
    }

    // Checks the current state of the controller and performs the
    // relevant actions
    void CheckControl()
    {
        if (_controller.TriggerValue < 0.2f)
        {
            if (_triggerDown)
            {
                GameObject pellet = Instantiate(_pellet, _spawnZone.transform.position, _spawnZone.transform.rotation) as GameObject;
                SpawnProjectile(pellet);
                _controller.StartFeedbackPatternVibe(MLInputControllerFeedbackPatternVibe.Click, MLInputControllerFeedbackIntensity.Low);
                _animator.SetBool("HoldTrig", false);
            }
            _triggerDown = false;
        }
        else
        {
            if (!_triggerDown)
            {
                _fireSound.transform.position = transform.position;
                _fireSound.Play(0);
                _animator.SetBool("HoldTrig", true);
            }
            GameObject reticleParticle = Instantiate(_reticleParticle, _spawnZone.transform.position, _spawnZone.transform.rotation);
            SpawnProjectile(reticleParticle);
            _triggerDown = true;
        }
    }

    // Update is called once per frame
    void Update () {
        transform.position = _controller.Position;
        transform.rotation = _controller.Orientation;
        CheckControl();
    }
    #endregion

    // Spawns a pellet from the controller
    void SpawnProjectile(GameObject projectile) {
        projectile.GetComponent<Rigidbody>().AddForce(transform.forward * _pelletVelocity);
    }
}