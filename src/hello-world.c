// currently reading: https://learnopengl.com/Getting-started/Hello-Triangle

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

const char *relative_path(char *dst, char *base, char *target) {
  dirname_r(base, dst);
  strlcat(dst, "/", MAXPATHLEN);
  strlcat(dst, target, MAXPATHLEN);
  return dst;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}

// info log - for storing error messages, etc.
char infoLog[512];

int makeShader(GLenum shaderType, const char *path, unsigned int *shader) {
  off_t filesize = fsize(path);
  char *source = malloc(filesize + 1);
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    perror("fopen");
    return 0;
  }
  if (fread(source, sizeof(char), filesize, file) != filesize) {
    perror("fread");
    return 0;
  }
  source[filesize] = 0;
  *shader = glCreateShader(shaderType);
  glShaderSource(*shader, 1, &source, NULL);
  glCompileShader(*shader);
  free(source);
  fclose(file);
  int success;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(*shader, sizeof(infoLog), NULL, infoLog);
    return 0;
  }
  return success;
}

int makeShaderProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int *program) {
  *program = glCreateProgram();
  glAttachShader(*program, vertexShader);
  glAttachShader(*program, fragmentShader);
  glLinkProgram(*program);
  int success;
  glGetProgramiv(*program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*program, sizeof(infoLog), NULL, infoLog);
    return 0;
  }
  return success;
}

unsigned int loadTexture() {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nrChannels;
  unsigned char *data = stbi_load("res/container.jpg", &width, &height, &nrChannels, 0);
  if (data)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  else
    {
      fprintf(stderr, "Failed to load texture\n");
    }
  stbi_image_free(data);
  return texture;
}

int main()
{
  // glfw initialization
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
    {
      printf("Failed to create GLFW window\n");
      glfwTerminate();
      return -1;
    }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad initialization
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      printf("Failed to initialize GLAD\n");
      return -1;
    }
  //glViewport(0, 0, 800, 600);

  unsigned int texture = loadTexture();

  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  printf("Max vertex attributes: %d\n", nrAttributes);

  char path_dst[MAXPATHLEN];

  unsigned int vertexShader;
  if (!makeShader(GL_VERTEX_SHADER, relative_path(path_dst, __FILE__, "hello-world.vert"), &vertexShader)) {
    printf("Vertex shader could not be made: %s\n", infoLog);
    return 1;
  }

  unsigned int fragmentShader;
  if (!makeShader(GL_FRAGMENT_SHADER, relative_path(path_dst, __FILE__, "hello-world.frag"), &fragmentShader)) {
    printf("Fragment shader could not be made: %s\n", infoLog);
    return 1;
  }

  unsigned int shaderProgram;
  if (!makeShaderProgram(vertexShader, fragmentShader, &shaderProgram)) {
    printf("Shader program could not be made: %s\n", infoLog);
    return 1;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  float vertices[] = {
    // positions          // colors           // texture coords
    0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
  };

  unsigned int indices[] = {  // note that we start from 0!
    0, 1, 2, 0, 2, 3   // first triangle
  };

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
  glEnableVertexAttribArray(1);

  // texture
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  // glBindVertexArray(0);

  // The event loop
  while(!glfwWindowShouldClose(window))
    {
      // input
      processInput(window);

      // rendering
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // activate the shader
      glUseProgram(shaderProgram);

      //glDrawArrays(GL_TRIANGLES, 0, sizeof(indices));
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      // call events
      glfwSwapBuffers(window);

      // swap buffers
      glfwPollEvents();
    }

  // Finish
  glfwTerminate();

  return 0;
}
