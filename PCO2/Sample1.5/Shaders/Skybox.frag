#version 330 core
//Returns a Color
out vec4 FragColor;

//Take in a vec3 textCoord
in vec3 texCoord;

//Add in our cubemap for later
uniform samplerCube skybox;

void main(){
	FragColor = texture(skybox, texCoord);
}