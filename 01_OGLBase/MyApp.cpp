#include "MyApp.h"

#include <math.h>
#include <vector>

#include <array>
#include <list>
#include <tuple>
#include<queue>
#include <imgui/imgui.h>
#include "includes/GLUtils.hpp"


CMyApp::CMyApp(void)
{
	m_camera.SetView(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_mesh = nullptr;
}

CMyApp::~CMyApp(void)
{
	for each (Model model in models)
	{
		for each (Joint * joint in model.Joints)
		{
			delete(joint);
		}
	}
}

void CMyApp::InitSkyBox()
{
	m_SkyboxPos.BufferData(
		std::vector<glm::vec3>{
		// hátsó lap
		glm::vec3(-1, -1, -1),
			glm::vec3(1, -1, -1),
			glm::vec3(1, 1, -1),
			glm::vec3(-1, 1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1, 1, 1),
			glm::vec3(-1, 1, 1),
	}
	);

	// és a primitíveket alkotó csúcspontok indexei (az előző tömbökből) - triangle list-el való kirajzolásra felkészülve
	m_SkyboxIndices.BufferData(
		std::vector<int>{
		// hátsó lap
		0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
	}
	);

	// geometria VAO-ban való regisztrálása
	m_SkyboxVao.Init(
		{
			{ CreateAttribute<0, glm::vec3, 0, sizeof(glm::vec3)>, m_SkyboxPos },
		}, m_SkyboxIndices
		);

	// skybox texture
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_skyboxTexture.AttachFromFile("assets/xpos.png", false, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	m_skyboxTexture.AttachFromFile("assets/xneg.png", false, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	m_skyboxTexture.AttachFromFile("assets/ypos.png", false, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	m_skyboxTexture.AttachFromFile("assets/yneg.png", false, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	m_skyboxTexture.AttachFromFile("assets/zpos.png", false, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	m_skyboxTexture.AttachFromFile("assets/zneg.png", true, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	// a GL_TEXTURE_MAG_FILTER-t és a GL_TEXTURE_MIN_FILTER-t beállítja az AttachFromFile
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void CMyApp::InitShaders()
{
	// a shadereket tároló program létrehozása az OpenGL-hez hasonló módon:
	m_programCylinder.AttachShaders({
		{ GL_VERTEX_SHADER, "myVert.vert"},
		{ GL_FRAGMENT_SHADER, "myFrag.frag"}
		});

	// attributomok osszerendelese a VAO es shader kozt
	m_programCylinder.BindAttribLocations({
		{ 0, "vs_in_pos" },				// VAO 0-as csatorna menjen a vs_in_pos-ba
		{ 1, "vs_in_norm" },			// VAO 1-es csatorna menjen a vs_in_norm-ba
		{ 2, "vs_in_tex" },				// VAO 2-es csatorna menjen a vs_in_tex-be
		});

	m_programCylinder.LinkProgram();

	m_programBody.AttachShaders({
		{ GL_VERTEX_SHADER, "myVert.vert"},
		{ GL_FRAGMENT_SHADER, "myFrag.frag"}
		});

	m_programBody.BindAttribLocations({
	{ 0, "vs_in_pos" },
	{ 1, "vs_in_norm" },
	{ 2, "vs_in_joints" },
		});

	m_programBody.LinkProgram();

	// shader program rövid létrehozása, egyetlen függvényhívással a fenti három:
	m_programSkybox.Init(
		{
			{ GL_VERTEX_SHADER, "skybox.vert" },
			{ GL_FRAGMENT_SHADER, "skybox.frag" }
		},
		{
			{ 0, "vs_in_pos" },				// VAO 0-as csatorna menjen a vs_in_pos-ba
		}
		);

	m_programJoints.Init(
		{
			{GL_VERTEX_SHADER,"jointVert.vert"},
			{GL_FRAGMENT_SHADER,"jointFrag.frag"}
		},
		{
			{0,"vs_in_pos"}
		});
	m_programJoints.LinkProgram();

	m_programBones.Init({
			{GL_VERTEX_SHADER,"bonesVert.vert"},
			{GL_FRAGMENT_SHADER,"bonesFrag.frag"}
		},
	{
			{0,"vs_in_pos"}
	});
	m_programBones.LinkProgram();
}
void CMyApp::InitCylinder() {
	Vertex vertices[(N + 1) * (M + 1)];

	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			vertices[i + j * (N + 1)].p = GetCylinderPos(u, v);
		}
	std::vector<int> indices;
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
		{

			indices.push_back((i)+(j) * (N + 1));
			indices.push_back((i)+(j + 1) * (N + 1));
			indices.push_back((i + 1) + (j) * (N + 1));
			indices.push_back((i + 1) + (j) * (N + 1));
			indices.push_back((i)+(j + 1) * (N + 1));
			indices.push_back((i + 1) + (j + 1) * (N + 1));
		}
	for (size_t i = 1; i < N; ++i)
	{
		indices.push_back(i + M * (N + 1));
		indices.push_back(0 + M * (N + 1));
		indices.push_back(i + 1 + M * (N + 1));
	}


	for (size_t i = 1; i < N; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	m_CylinderVertexBuffer.BufferData(vertices);
	m_CylinderIndices.BufferData(indices);
	m_CylinderVao.Init({
			{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_CylinderVertexBuffer},
			{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_CylinderVertexBuffer},
			{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_CylinderVertexBuffer},
		},
		m_CylinderIndices);
}
void CMyApp::InitSkeleton() {
	Joint* shoulder = new Joint();
	shoulder->bindPosition = glm::vec3(0, 10, 0);
	shoulder->parent = nullptr;
	shoulder->name = "Shoulder";

	Joint* head = new Joint();
	head->bindPosition = glm::vec3(0, 12, 0);
	head->parent = shoulder;
	head->name = "Head";

	Joint* left_elbow = new Joint();
	left_elbow->bindPosition = glm::vec3(3.5, 10, 0);
	left_elbow->parent = shoulder;
	left_elbow->name = "Lef_elbow";

	Joint* left_hand = new Joint();
	left_hand->bindPosition = glm::vec3(6, 10, 0);
	left_hand->parent = left_elbow;
	left_hand->name = "Left_hand";

	Joint* right_elbow = new Joint();
	right_elbow->bindPosition = glm::vec3(-3.5, 10, 0);
	right_elbow->parent = shoulder;
	right_elbow->name = "Right_elbow";

	Joint* right_hand = new Joint();
	right_hand->bindPosition = glm::vec3(-6, 10, 0);
	right_hand->parent = right_elbow;
	right_hand->name = "Right_hand";

	Joint* torso = new Joint();
	torso->bindPosition = glm::vec3(0, 7.5, 0);
	torso->parent = shoulder;
	torso->name = "Torso";

	Joint* hips = new Joint();
	hips->bindPosition = glm::vec3(0, 6, 0);
	hips->parent = shoulder;
	hips->name = "Hips";

	Joint* left_thigh = new Joint();
	left_thigh->bindPosition = glm::vec3(-2, 5, 0);
	left_thigh->parent = hips;
	left_thigh->name = "Left_thigh";

	Joint* left_knee = new Joint();
	left_knee->bindPosition = glm::vec3(-2, 3, 0);
	left_knee->parent = left_thigh;
	left_knee->name = "Left_knee";

	Joint* left_foot = new Joint();
	left_foot->bindPosition = glm::vec3(-2, 0, 0);
	left_foot->parent = left_knee;
	left_foot->name = "Left_foot";

	Joint* right_thigh = new Joint();
	right_thigh->bindPosition = glm::vec3(2, 5, 0);
	right_thigh->parent = hips;
	right_thigh->name = "Right_thigh";

	Joint* right_knee = new Joint();
	right_knee->bindPosition = glm::vec3(2, 3, 0);
	right_knee->parent = right_thigh;
	right_knee->name = "Right_knee";

	Joint* right_foot = new Joint();
	right_foot->bindPosition = glm::vec3(2, 0, 0);
	right_foot->parent = right_knee;
	right_foot->name = "Right_foot";

	CMyApp::models[modelInd].Joints.push_back(shoulder);

	CMyApp::models[modelInd].Joints.push_back(head);
	shoulder->children.push_back(head);

	CMyApp::models[modelInd].Joints.push_back(left_elbow);
	CMyApp::models[modelInd].Joints.push_back(left_hand);
	shoulder->children.push_back(left_elbow);
	left_elbow->children.push_back(left_hand);

	CMyApp::models[modelInd].Joints.push_back(right_elbow);
	CMyApp::models[modelInd].Joints.push_back(right_hand);
	shoulder->children.push_back(right_elbow);
	right_elbow->children.push_back(right_hand);

	CMyApp::models[modelInd].Joints.push_back(torso);
	CMyApp::models[modelInd].Joints.push_back(hips);
	shoulder->children.push_back(torso);
	torso->children.push_back(hips);

	CMyApp::models[modelInd].Joints.push_back(left_thigh);
	CMyApp::models[modelInd].Joints.push_back(left_knee);
	CMyApp::models[modelInd].Joints.push_back(left_foot);
	hips->children.push_back(left_thigh);
	left_thigh->children.push_back(left_knee);
	left_knee->children.push_back(left_foot);

	CMyApp::models[modelInd].Joints.push_back(right_thigh);
	CMyApp::models[modelInd].Joints.push_back(right_knee);
	CMyApp::models[modelInd].Joints.push_back(right_foot);
	hips->children.push_back(right_thigh);
	right_thigh->children.push_back(right_knee);
	right_knee->children.push_back(right_foot);

	models[modelInd].root = shoulder;

	std::vector<glm::vec3> joints_vertices;
	std::vector<glm::vec3> bones_vertices;

	for each (Joint * joint in models[modelInd].Joints)
	{
		joint->dist = 0;
		if (joint->parent != nullptr)
		{
			joint->parentOffset = joint->bindPosition - joint->parent->bindPosition;
			joint->position = joint->parent->position + joint->parentOffset;
			joint->dist = glm::distance(joint->parent->position, joint->position);
		}
		else
		{
			joint->position = joint->bindPosition;
			joint->parentOffset = joint->bindPosition - glm::vec3(0);
		}
		joint->bindOffset = joint->parentOffset;
	}



	SetJointTransforms();
	for (auto& joint : models[modelInd].Joints)
	{
		joints_vertices.push_back(joint->position);
		Bone b;
		b.startJoint = joint;
		b.endJoint = joint->parent;
		if (b.endJoint != nullptr)
		{
			CMyApp::models[modelInd].Bones.push_back(b);
		}

	}



	for (auto const& bone : models[modelInd].Bones)
	{
		bones_vertices.push_back(bone.startJoint->position);
		bones_vertices.push_back(bone.endJoint->position);
	}

	m_JointsVertexBuffer.BufferData(joints_vertices);

	m_JointsVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(glm::vec3)>,m_JointsVertexBuffer}
		});

	m_BonesVertexBuffer.BufferData(bones_vertices);
	m_BonesVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(glm::vec3)>,m_BonesVertexBuffer}
		});

}

glm::vec3 CMyApp::GetNorm(glm::vec3 vec)
{
	glm::vec3 midP = glm::vec3(0, vec.y, 0);
	return glm::normalize(vec - midP);

}
std::vector<int> CMyApp::getCylinderIndices() {
	std::vector<int> indices;
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
		{
			indices.push_back((i)+(j) * (N + 1));
			indices.push_back((i)+(j + 1) * (N + 1));
			indices.push_back((i + 1) + (j) * (N + 1));
			indices.push_back((i + 1) + (j) * (N + 1));
			indices.push_back((i)+(j + 1) * (N + 1));
			indices.push_back((i + 1) + (j + 1) * (N + 1));
		}

	for (size_t i = 1; i < M; i++)
	{
		indices.push_back((M + 1) * (N + 1));
		indices.push_back((M + 1) * (N + 1) + i);
		indices.push_back((M + 1) * (N + 1) + i + 1);
	}
	for (size_t i = 1; i < M; i++)
	{
		indices.push_back((M + 1) * (N + 1) + M + 1);
		indices.push_back((M + 1) * (N + 1) + M + 1 + i);
		indices.push_back((M + 1) * (N + 1) + M + 1 + i + 1);
	}
	return indices;
}
void CMyApp::FindClosestJoints(Vertex& vert) {
	int closestJoint = 0;
	float closestDist = glm::distance(vert.p, models[modelInd].Joints[0]->bindPosition);
	int secondJoint = 1;
	float secondDist = glm::distance(vert.p, models[modelInd].Joints[1]->bindPosition);
	if (secondDist < closestDist)
	{
		float tmpDist = closestDist;
		closestDist = secondDist;
		secondDist = tmpDist;

		int tmpind = closestJoint;
		closestJoint = secondJoint;
		secondJoint = tmpind;
	}

	for (size_t i = 2; i < models[modelInd].Joints.size(); i++)
	{
		float dist = glm::distance(vert.p, models[modelInd].Joints[i]->bindPosition);
		if (dist < closestDist)
		{
			secondDist = closestDist;
			closestDist = dist;

			secondJoint = closestJoint;
			closestJoint = i;
		}
		else if (dist < secondDist)
		{
			secondDist = dist;
			secondJoint = i;
		}
	}

	float closestWeight = closestDist / (closestDist + secondDist);
	float secondWeight = secondDist / (closestDist + secondDist);

	vert.closeJoints.x = closestJoint + (1 - closestWeight);
	vert.closeJoints.y = secondJoint + (closestWeight);
}
void CMyApp::InitBody() {

	//head
	Vertex vertices_head[(N + 1) * (M + 1) + (M + 1) * 2];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			Vertex vert;
			vert.p = GetCylinderPos(u, v);
			vert.n = GetNorm(vert.p);

			Vertex vert2;
			vert2.p.x = vert.p.x;
			vert2.p.y = vert.p.z;
			vert2.p.z = vert.p.y;
			vert2.p.x *= 1.5;
			vert2.p.z *= 4;
			vert2.p.y *= 1.5;
			vert2.p.y += 12;
			vert2.p.z -= 2;

			vert2.n.x = vert.n.x;
			vert2.n.y = vert.n.z;
			vert2.n.z = vert.n.y;


			FindClosestJoints(vert2);
			vertices_head[i + j * (N + 1)] = vert2;
		}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 1;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, 1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.x;
		vert2.p.y = vert.p.z;
		vert2.p.z = vert.p.y;
		vert2.p.x *= 1.5;
		vert2.p.z *= 4;
		vert2.p.y *= 1.5;
		vert2.p.y += 12;
		vert2.p.z -= 2;

		vert2.n.x = vert.n.x;
		vert2.n.y = vert.n.z;
		vert2.n.z = vert.n.y;

		FindClosestJoints(vert2);
		vertices_head[(N + 1) * (M + 1) + i] = vert2;
	}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 0;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, -1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.x;
		vert2.p.y = vert.p.z;
		vert2.p.z = vert.p.y;
		vert2.p.x *= 1.5;
		vert2.p.z *= 4;
		vert2.p.y *= 1.5;
		vert2.p.y += 12;
		vert2.p.z -= 2;

		vert2.n.x = vert.n.x;
		vert2.n.y = vert.n.z;
		vert2.n.z = vert.n.y;

		FindClosestJoints(vert2);
		vertices_head[(N + 1) * (M + 1) + (M + 1) + i] = vert2;
	}


	//arms
	Vertex vertices_arm[(N + 1) * (M + 1) + (M + 1) * 2];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			Vertex vert;
			vert.p = GetCylinderPos(u, v);
			vert.n = GetNorm(vert.p);

			Vertex vert2;
			vert2.p.x = vert.p.y;
			vert2.p.y = vert.p.x;
			vert2.p.z = vert.p.z;
			vert2.p.x *= 12;
			vert2.p.z *= 0.5;
			vert2.p.y *= 0.5;
			vert2.p.x -= 6;
			vert2.p.y += 10;

			vert2.n.x = vert.n.y;
			vert2.n.y = vert.n.x;
			vert2.n.z = vert.n.z;
			FindClosestJoints(vert2);
			vertices_arm[i + j * (N + 1)] = vert2;
		}

	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 1;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, 1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.y;
		vert2.p.y = vert.p.x;
		vert2.p.z = vert.p.z;
		vert2.p.x *= 12;
		vert2.p.z *= 0.5;
		vert2.p.y *= 0.5;
		vert2.p.x -= 6;
		vert2.p.y += 10;

		vert2.n.x = vert.n.y;
		vert2.n.y = vert.n.x;
		vert2.n.z = vert.n.z;
		FindClosestJoints(vert2);
		vertices_arm[(N + 1) * (M + 1) + i] = vert2;
	}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 0;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, -1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.y;
		vert2.p.y = vert.p.x;
		vert2.p.z = vert.p.z;
		vert2.p.x *= 12;
		vert2.p.z *= 0.5;
		vert2.p.y *= 0.5;
		vert2.p.x -= 6;
		vert2.p.y += 10;

		vert2.n.x = vert.n.y;
		vert2.n.y = vert.n.x;
		vert2.n.z = vert.n.z;
		FindClosestJoints(vert2);
		vertices_arm[(N + 1) * (M + 1) + (M + 1) + i] = vert2;
	}

	//Body
	Vertex vertices_body[(N + 1) * (M + 1) + (M + 1) * 2];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			Vertex vert;
			vert.p = GetCylinderPos(u, v);
			vert.n = GetNorm(vert.p);

			Vertex vert2;
			vert2.p.x = vert.p.z;
			vert2.p.y = vert.p.y;
			vert2.p.z = vert.p.x;
			vert2.p.x *= 2;
			vert2.p.y *= 6;
			vert2.p.z *= 2;
			vert2.p.y += 5;

			vert2.n.x = vert.p.z;
			vert2.n.y = vert.n.y;
			vert2.n.z = vert.n.x;

			FindClosestJoints(vert2);
			vertices_body[i + j * (N + 1)] = vert2;
		}

	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 1;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, 1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.x *= 2;
		vert2.p.y *= 6;
		vert2.p.z *= 2;
		vert2.p.y += 5;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;

		FindClosestJoints(vert2);
		vertices_body[(N + 1) * (M + 1) + i] = vert2;
	}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 0;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, -1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.x *= 2;
		vert2.p.y *= 6;
		vert2.p.z *= 2;
		vert2.p.y += 5;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;

		FindClosestJoints(vert2);
		vertices_body[(N + 1) * (M + 1) + (M + 1) + i] = vert2;
	}
	//Left leg
	Vertex vertices_leg_left[(N + 1) * (M + 1) + (M + 1) * 2];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			Vertex vert;
			vert.p = GetCylinderPos(u, v);
			vert.n = GetNorm(vert.p);

			Vertex vert2;
			vert2.p.x = vert.p.z;
			vert2.p.y = vert.p.y;
			vert2.p.z = vert.p.x;
			vert2.p.y *= 6;
			vert2.p.x += 2;

			vert2.n.x = vert.p.z;
			vert2.n.y = vert.n.y;
			vert2.n.z = vert.n.x;
			FindClosestJoints(vert2);
			vertices_leg_left[i + j * (N + 1)] = vert2;
		}

	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 1;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, 1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.y *= 6;
		vert2.p.x += 2;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;
		FindClosestJoints(vert2);
		vertices_leg_left[(N + 1) * (M + 1) + i] = vert2;
	}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 0;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, -1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.y *= 6;
		vert2.p.x += 2;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;
		FindClosestJoints(vert2);
		vertices_leg_left[(N + 1) * (M + 1) + (M + 1) + i] = vert2;
	}

	//Right leg
	Vertex vertices_leg_right[(N + 1) * (M + 1) + (M + 1) * 2];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			Vertex vert;
			vert.p = GetCylinderPos(u, v);
			vert.n = GetNorm(vert.p);

			Vertex vert2;
			vert2.p.x = vert.p.z;
			vert2.p.y = vert.p.y;
			vert2.p.z = vert.p.x;
			vert2.p.y *= 6;
			vert2.p.x -= 2;

			vert2.n.x = vert.p.z;
			vert2.n.y = vert.n.y;
			vert2.n.z = vert.n.x;
			FindClosestJoints(vert2);
			vertices_leg_right[i + j * (N + 1)] = vert2;
		}

	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 1;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, 1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.y *= 6;
		vert2.p.x -= 2;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;
		FindClosestJoints(vert2);
		vertices_leg_right[(N + 1) * (M + 1) + i] = vert2;
	}
	for (size_t i = 0; i <= M; i++)
	{
		float u = i / (float)M;
		float v = 0;
		Vertex vert;
		vert.p = GetCylinderPos(u, v);
		vert.n = glm::vec3(0, -1, 0);

		Vertex vert2;
		vert2.p.x = vert.p.z;
		vert2.p.y = vert.p.y;
		vert2.p.z = vert.p.x;
		vert2.p.y *= 6;
		vert2.p.x -= 2;

		vert2.n.x = vert.p.z;
		vert2.n.y = vert.n.y;
		vert2.n.z = vert.n.x;
		FindClosestJoints(vert2);
		vertices_leg_right[(N + 1) * (M + 1) + (M + 1) + i] = vert2;
	}


	std::vector<int> indices = getCylinderIndices();
	m_CylinderIndices.BufferData(indices);

	m_HeadVertexBuffer.BufferData(vertices_head);
	m_HeadVao.Init({
			{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_HeadVertexBuffer},
			{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_HeadVertexBuffer},
			{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_HeadVertexBuffer},
		},
		m_CylinderIndices);

	m_ArmVertexBuffer.BufferData(vertices_arm);
	m_ArmVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_ArmVertexBuffer},
		{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_ArmVertexBuffer},
		{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_ArmVertexBuffer},
		},
		m_CylinderIndices);

	m_BodyVertexBuffer.BufferData(vertices_body);
	m_BodyVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_BodyVertexBuffer},
		{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_BodyVertexBuffer},
		{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_BodyVertexBuffer},
		},
		m_CylinderIndices);

	m_Left_LegVertexBuffer.BufferData(vertices_leg_left);
	m_Left_legVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_Left_LegVertexBuffer},
		{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_Left_LegVertexBuffer},
		{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_Left_LegVertexBuffer},
		},
		m_CylinderIndices);

	m_Right_LegVertexBuffer.BufferData(vertices_leg_right);
	m_Right_legVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(Vertex)>,m_Right_LegVertexBuffer},
		{CreateAttribute<1,glm::vec3,(sizeof(glm::vec3)),sizeof(Vertex)>,m_Right_LegVertexBuffer},
		{CreateAttribute<2,glm::vec2,(2 * sizeof(glm::vec3)),sizeof(Vertex)>,m_Right_LegVertexBuffer},
		},
		m_CylinderIndices);
}
bool CMyApp::Init()
{
	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	//glEnable(GL_CULL_FACE); // kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	InitShaders();
	//InitCube();
	InitSkyBox();

	//InitWater();


	InitCylinder();
	InitSkeleton();
	InitBody();


	// kamera
	m_camera.SetProj(glm::radians(60.0f), 640.0f / 480.0f, 0.01f, 1000.0f);

	return true;
}
glm::vec3 CMyApp::GetCylinderPos(float u, float v)
{
	float alpha = -u * 2 * M_PI;
	float h = v;
	float r = 1;

	return glm::vec3(cosf(alpha) * r, h, sin(alpha) * r);
}
void CMyApp::Clean()
{
}
void CMyApp::SetJointTransforms() {
	if (!interpolating)
	{
		models[modelInd].root->position = glm::vec3(0) + models[modelInd].root->parentOffset;
	}
	models[modelInd].root->jointRotation = glm::mat4(1);
	models[modelInd].root->modelSpaceTransf = glm::translate(models[modelInd].root->position - glm::vec3(0));
	models[modelInd].root->basemodelSpaceTransform =
		glm::translate(models[modelInd].root->bindPosition - glm::vec3(0));
	models[modelInd].root->jointSpaceTransf = glm::mat4(1.0f);
	for each (Joint * j in models[modelInd].root->children)
	{
		CalculateTransforms(j);
	}
}
void CMyApp::CalculateTransforms(Joint* joint) {
	if (!interpolating)
	{
		joint->position = joint->parent->position + joint->parentOffset;
	}
	joint->jointSpaceTransf = glm::translate(joint->position - joint->parent->position);
	joint->basemodelSpaceTransform = glm::translate(joint->bindPosition - joint->parent->bindPosition)
		* joint->parent->basemodelSpaceTransform;

	joint->modelSpaceTransf = joint->jointSpaceTransf * joint->parent->modelSpaceTransf;
	for each (Joint * j in joint->children)
	{
		CalculateTransforms(j);
	}

}
void CMyApp::UpdateJoints() {
	SetJointTransforms();
	std::vector<glm::vec3> joint_vertices;
	for (auto& joint : models[modelInd].Joints)
	{
		joint_vertices.push_back(joint->position);
	}


	std::vector<glm::vec3> bones_vertices;

	for (auto const& bone : models[modelInd].Bones)
	{
		bones_vertices.push_back(bone.startJoint->position);
		bones_vertices.push_back(bone.endJoint->position);
	}

	m_JointsVertexBuffer.BufferData(joint_vertices);

	m_BonesVertexBuffer.BufferData(bones_vertices);

}

