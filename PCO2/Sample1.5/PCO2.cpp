#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//Modifier for the model's x Position
float x_mod = 0;
float y_mod = 0;
float z_mod = 0;

//Light Intensity
float pointBrightness_mod = 0;
float dirBrightness_mod = 0;

//Change between Object and Light Source
bool changeLight = false;

//Movement Flags
bool up = false;
bool down = false;
bool left = false;
bool right = false;

bool changeCamera = false;

//Variables for mouse input
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.f;
float pitch = 0.f;

//Escape key
bool escape = false;

//Toggle Light Colors
bool changeLightColor = false;

//Keyboard inputs
void Key_Callback(GLFWwindow* window, // the pointer to the window
    int key, // the keycode being pressed
    int scancode, // Physical position of the press on keyboard
    int action, // Either Press / Release
    int mods) //Which modifier keys is held down
{
    //Rotate right when user pressed D
    if (key == GLFW_KEY_D)
        y_mod += 5.f;
    //Rotate up when user pressed W
    if (key == GLFW_KEY_W)
        x_mod += 5.f;
    //Rotate left when user pressed A
    if (key == GLFW_KEY_A)
        y_mod -= 5.f;
    //Rotate down when user pressed S
    if (key == GLFW_KEY_S)
        x_mod -= 5.f;
    //Rotate back when user pressed Q
    if (key == GLFW_KEY_Q)
        z_mod -= 5.f;
    //Rotate front when user pressed E
    if (key == GLFW_KEY_E)
        z_mod += 5.f;

    //Adjust Point Light intensity
    if (key == GLFW_KEY_UP)
        pointBrightness_mod += 0.1f;

    //Adjust Point Light intensity
    if (key == GLFW_KEY_DOWN) {
        pointBrightness_mod -= 0.1f;
    }

    //Adjust Directional Light intensity
    if (key == GLFW_KEY_LEFT)
        dirBrightness_mod -= 0.1f;

    //Adjust Directional Light intensity
    if (key == GLFW_KEY_RIGHT) {
        dirBrightness_mod += 0.1f;
    }

    //Toggle between main object and light source
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        //toggle back to main obj
        if (changeLight == true) {
            changeLight = false;
            changeLightColor = false;
        }
        //toggle to light source
        else {
            changeLight = true;
            changeLightColor = true;
        }

    //Down
    if (key == GLFW_KEY_K) {
        if (action == GLFW_PRESS)
            down = true;
        else if (action == GLFW_RELEASE)
            down = false;
    }
    //Up
    if (key == GLFW_KEY_I) {
        if (action == GLFW_PRESS)
            up = true;
        else if (action == GLFW_RELEASE)
            up = false;
    }

    //Left
    if (key == GLFW_KEY_J) {
        if (action == GLFW_PRESS)
            left = true;
        else if (action == GLFW_RELEASE)
            left = false;
    }

    //Right
    if (key == GLFW_KEY_L) {
        if (action == GLFW_PRESS)
            right = true;
        else if (action == GLFW_RELEASE)
            right = false;
    }

    //Change to Perspective camera
    if (key == GLFW_KEY_1) {
        changeCamera = false;
    }
    //Change to Orthographic camera
    if (key == GLFW_KEY_2) {
        changeCamera = true;
    }
    //Escape window
    if (key == GLFW_KEY_ESCAPE) {
        escape = true;
    }
}

//Call Mouse
//Source::learnopengl.com/Getting-started/Camera
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

//Forward Declare Light
class Light;
class PointLight;

//Create Model
class Model3D {

//Fields for Model
private:
    const char* v; //vertex from Sample.vert
    const char* f; // frag from Sample.frag

    int img_width, //Width of the texture
        img_height, //Height of the texture
        colorChannels; //Number of color channels
    unsigned char* tex_bytes; // Tex_bytes

    //Shaders
    GLuint texture;
    GLuint vertexShader;
    GLuint fragShader;
    GLuint shaderProg;

    //Obj file attributes
    std::string path;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> material;
    std::string warning, error;

    tinyobj::attrib_t attributes;
    bool success;

    std::vector<GLuint> mesh_indices;
    std::vector<GLfloat> fullVertexData;

    //VertexArrayObject and VertexBufferObject
    GLuint VAO, VBO;

    //Matrices
    glm::mat4 identity_matrix4 = glm::mat4(1.0f);
    glm::mat4 transformation_matrix;

public:
    //Constructor & Destructor
    Model3D() {}

    //Delete Vertex Object
    ~Model3D() {
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->VBO);
    }

