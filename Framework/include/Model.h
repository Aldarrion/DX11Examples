#pragma once
#include "Mesh.h"

#include "Logging.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "ContextWrapper.h"

namespace Models {

class Model {
public:
    Model(const ContextWrapper& context, const std::string& path) {
        loadModel(context, path);
    }

    void draw(const ContextWrapper& context) const {
        ex::beginEvent(context.perf_, L"Draw Model");
        for(const auto& mesh : meshes_) {
            mesh.draw(context.immediateContext_);
        }
        ex::endEvent(context.perf_);
    }

private:
    std::vector<Mesh> meshes_;
    std::string directory_;

    static TextureType mapTextureType(const aiTextureType assimpType) {
        switch (assimpType) {
            case aiTextureType_DIFFUSE: 
                return TextureType::Diffuse;
            case aiTextureType_SPECULAR:
                return TextureType::Specular;
            case aiTextureType_AMBIENT:
                return TextureType::Height;
            case aiTextureType_HEIGHT:
                return TextureType::Normal;
            default: 
                return TextureType::Unknown;
        }
    }

    std::vector<ModelTexture> loadMaterialTextures(const ContextWrapper& context, aiMaterial *mat, const aiTextureType type) {
        std::vector<ModelTexture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string name(str.C_Str());
            const std::wstring textureFile(name.begin(), name.end());
            const std::wstring dir(directory_.begin(), directory_.end());
            const bool isSRGB = type == aiTextureType_DIFFUSE;

            Textures::PTexture texture = std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, (dir + L"/" + textureFile).c_str(), isSRGB);
            ModelTexture modelTexture(std::move(texture), mapTextureType(type));
            textures.push_back(std::move(modelTexture));
        }
        return textures;
    }

    Mesh processMesh(const ContextWrapper& context, aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for(unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to XMFLOAT3 class 
            // so we transfer the data to this placeholder XMFLOAT3 first.
            DirectX::XMFLOAT3 vector; 
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                DirectX::XMFLOAT2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else {
                vertex.TexCoords = DirectX::XMFLOAT2(0.0f, 0.0f);
            }
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            //vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            //vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<ModelTexture> textures;

        std::vector<ModelTexture> diffuseMaps = loadMaterialTextures(context, material, aiTextureType_DIFFUSE);
        textures.insert(textures.end(), make_move_iterator(diffuseMaps.begin()), make_move_iterator(diffuseMaps.end()));
        
        std::vector<ModelTexture> specularMaps = loadMaterialTextures(context, material, aiTextureType_SPECULAR);
        textures.insert(textures.end(), make_move_iterator(specularMaps.begin()), make_move_iterator(specularMaps.end()));
        
        std::vector<ModelTexture> normalMaps = loadMaterialTextures(context, material, aiTextureType_HEIGHT);
        textures.insert(textures.end(), make_move_iterator(normalMaps.begin()), make_move_iterator(normalMaps.end()));
        
        std::vector<ModelTexture> heightMaps = loadMaterialTextures(context, material, aiTextureType_AMBIENT);
        textures.insert(textures.end(), make_move_iterator(heightMaps.begin()), make_move_iterator(heightMaps.end()));

        return Mesh(context.d3dDevice_, vertices, indices, std::move(textures));
    }

    void processNode(const ContextWrapper& context, aiNode *node, const aiScene *scene) {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes_.push_back(processMesh(context, mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(context, node->mChildren[i], scene);
        }
    }

    void loadModel(const ContextWrapper& context, const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // if is Not Zero
            ex::log(ex::LogLevel::Error, "ASSIMP:: %s", importer.GetErrorString());
            return;
        }

        directory_ = path.substr(0, path.find_last_of('/'));

        processNode(context, scene->mRootNode, scene);
    }
};
}
