//#include "render/model_loader.h"
//#include "iostream"
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//
//namespace render {
//
//    class ModelLoader::Impl {
//    public:
//        tl::expected<Handle<Model>, std::string> load_model(const std::filesystem::path &path) {
//            Assimp::Importer import;
//            const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
//
//            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//                return tl::make_unexpected(import.GetErrorString());
//
//            processNode(scene->mRootNode, scene);
//        }
//
//        Model *get_model(Handle<Model> handle) {
//
//        }
//
//        bool unload_model(Handle<Model> handle) {
//
//        }
//
//    private:
//        Assimp::Importer importer;
//        HandleManager<Model> handle_manager;
//        std::vector<std::unique_ptr<Model>> models;
//
//        Mesh process_mesh(aiMesh *mesh, const aiScene *scene) {
//            Mesh result;
//
//            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//                Vertex vertex;
//
//                vertex.position.x = mesh->mVertices[i].x;
//                vertex.position.y = mesh->mVertices[i].y;
//                vertex.position.z = mesh->mVertices[i].z;
//
//                vertex.normal.x = mesh->mNormals[i].x;
//                vertex.normal.y = mesh->mNormals[i].y;
//                vertex.normal.z = mesh->mNormals[i].z;
//
//                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
//                    vertex.tex_coords.x = mesh->mTextureCoords[0][i].x;
//                    vertex.tex_coords.y = mesh->mTextureCoords[0][i].y;
//                } else {
//                    vertex.tex_coords = glm::vec2(0.0f, 0.0f);
//                }
//
//                result.vertices.push_back(vertex);
//            }
//            // process indices
//            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//                aiFace face = mesh->mFaces[i];
//                for (unsigned int j = 0; j < face.mNumIndices; j++)
//                    result.indices.push_back(face.mIndices[j]);
//            }
//            // process material
//            if (mesh->mMaterialIndex >= 0) {
//                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//                vector <Texture> diffuseMaps = loadMaterialTextures(material,
//                                                                    aiTextureType_DIFFUSE, "texture_diffuse");
//                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//                vector <Texture> specularMaps = loadMaterialTextures(material,
//                                                                     aiTextureType_SPECULAR, "texture_specular");
//                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//            }
//
//            return result;
//        }
//
//        void process_node(Model &model, aiNode *node, const aiScene *scene) {
//            // process all the node's meshes (if any)
//            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
//                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
//                model.meshes.push_back(process_mesh(mesh, scene));
//            }
//            // then do the same for each of its children
//            for (unsigned int i = 0; i < node->mNumChildren; i++) {
//                process_node(node->mChildren[i], scene);
//            }
//        }
//
//        vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name)
//        {
//            vector<Texture> textures;
//            for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//            {
//                aiString str;
//                mat->GetTexture(type, i, &str);
//                Texture texture;
//                texture.id = TextureFromFile(str.C_Str(), directory);
//                texture.type = typeName;
//                texture.path = str;
//                textures.push_back(texture);
//            }
//            return textures;
//        }
//    };
//
//    ModelLoader::ModelLoader() {
//        impl = new Impl();
//    }
//
//    ModelLoader::~ModelLoader() {
//        delete impl;
//    }
//
//    tl::expected<Handle<Model>, std::string> ModelLoader::load_model(const std::filesystem::path &path) {
//        return impl->load_model(path);
//    }
//
//    Model *ModelLoader::get_model(Handle<Model> handle) {
//        return impl->get_model(handle);
//    }
//
//    bool ModelLoader::unload_model(Handle<Model> handle) {
//        return impl->unload_model(handle);
//    }
//}