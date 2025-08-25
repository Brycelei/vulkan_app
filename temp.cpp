/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Model.cpp
 * Author: tron
 * Function: oad model
 * Created on 2023年5月9日, 上午11:00
 */

/**************************************************************************************************
 * @file model.hpp
 * @brief load model
 * 
 * @details 
 * 
 *  HISTORY
 *  -----------------------------------------------------------------------------------------------
 *  Version   Date        Author    Description
 *  -----------------------------------------------------------------------------------------------
 *  0.1       2023-04-20  TDZ       Migrate from https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/model.h
 *
 *          
**************************************************************************************************/
#include <sys/stat.h>

#ifdef USE_OPENCV
    #include <opencv2/opencv.hpp>
#else
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
#endif //USE_OPENCV


#ifdef HEADER_SPLIT
    #include "Model.hpp"
    #include "GLHelper.hpp"
#else
     #include "CarModelAnimation3D.hpp"
#endif // HEADER_SPLIT


namespace BaseLib{

/** @brief check define for PVRTC */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif /* GL_IMG_texture_compression_pvrtc */

#ifndef GL_IMG_texture_compression_pvrtc2
#define GL_IMG_texture_compression_pvrtc2 1
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0x9138
#endif /* GL_IMG_texture_compression_pvrtc2 */

#ifndef GL_EXT_pvrtc_sRGB
#define GL_EXT_pvrtc_sRGB 1
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT 0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT 0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT 0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT 0x8A57
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG 0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG 0x93F1
#endif /* GL_EXT_pvrtc_sRGB */

#ifndef GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG
    #define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG 0x93F0
#endif /* GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG */

#ifndef GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG
    #define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG 0x93F1
#endif /* GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG */


#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture 1
#define GL_ETC1_RGB8_OES                  0x8D64
#endif /* GL_OES_compressed_ETC1_RGB8_texture */

// #define ENABLE_TEX_FORMAT_PVRTC
#if defined(USE_GLES20) || defined(USE_GLES32)
#define ENABLE_TEX_FORMAT_ETC1
#endif // 


#define ENABLE_TEX_COORD_CLAMP

#define ENABLE_PACK_MATERIAL_TEX


#define PVR_HEADER_VERSION                        (0x03525650)
#define PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORMALISED (0)
#define PVR_CHANNEL_TYPE_SIGNED_BYTE_NORMALISED   (1)
#define PVR_CHANNEL_TYPE_UNSIGNED_BYTE            (2)
#define PVR_CHANNEL_TYPE_SIGNED_BYTE              (3)
#define PVR_CHANNGL_TYPE_UNSIGNED_SHORT           (6)
#define PVR_CHANNEL_TYPE_SIGNED_FLOAT             (12)


/**
 * @brief Compression pixel format
 * more info see
 * https://docs.imgtec.com/specifications/pvr-file-format-specification/topics/pvr-header-format.html
 */  
enum class CompressedPixelFormat:std::uint64_t
{
    PVRTCI_2bpp_RGB = 0,
    PVRTCI_2bpp_RGBA,
    PVRTCI_4bpp_RGB,
    PVRTCI_4bpp_RGBA,
    PVRTCII_2bpp,
    PVRTCII_4bpp,
    ETC1,
};

static bool isGlTF = false;

static long timestampS1 = 0;
static long timestampE1 = 0;
static long timestampS2 = 0;
static long timestampE2 = 0;
static long modelLoadTextDuration = 0;


LogPtrType BaseLib::Model::logPtr = nullptr;

Model::Model()
{
}

Model::~Model()
{
}

void Model::Initialization(std::string const &path, bool (*LOG)(std::string))
{
    this->path = path;

    Model::logPtr = LOG;
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader)
{
    for ( unsigned int i = 0; i < meshes.size(); i++ )
        meshes[i].Draw(shader, meshes[i].textures);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader, std::vector<Texture> &textures)
{
    for ( unsigned int i = 0; i < meshes.size(); i++ )
        meshes[i].Draw(shader, textures);
}

/**
 * @brief draw mesh by specified mesh index with given shader
 * 
 * @param meshIdx unsigned int mesh index
 * @param shader  Shader       compiled shader
 */
void Model::DrawMesh(unsigned int meshIdx, Shader &shader, GLuint texOffset)
{
    if (meshIdx < this->meshes.size())
    {
        this->meshes[meshIdx].Draw(shader, meshes[meshIdx].textures, texOffset);
    }
    else
    {
        if (Model::logPtr != nullptr)
        {
            Model::logPtr("Model::DrawMesh ignored for meshIdx >= this->meshes.size()");
        }
    }
    
}

void Model::DrawMesh(unsigned int meshIdx, Shader &shader, std::vector<Texture> &textures, GLuint texOffset)
{
    if (meshIdx < this->meshes.size())
    {
        this->meshes[meshIdx].Draw(shader, textures, texOffset);
    }
    else
    {
        if (Model::logPtr != nullptr)
        {
            Model::logPtr("Model::DrawMesh with textures ignored for meshIdx >= this->meshes.size()");
        }
    }
    
}


/**
 * @brief Get the frustum fit scale value
 * 
 * @return float 
 */
float Model::GetFrustumFitScale()
{
    float result = 0;
    result = sceneMax.x - sceneMin.x;
    result = fmax((sceneMax.y - sceneMin.y), result);
    result = fmax((sceneMax.z - sceneMin.z), result);

    return result;
}
float Model::GetMaxViewDistance()
{
    glm::vec3 vec3 = (abs(glm::vec3(sceneMin.x, sceneMin.y, sceneMin.z)) + abs(glm::vec3(sceneMax.x, sceneMax.y, sceneMax.z))) / 2.0f;
    float maxDis = fmax(vec3.x, fmax(vec3.y, vec3.z));
    return maxDis;
}
glm::vec3 Model::GetAdjustModelPosVec()
{
    glm::vec3 vec3(sceneCenter.x, sceneCenter.y, sceneCenter.z);

    return vec3;
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::LoadModel()
{
    modelLoadTextDuration = 0;
    timestampS1           = TimeHelper::GetTimestampMicros();

    if ((this->path.find(".gltf") != std::string::npos) || (this->path.find(".glb") != std::string::npos))
    {
        isGlTF = true;
    }

    // read file via ASSIMP
    Assimp::Importer importer;
    //aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
    // aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // const aiScene* scene = aiImportFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    // check for errors
    if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) // if is Not Zero
    {
        if (Model::logPtr != nullptr)
        {
            Model::logPtr("ERROR::ASSIMP:: " + std::string(importer.GetErrorString()));
        }
        return;
    }

    GetBoundBox(&sceneMin, &sceneMax, scene);
    sceneCenter.x = (sceneMin.x + sceneMax.x) / 2.0f;
    sceneCenter.y = (sceneMin.y + sceneMax.y) / 2.0f;
    sceneCenter.z = (sceneMin.z + sceneMax.z) / 2.0f;

    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene);

// #define ENABLE_PRINT_MEASH_TEXTURES
#ifdef ENABLE_PRINT_MEASH_TEXTURES
    for (size_t i = 0; i < this->meshes.size(); i++)
    {
        std::cout << "meash index=" << i << " " << std::endl;
        for (size_t j = 0; j < this->meshes[i].textures.size(); j++)
        {
            std::cout << " tex id=" << j << " type=" << this->meshes[i].textures[j].type << std::endl;
        }
    }
#endif // ENABLE_PRINT_MEASH_TEXTURES

