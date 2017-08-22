#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <map>
#include <sstream>

#include <FreeImage.h>
#include <iostream>
#include <mesh.h>



class Object
{
public:
    Object(bool debug = false);
//    Object(int window_x, int window_y = 0);
    Object(const std::string &filename, bool useDefaults = false);
    ~Object();

    // testing function
    bool saysomethingobjective(void);

    void reshape(int w, int h);

    FIBITMAP* readTexture(const std::string &filename);
    GLuint uploadTexture(FIBITMAP *tex);
    bool loadObject(const std::string& filename, unsigned int pFlags = aiProcess_JoinIdenticalVertices | aiProcess_SplitLargeMeshes);

    void uploadAllMeshes();
    void updateAllMeshes();

    void draw(glm::mat4 proj_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix);
    void drawWithoutParams();

    /**
     * getter for this object's Meshes
     */
    std::vector<Mesh*> getMeshes(void);


    /**
     * @brief setTrafo set transformation matrix of object
     * @param mat matrix to be set
     */
    void setTrafo(const glm::mat4& mat){m_ObjectTrafo = mat;}

    /**
     * @brief getTrafo get transformation matrix of object
     * @return returns transformation matrix
     */
    glm::mat4 getTrafo(){return m_ObjectTrafo;}

    /**
     * @brief getTrafo get transformation matrix of object
     * @param trafo[out] Transformation matrix is copied to this matrix
     */
    void getTrafo(glm::mat4& trafo){trafo = m_ObjectTrafo;}

    /**
     * @brief getMeshesVector get vector containing pointers to this object's meshes
     * @param meshVector[out] Vector is copied to this vector
     */
    void getMeshesVector(std::vector<Mesh*>& meshVector){meshVector = m_Meshes;}
    int getWindowX();
    int getWindowY();


private:
    bool m_debug;
    int old_window_x;
    int old_window_y;

    std::vector<Mesh*> m_Meshes;
    glm::mat4 m_ObjectTrafo;
    std::map<std::string, GLuint> m_textureIdMap; //for connecting textures to meshes
};

#endif // OBJECT_H
