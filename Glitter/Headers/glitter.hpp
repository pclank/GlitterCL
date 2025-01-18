// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CL/cl.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;

// **********************************************************************************
// OpenCL section
// **********************************************************************************

cl::Device default_device;
cl::Context context;
std::string kernel_source;
cl::Program::Sources sources;
cl::CommandQueue queue;
cl::Program program;
cl::Buffer test_buffer;
cl::Buffer debug_buffer;
cl::Kernel test_kernel;
cl::NDRange global_tex(mWidth, mHeight);

float hardcoded_vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

cl::make_kernel<cl::Image2D> tester(test_kernel);

cl::Image2D target_texture;

std::string ReadFile2(const char* f_name = "kernels.cl")
{
    std::string kernel_code;
    std::ifstream kernel_file;
    // ensure ifstream objects can throw exceptions:
    kernel_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        
        kernel_file.open(f_name);
        std::stringstream kernel_stream;
        
        kernel_stream << kernel_file.rdbuf();
        
        kernel_file.close();
        
        kernel_code = kernel_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    // return char sequence
    return kernel_code.c_str();
}

#endif //~ Glitter Header
