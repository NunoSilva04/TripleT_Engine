#version 450

layout(location = 0) in vec3 vert_in;
layout(location = 1) in vec4 color_in;

layout(location = 0) out vec4 color_out;

void main() {
    gl_Position = vec4(vert_in, 1.0);
    color_out = color_in;
}
