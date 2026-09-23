#ifndef RE_OPENGL_H
#define RE_OPENGL_H

#include "cglm/cglm.h"
#include "stb_img.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SIOPENGL static inline

typedef enum { TEXTURE_CLAMP, TEXTURE_REPEAT } TextureSettingS;
typedef enum { FILTER_LINEAR, FILTER_NEAREST } TextureFilterS;

typedef struct {
  GLuint id;
  int width;
  int height;
  size_t channels;
  TextureSettingS setting;
} TextureS;

typedef struct {
  mat4 view;
  mat4 projections;
} CameraS;

typedef struct {
  CameraS camera;
  vec3 position;
  vec3 front;
  vec3 up;
  float speed;
  float sensitivity;
  float yaw;
  float pitch;
  vec2 lastPosition; // instead of float x, float y
} CCameraS;

typedef struct {
  GLuint vao;
  GLuint vbo;
  TextureS texture;
  int vertexCount;
  float mvp[16];
} MeshS;

typedef struct {
  GLuint FBO;
  GLuint RBO;
  TextureS texture;
  int width;
  int height;
} FrameBufferS;

typedef struct {
  CCameraS camera;
  FrameBufferS framebuffer;
  GLuint defaultShader;
  GLuint quadVAO;
} RendererS;

typedef enum { UI_BUTTON, UI_TEXT, UI_LABEL, UI_BOX, UI_MOUSE } UItypeS;

typedef struct {
  TextureS texture;
  vec2 position;
  vec2 size;
  UItypeS type;
  bool isVisible;
} UIelementS;

typedef enum {
  UI_STATE_NORMAL,
  UI_STATE_HOVER,
  UI_STATE_PRESSED
} UIButtonState;

typedef struct {
  TextureS textureNormal;
  TextureS textureHover;
  TextureS texturePressed;
  vec2 position;
  vec2 size;
  UItypeS type;
  bool isVisible;
  UIButtonState state;
  void (*onClick)(struct UIelementS *btn);
} UIButtonS;

GLFWwindow *CreateWindowContext(int w, int h, const char *wname);
void InitGLFW();
void DestroyWindow(GLFWwindow *window);
GLuint CreateShaderStr(const char *vertexShSouce, const char *fragmentShSource);
GLuint CreateShaderFiles(const char *vertexShPath, const char *fragmentShPath);
GLuint CreateVertexArrayObject();
GLuint CreateVertexBufferObject(const void *data, size_t size);
bool IsShaderCompiled(GLuint shader, const char *shaderName);
bool IsProgramLinked(GLuint program);
TextureS LoadTexture(const char *path, TextureSettingS setting);
void SetUniform1i(GLuint program, const char *name, int value);
void SetUniform1f(GLuint program, const char *name, float value);
void SetUniform3f(GLuint program, const char *name, float x, float y, float z);
void SetUniform4f(GLuint program, const char *name, float x, float y, float z,
                  float w);
void SetUniformMat4(GLuint program, const char *name, const float *matrix);
void SetupVertexAttrib(GLuint index, GLint size, GLenum type, GLsizei stride,
                       const void *pointer);
void SetViewport(int x, int y, int w, int h);
char *ReadGlslfile(const char *filepath);
void EnableDepthTest();
void DisableDepthTest();
static inline bool IsKeyPressed(GLFWwindow *window, int key);
void CleanScreen(float r, float g, float b, float alpha);
void FreeTextureS(TextureS *tex);
void BindTextureS(TextureS *tex);
void UnbindTextureS();
void BindMeshS(MeshS *mesh, GLuint shaderProgram);
void DrawMeshS(MeshS *mesh);
void DeleteMeshS(MeshS *mesh);
void AboutRenderer();
void InitCamera(CCameraS *cam);
void UpdateCameraFront(CCameraS *cam);
void UpdateViewMatrix(CCameraS *cam);
void UpdateCamera(GLFWwindow *window, CCameraS *cam, float deltaTime);
void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void ProcessKeyboardInput(GLFWwindow *window, CCameraS *cam, float deltaTime);
float CalculateFPS();
bool CreateFrameBuffer(FrameBufferS *fb, int width, int height);
TextureS LoadEmptyTexture(int width, int height);
void FreeFrameBuffer(FrameBufferS *fb);
void BindFrameBuffer(FrameBufferS *fb);
void UnbindFrameBuffer(int windowWidth, int windowHeight);
void DrawFrameBufferTexture(FrameBufferS *fb, GLuint shaderProgram,
                            GLuint quadVAO);
