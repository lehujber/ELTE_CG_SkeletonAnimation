#version 330 core

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;

out vec4 fs_out_col;

// irány fényforrás: fény iránya
uniform vec3 light_dir = vec3(0,0,1);
uniform vec3 light_dir2 = vec3(1,0,0);

// fénytulajdonságok: ambiens, diffúz, ...
uniform vec3 La = vec3(0.4, 0.4, 0.4);
uniform vec3 Ld = vec3(0.6, 0.6, 0.6);
uniform vec3 Ls = vec3(0.9, 0.9, 0.9);

uniform vec3 Ka = vec3(0.6, 0.6, 0.6);
uniform vec3 Kd = vec3(0.4, 0.4, 0.4);
uniform vec3 Ks = vec3(0.8, 0.8, 0.8);


uniform sampler2D texImage;

void main()
{

	vec3 ambient = Ka * La;

	vec3 normal = normalize(vs_out_norm);
	vec3 to_light = normalize(-light_dir);

	vec3 to_light2 = normalize(-light_dir2);
	
	float cosa = clamp(dot(normal, to_light), 0, 1);
	float cosa2 = clamp(dot(normal, to_light2), 0, 1);

//	vec3 diffuse = cosa*Ld;
	vec3 diffuse = Kd*Ld*dot(light_dir,normal);
	vec3 diffuse2 = Kd*Ld*dot(light_dir2,normal);

//	vec3 diffuse2 = cosa2*Ld;

	vec3 specular = Ks*Ls;
	
//	fs_out_col = vec4(ambient + diffuse, 1) * texture(texImage, vs_out_tex);
	
	fs_out_col = vec4(ambient + diffuse, 1) * vec4(1,1,1,1);
	fs_out_col = (vec4(ambient+diffuse+specular,1) * vec4(ambient+diffuse2+specular,1) )* vec4(0.8,0.8,0.8,1);
//	fs_out_col = vec4(ambient + diffuse, 1) * vec4(ambient + diffuse2, 1) * vec4(1,1,1,1);

//	fs_out_col = vec4(0.6,0.6,0.6,1);


}