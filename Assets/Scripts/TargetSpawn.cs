using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TargetSpawn : MonoBehaviour {

    public GameObject _selectionButton;
    public GameObject _target;

    #region Unity Methods
    // Use this for initialization
    void Start () {
        
	}

    // Update is called once per frame
    void Update () {
	}
    #endregion

    #region Public Methods
    public GameObject SpawnSelectionButton()
    {
        GameObject spawnButton = Instantiate(_selectionButton, transform.position +  new Vector3(0f, 0.5f, 0f), Quaternion.identity);
        SelectPlane buttonScript = spawnButton.GetComponent<SelectPlane>();
        buttonScript.SetParentPlane(this.gameObject);
        return spawnButton;
    }
    public void SpawnTargets()
    {
        /*Vector3 zero = new Vector3(0.0f, 0.0f, 0.0f);
        createOneTowerIn(zero);
        createOneTowerIn(moveX(zero, 1f));
        createOneTowerIn(moveX(zero, -1f));*/
        float randomX = Random.Range(transform.position.x - transform.localScale.x / 2, transform.position.x + transform.localScale.x / 2);
        float randomY = Random.Range(transform.position.y - transform.localScale.y / 2, transform.position.y + transform.localScale.y / 2);
        float randomZ = Random.Range(transform.position.y - transform.localScale.z / 2, transform.position.y + transform.localScale.z / 2);
        Vector3 randomPosition = new Vector3(randomZ, randomY, randomX);
        Instantiate(_target, randomPosition, Quaternion.identity);
    }
    #endregion

    #region Private Methods
    private Vector3 moveX(Vector3 v, float amount)
    {
        return (new Vector3(v.x + amount, v.y, v.z));
    }
    private Vector3 moveY(Vector3 v, float amount)
    {
        return (new Vector3(v.x, v.y + amount, v.z));
    }
    private Vector3 moveZ(Vector3 v, float amount)
    {
        return (new Vector3(v.x, v.y, v.z + amount));
    }

    private void createOneTowerIn(Vector3 position)
    {
        createOneCubeIn(position);
        createOneCubeIn(moveY(moveX(position, 0.05f), 0.1f));
        createOneCubeIn(moveY(moveX(moveY(moveX(position, 0.05f), 0.1f), 0.05f), 0.1f));

        createOneCubeIn(moveX(position, 0.3f));
        createOneCubeIn(moveY(moveX(position, 0.3f - 0.05f), 0.1f));
        createOneCubeIn(moveY(moveX(moveY(moveX(position, 0.3f - 0.05f), 0.1f), -0.05f), 0.1f));

        createOneCubeIn(moveY(moveX(position, 0.15f), 0.3f));
        createOneCubeIn(moveY(moveX(position, 0.15f), 0.4f));
        createOneCubeIn(moveY(moveX(position, 0.15f), 0.5f));
    }

    private GameObject createOneCubeIn(Vector3 position)
    {
        GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
        cube.transform.localScale = Vector3.Scale(transform.localScale, new Vector3(0.099f, 0.099f, 0.099f));
        //cube.transform.parent = GameObject.Find("Surface").transform;
        //cube.transform.rotation = cube.transform.parent.rotation;
        cube.transform.position = moveZ(position, 2f);
        //cube.transform.position = moveZ(cube.transform.position, 2f);
        cube.name = "MyCube";
        cube.GetComponent<Renderer>().material.color = new Color(0f, 0f, 1f);
        //cube.GetComponent<Renderer>().material = CubeMaterial;
        Rigidbody gameObjectsRigidBody = cube.AddComponent<Rigidbody>(); // Add the rigidbody.
        gameObjectsRigidBody.mass = 5f;
        return cube;
    }
    #endregion
}
