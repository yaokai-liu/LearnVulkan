#version 450

layout(location = 0) in vec4 coord;
layout(location = 1) in vec4 color;

layout(binding = 0, row_major) uniform MVP {
  mat4 model;
  mat4 view;
  mat4 proj;
} mvp;

layout(location = 0) out vec4 fragColor;

void main() {
  gl_Position = mvp.proj * mvp.view * mvp.model * coord;
  gl_Position.y = -gl_Position.y;
  gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
//  gl_Position = coord;
  fragColor = color;
}
