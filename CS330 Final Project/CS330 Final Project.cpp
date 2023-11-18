/* 
* Matthew Muller
* SNHU CS-330 Final Project
* 10/23/22
*/

// Include Statements
#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <C:\OpenGL\OpenGL\GLEW\include\GL\glew.h>        // GLEW library
#include <C:\OpenGL\OpenGL\GLFW\include\GLFW\glfw3.h>     // GLFW library
// glm libraries
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <C:\Users\mattm\OneDrive\Documents\GitHub\CS-330\includes\learnOpengl\camera.h> // Camera class

#define STB_IMAGE_IMPLEMENTATION
#include <C:\Users\mattm\Downloads\CS-330-master-week4\CS-330-master\includes\stb_image.h>      // Image loading Utility functions


#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS330 Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 750;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
        float xPos, yPos, zPos;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Shader program
    GLuint gProgramId;
    GLuint nbgProgramId;
    // Texture id
    GLuint gTextureId;

    // Triangle mesh data
    GLMesh ptMesh;
    GLMesh pMesh;
    GLMesh cMesh;
    GLMesh dMesh;
    GLMesh nMesh;
    GLMesh lMesh;
    GLMesh cupMesh;
    
    // camera
    Camera gCamera(glm::vec3(0.0f, 4.0f, 8.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

/*
* Mesh Function Prototypes
*/
void CreatePencilTipMesh(GLMesh& mesh);
void CreatePencilMesh(GLMesh& mesh);
void CreateCoasterMesh(GLMesh& mesh);
void CreateCupMesh(GLMesh& mesh);
void CreateNotebookMesh(GLMesh& mesh);
void CreateLaptopMesh(GLMesh& mesh);
void CreateDeskMesh(GLMesh& mesh);

/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 2) in vec2 textureCoordinate;

out vec2 vertexTextureCoordinate;

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform sampler2D uTexture;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate); // Sends texture to the GPU for rendering
}
);



/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource2 = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource2 = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);


