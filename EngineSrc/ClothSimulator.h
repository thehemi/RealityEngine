typedef void* SphereHandle;

/// Simulates cloth physics on CPU
class ENGINE_API ClothSimulator {
public:
	/// Spheres perturb the cloth simulation
	struct Sphere {
		Vector pos;
		float  size;
	};
	vector<Sphere*> m_Spheres;

	/// Contains the geometry of the cloth simulation
	struct Cloth {
		int			SizeX;
		int			SizeY;
		class Mesh*		clothMesh;
		LPD3DXMESH	dxMesh;
		struct Vertex vertices[25][25];
	};
	vector<Cloth*>		m_Cloths;
	class SpringSystem*	spring;

	void			Tick(Camera* cam);
	void			AddCloth(Mesh* clothMesh = 0);
	//void			RemoveCloth();
	void			Initialize();
	void			UpdateSphere(SphereHandle sphere, Vector& pos, float radius);
	SphereHandle	AddSphere();
	//void			RemoveSphere(SphereHandle sphere);
	
};