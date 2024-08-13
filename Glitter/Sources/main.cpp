// Local Headers
#include "glitter.hpp"
#include <Shader.hpp>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <direct.h>

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // OpenCL initialization
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found.\n";
        exit(1);
    }

    default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    cl_context_properties properties[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
      CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)default_platform(),
      NULL
    };

    cl_int err = CL_SUCCESS;
    context = clCreateContext(properties, 1, &default_device(), NULL, NULL, &err);

    if (err != CL_SUCCESS) {
        std::cout << "Error creating context" << " " << err << "\n";
        //exit(-1);
    }

    queue = cl::CommandQueue(context, default_device);
    
    // Read kernel source
    char buffer[1024];
    getcwd(buffer, 1024);
    std::string kernel_char(buffer);
    kernel_char += "\\..\\Glitter\\Sources\\gpu_src\\test.cl";
    kernel_source = ReadFile2(kernel_char.c_str());
    sources.push_back({ kernel_source.c_str(), kernel_source.length() });

    // Build program and compile
    program = cl::Program(context, sources);

    if (program.build({ default_device }) != CL_SUCCESS)
    {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }

    // Prepare buffers
    debug_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * mWidth * mHeight);

    // OpenGL shaders
    std::string vs_char(buffer);
    std::string fs_char(buffer);
    vs_char += "\\..\\Glitter\\Shaders\\simple_shader.vs";
    fs_char += "\\..\\Glitter\\Shaders\\simple_shader.fs";
    //Shader simple_shader("C:/Repos/2D_Fluids/Glitter/Shaders/simple_shader.vs", "C:/Repos/2D_Fluids/Glitter/Shaders/simple_shader.fs");
    Shader simple_shader(vs_char.c_str(), fs_char.c_str());

    // Setup OpenGL Buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hardcoded_vertices), hardcoded_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // OpenGL texture
    unsigned int gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    std::string tex_char(buffer);
    tex_char += "\\..\\textures\\wall.jpg";
    unsigned char* data = stbi_load(tex_char.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);

    target_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    // Flush GL queue        
    glFinish();
    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;

    tester = cl::Kernel(program, "tex_test");
    cl::NDRange global_test(width, height);
    tester(cl::EnqueueArgs(queue, global_test), target_texture).wait();

    // We have to generate the mipmaps again!!!
    glGenerateMipmap(GL_TEXTURE_2D);

    // Release shared objects                                                          
    err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;

    // Flush CL queue
    err = clFinish(queue());
    std::cout << "Finished CL queue with err:\t" << err << std::endl;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, gl_texture);

        // render container
        simple_shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
