#include "opengl_renderer.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <chrono>
#include <cstring>
#include <iostream>

// OpenGL shader sources
namespace OpenGLShaders {
    const char* VERTEX_SHADER_SOURCE = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* FRAGMENT_SHADER_SOURCE = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D cefTexture;
        
        void main() {
            FragColor = texture(cefTexture, TexCoord);
        }
    )";
}

// Vertex data for fullscreen quad
const std::vector<QuadVertex> vertices = {
    {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},  // Bottom-left
    {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},  // Bottom-right
    {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},  // Top-right
    {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}}   // Top-left
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

OpenGLRenderer::OpenGLRenderer()
    : cefTexture_(0), framebuffer_(0), renderbuffer_(0), vertexBuffer_(0),
      indexBuffer_(0), vertexArray_(0), shaderProgram_(0), vertexShader_(0),
      fragmentShader_(0), glContext_(nullptr), window_(nullptr), width_(0),
      height_(0), initialized_(false), vsyncEnabled_(true), multiSampleCount_(1),
      lastFrameTime_(0), frameTime_(0.0), frameCount_(0), enablePartialUpdates_(false) {
    
    // Initialize buffer stats
    bufferStats_ = {0, 0, 0, 0.0};
    lastDirtyRegion_ = {0, 0, 0, 0};
}

OpenGLRenderer::~OpenGLRenderer() {
    Shutdown();
}

bool OpenGLRenderer::Initialize(void* window_handle, int width, int height) {
    if (initialized_) {
        return true;
    }

    window_ = static_cast<SDL_Window*>(window_handle);
    width_ = width;
    height_ = height;

    if (!CreateContext()) {
        LogError("Failed to create OpenGL context");
        return false;
    }

    if (!LoadExtensions()) {
        LogError("Failed to load OpenGL extensions");
        return false;
    }

    if (!CreateShaders()) {
        LogError("Failed to create shaders");
        return false;
    }

    if (!CreateGeometry()) {
        LogError("Failed to create geometry");
        return false;
    }

    if (!CreateFramebuffer()) {
        LogError("Failed to create framebuffer");
        return false;
    }

    if (!CreateCEFTexture(width_, height_)) {
        LogError("Failed to create CEF texture");
        return false;
    }

    // Set initial OpenGL state
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialized_ = true;
    return true;
}

void OpenGLRenderer::Shutdown() {
    if (!initialized_) {
        return;
    }

    CleanupCEFTexture();
    CleanupFramebuffer();
    CleanupGeometry();
    CleanupShaders();

    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }

    initialized_ = false;
}