//Methods
public:
    //read vertex and frag shader file
    void setShaders(const char* v, const char* f) {
        this->v = v;
        this->f = f;
    }

    //Create the texture and the object
    void setTextureAndObj(std::string image, std::string obj) {
        //Texture
        stbi_set_flip_vertically_on_load(true);

        this->tex_bytes =
            stbi_load(image.c_str(), //Texture path
                &this->img_width, //Fills out the width
                &this->img_height, //Fills out the height
                &this->colorChannels, //Fiills out the colo channels
                0);

        //Obj
        this->path = obj.c_str();

        this->success = tinyobj::LoadObj(
            &this->attributes,
            &this->shapes,
            &this->material,
            &this->warning,
            &this->error,
            this->path.c_str()
        );

    }

private:
    //Generate textures
    void createTexture() {
        //Generate reference
        glGenTextures(1, &this->texture);
        //Set the current texture we're
        //working
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, this->texture);


        //Assign the loaded teexture
        //to the OpenGL reference
        glTexImage2D(GL_TEXTURE_2D,
            0, //Texture 0
            GL_RGB, // Target color format of the texture
            this->img_width, // Texture width
            this->img_height,// Texture height
            0,
            GL_RGB,    //Color format of the texturue
            GL_UNSIGNED_BYTE, //Data type of texture
            this->tex_bytes); // Loaded texture in bytes

        //Generate thhe mipmaps to the current texture
        glGenerateMipmap(GL_TEXTURE_2D);

        //Free uo the loaded bytes
        stbi_image_free(this->tex_bytes);
    }

    //Compile vertex and frag shaders into one
    void compileShaders() {
        //Create a Vertex Shader
        this->vertexShader = glCreateShader(GL_VERTEX_SHADER);

        //Assign the source to the Vertex Shader
        glShaderSource(this->vertexShader, 1, &this->v, NULL);

        //Compile the Vertex Shader
        glCompileShader(this->vertexShader);

        //Create a Fragment Shader
        this->fragShader = glCreateShader(GL_FRAGMENT_SHADER);

        //Assign the source to the Fragment Shader
        glShaderSource(this->fragShader, 1, &this->f, NULL);

        //Compile the Fragment Shader
        glCompileShader(this->fragShader);

        //Create the Shader Program
        this->shaderProg = glCreateProgram();
        //Attach the compiled Vertex Shader
        glAttachShader(this->shaderProg, this->vertexShader);
        //Attach the compiled Fragment Shader
        glAttachShader(this->shaderProg, this->fragShader);

        //Finalize the compilation process
        glLinkProgram(this->shaderProg);
    }

    //set the Vertex and texture data of the object
    void setVertAndTex() {
        for (int i = 0; i < this->shapes[0].mesh.indices.size(); i++) {
            this->mesh_indices.push_back(
                this->shapes[0].mesh.indices[i].vertex_index
            );
        }

        for (int i = 0; i < this->shapes[0].mesh.indices.size(); i++) {

            //Assign the Index data for easy access
            tinyobj::index_t vData = this->shapes[0].mesh.indices[i];

            //Push the X position of the vertex
            this->fullVertexData.push_back(
                //Multiply the index by 3 to get the base offset
                this->attributes.vertices[(vData.vertex_index * 3)]
            );

            //Push the Y position of the vertex
            this->fullVertexData.push_back(
                //Add the base offset to 1 to get Y
                this->attributes.vertices[(vData.vertex_index * 3) + 1]
            );

            //Push the Z position of the vertex
            this->fullVertexData.push_back(
                //Add the base offset to 2 to get Z
                this->attributes.vertices[(vData.vertex_index * 3) + 2]
            );

            this->fullVertexData.push_back(
                //Add the base offset to 2 to get X
                this->attributes.normals[(vData.normal_index * 3)]
            );

            this->fullVertexData.push_back(
                //Add the base offset to 2 to get Y
                this->attributes.normals[(vData.normal_index * 3) + 1]
            );

            this->fullVertexData.push_back(
                //Add the base offset to 2 to get Z
                this->attributes.normals[(vData.normal_index * 3) + 2]
            );

            //Push the U of the Tex Coords
            this->fullVertexData.push_back(
                //Multiply the index by 3 to get the base offset
                this->attributes.texcoords[(vData.texcoord_index * 2)]
            );

            //Push the V of the Tex Coords
            this->fullVertexData.push_back(
                //Add the base offset to 1 to get V
                this->attributes.texcoords[(vData.texcoord_index * 2) + 1]
            );
        }

    }

public:
    //Createing thhe model
    void createModel() {
        //Call the neccessary functions to create model
        this->createTexture();
        this->compileShaders();
        this->setVertAndTex();

        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);

        //Generate VAO
        glGenVertexArrays(1, &this->VAO);

        //Generate VBO
        glGenBuffers(1, &this->VBO);


        //Bind VAO and VBO
        glBindVertexArray(this->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        //Position
        glBufferData(
            GL_ARRAY_BUFFER,
            //Size of the whole array in bytes
            sizeof(GLfloat) * this->fullVertexData.size(),
            //Data of the array
            this->fullVertexData.data(),
            GL_DYNAMIC_DRAW
        );

        glVertexAttribPointer(
            0, //index 0 is the vertex position
            3, //Position is 3 floats (x,y,z)
            GL_FLOAT, // Data type of array
            GL_FALSE,

            //Our vertex data has 8 floats in it
            //(X,Y,Z,Normals,U,V)
            8 * sizeof(GLfloat),//size of the vertex data in bytes
            (void*)0
        );

        glEnableVertexAttribArray(0);

        //Since our UV starts at index 3
        //or the 4th of our index data

        //Normalize
        GLintptr normalPtr = 3 * sizeof(float);

        glVertexAttribPointer(
            1, //index 0 is the vertex position
            3, //Position is 3 floats (x,y,z)
            GL_FLOAT, // Data type of array
            GL_FALSE,

            //Our vertex data has 8 floats in it
            //(X,Y,Z,Normals,U,V)
            8 * sizeof(GLfloat),//size of the vertex data in bytes
            (void*)normalPtr
        );

        glEnableVertexAttribArray(1);

        
        //UV
        GLintptr uvPtr = 6 * sizeof(float);

        glVertexAttribPointer(
            2, //index 0 is the vertex position
            2, //Position is 3 floats (x,,z)
            GL_FLOAT, // Data type of array
            GL_FALSE,

            //Our vertex data has 8 floats in it
            //(X,Y,Z,Normals,U,V)
            8 * sizeof(GLfloat),//size of the vertex data in bytes
            (void*)uvPtr
        );
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //Currently editing VBO = null

        //Currently editing VAO = VAO
        glBindVertexArray(0);
        //Currently editing VAO = null

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    //Set Object Position
    void updateTranslate(float translate_x, float translate_y, float translate_z) {
        this->transformation_matrix =
            glm::translate(this->identity_matrix4,
                glm::vec3(translate_x, translate_y, translate_z)
            );
    }
    //Scale the Object
    void updateScale(float scale_x, float scale_y, float scale_z) {
        this->transformation_matrix =
            glm::scale(this->transformation_matrix,
                glm::vec3(scale_x, scale_y, scale_z)
            );
    }

    //Rotate the Object
    void updateRotation(float rotate_x, float rotate_y, float rotate_z) {
        //X Rotation
        this->transformation_matrix =
            glm::rotate(this->transformation_matrix,
                glm::radians(rotate_x),
                glm::normalize(glm::vec3(1.f, 0.f, 0.f))
            );
        //Y Rotation
        this->transformation_matrix =
            glm::rotate(this->transformation_matrix,
                glm::radians(rotate_y),
                glm::normalize(glm::vec3(0.f, 1.f, 0.f))
            );
        //Z Rotation
        this->transformation_matrix =
            glm::rotate(this->transformation_matrix,
                glm::radians(rotate_z),
                glm::normalize(glm::vec3(0.f, 0.f, 1.f))
            );
    }

    //Revolve object around a center
    void updateRevolution(float revolve_x, float revolve_y, float revolve_z, float rotate_x,
        float rotate_y, float rotate_z, PointLight* lightPos);

    //Updating Transformation matrix
    void update() {

        unsigned int transformLoc = glGetUniformLocation(this->shaderProg, "transform");

        glUniformMatrix4fv(transformLoc,
            1,
            GL_FALSE,
            glm::value_ptr(this->transformation_matrix));
    }
    //Render Texture with light
    void renderTexture(Light* light, glm::vec3 cameraPos);

    //Render the Complete object
    void perform(Light* light, glm::vec3 cameraPos) {
        //Update object
        this->update();
        //Render Texture
        this->renderTexture(light, cameraPos);


        glBindVertexArray(this->VAO);

        //Rendering the model
        glDrawArrays(GL_TRIANGLES, 0, this->fullVertexData.size() / 8);
    }

    //Getters
    GLuint getShaderProg() {
        return this->shaderProg;
    }
};

//Create Camera Abstract Class
class MyCamera {
//Camera Fields
protected:
    //Matrix for the projection
    glm::mat4 projectionMatrix;

    //The camera's poistion
    glm::vec3 cameraPos;
    glm::mat4 cameraPositionMatrix;

    //The camera's eye
    glm::vec3 WorldUp;
    glm::vec3 Center;
    glm::vec3 F;
    glm::vec3 R;
    glm::vec3 U;

    //The camera's orientation
    glm::mat4 cameraOrientation;

    //The View of the Camera
    glm::mat4 viewMatrix;

    //Window height and width
    float window_height;
    float window_width;

//Default Constructor
public:
    MyCamera() {}


public:
    //Pure virtual function to implement projection type
    virtual void createProjection() = 0;
private:  
    //Create the initial camera position
    void createCameraPos() {
        this->cameraPos = glm::vec3(0.f, 3.f, 10.f);

        //Construct the Position Matrix

        this->cameraPositionMatrix =
            glm::translate(glm::mat4(1.0f), //Intialize it as an Identity Matrix
                this->cameraPos * -1.f); //Multiply to -1 since we need -P

        //World's Up Direction
        //Normally just 1 in Y
        this->WorldUp = glm::vec3(0, 1.f, 0);

        //Camera's Center
        this->Center = glm::vec3(0.f, 1.f, 0.f);

        //Get Forward
        this->F = glm::vec3(this->Center - this->cameraPos);

        //Normalize the Forward
        this->F = glm::normalize(this->F);

        this->R = glm::normalize(
            //R x F
            glm::cross(this->F, WorldUp)
        );

        this->U = glm::normalize(
            //R x F
            glm::cross(this->R, this->F)
        );
    }

    //Create the initial camera orientation
    void createCameraOrientation() {
        //Construct the Camera Orientation Matrix
        this->cameraOrientation = glm::mat4(1.f);

        //Manually assign the Matrix
        //Matrix[Column][Row]
        this->cameraOrientation[0][0] = R.x;
        this->cameraOrientation[1][0] = R.y;
        this->cameraOrientation[2][0] = R.z;

        this->cameraOrientation[0][1] = U.x;
        this->cameraOrientation[1][1] = U.y;
        this->cameraOrientation[2][1] = U.z;

        this->cameraOrientation[0][2] = -F.x;
        this->cameraOrientation[1][2] = -F.y;
        this->cameraOrientation[2][2] = -F.z;
    }

    //Create the initial camera view
    void createCameraView(){
        //Camera View Matrix
        this->viewMatrix = cameraOrientation * cameraPositionMatrix;
    }
public:
    //Create the Camera
    void createCamera() {
        this->createProjection();
        this->createCameraPos();
        this->createCameraOrientation();
        this->createCameraView();
    }
public:
    //Render the camera
    void render(GLuint shaderProg) {
        unsigned int projectionLoc = glGetUniformLocation(shaderProg, "projection");

        glUniformMatrix4fv(projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(this->projectionMatrix));

        unsigned int viewLoc = glGetUniformLocation(shaderProg, "view");

        glUniformMatrix4fv(viewLoc,
            1,
            GL_FALSE,
            glm::value_ptr(this->viewMatrix)); //View Matrix
    }
    //Virtual function for children to edit
    virtual void perform(GLuint shaderProg) {
        this->render(shaderProg);
    }

    //Getters
    glm::vec3 getCameraPos() {
        return this->cameraPos;
    }

};

//Create Camera with Orthographic Projection
class OrthoCamera :
    public MyCamera {
    //Constructor
public:
    OrthoCamera(float window_height, float window_width) : MyCamera() {
        this->window_height = window_height;
        this->window_width = window_width;
    }

public:
    //Create Orthographic Projection
    void createProjection() {
        this->projectionMatrix = glm::ortho(-10.0f, //Left
            10.0f, //Right
            -10.0f, //Bottom
            10.0f, //Top
            -0.1f, //Z-Near
            100.f); //Z-Far
    }
private:
    //New camera position
    void updateCameraPos() {
        this->cameraPos = glm::vec3(0.f, 20.f, 0.f);

        //Construct the Position Matrix

        this->cameraPositionMatrix =
            glm::translate(glm::mat4(1.0f), //Intialize it as an Identity Matrix
                this->cameraPos * -1.f); //Multiply to -1 since we need -P

        //World's Up Direction
        //Normally just 1 in Y
        this->WorldUp = glm::vec3(0, 0.f, -1.f);

        //Camera's Center
        this->Center = glm::vec3(0.f, 1.f, 0.f);

        //Get Forward
        this->F = glm::vec3(this->Center - this->cameraPos);

        //Normalize the Forward
        this->F = glm::normalize(this->F);

        this->R = glm::normalize(
            //R x F
            glm::cross(this->F, WorldUp)
        );

        this->U = glm::normalize(
            //R x F
            glm::cross(this->R, this->F)
        );
    }
    //Update view withh lookAt function
    //Source::learnopengl.com/Getting-started/Camera
    void updateViewMatrix() {
        //Using the lookAt function for easy calculation of camera orientation and camera position
        this->viewMatrix = glm::lookAt(this->cameraPos, this->cameraPos + this->F, this->U);
    }

    //Update function that performs all updates
    void update() {
        this->updateCameraPos();
        this->updateViewMatrix();
    }

public:
    //Perfrom camera
    void perform(GLuint shaderProg){
        this->update();
        this->render(shaderProg);
    }

};

