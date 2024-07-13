#version 330 core

//Point Light

//Textures
//Texture to be passed
uniform sampler2D tex0;

//Normal data
in vec3 normCoord;

//fragment data
in vec3 fragPos;


//Position of the light source
uniform vec3 lightPos;

//Color of the light 
uniform vec3 lightColor;

//Strength of the  ambient light
uniform float ambientStr;

//color of the ambient light
uniform vec3 ambientColor;

//Camera position
uniform vec3 cameraPos;

//Specular str
uniform float specStr;

//Specular Phong
uniform float specPhong;

//Attenuation Constant 
uniform float constant;

//Attenuation Linear
uniform float linear;

//Attenuation Exponential
uniform float exponent;

//Intensity of Light (Brightness)
uniform float brightness;

//Light Direction
uniform vec3 lightDirection;

//Should recieve the texCoord
//from the vertex shader
in vec2 texCoord;

out vec4 FragColor; //Returns a Color
//Simple shader that colors the model Red

void main(){
	
	vec3 result = vec3(0.0);


	//normalize the recieved normals
	vec3 normal = normalize(normCoord);

	//Get the direction of the light to the fragment
	vec3 lightDir = normalize(-lightDirection);

	//Apply the diffuse formula heree
	float diff = max(dot(normal, lightDir), 0.0);

	//Multiply it to the desired light color and intensity
	vec3 diffuse = diff * lightColor * brightness;

	//Get the ambient light
	vec3 ambientCol = ambientColor * ambientStr;

	//Get our view direction from the camera to the fragment
	vec3 viewDir = normalize(cameraPos - fragPos);

	//Get the reflection vector
	vec3 reflectDir = reflect(-lightDir, normal);

	//Get the specular light
	float spec = pow(max(dot(reflectDir, viewDir), 0.1), specPhong);

	//Get the specColor
	vec3 specColorDir = spec * specStr * lightColor * brightness;

	result += diffuse + ambientCol + specColorDir;

	//normalize the recieved normals
	vec3 normalPoint = normalize(normCoord);

	//Get the direction of the light to the fragment
	vec3 lightDirPoint = normalize(lightPos - fragPos);

	//Apply the diffuse formula heree
	float diffPoint = max(dot(normalPoint, lightDirPoint), 0.0);

	//Multiply it to the desired light color and intensity
	vec3 diffusePoint = diffPoint * lightColor * brightness;

	//Get the ambient light
	vec3 ambientColPoint = ambientColor * ambientStr;

	//Get our view direction from the camera to the fragment
	vec3 viewDirPoint = normalize(cameraPos - fragPos);

	//Get the reflection vector
	vec3 reflectDirPoint = reflect(-lightDirPoint, normalPoint);

	//Get the specular light
	float specPoint = pow(max(dot(reflectDirPoint, viewDirPoint), 0.1), specPhong);
	//Get distance of object to light direction
	float distancePoint = length(lightDirPoint);

	//Get the Attenuation factor
	float attenuation = constant + linear * distancePoint + exponent * distancePoint * distancePoint;

	//Get the specColor
	vec3 specColorPoint = specPoint * specStr * lightColor * brightness;

	result += (diffusePoint + ambientColPoint + specColorPoint) / attenuation;
	
	//				 r   g	  b   a		Ranges from 0->1
	//FragColor = vec4(0.7f,0.f,0.f, 1.f); //Sets the color of the Fragment

	//Apply it to the texture
	//Assign the texture color using the function
	//Divide the texture color by the attenuation factor to get the point light
	FragColor = vec4(result,1.0) * texture(tex0, texCoord);
}