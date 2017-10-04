#version 410

layout(location = 0) in vec2 position;
//layout(location = 1) in vec4 vertexColor;

//out vec4 fragmentColor;
out vec2 vertexPosition;

void main() {
  gl_Position = vec4(position, 0, 2);

  vertexPosition = position;

  //fragmentColor = vertexColor;
}