void UpdateCameraProjection(CCameraS *cam, float fovDeg, float aspect,
                            float nearZ, float farZ);
void UpdateCameraOrtho(CCameraS *cam, float left, float right, float bottom,
                       float top, float nearZ, float farZ);
/* framebuffersizecallback for updating the view port when resizing the window*/
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
/* framebuffersizecallback update the view port and the camera projection */
void SizeCallbackViewCam(GLFWwindow *window, int width, int height);

#ifdef RE_OPENGL_IMPLEMENTATION

GLFWwindow *CreateWindowContext(int w, int h, const char *wname) {
  GLFWwindow *window = glfwCreateWindow(w, h, wname, NULL, NULL);
  if (!window) {
    fprintf(stderr, "failed to create a window ...\n");
    glfwTerminate();
    return NULL;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
    glfwDestroyWindow(window);
    glfwTerminate();
    return NULL;
  }
  return window;
}

void InitGLFW() {
  if (!glfwInit()) {

    fprintf(stderr, "GLFW init failed..\n");
    exit(1);
  }
}
void DestroyWindow(GLFWwindow *window) {
  if (window) {
    glfwDestroyWindow(window);
  }

  glfwTerminate();
}
GLuint CreateShaderStr(const char *vertexShSouce,
                       const char *fragmentShSource) {
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShSouce, NULL);
  glCompileShader(vertexShader);

  if (!IsShaderCompiled(vertexShader, "vertex shader")) {
    glDeleteShader(vertexShader);

    return 0;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShSource, NULL);
  glCompileShader(fragmentShader);

  if (!IsShaderCompiled(fragmentShader, "fragment shader")) {
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
  }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  if (!IsProgramLinked(shaderProgram)) {
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
    return 0;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

GLuint CreateShaderFiles(const char *vertexShPath, const char *fragmentShPath) {

  char *vertexSource = ReadGlslfile(vertexShPath);
  if (!vertexSource) {
    fprintf(stderr, "Error reading vertex shader file: %s\n", vertexShPath);
    return 0;
  }

  char *fragmentSource = ReadGlslfile(fragmentShPath);
  if (!fragmentSource) {
    fprintf(stderr, "Error reading fragment shader file: %s\n", fragmentShPath);
    free(vertexSource);
    return 0;
  }

  GLuint shaderProgram = CreateShaderStr(vertexSource, fragmentSource);
  if (shaderProgram == 0) {
    fprintf(stderr, "Shader compilation/linking failed.\n");
  }

  free(vertexSource);
  free(fragmentSource);

  return shaderProgram;
}

char *ReadGlslfile(const char *filepath) {
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filepath);
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fprintf(stderr, "Failed to seek to end of file: %s\n", filepath);
    fclose(file);
    return NULL;
  }

  long len = ftell(file);
  if (len < 0) {
    fprintf(stderr, "Failed to determine file size: %s\n", filepath);
    fclose(file);
    return NULL;
  }
  rewind(file);

  char *buffer = (char *)malloc(len + 1);
  if (!buffer) {
    fprintf(stderr, "Memory allocation failed while reading file: %s\n",
            filepath);
    fclose(file);
    return NULL;
  }

  size_t readSize = fread(buffer, 1, len, file);
  if (readSize != (size_t)len) {
    fprintf(stderr, "Only read %zu of %ld bytes from %s\n", readSize, len,
            filepath);
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[len] = '\0';
  fclose(file);

  return buffer;
}

bool IsShaderCompiled(GLuint shader, const char *shaderName) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      char *infoLog = (char *)malloc(logLength);
      glGetShaderInfoLog(shader, logLength, NULL, infoLog);
      fprintf(stderr, "ERROR: Shader compilation failed (%s):\n%s\n",
              shaderName, infoLog);
      free(infoLog);
    } else {
      fprintf(
          stderr,
          "ERROR: Shader compilation failed (%s): Unknown error (no log).\n",
          shaderName);
    }
    return false;
  }
  return true;
}

bool IsProgramLinked(GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      char *infoLog = (char *)malloc(logLength);
      glGetProgramInfoLog(program, logLength, NULL, infoLog);
      fprintf(stderr, "ERROR: Program linking failed:\n%s\n", infoLog);
      free(infoLog);
    } else {
      fprintf(stderr,
              "ERROR: Program linking failed: Unknown error (no log).\n");
    }
    return false;
  }
  return true;
}