bool OpenGLRenderer::BeginFrame() {
    if (!initialized_) {
        return false;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    if (lastFrameTime_ > 0) {
        frameTime_ = (currentTime - lastFrameTime_) / 1000.0; // Convert to milliseconds
        frameCount_++;
    }
    lastFrameTime_ = currentTime;

    // Make context current
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
        LogError("Failed to make OpenGL context current: " + std::string(SDL_GetError()));
        return false;
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    return CheckGLError("BeginFrame");
}

bool OpenGLRenderer::EndFrame() {
    if (!initialized_) {
        return false;
    }

    return CheckGLError("EndFrame");
}

bool OpenGLRenderer::Present() {
    if (!initialized_) {
        return false;
    }

    SDL_GL_SwapWindow(window_);
    return true;
}

bool OpenGLRenderer::Render() {
    if (!initialized_ || cefTexture_ == 0) {
        return false;
    }

    // Use shader program
    glUseProgram(shaderProgram_);
    
    // Bind CEF texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cefTexture_);
    glUniform1i(glGetUniformLocation(shaderProgram_, "cefTexture"), 0);

    // Bind vertex array and draw
    glBindVertexArray(vertexArray_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    return CheckGLError("Render");
}

bool OpenGLRenderer::CreateTextureFromBuffer(const void* buffer, int width, int height) {
    if (!initialized_ || !buffer) {
        return false;
    }

    if (cefTexture_ == 0 || width != width_ || height != height_) {
        if (!CreateCEFTexture(width, height)) {
            return false;
        }
        width_ = width;
        height_ = height;
    }

    return UpdateTexture(buffer, width, height);
}

bool OpenGLRenderer::UpdateTexture(const void* buffer, int width, int height) {
    if (!initialized_ || !buffer || cefTexture_ == 0) {
        return false;
    }

    auto start = std::chrono::high_resolution_clock::now();

    glBindTexture(GL_TEXTURE_2D, cefTexture_);
    
    if (enablePartialUpdates_ && !lastDirtyRegion_.isEmpty()) {
        // Update only the dirty region
        glTexSubImage2D(GL_TEXTURE_2D, 0, 
                       lastDirtyRegion_.x, lastDirtyRegion_.y,
                       lastDirtyRegion_.width, lastDirtyRegion_.height,
                       GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    } else {
        // Update entire texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                       GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Update buffer stats
    bufferStats_.totalUpdates++;
    bufferStats_.avgUpdateTime = (bufferStats_.avgUpdateTime * (bufferStats_.totalUpdates - 1) + 
                                 duration.count() / 1000.0) / bufferStats_.totalUpdates;

    return CheckGLError("UpdateTexture");
}

bool OpenGLRenderer::ResizeTextures(int width, int height) {
    if (!initialized_) {
        return false;
    }

    width_ = width;
    height_ = height;

    glViewport(0, 0, width_, height_);

    // Recreate CEF texture with new dimensions
    CleanupCEFTexture();
    if (!CreateCEFTexture(width_, height_)) {
        return false;
    }

    return CheckGLError("ResizeTextures");
}

void OpenGLRenderer::EnableVSync(bool enable) {
    vsyncEnabled_ = enable;
    SDL_GL_SetSwapInterval(enable ? 1 : 0);
}

void OpenGLRenderer::SetMultiSampleCount(int samples) {
    multiSampleCount_ = samples;
    // Note: Multisampling setup would need to be done during context creation
}

void OpenGLRenderer::EnablePartialUpdates(bool enable) {
    enablePartialUpdates_ = enable;
}

void OpenGLRenderer::ClearTextureCache() {
    textureCache_.clear();
}

void OpenGLRenderer::SetDirtyRegion(int x, int y, int width, int height) {
    lastDirtyRegion_ = {x, y, width, height};
}

bool OpenGLRenderer::CheckFeatureSupport() {
    if (!initialized_) {
        return false;
    }

    // Check for required OpenGL version (3.3+)
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!version) {
        return false;
    }

    // Parse version string
    int major = 0, minor = 0;
    if (sscanf(version, "%d.%d", &major, &minor) != 2) {
        return false;
    }

    return (major > 3) || (major == 3 && minor >= 3);
}

std::string OpenGLRenderer::GetAdapterInfo() {
    if (!initialized_) {
        return "OpenGL not initialized";
    }

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    std::string info = "OpenGL Renderer\n";
    info += "Vendor: " + std::string(vendor ? vendor : "Unknown") + "\n";
    info += "Renderer: " + std::string(renderer ? renderer : "Unknown") + "\n";
    info += "Version: " + std::string(version ? version : "Unknown");

    return info;
}

void OpenGLRenderer::LogPerformanceStats() {
    if (frameCount_ > 0) {
        double avgFrameTime = frameTime_;
        double fps = 1000.0 / avgFrameTime;
        
        std::cout << "OpenGL Renderer Performance Stats:" << std::endl;
        std::cout << "  Average Frame Time: " << avgFrameTime << " ms" << std::endl;
        std::cout << "  FPS: " << fps << std::endl;
        std::cout << "  Buffer Updates: " << bufferStats_.totalUpdates << std::endl;
        std::cout << "  Average Update Time: " << bufferStats_.avgUpdateTime << " ms" << std::endl;
    }
}

bool OpenGLRenderer::CreateContext() {
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if (multiSampleCount_ > 1) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multiSampleCount_);
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        LogError("Failed to create OpenGL context: " + std::string(SDL_GetError()));
        return false;
    }

    // Make context current
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
        LogError("Failed to make OpenGL context current: " + std::string(SDL_GetError()));
        return false;
    }

    return true;
}