void CMyApp::InterpolatePositions(float dt) {
	int prevInd = modelInd - 1;
	if (prevInd < 0)
	{
		prevInd = MODEL_COUNT;
	}
	for (size_t i = 0; i < models[modelInd].Joints.size(); i++)
	{
		Joint* from = models[modelInd].Joints[i];
		Joint* to = models[prevInd].Joints[i];
		float x = from->position.x + (to->position.x - from->position.x) * ((double)1 - progression) * dt;
		float y = from->position.y + (to->position.y - from->position.y) * ((double)1 - progression) * dt;
		float z = from->position.z + (to->position.z - from->position.z) * ((double)1 - progression) * dt;

		models[modelInd].Joints[i]->position = glm::vec3(x, y, z);
	}
	progression -= dt * tick;
}
void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(delta_time);
	if (interpolating)
	{
		InterpolatePositions(delta_time);
	}
	UpdateJoints();


	if (progression <= 0)
	{
		remFrames -= 1;
		if (remFrames <= 0)
		{
			interpolating = false;
		}

		modelInd -= 1;

		//UpdateJoints();
		if (modelInd < 0)
		{
			modelInd = MODEL_COUNT;
		}
		progression = 1;


	}


	last_time = SDL_GetTicks();
}
void CMyApp::DrawSkyBox(glm::mat4 viewProj) {
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	glDepthFunc(GL_LEQUAL);

	m_SkyboxVao.Bind();
	m_programSkybox.Use();
	m_programSkybox.SetUniform("MVP", viewProj * glm::translate(m_camera.GetEye()));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
	glUniform1i(m_programSkybox.GetLocation("skyboxTexture"), 0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	m_programSkybox.Unuse();

	glDepthFunc(prevDepthFnc);
}
void CMyApp::DrawCylinder(glm::mat4 viewProj, glm::mat4 cylinderWorld) {
	m_programCylinder.Use();
	m_CylinderVao.Bind();
	m_programCylinder.SetUniform("MVP", viewProj * cylinderWorld);
	m_programCylinder.SetUniform("world", cylinderWorld);
	m_programCylinder.SetUniform("worldIT", glm::inverse(glm::transpose(cylinderWorld)));
	//m_program.SetUniform("time", time);

	glDrawElements(GL_TRIANGLES, 3 * 2 * (N) * (M)+(N - 1) * 6, GL_UNSIGNED_INT, nullptr);
	m_programCylinder.Unuse();
}
void CMyApp::DrawJoints(glm::mat4 viewProj) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	m_programJoints.Use();
	m_JointsVao.Bind();
	m_programJoints.SetUniform("MVP", viewProj);

	glDrawArrays(GL_POINTS, 0, models[modelInd].Joints.size());
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_PROGRAM_POINT_SIZE);

}
void CMyApp::DrawBones(glm::mat4 viewProj) {
	glDisable(GL_DEPTH_TEST);
	m_programBones.Use();
	m_BonesVao.Bind();
	m_programBones.SetUniform("MVP", viewProj);
	glDrawArrays(GL_LINES, 0, models[modelInd].Bones.size() * 2);
	m_programBones.Unuse();
	glEnable(GL_DEPTH_TEST);
}
void CMyApp::DrawBody(glm::mat4 viewProj) {
	std::vector<glm::mat4> transforms;
	for each (Joint * joint in models[modelInd].Joints)
	{
		transforms.push_back(joint->modelSpaceTransf * glm::inverse(joint->basemodelSpaceTransform));
	}


	m_programBody.Use();
	m_programBody.SetUniform("transformations", transforms);
	m_programBody.SetUniform("MVP", viewProj);
	m_programBody.SetUniform("world", glm::mat4(1.0f));
	m_programBody.SetUniform("worldIT", glm::inverse(glm::transpose(glm::mat4(1.0f))));

	//int triganleCount = ((3 * 2 * (N) * (M)+(N - 1)) * 6);
	int triganleCount = (N * M * 3 * 2) + (M * 3) * 2;
	m_HeadVao.Bind();
	glDrawElements(GL_TRIANGLES, triganleCount, GL_UNSIGNED_INT, nullptr);

	m_ArmVao.Bind();
	glDrawElements(GL_TRIANGLES, triganleCount, GL_UNSIGNED_INT, nullptr);

	m_BodyVao.Bind();
	glDrawElements(GL_TRIANGLES, triganleCount, GL_UNSIGNED_INT, nullptr);

	m_Left_legVao.Bind();
	glDrawElements(GL_TRIANGLES, triganleCount, GL_UNSIGNED_INT, nullptr);

	m_Right_legVao.Bind();
	glDrawElements(GL_TRIANGLES, triganleCount, GL_UNSIGNED_INT, nullptr);

	m_programBody.Unuse();
}
void CMyApp::DrawBody_Cylinders(glm::mat4 viewProj) {
	//head
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(0, 12, 0))
		* glm::scale(glm::vec3(2, 2, 4))
		* glm::rotate(glm::pi<float>() / 2, glm::vec3(1, 0, 0)));
	//left arm
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(0, 10, 0))
		* glm::scale(glm::vec3(6, 0.5, 0.5))
		* glm::rotate(glm::pi<float>() / 2, glm::vec3(0, 0, 1)));
	//right arm
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(0, 10, 0))
		* glm::scale(glm::vec3(6, 0.5, 0.5))
		* glm::rotate(glm::pi<float>() / 2, glm::vec3(0, 0, 1))
		* glm::rotate(glm::pi<float>(), glm::vec3(1, 0, 0)));
	//body
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(0, 5, 0))
		* glm::scale(glm::vec3(2, 6, 2)));
	//left leg
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(-2, 0, 0))
		* glm::scale(glm::vec3(1, 6, 1)));
	//right leg
	DrawCylinder(viewProj, glm::mat4(1.0f)
		* glm::translate(glm::vec3(2, 0, 0))
		* glm::scale(glm::vec3(1, 6, 1)));
}
void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewProj = m_camera.GetViewProj();


	float time = SDL_GetTicks() / 1000.0f;

	DrawBody(viewProj);
	if (showSekelton)
	{
		DrawJoints(viewProj);
		DrawBones(viewProj);
	}

	//DrawSkyBox(viewProj);


	//ImGui Testwindow
	//ImGui::ShowTestWindow();
	static std::vector<glm::vec3> offsets;

	ImGui::Begin("Skeleton editor");
	std::queue<Joint*> q;
	q.push(models[modelInd].root);
	while (!q.empty())
	{
		Joint* j = q.front();
		q.pop();
		ImGui::Text(JointToString(*j).c_str(), "");
		for each (Joint * joint in j->children)
		{
			q.push(joint);
		}
	}
	ImGui::Separator();
	static char name[50] = "";
	ImGui::InputText("Name of new joint", name, IM_ARRAYSIZE(name));
	static char parent[50] = "";
	ImGui::InputText("Name of parent joint", parent, IM_ARRAYSIZE(parent));
	static float x_pos;
	static float y_pos;
	static float z_pos;
	ImGui::InputFloat("X position", &x_pos);
	ImGui::InputFloat("Y position", &y_pos);
	ImGui::InputFloat("Z position", &z_pos);
	if (ImGui::Button("Create new joint"))
	{
		std::string parent_string = parent;
		std::string name_string = name;
		int parentInd = -1;
		bool validName = true;
		for (size_t i = 0; i < models[modelInd].Joints.size(); i++)
		{
			validName = validName && models[modelInd].Joints[i]->name != name_string;
			if (models[modelInd].Joints[i]->name == parent_string)
			{
				parentInd = i;
			}
		}
		if (!validName || parentInd == -1)
		{
			std::cout << "Parent or joint name is not valid" << std::endl;
		}
		else
		{
			Joint* j = new Joint();
			j->name = name_string;
			j->parent = models[modelInd].Joints[parentInd];
			j->position = glm::vec3(x_pos, y_pos, z_pos);
			j->bindOffset = j->position - j->parent->position;
			j->parentOffset = j->bindOffset;
			j->dist = glm::distance(j->position, j->parent->position);
			j->parent->children.push_back(j);
			j->bindPosition = j->parent->bindPosition + j->bindOffset;

			models[modelInd].Joints.push_back(j);

			Bone b;
			b.startJoint = j->parent;
			b.endJoint = j;
			models[modelInd].Bones.push_back(b);

			Model m = CopyModel(models[modelInd]);
			for each (Joint * joint in models[modelInd].Joints)
			{
				delete(joint);
			}

			models[modelInd] = m;
			offsets.clear();
		}
	}

	if (ImGui::Button("Reskin model"))
	{
		InitBody();
	}


	ImGui::End();

	ImGui::Begin("Animation editor");
	ImGui::Checkbox("Show skeleton", &showSekelton);


	if (offsets.size() == 0)
	{
		for each (Joint * joint in models[modelInd].Joints)
		{
			offsets.push_back(joint->parentOffset);
		}
	}
	for (int i = 1; i < offsets.size(); i++)
	{
		ImGui::InputFloat((models[modelInd].Joints[i]->name + " X:").c_str(), &offsets[i].x);
		ImGui::InputFloat((models[modelInd].Joints[i]->name + " Y:").c_str(), &offsets[i].y);
		ImGui::InputFloat((models[modelInd].Joints[i]->name + " Z:").c_str(), &offsets[i].z);

	}

	if (ImGui::Button("Save given state"))
	{
		int nextInd = modelInd + 1;
		if (nextInd > MODEL_COUNT)
		{
			nextInd = 0;
		}
		for each (Joint * joint in models[nextInd].Joints)
		{
			delete(joint);
		}
		models[nextInd].Joints.clear();
		models[nextInd].Bones.clear();
		models[nextInd] = CopyModel(models[modelInd]);
		modelInd = nextInd;

		//InitSkeleton();
		for (size_t i = 0; i < models[modelInd].Joints.size(); i++)
		{
			Joint* j = models[modelInd].Joints[i];
			j->parentOffset = glm::normalize(offsets[i]) * j->dist;
		}
		models[modelInd].root->bindPosition = glm::vec3(0, 10, 0);
		models[modelInd].root->parentOffset = models[modelInd].root->bindPosition - glm::vec3(0);
		SetJointTransforms();
		offsets.clear();
	}
	if (ImGui::Button("Swith to next model"))
	{
		modelInd += 1;
		if (modelInd > MODEL_COUNT)
		{
			modelInd = 0;
		}
	}
	if (ImGui::Button("Switch to previous model"))
	{
		modelInd -= 1;
		if (modelInd < 0)
		{
			modelInd = MODEL_COUNT;
		}
	}

	static int interpolateTime = 10;
	if (ImGui::Button("Interpolate between current and previous model"))
	{
		progression = 1;
		tick = 1 / (double)interpolateTime;
		interpolating = true;
	}
	ImGui::InputInt("Interpolation duration in seconds", &interpolateTime);
	static int frameCount = 3;
	if (ImGui::Button("Keyframe animation with past positions"))
	{
		keyframeAnimationActive = true;
		interpolating = true;
		progression = 1;
		tick = (double)frameCount / interpolateTime;
		remFrames = frameCount - 1;
	}
	ImGui::InputInt("Number of keyframes for animation", &frameCount);

	if (ImGui::Button("Reset model state"))
	{
		for each (Joint * joint in models[modelInd].Joints)
		{
			delete(joint);
		}
		models[modelInd].Joints.clear();
		models[modelInd].Bones.clear();
		offsets.clear();

		InitSkeleton();
	}

	ImGui::LabelText(("Index of current model:" + std::to_string(modelInd)).c_str(), "");
	ImGui::End();

}