GLuint CreateVertexArrayObject() {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  return vao;
}

GLuint CreateVertexBufferObject(const void *data, size_t size) {
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  return vbo;
}
/*
    index : attribute location : i.e layout(location=0).
    size : number of components : i.e (3 for vec3).
    type : Data type ( most common case : GL_FLOAT)
    stride : size of each vertex in bytex ( sizeof(data) -> return nb bytes)
    pointer: offset within the vertex struct ( raw data)
*/
void SetupVertexAttrib(GLuint index, GLint size, GLenum type, GLsizei stride,
                       const void *pointer) {
  glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
  glEnableVertexAttribArray(index);
}

/*
    ill consider removing : glUseProgram() inside the functions of SetUniform.
    but i still think it's safe and not cause any error for the momment.
*/
void SetUniform1i(GLuint program, const char *name, int value) {
  GLint location = glGetUniformLocation(program, name);
  if (location == -1) {
    fprintf(stderr, "Uniform '%s' not found in program %u\n", name, program);
    return;
  }
  glUseProgram(program);
  glUniform1i(location, value);
}

void SetUniform1f(GLuint program, const char *name, float value) {
  GLint location = glGetUniformLocation(program, name);
  if (location == -1) {
    fprintf(stderr, "Uniform '%s' not found in program %u\n", name, program);
    return;
  }
  glUseProgram(program);
  glUniform1f(location, value);
}

void SetUniform3f(GLuint program, const char *name, float x, float y, float z) {
  GLint location = glGetUniformLocation(program, name);
  if (location == -1) {
    fprintf(stderr, "Uniform '%s' not found in program %u\n", name, program);
    return;
  }
  glUseProgram(program);
  glUniform3f(location, x, y, z);
}

void SetUniform4f(GLuint program, const char *name, float x, float y, float z,
                  float w) {
  GLint location = glGetUniformLocation(program, name);
  if (location == -1) {
    fprintf(stderr, "Uniform '%s' not found in program %u\n", name, program);
    return;
  }
  glUseProgram(program);
  glUniform4f(location, x, y, z, w);
}

void SetUniformMat4(GLuint program, const char *name, const float *matrix) {
  GLint location = glGetUniformLocation(program, name);
  if (location == -1) {
    fprintf(stderr, "Uniform '%s' not found in program %u\n", name, program);
    return;
  }
  glUseProgram(program);
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}

TextureS LoadTexture(const char *path, TextureSettingS setting) {
  TextureS tex = {0};

  stbi_set_flip_vertically_on_load(1);
  int width, height, channels;
  unsigned char *data = stbi_load(path, &width, &height, &channels, 0);
  if (!data) {
    fprintf(stderr, "Failed to load texture: %s\n", path);
    return tex;
  }

  GLenum format;
  if (channels == 1)
    format = GL_RED;
  else if (channels == 3)
    format = GL_RGB;
  else if (channels == 4)
    format = GL_RGBA;
  else {
    fprintf(stderr, "Unsupported number of channels (%d) in texture: %s\n",
            channels, path);
    stbi_image_free(data);
    return tex;
  }

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Wrapping
  GLenum wrapMode = (setting == TEXTURE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

  // Filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload data
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  tex.id = textureID;
  tex.width = width;
  tex.height = height;
  tex.channels = channels;
  tex.setting = setting;

  return tex;
}

/* make sure to call this :  glfwSetFramebufferSizeCallback(window,
 * FramebufferSizeCallback);  */
void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}
/* this updates the view port also update the camera projection
and call this : glfwSetFramebufferSizeCallback(window, SizeCallbackViewCam);
*/
void SizeCallbackViewCam(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);

  // Update the camera projection matrix aspect ratio
  CCameraS *cam = (CCameraS *)glfwGetWindowUserPointer(window);
  if (cam) {
    UpdateCameraProjection(cam, 45.0f, (float)width / (float)height, 0.1f,
                           100.0f);
  }
}

void FreeTextureS(TextureS *tex) {
  if (tex && tex->id != 0) {
    glDeleteTextures(1, &tex->id);
    tex->id = 0;
  }
}