/*
*   MAIN FUNCTION
*/
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create meshes
    CreatePencilTipMesh(ptMesh);
    CreatePencilMesh(pMesh);
    CreateCoasterMesh(cMesh);
    CreateDeskMesh(dMesh);
    CreateNotebookMesh(nMesh);
    CreateLaptopMesh(lMesh);
    CreateCupMesh(cupMesh);

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(vertexShaderSource2, fragmentShaderSource2, nbgProgramId))
        return EXIT_FAILURE;
    
    // Load texture (relative to project's directory)
    const char* texFilename = "C:\\Users\\mattm\\Downloads\\notebook_texture.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(ptMesh);
    UDestroyMesh(pMesh);
    UDestroyMesh(cMesh);
    UDestroyMesh(dMesh);
    UDestroyMesh(nMesh);
    UDestroyMesh(lMesh);
    UDestroyMesh(cupMesh);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(nbgProgramId);
    // Release texture
    UDestroyTexture(gTextureId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        gCamera.ProcessKeyboard(VIEW, gDeltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set the shader to be used
    glUseProgram(nbgProgramId);
    
    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(nbgProgramId, "model");
    GLint viewLoc = glGetUniformLocation(nbgProgramId, "view");
    GLint projLoc = glGetUniformLocation(nbgProgramId, "projection");
    
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::mat4(1.0f);
    
    // View matrix : camera/view transformation
    glm::mat4 view;

    // Projection matrix : Creates a perspective/orthographic projection
    glm::mat4 projection;
    if (gCamera.ORTHO) {
        view = glm::lookAt(glm::vec3(5.0f, 2.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), -gCamera.WorldUp);
        projection = glm::ortho(-(GLfloat)WINDOW_WIDTH * 0.01f, (GLfloat)WINDOW_WIDTH * 0.01f, (GLfloat)WINDOW_HEIGHT * 0.01f, -(GLfloat)WINDOW_HEIGHT * 0.01f, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }
    

/*
*  DRAW OBJECTS  
*/

// Draw Desk Plane
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(dMesh.vao);

    // Apply transformations using model matrix
    model = glm::scale(glm::vec3(75.0f, 0.0f, 50.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, dMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

// Draw Pencil Tip Hexagonal Pyramid
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(ptMesh.vao);
    
    // Create pyramid using transformation loop
    for (int i = 0; i < 6; i++) {
        // Apply transformations using model matrix 
        model = glm::translate(glm::vec3(5.0f, 0.575f, -0.25f)); // Position strip at 0,0,0
        model = glm::rotate(model, glm::radians(60.0f * i), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array		
        model = glm::scale(model, glm::vec3(0.20f, 0.20f, 1.0f));
        
        //Copy model matrix to uniform
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //Draw triangles
        glDrawElements(GL_TRIANGLES, ptMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    }

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

// Draw Pencil Cylinder 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(pMesh.vao);
    
    // Create cylinder using transformation loop
    for (int i = 0; i < 6; i++) {
        // Apply transformations using model matrix 
        model = glm::translate(glm::vec3(5.0f, 0.575f, -0.25f)); // Position strip at 0,0,0
        model = glm::rotate(model, glm::radians(60.0f * i), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array		
        model = glm::scale(model, glm::vec3(0.20f, 0.20f, 2.75f));
        //Copy model matrix to uniform
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //Draw triangles
        glDrawElements(GL_TRIANGLES, pMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    }

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


// Draw Coaster Cylinder 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(cMesh.vao);

    // Create cylinder using transformation loop
    for (int i = 0; i < 20; i++) {
        // Apply transformations using model matrix 
        model = glm::translate(glm::vec3(-5.0f, 0.01f, 0.25f)); // Position strip at 0,0,0
        model = glm::rotate(model, glm::radians(18.0f * i), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate strip on z by increments in array		

        //Copy model matrix to uniform
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //Draw triangles
        glDrawElements(GL_TRIANGLES, cMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    }
    
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    

// Draw Cup Cylinder 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(cupMesh.vao);

    // Create cylinder using transformation loop
    for (int i = 0; i < 12; i++) {
        // Apply transformations using model matrix 
        model = glm::translate(glm::vec3(-5.0f, 0.11f, 0.25f)); // Position strip at 0,0,0
        model = glm::rotate(model, glm::radians(30.0f * i), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate strip on z by increments in array		
        model = glm::scale(model, glm::vec3(0.55f, 2.1f, 0.55f));

        //Copy model matrix to uniform
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //Draw triangles
        glDrawElements(GL_TRIANGLES, cupMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    }

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

// Draw Laptop Cuboids
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(lMesh.vao);

    // Apply transformations using model matrix
    model = glm::translate(glm::vec3(0.0f, 0.101f, -3.0f)); 
    model = glm::scale(model, glm::vec3(5.0f, 1.0f, 10.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draw lower half of laptop
    glDrawElements(GL_TRIANGLES, lMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    
    // Apply transformations using model matrix
    model = glm::translate(glm::vec3(0.0f, 1.202f, -3.0f)); 
    model = glm::scale(model, glm::vec3(5.0f, 10.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    // Draw upper half of laptop
    glDrawElements(GL_TRIANGLES, lMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

// Draw Notebook Cuboid    
    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    // Model matrix: transformations are applied right-to-left order
    model = glm::mat4(1.0f);

    // View matrix : camera/view transformation
    view;

    // Projection matrix : Creates a perspective/orthographic projection
    projection;
    if (gCamera.ORTHO) {
        view = glm::lookAt(glm::vec3(5.0f, 2.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), -gCamera.WorldUp);
        projection = glm::ortho(-(GLfloat)WINDOW_WIDTH * 0.01f, (GLfloat)WINDOW_WIDTH * 0.01f, (GLfloat)WINDOW_HEIGHT * 0.01f, -(GLfloat)WINDOW_HEIGHT * 0.01f, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }
    
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(nMesh.vao);

    // Apply transformations using model matrix
    model = glm::translate(glm::vec3(5.0f, 0.201f, 2.25f)); // Position strip at 0,0,0
    model = glm::scale(model, glm::vec3(4.5f, 4.0f, 5.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, nMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

/*
* Function Implementations
*/

// Implements the UDestroyMesh function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaderProgram function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

// Implements the UDestroyShaderProgram function
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}



/*
*  Mesh Function Implementations 
*/

// Implements the CreatePencilTipMesh function
void CreatePencilTipMesh(GLMesh& mesh)
{
    
    GLfloat verts[] = {
        //Vertex positions  /  colors
        1.0f, 0.0f,   0.0f,    0.933f, 0.875f, 0.600f, 1.0f, //Pyramid base vertex 1
        0.5f, 0.866f, 0.0f,    0.933f, 0.875f, 0.600f, 1.0f, //Pyramid base vertex 2
        0.0f, 0.0f,   0.0f,    0.933f, 0.875f, 0.600f, 1.0f, //Pyramid base center
        0.0f, 0.0f,  -0.6f,    0.0f,   0.0f,   0.0f,   1.0f, //Apex
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //Base triangle
        0, 1, 3     //Face triangle
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    
    //// unbind vao
    glBindVertexArray(0);
}

// Implements the CreatePencilMesh function
void CreatePencilMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        0.0f, 0.0f,  0.0f,    0.871f, 0.722f, 0.129f, 1.0f, //Bottom Left
        1.0f, 0.0f,  0.0f,    0.871f, 0.722f, 0.129f, 1.0f, //Pyramid Base Bottom Right
        0.5f, 0.866f, 0.0f,   0.871f, 0.722f, 0.129f, 1.0f, //Pyramid Base Top Left

        1.0f, 0.0f,  0.6f,    0.871f, 0.722f, 0.129f, 1.0f, //Pyramid Base Bottom Right
        0.5f, 0.866f, 0.6f,   0.871f, 0.722f, 0.129f, 1.0f, //Pyramid Base Top Left
        0.0f, 0.0f,  0.6f,    0.871f, 0.722f, 0.129f, 1.0f, //Pyramid Base Bottom Left

    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //
        1, 2, 3,    //
        2, 3, 4,
        3, 4, 5
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    //// unbind vao
    glBindVertexArray(0);
}

// Implements the CreateCoasterMesh function
void CreateCoasterMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        0.0f,   0.0f,   0.0f,    0.294f, 0.000f, 0.710f, 1.0f, //Bottom vertex 1
        1.0f,   0.0f,   0.0f,    0.294f, 0.000f, 0.710f, 1.0f, //Bottom vertex 2
        0.951f, 0.0f,   0.309f,  0.294f, 0.000f, 0.710f, 1.0f, //Bottom vertex 3
        
        1.0f,   0.1f,   0.0f,    0.294f, 0.000f, 0.710f, 1.0f, //Top vertex 1
        0.951f, 0.1f,   0.309f,  0.294f, 0.000f, 0.710f, 1.0f, //Top vertex 2
        0.0f,   0.1f,   0.0f,    0.294f, 0.000f, 0.710f, 1.0f, //Top vertex 3
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //PBase Triangle 1
        1, 2, 3,    //PBase Triangle 2 
        2, 3, 4,
        3, 4, 5
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    //// unbind vao
    glBindVertexArray(0);
}

// Implements the CreateCupMesh function
void CreateCupMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        0.0f,   0.0f,   0.0f,    0.8f, 0.4f, 0.0f, 1.0f, //Bottom vertex 1
        1.0f,   0.0f,   0.0f,    0.8f, 0.4f, 0.0f, 1.0f, //Bottom vertex 2
        0.866f, 0.0f,   0.5f,    0.8f, 0.4f, 0.0f, 1.0f, //Bottom vertex 3

        1.0f,   0.5f,   0.0f,    0.8f, 0.4f, 0.0f, 1.0f, //Top vertex 1
        0.866f, 0.5f,   0.5f,    0.8f, 0.4f, 0.0f, 1.0f, //Top vertex 2
        0.0f,   0.5f,   0.0f,    0.8f, 0.4f, 0.0f, 1.0f //Top vertex 3
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //PBase Triangle 1
        1, 2, 3,    //PBase Triangle 2 
        2, 3, 4,
        3, 4, 5
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    //// unbind vao
    glBindVertexArray(0);
}

// Implements the CreateNotebookMesh function
void CreateNotebookMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        -0.3f,  -0.05f,  0.0f,    0.171f, 0.522f, 0.129f, 1.0f,   0.0f, 0.0f,     //Bottom Lower Left
         0.3f,  -0.05f,  0.0f,    0.171f, 0.522f, 0.129f, 1.0f,   1.0f, 0.0f,     //Bottom Lower Right
        -0.3f,  0.05f,  0.0f,    0.171f, 0.522f, 0.129f, 1.0f,   0.0f, 0.0f,     //Bottom Base Top Left
         0.3f,  0.05f,  0.0f,    0.171f, 0.522f, 0.129f, 1.0f,   1.0f, 0.0f,     //Bottom Base Top Right

        -0.3f,  -0.05f,  -0.9f,    0.171f, 0.522f, 0.129f, 1.0f,   0.0f, 1.0f,     //Top Lower Left
         0.3f,  -0.05f,  -0.9f,    0.171f, 0.522f, 0.129f, 1.0f,   1.0f, 1.0f,     //Top Lower Right
        -0.3f,  0.05f,  -0.9f,    0.171f, 0.522f, 0.129f, 1.0f,   0.0f, 1.0f,     //Top Upper Left
         0.3f,  0.05f,  -0.9f,    0.171f, 0.522f, 0.129f, 1.0f,   1.0f, 1.0f,     //Top Upper Right

    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //PBase Triangle 1
        1, 2, 3,    //PBase Triangle 2 
        0, 2, 4,    //Left Face T1
        2, 4, 6,    //Left Face T2
        1, 3, 5,    //Right Face T1
        3, 5, 7,    //Right Face T2
        2, 3, 6,    //Top Face T1
        3, 6, 7,    //Top Face T2
        4, 5, 6,    //Bottom Face T1
        5, 6, 7     //Bottom Face T2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerTexture = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerTexture);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);

    //// unbind vao
    glBindVertexArray(0);
}

// Implements the CreateLaptopMesh function
void CreateLaptopMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        -0.4f, -0.1f,  0.0f,    0.171f, 0.022f, 0.829f, 1.0f, //Pyramid Base Bottom Left
         0.4f, -0.1f,  0.0f,    0.171f, 0.022f, 0.829f, 1.0f, //Pyramid Base Bottom Right
        -0.4f,  0.1f,  0.0f,    0.171f, 0.022f, 0.829f, 1.0f, //Pyramid Base Top Left
         0.4f,  0.1f,  0.0f,    0.171f, 0.022f, 0.829f, 1.0f, //Pyramid Base Top Right

        -0.4f, -0.1f,  0.25f,    0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left
         0.4f, -0.1f,  0.25f,    0.0f, 0.0f, 0.0f, 1.0f, //Bottom Right
        -0.4f,  0.1f,  0.25f,    0.0f, 0.0f, 0.0f, 1.0f, //Top Left
         0.4f,  0.1f,  0.25f,    0.0f, 0.0f, 0.0f, 1.0f, //Top Right
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //PBase Triangle 1
        1, 2, 3,    //PBase Triangle 2 
        0, 2, 4,    //Left Face T1
        2, 4, 6,    //Left Face T2
        1, 3, 5,    //Right Face T1
        3, 5, 7,    //Right Face T2
        2, 3, 6,    //Top Face T1
        3, 6, 7,    //Top Face T2
        4, 5, 6,    //Bottom Face T1
        5, 6, 7     //Bottom Face T2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    //// unbind vao
    glBindVertexArray(0);
}

//Implements the CreateDeskMesh function
void CreateDeskMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Vertex positions / colors
        -0.1f, 0.0f, -0.1f,    0.641f, 0.518f, 0.220f, 1.0f, //Bottom Left
         0.1f, 0.0f, -0.1f,    0.641f, 0.518f, 0.220f, 1.0f, //Bottom Right
        -0.1f, 0.0f,  0.1f,    0.641f, 0.518f, 0.220f, 1.0f, //Top Left
         0.1f, 0.0f,  0.1f,    0.641f, 0.518f, 0.220f, 1.0f, //Top Right
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //Triangle 1
        1, 2, 3,    //Triangle 2 
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    //// unbind vao
    glBindVertexArray(0);
}