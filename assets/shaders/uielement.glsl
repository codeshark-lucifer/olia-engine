#shader vertex
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 TexCoord;

void main()
{
    TexCoord = aUV;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 0.0, 1.0);
}

#shader fragment
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, TexCoord);
}