void BindTextureS(TextureS *tex) {
  if (tex && tex->id != 0) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
  } else {
    fprintf(stderr, "Attempted to bind an invalid or uninitialized texture.\n");
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void UnbindTextureS() { glBindTexture(GL_TEXTURE_2D, 0); }

void AboutRenderer() {
  const GLubyte *vendor = glGetString(GL_VENDOR);
  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  printf("Renderer Information:\n");
  printf("  Vendor:   %s\n", vendor ? (const char *)vendor : "Unknown");
  printf("  Renderer: %s\n", renderer ? (const char *)renderer : "Unknown");
  printf("  OpenGL Version: %s\n", version ? (const char *)version : "Unknown");
  printf("  GLSL Version: %s\n",
         glslVersion ? (const char *)glslVersion : "Unknown");
}

void SetViewport(int x, int y, int w, int h) { glViewport(x, y, w, h); }

void BindMeshS(MeshS *mesh, GLuint shaderProgram) {
  if (!mesh) {
    return;
  }
  glUseProgram(shaderProgram);
  glBindVertexArray(mesh->vao);
  glActiveTexture(GL_TEXTURE0);
  BindTextureS(&mesh->texture);
  GLint mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP");
  if (mvpLocation != -1) {
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *)mesh->mvp);
  } else {
    fprintf(stderr, "Warning: uniform 'u_MVP' not found in shader program %u\n",
            shaderProgram);
  }
}

void DrawMeshS(MeshS *mesh) {
  if (!mesh) {
    return;
  }

  glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
  glBindVertexArray(0);
}

void DeleteMeshS(MeshS *mesh) {
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteVertexArrays(1, &mesh->vao);
  FreeTextureS(&mesh->texture);
}

void EnableDepthTest() { glEnable(GL_DEPTH_TEST); }

void DisableDepthTest() { glDisable(GL_DEPTH_TEST); }

static inline bool IsKeyPressed(GLFWwindow *window, int key) {
  return glfwGetKey(window, key) == GLFW_PRESS;
}

void CleanScreen(float r, float g, float b, float alpha) {
  glClearColor(r, g, b, alpha);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void InitCamera(CCameraS *cam) {
  glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam->position);
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->front);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam->up);
  cam->speed = 2.5f;
  cam->sensitivity = 0.1f;
  cam->yaw = -90.0f;
  cam->pitch = 0.0f;
  cam->lastPostion = (vec2){.x = 100, .y = 100};
  glm_mat4_identity(cam->camera.view);
  glm_mat4_identity(cam->camera.projections);
}

void UpdateCameraFront(CCameraS *cam) {
  float yawRad = glm_rad(cam->yaw);
  float pitchRad = glm_rad(cam->pitch);

  vec3 front;
  front[0] = cosf(yawRad) * cosf(pitchRad);
  front[1] = sinf(pitchRad);
  front[2] = sinf(yawRad) * cosf(pitchRad);

  glm_vec3_normalize_to(front, cam->front);
}

void UpdateViewMatrix(CCameraS *cam) {
  vec3 center;
  glm_vec3_add(cam->position, cam->front, center);
  glm_lookat(cam->position, center, cam->up, cam->camera.view);
}

void ProcessKeyboardInput(GLFWwindow *window, CCameraS *cam, float deltaTime) {
  float velocity = cam->speed * deltaTime;

  vec3 temp;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(cam->front, velocity, temp);
    glm_vec3_add(cam->position, temp, cam->position);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    glm_vec3_scale(cam->front, velocity, temp);
    glm_vec3_sub(cam->position, temp, cam->position);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    vec3 cross;
    glm_vec3_cross(cam->front, cam->up, cross);
    glm_vec3_normalize(cross);
    glm_vec3_scale(cross, velocity, temp);
    glm_vec3_sub(cam->position, temp, cam->position);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    vec3 cross;
    glm_vec3_cross(cam->front, cam->up, cross);
    glm_vec3_normalize(cross);
    glm_vec3_scale(cross, velocity, temp);
    glm_vec3_add(cam->position, temp, cam->position);
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    glm_vec3_scale(cam->up, velocity, temp);
    glm_vec3_add(cam->position, temp, cam->position);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    glm_vec3_scale(cam->up, velocity, temp);
    glm_vec3_sub(cam->position, temp, cam->position);
  }
}

