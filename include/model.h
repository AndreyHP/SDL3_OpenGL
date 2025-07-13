#ifndef MODEL_H
#define MODEL_H

#include "../include/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../include/tiny_gltf.h"
#include "./stb/stb_image.h"

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
    // model data
    glm::mat4 model         = glm::mat4(1.0f);
    int id;
    bool outline{false};
    vector<Texture> textures_loaded; // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

     // Default constructor for an empty model
    Model() : gammaCorrection(false) {
        // Initialize any other member variables if necessary
    }

    // Function to load a model from a file
    void Load(string const &path) {
        loadModel(path);
    }

    void unload(){
        for(unsigned int i = 0; i < meshes.size(); i++){
                meshes[i].Delete();
            }
        meshes.clear();
    }



    // draws the model, and thus all its meshes
    void Draw(Shader &shader)
    {
        if (modelloaded){
           model = glm::mat4(1.0f);
            for(unsigned int i = 0; i < meshes.size(); i++){
                meshes[i].Draw(shader);
            }
        }else{
            static bool printed{false};
            if (!printed) {
            std::cout << "Model_" << id <<  " not loaded" << std::endl;
            printed = true;
            }
        }

    }

    void Translate(float x, float y, float z){
        model = glm::translate(model, glm::vec3(x, y, z));
    }
    void Rotate(float x, float y, float z, float angle){
        model = glm::rotate(model, glm::radians(angle), glm::vec3(x, y, z));
    }
    void Scale(float x, float y, float z){
        model = glm::scale(model, glm::vec3(x, y, z));
    }

    void OutlineInit(Shader &shader){

         if (outline){
            glDepthFunc(GL_LESS);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }else{
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
        }
        shader.use();

        shader.setMat4("model", model);


        glStencilMask(0x00);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
    }
    void drawOutline(Shader &shader){
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        shader.use();

        Draw(shader);

        shader.setMat4("model", model);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }

private:
    bool modelloaded{false};
    string name;
    // loads a model with tinygltf from file and stores the resulting meshes in the meshes vector
    void loadModel(string const &path)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        // determine file extension to load as binary (glb) or text (gltf)
        bool isBinary = (path.substr(path.find_last_of(".") + 1) == "glb");
        bool ret;
        if (isBinary)
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
        else
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

        // check for errors
        if (!warn.empty()) {
            std::cout << "WARNING::TINYGLTF:: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cout << "ERROR::TINYGLTF:: " << err << std::endl;
            return;
        }
        if (!ret) {
            std::cout << "ERROR::TINYGLTF:: Failed to load glTF file: " << path << std::endl;
            return;
        }

        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process the glTF scene
        for (const auto& scene : model.scenes) {
            for (int nodeIndex : scene.nodes) {
                processNode(nodeIndex, model);
            }
        }

        name = path.substr(path.find_last_of('/') + 1);

        std::cout << "Model_" << id << "_" << name  << " loaded" << std::endl;
        modelloaded = true;
    }

    // processes a node recursively
    void processNode(int nodeIndex, const tinygltf::Model &model)
    {
        const tinygltf::Node &node = model.nodes[nodeIndex];

        // process each mesh at the current node
        if (node.mesh >= 0) {
            const tinygltf::Mesh &mesh = model.meshes[node.mesh];
            for (const auto& primitive : mesh.primitives) {
                meshes.push_back(processMesh(primitive, model));
            }
        }

        // recursively process children nodes
        for (int childIndex : node.children) {
            processNode(childIndex, model);
        }
    }

    Mesh processMesh(const tinygltf::Primitive &primitive, const tinygltf::Model &model)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // get vertex data
        // positions
        glm::vec3 vector;
        Vertex vertex;
        const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
        const auto& posBuffer = model.buffers[posBufferView.buffer];
        const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

        // normals
        const float* normals = nullptr;
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
            const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
            const auto& normBuffer = model.buffers[normBufferView.buffer];
            normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
        }

        // texture coordinates
        const float* texCoords = nullptr;
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const auto& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
            const auto& texBuffer = model.buffers[texBufferView.buffer];
            texCoords = reinterpret_cast<const float*>(&texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
        }

        // tangents
        const float* tangents = nullptr;
        if (primitive.attributes.find("TANGENT") != primitive.attributes.end()) {
            const auto& tanAccessor = model.accessors[primitive.attributes.at("TANGENT")];
            const auto& tanBufferView = model.bufferViews[tanAccessor.bufferView];
            const auto& tanBuffer = model.buffers[tanBufferView.buffer];
            tangents = reinterpret_cast<const float*>(&tanBuffer.data[tanBufferView.byteOffset + tanAccessor.byteOffset]);
        }

        // bitangents (computed if not provided)
        for (size_t i = 0; i < posAccessor.count; i++) {
            // position
            vector.x = positions[i * 3];
            vector.y = positions[i * 3 + 1];
            vector.z = positions[i * 3 + 2];
            vertex.Position = vector;

            // normal
            if (normals) {
                vector.x = normals[i * 3];
                vector.y = normals[i * 3 + 1];
                vector.z = normals[i * 3 + 2];
                vertex.Normal = vector;
            }

            // texture coordinates
            if (texCoords) {
                vertex.TexCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            // tangent
            if (tangents) {
                vector.x = tangents[i * 4]; // glTF tangents are vec4
                vector.y = tangents[i * 4 + 1];
                vector.z = tangents[i * 4 + 2];
                vertex.Tangent = vector;
            }

            // bitangent (compute if not provided)
            if (tangents && normals) {
                vertex.Bitangent = glm::cross(vertex.Normal, vertex.Tangent) * tangents[i * 4 + 3]; // w component for handedness
            } else {
                vertex.Bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // get indices
        if (primitive.indices >= 0) {
            const auto& indexAccessor = model.accessors[primitive.indices];
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = model.buffers[indexBufferView.buffer];
            const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

            for (size_t i = 0; i < indexAccessor.count; i++) {
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    indices.push_back(reinterpret_cast<const uint32_t*>(indexData)[i]);
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    indices.push_back(reinterpret_cast<const uint16_t*>(indexData)[i]);
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    indices.push_back(indexData[i]);
                }
            }
        }

        // process materials
        if (primitive.material >= 0) {
            const tinygltf::Material& material = model.materials[primitive.material];

            // diffuse textures
            if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                std::vector<Texture> diffuseMaps = loadMaterialTexture(model, material.pbrMetallicRoughness.baseColorTexture.index, "texture_diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            }

            // specular (approximated using metallic-roughness)
            if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
                std::vector<Texture> specularMaps = loadMaterialTexture(model, material.pbrMetallicRoughness.metallicRoughnessTexture.index, "texture_specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            }

            // normal maps
            if (material.normalTexture.index >= 0) {
                std::vector<Texture> normalMaps = loadMaterialTexture(model, material.normalTexture.index, "texture_normal");
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            }
        }

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTexture(const tinygltf::Model &model, int textureIndex, string typeName)
    {
        vector<Texture> textures;
        if (textureIndex < 0) return textures;

        const tinygltf::Texture &gltfTexture = model.textures[textureIndex];
        const tinygltf::Image &image = model.images[gltfTexture.source];

        // check if texture was loaded before
        bool skip = false;
        for (const auto& loaded : textures_loaded) {
            if (loaded.path == image.uri) {
                textures.push_back(loaded);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            if (!image.uri.empty()) {
                texture.id = TextureFromFile(image.uri.c_str(), directory);
                texture.type = typeName;
                texture.path = image.uri;
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            } else if (!image.image.empty()) {
                // handle embedded images
                unsigned int textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);

                GLenum format = GL_RGBA;
                if (image.component == 3) format = GL_RGB;
                else if (image.component == 1) format = GL_RED;

                glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.image.data());
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                texture.id = textureID;
                texture.type = typeName;
                texture.path = "embedded_" + to_string(textureIndex);
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }

        return textures;
    }
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif

