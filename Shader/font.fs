#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform float base;
uniform float scale;

void main()
{
	vec4 color = texture(texture1, TexCoord);
	float distance = color.r * scale + base;
	float alpha = distance;
	FragColor = vec4(1.0);
	FragColor.a = alpha;
}