bool OpenGLRenderer::CreateShaders() {
    // Compile vertex shader
    if (!CompileShader(GL_VERTEX_SHADER, OpenGLShaders::VERTEX_SHADER_SOURCE, vertexShader_)) {
        return false;
    }

    // Compile fragment shader
    if (!CompileShader(GL_FRAGMENT_SHADER, OpenGLShaders::FRAGMENT_SHADER_SOURCE, fragmentShader_)) {
        return false;
    }

    // Link program
    if (!LinkProgram(vertexShader_, fragmentShader_, shaderProgram_)) {
        return false;
    }

    return true;
}

bool OpenGLRenderer::CreateGeometry() {
    // Generate vertex array object
    glGenVertexArrays(1, &vertexArray_);
    glBindVertexArray(vertexArray_);

    // Generate and bind vertex buffer
    glGenBuffers(1, &vertexBuffer_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(QuadVertex), vertices.data(), GL_STATIC_DRAW);

    // Generate and bind index buffer
    glGenBuffers(1, &indexBuffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    // Set vertex attributes
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return CheckGLError("CreateGeometry");
}

bool OpenGLRenderer::CreateFramebuffer() {
    // For now, we'll render directly to the default framebuffer
    // This can be extended later for off-screen rendering
    return true;
}

bool OpenGLRenderer::CreateCEFTexture(int width, int height) {
    CleanupCEFTexture();

    glGenTextures(1, &cefTexture_);
    glBindTexture(GL_TEXTURE_2D, cefTexture_);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return CheckGLError("CreateCEFTexture");
}

void OpenGLRenderer::CleanupFramebuffer() {
    if (framebuffer_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_);
        framebuffer_ = 0;
    }
    if (renderbuffer_ != 0) {
        glDeleteRenderbuffers(1, &renderbuffer_);
        renderbuffer_ = 0;
    }
}

void OpenGLRenderer::CleanupCEFTexture() {
    if (cefTexture_ != 0) {
        glDeleteTextures(1, &cefTexture_);
        cefTexture_ = 0;
    }
}

void OpenGLRenderer::CleanupShaders() {
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
    if (vertexShader_ != 0) {
        glDeleteShader(vertexShader_);
        vertexShader_ = 0;
    }
    if (fragmentShader_ != 0) {
        glDeleteShader(fragmentShader_);
        fragmentShader_ = 0;
    }
}

void OpenGLRenderer::CleanupGeometry() {
    if (vertexArray_ != 0) {
        glDeleteVertexArrays(1, &vertexArray_);
        vertexArray_ = 0;
    }
    if (vertexBuffer_ != 0) {
        glDeleteBuffers(1, &vertexBuffer_);
        vertexBuffer_ = 0;
    }
    if (indexBuffer_ != 0) {
        glDeleteBuffers(1, &indexBuffer_);
        indexBuffer_ = 0;
    }
}

bool OpenGLRenderer::CompileShader(GLenum type, const std::string& source, GLuint& shader) {
    shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LogError("Shader compilation failed: " + std::string(infoLog));
        glDeleteShader(shader);
        shader = 0;
        return false;
    }

    return true;
}

bool OpenGLRenderer::LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint& program) {
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        LogError("Program linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    return true;
}

void OpenGLRenderer::LogError(const std::string& message) {
    std::cerr << "OpenGL Renderer Error: " << message << std::endl;
}

std::string OpenGLRenderer::GetGLErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "Unknown OpenGL error";
    }
}

bool OpenGLRenderer::CheckGLError(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LogError(operation + " failed: " + GetGLErrorString(error));
        return false;
    }
    return true;
}

bool OpenGLRenderer::LoadExtensions() {
    // For now, we'll assume basic OpenGL 3.3 functionality is available
    // Extension loading can be added here if needed
    return true;
}

bool OpenGLRenderer::CheckExtension(const std::string& extension) {
    const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (!extensions) {
        return false;
    }
    
    return std::string(extensions).find(extension) != std::string::npos;
}