//Create Camera with Perspective Projection
class PerspectiveCamera :
    public MyCamera {
public:
    //Constructor
    PerspectiveCamera(float window_height, float window_width) : MyCamera() {
        this->window_height = window_height;
        this->window_width = window_width;
    }

public:
    //Create Perpective Projection
    void createProjection() {
        this->projectionMatrix = glm::perspective(
            glm::radians(90.f), //FOV
            this->window_height / this->window_width, //Aspect ratio
            0.1f, //ZNear > 0
            100.f // ZFar
        );

    }
private:
    //Update camera's position
    //Source::learnopengl.com/Getting-started/Camera
    void updateCameraPos() {
        float speed = 0.005f;

        if (up)
            this->cameraPos += speed * this->F;
        if (down)
            this->cameraPos -= speed * this->F;
        if (left)
            this->cameraPos -= glm::normalize(glm::cross(this->F, this->U)) * speed;
        if (right)
            this->cameraPos += glm::normalize(glm::cross(this->F, this->U)) * speed;

        this->cameraPositionMatrix =
            glm::translate(glm::mat4(1.0f), //Intialize it as an Identity Matrix
                this->cameraPos * -1.f); //Multiply to -1 since we need -P


    }

    //Update camera's orientation
    //Source::learnopengl.com/Getting-started/Camera
    void updateCameraOrientation() {

        //Direction vector
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        //Normalize F
        this->F = glm::normalize(direction);


    }

    //Update view withh lookAt function
    //Source::learnopengl.com/Getting-started/Camera
    void updateViewMatrix() {
        //Using the lookAt function for easy calculation of camera orientation and camera position
        this->viewMatrix = glm::lookAt(this->cameraPos, this->cameraPos + this->F, this->U);
    }

    //Update function that performs all updates
    void update() {
        this->updateCameraPos();
        this->updateCameraOrientation();
        this->updateViewMatrix();
    }
public:
    //Perform Camera
    void perform(GLuint shaderProg) {
        this->update();
        this->render(shaderProg);
    }


};

//Create Light Abstract Class
class Light {
protected:
    //Light Fields
   
    //Light Color
    glm::vec3 lightColor;

    //Ambient strength
    float ambientStr;

    //Ambient Color
    glm::vec3 ambientColor;

    //Spec strength
    float specStr;

    //Spec phong
    float specPhong;

    //Light Brightness
    float brightness;

    //Light Type
    int lightType;

    //Variable address of frag shaders
    GLuint tex0Address;
    GLuint lightColorAddress;
    GLuint ambientStrAddress;
    GLuint ambientColorAddress;
    GLuint cameraPosAddress;
    GLuint specStrAddress;
    GLuint specPhongAddress;
    GLuint brightnessAddress;

public:
    //Constructor
    Light(glm::vec3 lightColor, glm::vec3 ambientColor, float ambientStr,
        float specStr, float specPhong, float brightness) {
        this->lightColor = lightColor;
        this->ambientColor = ambientColor;
        this->ambientStr = ambientStr;
        this->specStr = specStr;
        this->specPhong = specPhong;
        this->brightness = brightness;
    }

    //Pure Virtual Function for children to implement
    virtual void createLight(GLuint shaderProg, GLuint texture, glm::vec3 cameraPos) = 0;
};