void MouseCallback(GLFWwindow *window, float xpos, float ypos) {
  static bool firstMouse = true;
  static float lastX = 400, lastY = 300;

  CCameraS *cam = (CCameraS *)glfwGetWindowUserPointer(window);
  if (!cam)
    return;

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  xoffset *= cam->sensitivity;
  yoffset *= cam->sensitivity;

  cam->yaw += xoffset;
  cam->pitch += yoffset;

  if (cam->pitch > 89.0f)
    cam->pitch = 89.0f;
  if (cam->pitch < -89.0f)
    cam->pitch = -89.0f;

  // cam->lastPosition = {lastX,lastY};
  UpdateCameraFront(cam);
}

void UpdateCamera(GLFWwindow *window, CCameraS *cam, float deltaTime) {
  ProcessKeyboardInput(window, cam, deltaTime);
  UpdateViewMatrix(cam);
}

float CalculateFPS() {
  static double previousTime = 0.0;
  static int frameCount = 0;
  double currentTime = glfwGetTime();
  frameCount++;
  double delta = currentTime - previousTime;

  if (delta >= 1.0) {
    float fps = (float)(frameCount / delta);
    frameCount = 0;
    previousTime = currentTime;
    return fps;
  }
  return -1.0f;
}

TextureS LoadEmptyTexture(int width, int height) {
  TextureS tex = {0};
  tex.width = width;
  tex.height = height;
  tex.channels = 4;

  glGenTextures(1, &tex.id);
  glBindTexture(GL_TEXTURE_2D, tex.id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}

bool CreateFrameBuffer(FrameBufferS *fb, int width, int height) {
  fb->width = width;
  fb->height = height;
  fb->texture = LoadEmptyTexture(width, height);
  glGenFramebuffers(1, &fb->FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, fb->FBO);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fb->texture.id, 0);

  // renderbuffer for depth and stencil
  glGenRenderbuffers(1, &fb->RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, fb->RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, fb->RBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  return true;
}
void BindFrameBuffer(FrameBufferS *fb) {
  glBindFramebuffer(GL_FRAMEBUFFER, fb->FBO);
  glViewport(0, 0, fb->width, fb->height);
}
void UnbindFrameBuffer(int windowWidth, int windowHeight) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, windowWidth, windowHeight);
}

void FreeFrameBuffer(FrameBufferS *fb) {
  if (fb->RBO)
    glDeleteRenderbuffers(1, &fb->RBO);
  if (fb->FBO)
    glDeleteFramebuffers(1, &fb->FBO);
  FreeTextureS(&fb->texture);
}
void DrawFrameBufferTexture(FrameBufferS *fb, GLuint shaderProgram,
                            GLuint quadVAO) {
  glUseProgram(shaderProgram);
  mat4 ortho;
  glm_ortho(0.0f, (float)fb->width, 0.0f, (float)fb->height, -1.0f, 1.0f,
            ortho);
  mat4 model;
  glm_mat4_identity(model);
  glm_scale(model, (vec3){(float)fb->width, (float)fb->height, 1.0f});
  mat4 mvp;
  glm_mat4_mul(ortho, model, mvp);
  GLint mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
  glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, (const GLfloat *)mvp);
  BindTextureS(&fb->texture);
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  UnbindTextureS();

  glUseProgram(0);
}
void UpdateCameraProjection(CCameraS *cam, float fovDeg, float aspect,
                            float nearZ, float farZ) {
  if (!cam) {
    return;
  }

  glm_perspective(glm_rad(fovDeg), aspect, nearZ, farZ,
                  cam->camera.projections);
}

void UpdateCameraOrtho(CCameraS *cam, float left, float right, float bottom,
                       float top, float nearZ, float farZ) {
  if (!cam) {
    return;
  }
  glm_ortho(left, right, bottom, top, nearZ, farZ, cam->camera.projections);
}

#endif

#ifdef RE_OPENGL_DEPREC
void ComputeMVP(float out[16], mat4 model, mat4 view, mat4 projection);
mat4 LookAt4f(vec3 eye, vec3 center, vec3 up);
mat4 Perspective4f(float fov, float aspect, float near, float far);
mat4 Ortho4f(float right, float left, float bottom, float top, float near,
             float far);
void glm_perspective(float fovy, float aspect, float nearZ, float farZ,
                     mat4 dest);
//->Orthographic projection:
void glm_ortho(float left, float right, float bottom, float top, float nearZ,
               float farZ, mat4 dest);
//->look at :
void glm_lookat(vec3 eye, vec3 center, vec3 up, mat4 dest);
//->Multiply MVP:
// glm_mat4_mul(projection, view, pv);
// glm_mat4_mul(pv, model, mvp);

#endif

#endif // RE_OPENGL_H
