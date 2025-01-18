#version 400 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);
	FragColor = vec4(TexCoords, 0.0f, 1.0f);
}

