#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <sstream>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define FREEIMAGE_LIB
#include <FreeImage.h>




class Mesh
{
public:
    struct Material{
        float ambient[4] = {0.f,0.0f,0.0f,1.0};
        float diffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f};
        float specular[4] = {0.7f, 0.7f, 0.7f, 1.0f};
        float emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f};
        float shininess = .25;
        float shininess_strength = 128.;
        GLenum fill_mode = GL_FILL;
    };

    Mesh(const bool& debug = false);

    ~Mesh();

    bool saysomethingmeshy(void);

    /**
     * getter for private m_vertices
     */
    std::vector<glm::vec3> get_m_vertices(void);

    /**
     * @brief addTexture Add a texture and its coordinates to this mesh
     * @param id Id of texture buffer
     * @param tcoords vector containing texture coordinates
     * @return position in internal vector (if we want to replace the texture buffer - think of replaceable textures). Returns -1 if failed
     */
    int addTexture(const GLuint& id, const std::vector<glm::vec2> tcoords);

    /**
     * @brief addTexture Add a texture to this mesh
     * @param id Id of texture buffer
     * @return position in internal vector (if we want to replace the texture buffer - think of replaceable textures). Returns -1 if failed
     */
    int addTexture(const GLuint &id);

    /**
     * @brief replaceTexture Replace texture at "position" position in internal vector (return value from addTexture() )
     * @param id new buffer id
     * @param position position in internal vector
     * @return false if failed (wrong position)
     */
    bool replaceTexture(const GLuint &id, const unsigned int& position);
    /**
     * @brief loadMesh Prepare given mesh to be used for rendering
     * @param mesh Mesh to be prepared
     * @return false if failed
     */
    bool loadMesh(const aiMesh* mesh);
    /**
     * @brief setMaterialandTextures Retrieve textures and materials for this mesh from given aiMaterial
     * @param mat Material in assimp format
     * @param textureIdMap Map with filepath as key and id of texture buffer as data
     * @return false if failed
     */
    bool setMaterialandTextures(const aiMaterial *mat, const std::map<std::string, GLuint> &textureIdMap, const std::string path);

    /**
     * @brief just for testing
     * @return void
     */

    /**
     * @brief uploadToGPU upload this mesh to gpu memory
     * @return false if failed
     */
    bool uploadToGPU(void);
    /**
     * @brief draw Draw with OpenGL calls, needs created context. Can be used in display function
     */
    void draw();


    /**
     * upload set of selected vertices to the GPU
     *
     * @return bool
     */
    bool uploadSelectionToGPU();

    /**
	 * changes color for all vertices of this mesh
	 *
	 * @param float color[4] - RGB value of desired new color
	 */
	void changeAllColors (float color[4]);

    /**
     * change color of vertices at position(s) in selectionIndices to color given in color
     * order of values in color-array: [0]-R; [1]-G; [2]-B, [3]-alpha
     * also uses the alpha value of m_colors: Sets it to .99 to mark a vertex to be highlighted. These are ignored during the automatic update each frame
     *
     * @PARAM std::set <size_t> selectionIndices - positions in m_colors to be changed
     * @param float color[4] - RGB value of desired new color
     */
    void changeColorsAdd (std::set <size_t> &selectionIndices, float color[4]);

    /**
     * reset color of non selected vertices
     * order of values in color-array: [0]-R; [1]-G; [2]-B, [3]-alpha
     *
     * @PARAM std::set <size_t> selectionIndices - positions in m_colors to be changed
     * @param float color[4] - RGB value of desired new color
     */
    void changeColorsRem (std::set <size_t> &selectionIndices, float color[4]);

    /**
     * change color of a single vertex at position given via index to color given via color
     * order of values in color-array: [0]-R; [1]-G; [2]-B, [3]-alpha
     *
     * @PARAM size_t index - position in m_colors to be changed
     * @param float color[4] - RGB value of desired new color
     */
    void changeColor (size_t index, float color[4]);

    /**
     * reset colors of all vertices back to grey and not selected
     */
    void resetColors (void);

    /**
     * @brief setTrafo set transformation matrix of mesh
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

    unsigned int getNumVertices(){return m_vertices.size();}
    unsigned int getNumIndices(){return m_indices.size();}
    unsigned int getNumTextures(){return m_textureIDs.size();}

    void setDrawMode(GLenum mode){m_drawMode = mode;}
    void setMaterial(const Material& material){m_material = material;}
    Material getMaterial(){return m_material;}
    void getTexIdVector(std::vector<GLuint>& texIDVec){texIDVec = m_textureIDs;}


private:

    /* Internal storage for data */
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec4> m_colors;
    std::vector<std::vector<glm::vec2> > m_tCoords;
    std::vector<unsigned int> m_indices;
    Material m_material;
    GLenum m_drawMode;

    glm::mat4x4 m_SceneTrafo;
    glm::mat4x4 m_ObjectTrafo; //adjustable from outside

    GLuint m_vao = 0;
    GLuint m_vao_selection = 1;

    /* id of buffers on gpu */
    GLuint m_vertexID = 0;
    GLuint m_normalID = 0;
    GLuint m_colorsID = 0;
    GLuint m_indicesID = 0;

    std::vector<GLuint> m_tcoordIDs;
    std::vector<GLuint> m_textureIDs;

    bool m_debug;

};

#endif // MESH_H