    isGlTF = false;

    timestampE1 = TimeHelper::GetTimestampMicros();

    if (Model::logPtr != nullptr)
    {
        Model::logPtr("Model::LoadModel total(model's vao data + model's texture data) duration:" + std::to_string((timestampE1 - timestampS1)) + "(us)");
        Model::logPtr("Model::LoadModel model's texture duration:" + std::to_string(modelLoadTextDuration) + "(us)");
    }
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::ProcessNode(aiNode *node, const aiScene *scene)
{
    // process each mesh located at the current node
    for ( unsigned int i = 0; i < node->mNumMeshes; i++ )
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
        
        meshsCenter.push_back(GetMeshCenter(i, mesh));

    //#define ENABLE_PRINT_MESH_NAME
    #ifdef ENABLE_PRINT_MESH_NAME
        LOGGER_I("mesh i=%ld, name=%s", meshes.size(), mesh->mName.C_Str());
    #endif // ENABLE_PRINT_MESH_NAME
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for ( unsigned int i = 0; i < node->mNumChildren; i++ )
    {
        ProcessNode(node->mChildren[i], scene);
    }

}
Mesh Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<EBO_INDICES_TYPE> indices;
    std::vector<Texture> textures;

    // walk through each of the mesh's vertices
    for ( unsigned int i = 0; i < mesh->mNumVertices; i++ )
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // for(int k = 0; k < 4; k++)
        // {
        //     vertex.mBoneIDs[k] = 0;
        //     vertex.mWeights[k] = 0.f;
        // }
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

    #ifdef ENABLE_VERTEX_NORMAL
        // normals
        if ( mesh->HasNormals() )
        {
            vector.x      = mesh->mNormals[i].x;
            vector.y      = mesh->mNormals[i].y;
            vector.z      = mesh->mNormals[i].z;
            vertex.normal = vector;
            // LOGGER_I("vertex.normal(%f,%f,%f)", vertex.normal.x,vertex.normal.y,vertex.normal.z);
        }
    #endif // ENABLE_VERTEX_NORMAL
    
        // texture coordinates
        if ( mesh->mTextureCoords[0] ) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            

