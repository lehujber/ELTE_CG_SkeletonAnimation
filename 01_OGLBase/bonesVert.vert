#version 130

uniform mat4 MVP;

in vec3 vs_in_pos;

out vec4 vs_out_color;

void main()
{
	gl_Position = MVP * vec4(vs_in_pos,1);
}

// 1. feladat: hogyan lehetne a positions tömb méretét lecsökkenteni 4-re? 