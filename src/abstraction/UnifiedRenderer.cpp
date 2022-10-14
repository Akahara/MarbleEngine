#include "UnifiedRenderer.h"
#include "Mesh.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace Renderer {

static struct KeepAliveResources {
  Shader standardMeshShader;
} *s_keepAliveResources = nullptr;


Shader LoadShaderFromFiles(const fs::path &vertexPath, const fs::path &fragmentPath)
{
  std::ifstream vertexFile{ vertexPath };
  std::ifstream fragmentFile{ fragmentPath };
  std::stringstream buffer;
  buffer << vertexFile.rdbuf();
  std::string vertexCode = buffer.str();
  buffer.str("");
  buffer << fragmentFile.rdbuf();
  std::string fragmentCode = buffer.str();
  return Shader{ vertexCode, fragmentCode };
}

Mesh CreateCubeMesh()
{
  // TODO add UVs
  float s3 = std::sqrtf(3);
  std::vector<Vertex> vertices{
    // position              uv            normal
    { { -.5f, -.5f, -.5f }, { 0.f, 0.f }, { -s3, -s3, -s3 } },
    { { +.5f, -.5f, -.5f }, { 0.f, 0.f }, { +s3, -s3, -s3 } },
    { { +.5f, +.5f, -.5f }, { 0.f, 0.f }, { +s3, +s3, -s3 } },
    { { -.5f, +.5f, -.5f }, { 0.f, 0.f }, { -s3, +s3, -s3 } },
    { { -.5f, -.5f, +.5f }, { 0.f, 0.f }, { -s3, -s3, +s3 } },
    { { +.5f, -.5f, +.5f }, { 0.f, 0.f }, { +s3, -s3, +s3 } },
    { { +.5f, +.5f, +.5f }, { 0.f, 0.f }, { +s3, +s3, +s3 } },
    { { -.5f, +.5f, +.5f }, { 0.f, 0.f }, { -s3, +s3, +s3 } },
  };
  std::vector<unsigned int> indices{
    0, 3, 1, 1, 3, 2,
    1, 2, 5, 5, 2, 6,
    5, 6, 4, 4, 6, 7,
    4, 7, 0, 0, 7, 3,
    3, 7, 2, 2, 7, 6,
    4, 0, 5, 5, 0, 1
  };
  return Mesh(vertices, indices);
}

Mesh CreatePlaneMesh()
{
  std::vector<Renderer::Vertex> vertices{
    // position             uv            normal (up)   textId
    { { +.5f, 0.f, +.5f }, { 0.f, 0.f }, { 0, 1.f, 0 }},
    { { -.5f, 0.f, +.5f }, { 1.f, 0.f }, { 0, 1.f, 0 }},
    { { -.5f, 0.f, -.5f }, { 1.f, 1.f }, { 0, 1.f, 0 }},
    { { +.5f, 0.f, -.5f }, { 0.f, 1.f }, { 0, 1.f, 0 }},
  };
  std::vector<unsigned int> indices{
    3, 2, 0, 1,0,2
  };
  return Mesh(vertices, indices);
}

static void SkipStreamText(std::istream& stream, const char* text)
{
    char c;
    while (*text) {
        if ((c = stream.get()) != *text) {
            std::cerr << "Expected " << *text << " but got " << c;
            throw std::runtime_error("Unexpected charactor got in stream");
        }
        text++;
    }
}

Mesh LoadMeshFromFile(const fs::path& objPath)
{
    std::ifstream modelFile{ objPath };
    constexpr size_t bufSize = 10000;
    char lineBuffer[bufSize];

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    uvs.emplace_back();

    std::vector<std::tuple<int, int, int>> cachedVertices;
    std::vector<unsigned int> indices;
    std::vector<Vertex> vertices;

    while (modelFile.good()) {
        modelFile.getline(lineBuffer, bufSize);
        if (lineBuffer[0] == '#')
            continue;
        int space = 0;
        while (lineBuffer[space] != '\0' && lineBuffer[space] != ' ')
            space++;
        if (lineBuffer[space] == '\0')
            continue;
        std::stringstream ss;
        ss.str(lineBuffer + space + 1);

        float f1, f2, f3;
        int i1, i2, i3;
        if (strstr(lineBuffer, "v ") == lineBuffer) { // vertex
            ss >> f1 >> f2 >> f3;
            positions.push_back({ f1, f2, f3 });
        }
        else if (strstr(lineBuffer, "vt ") == lineBuffer) { // texture coordinate
            ss >> f1 >> f2;
            uvs.push_back({ f1, f2 });
        }
        else if (strstr(lineBuffer, "vn ") == lineBuffer) { // normal
            ss >> f1 >> f2 >> f3;
            normals.push_back({ f1, f2, f3 });
        }
        else if (strstr(lineBuffer, "f ") == lineBuffer) { // face
            for (size_t i = 0; i < 3; i++) {
                ss >> i1; SkipStreamText(ss, "/");
                if (ss.peek() != '/')
                    ss >> i2;
                else
                    i2 = 1;
                SkipStreamText(ss, "/");
                ss >> i3;
                std::tuple<int, int, int> cacheKey{ i1, i2, i3 };
                auto inCacheIndex = std::find(cachedVertices.begin(), cachedVertices.end(), cacheKey);
                if (inCacheIndex == cachedVertices.end()) {
                    vertices.emplace_back(positions[i1 - 1ll], uvs[i2 - 1ll], normals[i3 - 1ll]);
                    indices.push_back((unsigned int)cachedVertices.size());
                    cachedVertices.push_back(cacheKey);
                }
                else {
                    indices.push_back((unsigned int)(inCacheIndex - cachedVertices.begin()));
                }
            }
        }
        else { // unrecognized line
            continue;
        }
    }

    return Mesh(vertices, indices);
}

void Init()
{
  s_keepAliveResources = new KeepAliveResources;
  s_keepAliveResources->standardMeshShader = LoadShaderFromFiles("res/shaders/standard.vs", "res/shaders/standard_mesh.fs");
}

void Shutdown()
{
  delete s_keepAliveResources;
}

void RenderMesh(glm::vec3 position, glm::vec3 size, const Mesh &mesh, const glm::mat4 &VP)
{
  glm::mat4 M(1.f);
  M = glm::translate(M, position);
  M = glm::scale(M, size);
  s_keepAliveResources->standardMeshShader.Bind();
  s_keepAliveResources->standardMeshShader.SetUniformMat4f("u_M", M);
  s_keepAliveResources->standardMeshShader.SetUniformMat4f("u_VP", VP);
  mesh.Draw();
}

Shader& getShader() { if (s_keepAliveResources) return s_keepAliveResources->standardMeshShader; }

}


static void SkipStreamText(std::istream& stream, const char* text)
{
    char c;
    while (*text) {
        if ((c = stream.get()) != *text) {
            std::cerr << "Expected " << *text << " but got " << c;
            throw std::runtime_error("Unexpected charactor got in stream");
        }
        text++;
    }
}