            // if (vec.x > 1.0f || vec.y > 1.0f)
            // {
            //     LOGGER_E("vec.x > 1.0f || vec.y > 1.0f, mesh.name=%s vec2(%f, %f)", mesh->mName.C_Str(), vec.x, vec.y);
            // }

        #ifdef ENABLE_TEX_COORD_CLAMP
            if (vec.x > 1.0f)
            {
                vec.x = vec.x - static_cast<int>(vec.x);
            }

            if (vec.y > 1.0f)
            {
                vec.y = vec.y - static_cast<int>(vec.y);
            }
        #endif // ENABLE_TEX_COORD_CLAMP

            vertex.texCoords = vec;

    #ifdef ENABLE_VERTEX_TANGENT
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;

            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
    #endif // ENABLE_VERTEX_TANGENT
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for ( unsigned int i = 0; i < mesh->mNumFaces; i++ )
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for ( unsigned int j = 0; j < face.mNumIndices; j++ )
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    timestampS2 = TimeHelper::GetTimestampMicros();

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    if (isGlTF)
    {
        /**************************************************************************************************
         * @brief glTF2 format
         * 
        **************************************************************************************************/

        // 2. map_bump for normal
        std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 3. for gltf ao, roughness, material can be the same. r
        std::vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_LIGHTMAP, "texture_ao");
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

    #ifndef ENABLE_PACK_MATERIAL_TEX
    // #define ENABLE_READ_RM_FROM_MATERIAL
    #if defined(ENABLE_READ_RM_FROM_MATERIAL)
        // 4. for gltf ao, roughness, material can be the same. g
        std::vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        // 5. for gltf ao, roughness, material can be the same. b
        std::vector<Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic");
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    #endif //ENABLE_READ_RM_FROM_MATERIAL

    #ifndef ENABLE_READ_RM_FROM_MATERIAL
        if (aoMaps.size() > 0)
        {
            std::vector<Texture> roughnessMaps;
            std::vector<Texture> metallicMaps;
            for (Texture tex: aoMaps)
            {
                tex.type = "texture_roughness";
                roughnessMaps.push_back(tex);

                tex.type = "texture_metallic";
                metallicMaps.push_back(tex);
            }

            textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
            textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
        }
    #endif // ENABLE_READ_RM_FROM_MATERIAL
    #endif // ENABLE_PACK_MATERIAL_TEX
    }
    else 
    {
        /**************************************************************************************************
         * @brief default OBJ mtl format
         * 
        **************************************************************************************************/
        
        // 2. map_bump for normal
        std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 3. ambient, map_Ka
        std::vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ao");
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

    #ifndef ENABLE_PACK_MATERIAL_TEX
        // 4. shinness maps, map_Ns
        std::vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_SHININESS, "texture_roughness");
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        // 5. specular maps, map_Ks
        std::vector<Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_metallic");
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    #endif // ENABLE_PACK_MATERIAL_TEX
    }

    ///////////////
    // #define ENABLE_PRINT_ALL_MATERIAL_TEXTURES
    #ifdef ENABLE_PRINT_ALL_MATERIAL_TEXTURES
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    for (int texType = 0; texType < 30; texType++ )
    {
        uint32_t texCount = material->GetTextureCount((aiTextureType)texType);
        for ( unsigned int i = 0; i < texCount; i++ )
        {
            aiString str;
            material->GetTexture((aiTextureType)texType, i, &str);

            aiString matName;
            material->Get(AI_MATKEY_NAME, matName);

            LOGGER_I("GetTexture: matName=%s type=%d count=%u i=%d str=%s",matName.C_Str(), texType, texCount, i, str.C_Str());
        }
    }
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
   
    #endif // ENABLE_PRINT_ALL_MATERIAL_TEXTURES


    ////////////////


    timestampE2 = TimeHelper::GetTimestampMicros();

    modelLoadTextDuration += (timestampE2 - timestampS2);

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<Texture> Model::LoadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    // static int count = 0;
    std::vector<Texture> textures;
    // LOGGER_I("typeName:%s type:%d texCount:%u", typeName.c_str(), type, mat->GetTextureCount(type));
    
    for ( unsigned int i = 0; i < mat->GetTextureCount(type); i++ )
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // LOGGER_I("str:%s", str.C_Str());
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for ( unsigned int j = 0; j < texturesLoaded.size(); j++ )
        {
            if ( std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0 )
            {
                Texture tmpTexture = texturesLoaded[j];

                if (std::strcmp(tmpTexture.type.c_str(), typeName.c_str()) != 0)
                {
                    tmpTexture.type = typeName;
                }

                textures.push_back(tmpTexture);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)

                // LOGGER_D("skip for type:%s", texturesLoaded[j].type.c_str());

                break;
            }
        }

        if ( !skip )
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            stbi_set_flip_vertically_on_load(false);
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
}


/**************************************************************************************************
 * @brief Load Texture
 * 
 * @param typeName  std::string  type name, one of:
 *                               {texture_diffuse, texture_specular, texture_normal, texture_height}
 * @param path      std::string file path
 * @param directory std::string directory
 * @return Texture 
**************************************************************************************************/
Texture Model::LoadTexture(std::string typeName, std::string path, const std::string &directory)
{
    Texture texture;
    texture.id   = TextureFromFile(path.c_str(), directory);
    texture.type = typeName;
    texture.path = path;

    // printf("\n texture.type=%s\n, texture.path = %s \n", texture.type.c_str(), texture.path.c_str());

    return texture;
}


void Model::GetBoundBox(C_STRUCT aiVector3D* min, C_STRUCT aiVector3D* max, const C_STRUCT aiScene* scene)
{
    C_STRUCT aiMatrix4x4 trafo;
    aiIdentityMatrix4(&trafo);

    min->x = min->y = min->z =  1e10f;
    max->x = max->y = max->z = -1e10f;
    GetBoundBox4node(scene->mRootNode, min, max, &trafo, scene);
}
glm::vec3 Model::GetMeshCenter(unsigned int meshIndex, C_STRUCT aiMesh* mesh)
{
    (void) meshIndex;
    unsigned int i;
    glm::vec3 tmp;
    glm::vec3 tmpMin(1e10f, 1e10f, 1e10f);
    glm::vec3 tmpMax(-1e10f, -1e10f, -1e10f);
    glm::vec3 center;

    for ( i = 0; i < mesh->mNumVertices; i++ )
    {
        tmp      = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        tmpMin.x = fmin(tmp.x, tmpMin.x);
        tmpMin.y = fmin(tmp.y, tmpMin.y);
        tmpMin.z = fmin(tmp.z, tmpMin.z);

        tmpMax.x = fmax(tmp.x, tmpMax.x);
        tmpMax.y = fmax(tmp.y, tmpMax.y);
        tmpMax.z = fmax(tmp.z, tmpMax.z);
    }

    center = (tmpMin + tmpMax) / 2.0f;

    return center;
}
/**
 * @brief Get the Bound Box for node object
 * 
 * @param nd 
 * @param min 
 * @param max 
 * @param trafo 
 */
void Model::GetBoundBox4node (const C_STRUCT aiNode* nd,
        C_STRUCT aiVector3D* min,
        C_STRUCT aiVector3D* max,
        C_STRUCT aiMatrix4x4* trafo, 
        const C_STRUCT aiScene* scene
        )
{
    C_STRUCT aiMatrix4x4 prev;
    unsigned int n = 0, t;

    prev = *trafo;
    aiMultiplyMatrix4(trafo, &nd->mTransformation);

    for ( ; n < nd->mNumMeshes; ++n )
    {
        const C_STRUCT aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for ( t = 0; t < mesh->mNumVertices; ++t )
        {

            C_STRUCT aiVector3D tmp = mesh->mVertices[t];
            aiTransformVecByMatrix4(&tmp, trafo);

            min->x = fmin(min->x, tmp.x);
            min->y = fmin(min->y, tmp.y);
            min->z = fmin(min->z, tmp.z);

            max->x = fmax(max->x, tmp.x);
            max->y = fmax(max->y, tmp.y);
            max->z = fmax(max->z, tmp.z);
        }
    }

    for ( n = 0; n < nd->mNumChildren; ++n )
    {
        GetBoundBox4node(nd->mChildren[n], min, max, trafo, scene);
    }

    *trafo = prev;
}

unsigned int Model::TextureFromFile(const char *path, const std::string &directory)
{
    // LOGGER_I("TextureFromFile start path=%s, dir=%s", path, directory.c_str());
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID = 0;

// #if defined(GL_ES_TDA) || defined(USE_GLES20) || defined(USE_GLES32)
    std::string filenamePvr = filename + ".pvr";
    struct stat fileStat;
    int statRet = stat(filenamePvr.c_str(), &fileStat);
    if (statRet == 0)
    {
        /** @brief find texture .pvr file and gen texture */
        ParsePVR(filenamePvr.c_str(), &textureID);

        // LOGGER_I("TextureFromFile ParsePVR filename=%s, textureID=%u", filenamePvr.c_str(), textureID);
    }

    /** @brief invalid textureID, use the origin texture image file */
    if(textureID == 0)
    {
        textureID = Model::GenImageTexture(filename);
    }

// #else // GL_ES_TDA

//     textureID = Model::GenImageTexture(filename);

// #endif // GL_ES_TDA

    // LOGGER_GLE("TextureFromFile done, filename:%s", filename.c_str());

    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    return textureID;
}


GLuint Model::GenImageTexture(std::string filename)
{
    GLuint textureID = GL_NONE;

    bool isGenerateMipmap = true;

    if (filename.find("disable_mipmap") != std::string::npos)
    {
        isGenerateMipmap = false;
    }

#ifdef USE_OPENCV
        glGenTextures(1, &textureID);
        cv::Mat mat = cv::imread(filename, cv::IMREAD_UNCHANGED);
        GLenum format         = GL_RGBA;
        GLint  internalformat = GL_RGBA;

        if (mat.channels() == 4)
        {
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2RGBA);
            internalformat = GL_RGBA;
        }
      
        if (mat.channels() == 3)
        {
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
            format         = GL_RGB;
            internalformat = GL_RGB;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, mat.cols, mat.rows, 0, format, GL_UNSIGNED_BYTE, mat.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#else // USE_OPENCV

        bool isFloat = false;
        if ((filename.find(".psd") != std::string::npos) || 
            (filename.find(".exr") != std::string::npos) ||
            (filename.find(".hdr") != std::string::npos))
        {
            isFloat = true;
        }

        int width, height, nrComponents;
        void *data = nullptr;

        if(isFloat)
        {
            data = (float *) stbi_loadf(filename.c_str(), &width, &height, &nrComponents, 0);
        }
        else
        {
            data = (unsigned char *) stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        }
        
        if(Model::logPtr != nullptr)
        {
            Model::logPtr("Model::GenImageTexture filename="+ filename +",width=" + std::to_string(width)+ ",height=" + std::to_string(height) + ",nr=" + std::to_string(nrComponents) + ",isFloat="+ std::to_string(isFloat) +",isGenerateMipmap=" + std::to_string(isGenerateMipmap));
        }
        
        if (data)
        {
            GLint internalFormat  = GL_NONE;
            GLenum format         = GL_NONE;
            GLenum texImage2dType = GL_UNSIGNED_BYTE;
            
            if (nrComponents == 1)
            {
                internalFormat = isFloat ? GL_R16F : GL_RED;
                format         = GL_RED;
                texImage2dType  = isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE;
            }   
            else if (nrComponents == 3)
            {
                internalFormat  = isFloat ? GL_RGB16F : GL_RGB;
                format          = GL_RGB;
                texImage2dType  = isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE;
            }
            else if (nrComponents == 4)
            {
                internalFormat  = isFloat ? GL_RGBA16F : GL_RGBA;
                format          = GL_RGBA;
                texImage2dType  = isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE;
            }


            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, texImage2dType, data);
            

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (isGenerateMipmap)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }

            stbi_image_free(data);
        }
        else
        {
            if(Model::logPtr != nullptr)
            {
                Model::logPtr("error GenImageTexture Texture failed to load at path:" + filename);
            }
        }

#endif // USE_OPENCV

    return textureID;
}