std::string CMyApp::JointToString(Joint j) {
	return j.name + " [" + std::to_string(j.position.x) + ", " + std::to_string(j.position.y) +
		", " + std::to_string(j.position.z) + "]";

}
void CMyApp::CopyJoints(Joint* root, Joint* newRoot, std::vector<Joint*>& joints) {
	for each (Joint * joint in root->children)
	{
		Joint* j = new Joint();
		*j = *joint;
		j->parent = newRoot;
		newRoot->children.push_back(j);
		joints.push_back(j);
		j->children.clear();
		CopyJoints(joint, j, joints);
	}
}

CMyApp::Model CMyApp::CopyModel(CMyApp::Model oldModel) {
	Model newModel;
	Joint* newRoot = new Joint();
	*newRoot = *oldModel.root;
	newRoot->children.clear();
	newModel.Joints.push_back(newRoot);
	newModel.root = newRoot;
	CopyJoints(oldModel.root, newRoot, newModel.Joints);

	std::vector<glm::vec3> joints_vertices;
	std::vector<glm::vec3> bones_vertices;

	for each (Joint * joint in newModel.Joints)
	{
		joint->dist = 0;
		if (joint->parent != nullptr)
		{
			joint->parentOffset = joint->bindPosition - joint->parent->bindPosition;
			joint->position = joint->parent->position + joint->parentOffset;
			joint->dist = glm::distance(joint->parent->position, joint->position);
		}
		else
		{
			joint->position = joint->bindPosition;
			joint->parentOffset = joint->bindPosition - glm::vec3(0);
		}
		joint->bindOffset = joint->parentOffset;
	}



	//SetJointTransforms();
	for (auto& joint : newModel.Joints)
	{
		joints_vertices.push_back(joint->position);
		Bone b;
		b.startJoint = joint;
		b.endJoint = joint->parent;
		if (b.endJoint != nullptr)
		{
			newModel.Bones.push_back(b);
		}

	}



	for (auto const& bone : models[modelInd].Bones)
	{
		bones_vertices.push_back(bone.startJoint->position);
		bones_vertices.push_back(bone.endJoint->position);
	}

	m_JointsVertexBuffer.BufferData(joints_vertices);

	m_JointsVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(glm::vec3)>,m_JointsVertexBuffer}
		});

	m_BonesVertexBuffer.BufferData(bones_vertices);
	m_BonesVao.Init({
		{CreateAttribute<0,glm::vec3,0,sizeof(glm::vec3)>,m_BonesVertexBuffer}
		});

	return newModel;
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{

}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_camera.Resize(_w, _h);
}
