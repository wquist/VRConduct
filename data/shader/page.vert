#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

out vec3 Position0;
out vec2 TexCoord0;
out vec3 Normal0;

uniform mat4 M;
uniform mat4 VP;

void main() {
	vec4 hpos = vec4(position, 1.f);

	Position0 = (M * hpos).xyz;
	TexCoord0 = texcoord;
	Normal0   = (M * vec4(normal, 0.f)).xyz;

	gl_Position = VP * M * hpos;
}
