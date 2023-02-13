#version 330 core

// VBO-ból érkező változók
in vec3 vs_in_pos;
in vec3 vs_in_norm;
in vec2 vs_in_joints;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// shader külső paraméterei
uniform mat4 MVP;
uniform mat4 world;
uniform mat4 worldIT;

uniform mat4[50] transformations;
uniform vec3[50] jointPositions;

uniform float time;

void main()
{
	int joint1 = int(floor(vs_in_joints.x));
	float weight1 = vs_in_joints.x - joint1;

	int joint2 = int(floor(vs_in_joints.y));
	float weight2 = vs_in_joints.y-joint2;

//	mat4 trans = transformations[joint1] * weight1 + transformations[joint2] * weight2;

	gl_Position = MVP* vec4( vs_in_pos, 1 );
	
	vec4 transform1 = transformations[joint1] * vec4(vs_in_pos,1.0);
	vec4 transform2 = transformations[joint2] * vec4(vs_in_pos,1.0);

	vec4 newpos = (transform1 * weight1) + (transform2*weight2);
//	newpos += transform2 * vec4(vs_in_pos,1);

	vec4 newnorm = transformations[joint1] * vec4(vs_in_norm,0);
	newnorm += transformations[joint2] * vec4(vs_in_norm,0);

	gl_Position = MVP * newpos;




	vs_out_pos = (world * vec4(vs_in_pos, 1)).xyz;
	vs_out_norm = (worldIT * newnorm).xyz;
}