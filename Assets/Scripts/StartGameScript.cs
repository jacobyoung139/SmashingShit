using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class StartGameScript : MonoBehaviour {

    public GameObject _loadingAnimation;
    public GameObject _loadingAnimationGazeTarget;

    #region Unity Methods
    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}
    #endregion

    #region Event Handlers
    private void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.CompareTag("StartGameButton"))
        {
            Destroy(this.gameObject);
            Destroy(collision.gameObject);
            Instantiate(_loadingAnimation, transform.position, Quaternion.identity)
                .transform.LookAt(_loadingAnimationGazeTarget.transform);
            new WaitForSeconds(3);
            SceneManager.LoadSceneAsync("SampleScene");
        }
    }
    #endregion
}
