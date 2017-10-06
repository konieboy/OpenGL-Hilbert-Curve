#version 410

out vec4 FragmentColour;
//in vec4 fragmentColor;

in vec2 vertexPosition;

//out vec4 color;

void main() {
  	//color = fragmentColor;
    FragmentColour = vec4(cos(vertexPosition[0] + 1), sin(vertexPosition[1]), cos(vertexPosition[0] - 1), 1);
}