/**************************************************************************************************
 * @brief parse .pvr format file and generate a texture
 * 
 * @param filename pvr format file.
 * @param textureID generated texture, a valid texture id would be > 0
**************************************************************************************************/
void Model:: ParsePVR(const char *filename, unsigned int* textureID)
{
    *textureID = GL_NONE;

    bool checkResult = true;
 
    FILE* mpFile = fopen(filename, "rb");

    if (mpFile == nullptr)
    {
        if(Model::logPtr != nullptr)
        {
            Model::logPtr("ParsePVR open file failed of " + std::string(filename));
        }
        
        checkResult = false;
    }

    unsigned char *mpData        = nullptr;
    unsigned char *originDataPtr = nullptr;
    unsigned char *pCurrentPos   = nullptr;

    uint32_t uiSmallestWidth  = 1;
    uint32_t uiSmallestHeight = 1;
    uint32_t uiSmallestDepth  = 1;
    uint32_t bitsPerPixel     = 0;

    GLint  internalformat   = GL_NONE; // GL_NONE is important
    GLenum format           = GL_NONE;
    GLenum texImage2dType   = GL_UNSIGNED_BYTE;
    bool isCompressedFormat = false;
    
    uint32_t nVersion;
    uint64_t nFormat;
    uint32_t nColorSpace;
    uint32_t nChannelType;
    uint32_t nPicWid;
    uint32_t nPicHgt;
    uint32_t nPicDept;
    uint32_t nSurface;
    // uint32_t nFace;
    // uint32_t nSize;
    uint32_t nLevel;
    uint32_t nMetaData;

    long int nFileLen = 0;
    
    /** @brief file length, PVR header size=52 */
    if (checkResult)
    {
        fseek(mpFile, 0, SEEK_END);
        nFileLen = ftell(mpFile);

        if(nFileLen < 52)  
        {
            if(Model::logPtr != nullptr)
            {
                Model::logPtr("invalid PowerVR file format, cause: fileLen=" + std::to_string(nFileLen) + " for filename:" + std::string(filename));
            }
           
            checkResult = false;
        }
    }
  

    if (checkResult)
    {
        mpData        = (unsigned char *)malloc(nFileLen * sizeof(unsigned char));
        originDataPtr = mpData;
            
        memset(mpData, 0, nFileLen);
        rewind(mpFile);

        int nError = fread(mpData, sizeof(mpData[0]), nFileLen, mpFile);

        if (nError != nFileLen)
        {
            if(Model::logPtr != nullptr)
            {
                Model::logPtr("invalid PowerVR file format, cause: fread IO error for filename:" + std::string(filename));
            }

            checkResult = false;
        }
    }

    fclose(mpFile);

    if (checkResult)
    {
        nVersion = *(uint32_t*)(mpData);

        if (nVersion != PVR_HEADER_VERSION)
        {
            if(Model::logPtr != nullptr)
            {
                Model::logPtr("invalid PowerVR file format or version, cause: header version is not " + std::to_string(PVR_HEADER_VERSION) + " for filename:" + std::string(filename));
            }
            checkResult = false;
        }
    }
    
    
    if (checkResult)
    {
         /** @brief PVR header info */
        nFormat       = *(uint64_t*)(mpData + 8);
        nColorSpace   = *(uint32_t*)(mpData + 16);
        nChannelType  = *(uint32_t*)(mpData + 20);
        nPicHgt       = *(uint32_t*)(mpData + 24);
        nPicWid       = *(uint32_t*)(mpData + 28);
        nPicDept      = *(uint32_t*)(mpData + 32);
        nSurface      = *(uint32_t*)(mpData + 36);
        // nFace      = *(uint32_t*)(mpData + 40);
        nLevel        = *(uint32_t*)(mpData + 44);
        nMetaData     = *(uint32_t*)(mpData + 48);

        isCompressedFormat = ((nFormat & 0xFFFFFFFF00000000)) == 0;

        if(Model::logPtr != nullptr)
        {
            Model::logPtr("ParsePVR filename:"+ std::string(filename) +" " + std::to_string(nPicWid) + "x" + std::to_string(nPicHgt) + " " +(nColorSpace==0 ? "lRGB" : "sRGB"));
        }

        /** @note don't not support PVR texture array */
        assert(nSurface == 1);

        mpData    += nMetaData + 52;

        

        

        if (isCompressedFormat)
        {
			#ifdef ENABLE_TEX_FORMAT_PVRTC
			    /** @brief use lRGB better  */
	            nColorSpace = 0;
	        #endif // ENABLE_TEX_FORMAT_PVRTC
		
            switch (nFormat)
            {
                #ifdef ENABLE_TEX_FORMAT_PVRTC
                /** @brief PVRTCI 2bpp*/
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
                    bitsPerPixel     = 2;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT;
                    uiSmallestWidth  = 16;
                    uiSmallestHeight = 8;
                    uiSmallestDepth  = 1;
                    break;
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
                    bitsPerPixel     = 2;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT;
                    uiSmallestWidth  = 16;
                    uiSmallestHeight = 8;
                    uiSmallestDepth  = 1;
                    break;
                /** @brief PVRTCI 4bpp*/
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
                    bitsPerPixel     = 4;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT;
                    uiSmallestWidth  = 8;
                    uiSmallestHeight = 8;
                    uiSmallestDepth  = 1;
                    break;
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
                    bitsPerPixel     = 4;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT;
                    uiSmallestWidth  = 8;
                    uiSmallestHeight = 8;
                    uiSmallestDepth  = 1;
                    break;
                /** @brief PVRTCII 2bpp */
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
                    bitsPerPixel     = 2;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG;
                    uiSmallestWidth  = 8;
                    uiSmallestHeight = 4;
                    uiSmallestDepth  = 1;
                    break;
                /** @brief PVRTCII 4bpp */
                case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
                    bitsPerPixel     = 4;
                    if (nColorSpace == 0) internalformat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
                    else                  internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG;
                    uiSmallestWidth  = 4;
                    uiSmallestHeight = 4;
                    uiSmallestDepth  = 1;
                    break;
                #endif // ENABLE_TEX_FORMAT_PVRTC

                #if defined(ENABLE_TEX_FORMAT_ETC1)
                case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
                    // if (BaseLib::GLHelper::IsExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture"))
                    // {
                        bitsPerPixel     = 4;
                        internalformat = GL_ETC1_RGB8_OES;
                        uiSmallestWidth  = 4;
                        uiSmallestHeight = 4;
                        uiSmallestDepth  = 1;
                    // }
                    
                    break;
                #endif // ENABLE_TEX_FORMAT_ETC1
                default:
                    bitsPerPixel = 0;

                    break;
                }
        }
        else
        {
            /** @brief none compression */
            if ((originDataPtr[8] == 'r') && (originDataPtr[9] == 'g') && (originDataPtr[10] == 'b') && (originDataPtr[11] == 'a'))
            {
                format          = GL_RGBA;
            
                if ((originDataPtr[12] == 8) && (originDataPtr[13] == 8) && (originDataPtr[14] == 8) && (originDataPtr[15] == 8))
                {
                    if ((nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORMALISED) || (nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE))
                    {
                        internalformat  = GL_RGBA8;
                        texImage2dType  = GL_UNSIGNED_BYTE;
                    }
                }
                else if ((originDataPtr[12] == 16) && (originDataPtr[13] == 16) && (originDataPtr[14] == 16) && (originDataPtr[15] == 16))
                {
                    switch (nChannelType)
                    {
                        case PVR_CHANNEL_TYPE_SIGNED_FLOAT:
                            internalformat  = GL_RGBA16F;
                            texImage2dType  = GL_HALF_FLOAT;
                            break;
                        case PVR_CHANNGL_TYPE_UNSIGNED_SHORT:
                            internalformat  = GL_RGBA16UI;
                            texImage2dType  = GL_UNSIGNED_SHORT;
                            break;
                        default:
                            break;
                    }
                }
                else if ((originDataPtr[12] == 32) && (originDataPtr[13] == 32) && (originDataPtr[14] == 32) && (originDataPtr[15] == 32))
                {
                    if (nChannelType == PVR_CHANNEL_TYPE_SIGNED_FLOAT)
                    {
                        internalformat  = GL_RGBA32F;
                        texImage2dType  = GL_FLOAT;
                    }
                }
                else
                {
                    // pass
                }

            }
            else if ((originDataPtr[8] == 'r') && (originDataPtr[9] == 'g') && (originDataPtr[10] == 'b') && (originDataPtr[11] == '\0'))
            {

                format          = GL_RGB;
            
                if ((originDataPtr[12] == 8) && (originDataPtr[13] == 8) && (originDataPtr[14] == 8))
                {
                    if ((nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORMALISED) || (nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE))
                    {
                        internalformat  = GL_RGB8;
                        texImage2dType  = GL_UNSIGNED_BYTE;
                    }
                    
                }
                else if ((originDataPtr[12] == 16) && (originDataPtr[13] == 16) && (originDataPtr[14] == 16))
                {
                    switch (nChannelType)
                    {
                        case PVR_CHANNEL_TYPE_SIGNED_FLOAT:
                            internalformat  = GL_RGB16F;
                            texImage2dType  = GL_HALF_FLOAT;
                            break;
                        case PVR_CHANNGL_TYPE_UNSIGNED_SHORT:
                            internalformat  = GL_RGB16UI;
                            texImage2dType  = GL_UNSIGNED_SHORT;
                            break;
                        default:
                            break;
                    }
                   
                }
                else if ((originDataPtr[12] == 32) && (originDataPtr[13] == 32) && (originDataPtr[14] == 32))
                {
                    if (nChannelType == PVR_CHANNEL_TYPE_SIGNED_FLOAT)
                    {
                        internalformat  = GL_RGB32F;
                        texImage2dType  = GL_FLOAT;
                    }
                }
                else
                {
                    // pass
                }
            }
            else if ((originDataPtr[8] == 'r') && (originDataPtr[9] == 'g') && (originDataPtr[10] == '\0') && (originDataPtr[11] == '\0'))
            {

                format          = GL_RG;
            
                if ((originDataPtr[12] == 8) && (originDataPtr[13] == 8))
                {
                    if ((nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORMALISED) || (nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE))
                    {
                        internalformat  = GL_RG8;
                        texImage2dType  = GL_UNSIGNED_BYTE;
                    }
                    
                }
                else if ((originDataPtr[12] == 16) && (originDataPtr[13] == 16))
                {
                    switch (nChannelType)
                    {
                        case PVR_CHANNEL_TYPE_SIGNED_FLOAT:
                            internalformat  = GL_RG16F;
                            texImage2dType  = GL_HALF_FLOAT;
                            break;
                        case PVR_CHANNGL_TYPE_UNSIGNED_SHORT:
                            internalformat  = GL_RG16UI;
                            texImage2dType  = GL_UNSIGNED_SHORT;
                            break;
                        default:
                            break;
                    }
                   
                }
                else if ((originDataPtr[12] == 32) && (originDataPtr[13] == 32))
                {
                    if (nChannelType == PVR_CHANNEL_TYPE_SIGNED_FLOAT)
                    {
                        internalformat  = GL_RG32F;
                        texImage2dType  = GL_FLOAT;
                    }
                }
                else
                {
                    // pass
                }
            }
            else if ((originDataPtr[8] == 'r') && (originDataPtr[9] == '\0') && (originDataPtr[10] == '\0') && (originDataPtr[11] == '\0'))
            {
                format          = GL_RED;
            
                if ((originDataPtr[12] == 8))
                {
                    if ((nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORMALISED) || (nChannelType == PVR_CHANNEL_TYPE_UNSIGNED_BYTE))
                    {
                        internalformat  = GL_R8;
                        texImage2dType  = GL_UNSIGNED_BYTE;
                    }
                    
                }
                else if ((originDataPtr[12] == 16))
                {
                    
                    if (nChannelType == PVR_CHANNEL_TYPE_SIGNED_FLOAT)
                    {
                        internalformat  = GL_R16F;
                        texImage2dType  = GL_HALF_FLOAT;
                    }
                   
                }
                else if ((originDataPtr[12] == 32))
                {
                    if (nChannelType == PVR_CHANNEL_TYPE_SIGNED_FLOAT)
                    {
                        internalformat  = GL_R32F;
                        texImage2dType  = GL_FLOAT;
                    }
                }
                else
                {
                    // pass
                }
            }
            else
            {
                // pass
            }

            if (internalformat != GL_NONE)
            {
                bitsPerPixel = originDataPtr[12] + originDataPtr[13] + originDataPtr[14] + originDataPtr[15];
            }
        }

        // LOGGER_D("step bitsPerPixel=%u, nLevel=%u, isCompressedFormat=%d", bitsPerPixel, nLevel, isCompressedFormat);
        // LOGGER_I("glTexImage2D internalformat=0x%X,format=0x%X,texImage2dType=0x%X",internalformat, format, texImage2dType);


        if (bitsPerPixel == 0) 
        {
            if(Model::logPtr != nullptr)
            {
                Model::logPtr("invalid PowerVR file format, cause: unsupport format:" + std::to_string(nFormat));
            }
            
            checkResult = false;
        }
    }

    if (checkResult)
    {
        pCurrentPos = mpData;

        glGenTextures(1, textureID);
        glBindTexture(GL_TEXTURE_2D, *textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // glTexStorage2D(GL_TEXTURE_2D, static_cast<GLsizei>(nLevel), internalformat, static_cast<GLsizei>(nPicWid), static_cast<GLsizei>(nPicHgt));

        // LOGGER_GLE("glPixelStorei GL_UNPACK_ALIGNMENT");
        
        uint32_t dataSizePre = 0;

        /** @brief parse for texture data */
        for (uint32_t i=0; i < nLevel; i++)
        {
            /** @brief level and padding for min */
            uint32_t uiWidth      = std::max<uint32_t>(nPicWid >> i, 1);
            uint32_t uiHeight     = std::max<uint32_t>(nPicHgt >> i, 1);
            uint32_t uiDepth      = std::max<uint32_t>(nPicDept >> i, 1);
            uint32_t uiSizeWidth  = uiWidth  + ((-1 * uiWidth)  % uiSmallestWidth);
            uint32_t uiSizeHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
            uint32_t uiSizeDepth  = uiDepth  + ((-1 * uiDepth)  % uiSmallestDepth);

            /** @brief calcaulte data size */
            uint64_t uiDataSize = static_cast<uint64_t>(bitsPerPixel) * static_cast<uint64_t>(uiSizeWidth) * static_cast<uint64_t>(uiSizeHeight) * static_cast<uint64_t>(uiSizeDepth);
            uint32_t dataSize   = static_cast<uint32_t>(uiDataSize / 8);

            // LOGGER_D("step bitsPerPixel=%u, nLevel=%u, i=%u, uiWidth=%u, uiHeight=%u", bitsPerPixel, nLevel, i, uiWidth, uiHeight);

            if (isCompressedFormat)
            {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, internalformat, uiWidth, uiHeight, 0 , dataSize, (void*)pCurrentPos);
            }
            else
            {
                // glTexImage2D(GL_TEXTURE_2D, i, internalformat, uiWidth, uiHeight, 0, format, texImage2dType, (void*)pCurrentPos);

                glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), static_cast<GLint>(internalformat), static_cast<GLsizei>(uiWidth), static_cast<GLsizei>(uiHeight), 0, static_cast<GLenum>(format), static_cast<GLenum>(texImage2dType), (unsigned char*)pCurrentPos);
                // glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), 0, 0, static_cast<GLsizei>(uiWidth), static_cast<GLsizei>(uiHeight), static_cast<GLenum>(format), static_cast<GLenum>(texImage2dType), (void *)pCurrentPos);

                // LOGGER_GLE("glTexImage2D internalformat=0x%X,format=0x%X,texImage2dType=0x%X",internalformat, format, texImage2dType);
            }
            

            if (dataSizePre != dataSize)
            {
                pCurrentPos = pCurrentPos + dataSize;  
                dataSizePre = dataSize;
            }
        }
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (nLevel > 1)
        {
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // LOGGER_GLE("ParsePVR glTexParameteri file:%s", filename); 
    }
    
    if (originDataPtr)
    {
       free(originDataPtr);
    }

    return;
}

}


/**************************************************************************************************
 * END OF FILE
 *************************************************************************************************/
