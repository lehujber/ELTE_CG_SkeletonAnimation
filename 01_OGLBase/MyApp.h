#pragma once

// C++ includes
#include <memory>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "includes/gCamera.h"

#include "includes/ProgramObject.h"
#include "includes/BufferObject.h"
#include "includes/VertexArrayObject.h"
#include "includes/TextureObject.h"

// mesh
#include "includes/ObjParser_OGL3.h"

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);

protected:
	// shaderekhez szükséges változók
	ProgramObject		m_programBody;
	ProgramObject		m_programCylinder;	
	ProgramObject		m_programJoints;
	ProgramObject		m_programBones;
	ProgramObject		m_programSkybox;

	VertexArrayObject	m_SkyboxVao;
	IndexBuffer			m_SkyboxIndices;	
	ArrayBuffer			m_SkyboxPos;	

	VertexArrayObject	m_CylinderVao;
	ArrayBuffer			m_CylinderVertexBuffer;

	IndexBuffer			m_CylinderIndices;

	VertexArrayObject	m_HeadVao;
	ArrayBuffer			m_HeadVertexBuffer;

	VertexArrayObject	m_ArmVao;
	ArrayBuffer			m_ArmVertexBuffer;

	VertexArrayObject	m_BodyVao;
	ArrayBuffer			m_BodyVertexBuffer;

	VertexArrayObject	m_Left_legVao;
	ArrayBuffer			m_Left_LegVertexBuffer;

	VertexArrayObject	m_Right_legVao;
	ArrayBuffer			m_Right_LegVertexBuffer;


	VertexArrayObject	m_JointsVao;
	ArrayBuffer			m_JointsVertexBuffer;

	VertexArrayObject	m_BonesVao;
	ArrayBuffer			m_BonesVertexBuffer;

	gCamera				m_camera;

	TextureCubeMap		m_skyboxTexture;

	struct Vertex
	{
		glm::vec3 p;
		glm::vec3 n;

		glm::vec2 closeJoints;
	};

	struct Joint {
		glm::vec3 position;
		glm::vec3 bindPosition;
		glm::vec3 parentOffset;

		glm::vec3 bindOffset;
		glm::mat4 jointSpaceTransf;
		glm::mat4 jointRotation;
		glm::mat4 modelSpaceTransf;
		glm::mat4 basemodelSpaceTransform;
		Joint* parent;
		std::vector<Joint*> children;
		
		
		float dist;

		std::string name;
	};

	struct Bone {
		Joint* startJoint;
		Joint* endJoint;
	};

	struct Model
	{
		std::vector<Joint*> Joints;
		std::vector<Bone> Bones;
		Joint* root;
	};
	const static int MODEL_COUNT = 3;

	Model models[MODEL_COUNT+1];
	int modelInd = 0;

	// mesh adatok
	std::unique_ptr<Mesh> m_mesh;

	// a jobb olvashatóság kedvéért
	glm::vec3 CMyApp::GetCylinderPos(float u, float v);

	void InitShaders();
	void InitSkyBox();
	void InitCylinder();
	void InitSkeleton();
	void InitBody();

	glm::vec3 GetNorm(glm::vec3);

	void DrawJoints(glm::mat4);
	void DrawSkyBox(glm::mat4);
	void DrawCylinder(glm::mat4,glm::mat4);
	void DrawBones(glm::mat4);
	void DrawBody(glm::mat4);
	void DrawBody_Cylinders(glm::mat4);

	void SetJointTransforms();
	void UpdateJoints();

	void InterpolatePositions(float);

	//Joint* root;
	//std::vector<Joint*> Joints;
	//std::vector<Bone> Bones;

	std::string JointToString(Joint);
	std::vector<int> getCylinderIndices();

	void FindClosestJoints(Vertex&);
	void CalculateTransforms(Joint*);

	Model CopyModel(Model);

	bool showSekelton = true;
	bool interpolating = false;
	bool keyframeAnimationActive = false;


	const int SIZE = 25;


	static const int acc = 10;
	static const int N =2 * acc;
	static const int M =2 * acc;

	double progression = 1;
	double tick;
	int remFrames = 0;

	void CopyJoints(Joint*,Joint*, std::vector<Joint*>&);
};

