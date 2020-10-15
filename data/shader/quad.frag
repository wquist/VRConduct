#version 330 core

in vec2 TexCoord0;

out vec3 FragColor;

uniform sampler2D Texture;

void main() {
	FragColor = texture(Texture, TexCoord0).rgb;
}