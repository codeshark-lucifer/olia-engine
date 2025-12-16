#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;

void main() {
    vec4 texColor = texture(screenTexture, TexCoord);
    FragColor = vec4(texColor.rgb, 1.0);
}