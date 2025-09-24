#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D uTex;

void main()
{
	vec3 texCol = texture(uTex, TexCoords).rgb;
	FragColor = vec4(texCol, 1.0);
}