//Create Point Light from Light
class PointLight : 
    public Light {
    //Point Light Fields
private:
    glm::vec3 lightPos; //Position of light

    //Attentuation factor
    float constant;
    float linear;
    float exponent;

    //Neccessary address to frag Shader
    GLuint linearAddress;
    GLuint constantAddress;
    GLuint exponentAddress;
    GLuint lightAddress;

public:
    //Constructor
    PointLight(glm::vec3 lightColor, glm::vec3 ambientColor, float ambientStr,
        float specStr, float specPhong, float brightness, 
        glm::vec3 lightPos, float constant, float linear, float exponent) :
        Light(lightColor, ambientColor, ambientStr, specStr, specPhong, brightness) {
        this->lightPos = lightPos;
        this->constant = constant;
        this->linear = linear;
        this->exponent = exponent;
    }

    //Creating the Light Source
    void createLight(GLuint shaderProg, GLuint texture, glm::vec3 cameraPos) {

        glActiveTexture(GL_TEXTURE0);

        this->tex0Address = glGetUniformLocation(shaderProg, "tex0");

        glBindTexture(GL_TEXTURE_2D, texture);

        this->lightAddress = glGetUniformLocation(shaderProg, "lightPos");
        glUniform3fv(this->lightAddress, 1, glm::value_ptr(this->lightPos));

        this->lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(this->lightColorAddress, 1, glm::value_ptr(this->lightColor));

        this->ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(this->ambientStrAddress, this->ambientStr);

        this->ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(this->ambientColorAddress, 1, glm::value_ptr(this->ambientColor));

        this->cameraPosAddress = glGetUniformLocation(shaderProg, "cameraPos");
        glUniform3fv(this->cameraPosAddress, 1, glm::value_ptr(cameraPos));

        this->specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(this->specStrAddress, this->specStr);

        this->specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(this->specPhongAddress, this->specPhong);

        this->constantAddress = glGetUniformLocation(shaderProg, "constant");
        glUniform1f(this->constantAddress, this->constant);

        this->linearAddress = glGetUniformLocation(shaderProg, "linear");
        glUniform1f(this->linearAddress, this->linear);

        this->exponentAddress = glGetUniformLocation(shaderProg, "exponent");
        glUniform1f(this->exponentAddress, this->exponent);

        this->brightnessAddress = glGetUniformLocation(shaderProg, "brightness");
        glUniform1f(this->brightnessAddress, this->brightness);

        glUniform1i(this->tex0Address, 0);
    }

    //Getters & Setters
    glm::vec3 getLightPos() {
        return this->lightPos;
    }

    void setLightPos(glm::vec3 lightPos) {
        this->lightPos = lightPos;
    }

    void setIntensity(float brightness) {
        this->brightness = brightness;
    }
    void setColor(glm::vec3 lightColor) {
        this->lightColor = lightColor;
    }
};

//Create Directional Light from Light
class DirectionLight :
    public Light {
private:
    //Direction of light
    glm::vec3 lightDirection;

    //light Address to frag Shader
    GLuint lightAddress;
public:
    //Constructor
    DirectionLight(glm::vec3 lightColor, glm::vec3 ambientColor, float ambientStr,
        float specStr, float specPhong, float brightness, glm::vec3 lightDirection) :
        Light(lightColor, ambientColor, ambientStr, specStr, specPhong, brightness){
        this->lightDirection = lightDirection;
    }

    //Create the light source
    void createLight(GLuint shaderProg, GLuint texture, glm::vec3 cameraPos) {
        glActiveTexture(GL_TEXTURE0);

        this->tex0Address = glGetUniformLocation(shaderProg, "tex0");

        glBindTexture(GL_TEXTURE_2D, texture);


        this->lightAddress = glGetUniformLocation(shaderProg, "lightDirection");
        glUniform3fv(this->lightAddress, 1, glm::value_ptr(this->lightDirection));

        this->lightColorAddress = glGetUniformLocation(shaderProg, "lightColor");
        glUniform3fv(this->lightColorAddress, 1, glm::value_ptr(this->lightColor));

        this->ambientStrAddress = glGetUniformLocation(shaderProg, "ambientStr");
        glUniform1f(this->ambientStrAddress, this->ambientStr);

        this->ambientColorAddress = glGetUniformLocation(shaderProg, "ambientColor");
        glUniform3fv(this->ambientColorAddress, 1, glm::value_ptr(this->ambientColor));

        this->cameraPosAddress = glGetUniformLocation(shaderProg, "cameraPos");
        glUniform3fv(this->cameraPosAddress, 1, glm::value_ptr(cameraPos));

        this->specStrAddress = glGetUniformLocation(shaderProg, "specStr");
        glUniform1f(this->specStrAddress, this->specStr);

        this->specPhongAddress = glGetUniformLocation(shaderProg, "specPhong");
        glUniform1f(this->specPhongAddress, this->specPhong);

        this->brightnessAddress = glGetUniformLocation(shaderProg, "brightness");
        glUniform1f(this->brightnessAddress, this->brightness);

        glUniform1i(this->tex0Address, 0);
    }

    //Setter
    void setIntensity(float brightness) {
        this->brightness = brightness;
    }
};

int main(void)
{
    //Instantiate the two objects
    Model3D object;
    Model3D object2;

    //Load the shader file into a string stream
    std::fstream vertSrc("Shaders/Sample.vert");
    std::stringstream vertBuff;

    //Add the file stream to the string stream
    vertBuff << vertSrc.rdbuf();

    //Convert the stream to a character array
    std::string vertS = vertBuff.str();
    const char* v = vertS.c_str();

    //Load the shader file into a string stream
    std::fstream fragSrc("Shaders/Sample.frag");
    std::stringstream fragBuff;

    //Add the file stream to the string stream
    fragBuff << fragSrc.rdbuf();

    //Convert the stream to a character array
    std::string fragS = fragBuff.str();
    const char* f = fragS.c_str();

    //Set the shaders to the object
    object.setShaders(v, f);
    object2.setShaders(v, f);

    //Create window
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(600, 600, "Mathieu Pobre", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    //Window height and width
    float window_widith = 600;
    float window_heigth = 600;

    //Create Camera Projections
    //Create Perspective Projection
    MyCamera* cameraPerspective = new PerspectiveCamera(window_heigth, window_widith);
    //Type-cast MyCamera to PerspectiveCamera to use child methods
    PerspectiveCamera* pCameraPerspective = (PerspectiveCamera*)cameraPerspective;

    //Creaete Orthographic Projection
    MyCamera* cameraOrtho = new OrthoCamera(window_heigth, window_widith);
    OrthoCamera* pCameraOrtho = (OrthoCamera*)cameraOrtho;
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL();

    //Hydrant obj and png file source:
    //cgtrader.com/free-3d-models/industrial/industrial-machine/fire-hydrant-7ba25670-3f38-4a77-a0c9-56ce888c9df2
    object.setTextureAndObj("3D/hydrant_BaseColor.png", "3D/hydrant_low.obj");

    //Brick obj file source:
    //cgtrader.com/free-3d-models/architectural/decoration/red-brick-lowpoly-pack-of-bricks-blocks-low-poly 
    //Brick png source: freepik.com/free-photos-vectors/white-background
    object2.setTextureAndObj("3D/white.jpg", "3D/redBrick.obj");

    //Keyboard and Mouse inputs
    glfwSetKeyCallback(window, Key_Callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    //MODEL 1
    object.createModel();
 
    //MODEL 2
    object2.createModel();
    

    //CAMERA 1
    cameraPerspective->createCamera();
    //CAMERA 2
    cameraOrtho->createCamera();


    //center of screen
    glm::vec3 center = glm::vec3(0, 0, 0);

    //Position of Light
    glm::vec3 lightPos = glm::vec3(4, -5, 0);

    //Direction of light
    glm::vec3 lightDirection = glm::normalize(center - lightPos);

    //Position of Light
    glm::vec3 lightPosPoint = glm::vec3(-8, 0, 0);

    //Light Color
    glm::vec3 lightColor = glm::vec3(1, 1, 1);

    //Ambient strength
    float ambientStr = 0.1f;

    //Ambient Color
    glm::vec3 ambientColor = lightColor;

    //Spec strength
    float specStr = 10.0f;

    //Spec phong
    float specPhong = 50.f;

    //Constant
    float constant = 1.f;

    //Linear
    float linear = 0.1f;

    //Exponent
    float exponent = 0.f;

    //Brightness
    float brightness = 1.f;
    
    //Instantiate point light and directional light by type-casting light 
    Light* pointLight = new PointLight(lightColor,ambientColor,ambientStr,specStr,
        specPhong,brightness,lightPosPoint,constant,linear,exponent);

    PointLight* pPointLight = (PointLight*)pointLight;

    Light* directionlight = new DirectionLight(lightColor, ambientColor, ambientStr, specStr,
        specPhong, brightness, lightDirection);

    DirectionLight* pDirectionlight = (DirectionLight*)directionlight;


    float last_x = 0.f, last_y = 0.f, last_z = 0.f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && !escape)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        
        glUseProgram(object.getShaderProg());

        //Set Light intensity
        pPointLight->setIntensity(pointBrightness_mod);
        pDirectionlight->setIntensity(dirBrightness_mod);

        //Toggle Perspcetive & Orthographic Camera

        //Perspective Camera
        if (!changeCamera) {
            //Make Perspective Camera
            pCameraPerspective->perform(object.getShaderProg());

            //Set Position and Scale of MODEL1
            object.updateTranslate(0.f, 0.f, 0.f);
            object.updateScale(0.05f, 0.05f, 0.05f);

            //Toggle between Main obj and Light Source
            if (!changeLight) {
                object.updateRotation(x_mod, y_mod, z_mod);
                last_x = x_mod;
                last_y = y_mod;
                last_z = z_mod;
                pPointLight->setColor(glm::vec3(1.f, 1.f, 1.f));
            }
            else {
                //Remember last position
                object.updateRotation(last_x, last_y, last_z);
            }
            //Render MODEL1
            object.perform(directionlight, pCameraPerspective->getCameraPos());

            //Set Position and Scale of MODEL2
            object2.updateTranslate(-8.0f, 0.f, 0.f);
            object2.updateScale(10.f, 10.f, 10.f);

            //Toggle
            if (changeLight) {
                //Revolve Light source around Main obj
                object2.updateRevolution(8.f, 0.f, 0.f, x_mod, y_mod, z_mod, pPointLight);
                object2.updateScale(10.f, 10.f, 10.f);
                pPointLight->setColor(glm::vec3(1.f, 3.f, 1.f));

            }
            //Render MODEL2
            object2.perform(pPointLight, pCameraPerspective->getCameraPos());
        }

        //Orthographic Camera
        else {
            //Make OrthoGraphic Camera
            pCameraOrtho->perform(object.getShaderProg());

            //Set Position and Scale of MODEL1
            object.updateTranslate(0.f, 0.f, 0.f);
            object.updateScale(0.05f, 0.05f, 0.05f);

            //Toggle between Main obj and Light Source
            if (!changeLight) {
                object.updateRotation(x_mod, y_mod, z_mod);
                last_x = x_mod;
                last_y = y_mod;
                last_z = z_mod;
                pPointLight->setColor(glm::vec3(1.f, 1.f, 1.f));
            }
            else {
                //Remember last position
                object.updateRotation(last_x, last_y, last_z);
            }

            //Render MODEL1
            object.perform(directionlight, pCameraOrtho->getCameraPos());

            //Set Position and Scale of MODEL2
            object2.updateTranslate(-8.0f, 0.f, 0.f);
            object2.updateScale(10.f, 10.f, 10.f);

            //Toggle
            if (changeLight) {

                //Revolve light source around Main obj
                object2.updateRevolution(8.f, 0.f, 0.f, x_mod, y_mod, z_mod, pPointLight);
                object2.updateScale(10.f, 10.f, 10.f);
                pPointLight->setColor(glm::vec3(1.f, 3.f, 1.f));

            }
            //Render MODEL2
            object2.perform(pPointLight, pCameraOrtho->getCameraPos());
        }
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void Model3D::renderTexture(Light* light, glm::vec3 cameraPos) {

    light->createLight(this->shaderProg, this->texture, cameraPos);
}

void Model3D::updateRevolution(float revolve_x, float revolve_y, float revolve_z, float rotate_x,
    float rotate_y, float rotate_z, PointLight* lightPos) {
    //Set Postion
    this->transformation_matrix = glm::translate(this->identity_matrix4, glm::vec3(-revolve_x, -revolve_y, -revolve_z));
    this->transformation_matrix = glm::translate(this->transformation_matrix, glm::vec3(revolve_x, revolve_y, revolve_z));

    // Calculate rotation matrices for each axis
    this->transformation_matrix =
        glm::rotate(this->transformation_matrix,
            glm::radians(rotate_x),
            glm::normalize(glm::vec3(1.f, 0.f, 0.f))
        );
    this->transformation_matrix =
        glm::rotate(this->transformation_matrix,
            glm::radians(rotate_y),
            glm::normalize(glm::vec3(0.f, 1.f, 0.f))
        );
    this->transformation_matrix =
        glm::rotate(this->transformation_matrix,
            glm::radians(rotate_z),
            glm::normalize(glm::vec3(0.f, 0.f, 1.f))
        );

    // Calculate translation matrix to move object back from center point
    this->transformation_matrix = glm::translate(this->transformation_matrix, glm::vec3(-revolve_x, -revolve_y, -revolve_z));

  

    /*glm::vec4 ligthPositionToObjectPosition = this->transformation_matrix * glm::vec4(lightPos->getLightPos(), 1.0f);

    lightPos->setLightPos(glm::vec3(ligthPositionToObjectPosition));

    std::cout << lightPos->getLightPos().x << " " << lightPos->getLightPos().y << " " << lightPos->getLightPos().z << std::endl;*/

}