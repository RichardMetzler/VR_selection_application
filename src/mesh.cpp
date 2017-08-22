#include "mesh.h"

Mesh::Mesh(const bool &debug) : m_debug(debug)
{

}

// destructor
Mesh::~Mesh()
{
    //delete buffers
    if(m_vertexID)
        glDeleteBuffers(1,&m_vertexID);
    if(m_colorsID)
        glDeleteBuffers(1,&m_colorsID);
    if(m_normalID)
        glDeleteBuffers(1,&m_normalID);
    if(m_indicesID)
        glDeleteBuffers(1,&m_indicesID);
    for(std::vector<GLuint>::iterator it = m_tcoordIDs.begin(); it != m_tcoordIDs.end(); ++it){
        glDeleteBuffers(1,&(*it));
    }
    //texture buffers are not create here and consequently must be deleted elsewhere!

}

bool Mesh::saysomethingmeshy(void) {
	std::cout << "Mesh.cpp says hello" << std::endl;
	return true;
}

std::vector<glm::vec3> Mesh::get_m_vertices(void) {
	return m_vertices;
}

int Mesh::addTexture(const GLuint &id, const std::vector<glm::vec2> tcoords)
{
    if(tcoords.size() != m_vertices.size())
        return -1;

    int retValue = m_tCoords.size(); //new position in vector after push_back
    m_textureIDs.push_back(id);
    m_tCoords.push_back(tcoords);
    return retValue;
}

int Mesh::addTexture(const GLuint &id)
{
    int retValue = m_textureIDs.size(); //new position in vector after push_back
    m_textureIDs.push_back(id);

    return retValue;
}

bool Mesh::replaceTexture(const GLuint &id, const unsigned int &position)
{
    if(position >= m_textureIDs.size())
        return false;
    m_textureIDs[position] = id;
    return true;
}

bool Mesh::loadMesh(const aiMesh* mesh)
{
    m_vertices.resize(mesh->mNumVertices);
    float Xmax = 0., Xmin = 0., Ymax = 0., Ymin = 0., Zmax = 0., Zmin = 0.;

    for(int i = 0; i < mesh->mNumVertices; ++i) {
        m_vertices[i][0] = mesh->mVertices[i].x;
        m_vertices[i][1] = mesh->mVertices[i].y;
        m_vertices[i][2] = mesh->mVertices[i].z;

        if (m_vertices[i][0] < Xmin) {
        	Xmin = m_vertices[i][0];
        }
        if (m_vertices[i][0] > Xmax) {
        	Xmax = m_vertices[i][0];
        }
        if (m_vertices[i][1] < Ymin) {
			Ymin = m_vertices[i][1];
		}
        if (m_vertices[i][1] > Ymax) {
			Ymax = m_vertices[i][1];
		}
        if (m_vertices[i][2] < Zmin) {
			Zmin = m_vertices[i][2];
		}
        if (m_vertices[i][2] > Zmax) {
        	Zmax = m_vertices[i][2];
		}
    }

    if(mesh->HasTextureCoords(0)) {		// check if first set of texture coordinates is available
        bool hasCoords = true;
        int curCoord = 0;
        while(hasCoords){
            if(mesh->HasTextureCoords(curCoord)) {
                std::vector<glm::vec2 > tmpvector;
                tmpvector.resize(mesh->mNumVertices);
                for(int i = 0; i < mesh->mNumVertices; ++i) {
                    tmpvector[i][0]= mesh->mTextureCoords[curCoord][i].x;
                    tmpvector[i][1] = mesh->mTextureCoords[curCoord][i].y;
                }
                ++curCoord;
                m_tCoords.push_back(tmpvector); //slow but sufficient

            } else {
                hasCoords = false;
            }
        }
    }

    if(mesh->HasNormals()){
        m_normals.resize(mesh->mNumVertices);
        for(int i = 0; i < mesh->mNumVertices; ++i) {
            m_normals[i][0] = mesh->mNormals[i].x;
            m_normals[i][1] = mesh->mNormals[i].y;
            m_normals[i][2] = mesh->mNormals[i].z;
        }
    }

//TODO: we only support one color channel (multiple need custom shaders)
    if(mesh->HasVertexColors(0)){
    	m_colors.resize(mesh->mNumVertices);
        for(int i = 0; i < mesh->mNumVertices; ++i) {
			// actual colors imported form the mesh
//			m_colors[i][0] = mesh->mColors[0][i].r;
//			m_colors[i][1] = mesh->mColors[0][i].g;
//			m_colors[i][2] = mesh->mColors[0][i].b;
//			m_colors[i][3] = mesh->mColors[0][i].a;

        	// light grey per default
			m_colors[i][0] = 0.89;
			m_colors[i][1] = 0.89;
			m_colors[i][2] = 0.89;
			m_colors[i][3] = 1.0;			// not selected
        }
    } else {
    	m_colors.resize(mesh->mNumVertices);
    	for(int i = 0; i < mesh->mNumVertices; ++i) {
			// light grey per default
			m_colors[i][0] = 0.89;
			m_colors[i][1] = 0.89;
			m_colors[i][2] = 0.89;
			m_colors[i][3] = 1.0;			// not selected
    	}
    }


    if(mesh->HasFaces()) {
        //only works if each face has same number of indices (otherwise it's a little "strange")
        m_indices.resize(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);
        for(int i = 0; i < mesh->mNumFaces; ++i) {
            for(int j = 0; j < mesh->mFaces[i].mNumIndices; ++j){
                m_indices[i * mesh->mFaces[i].mNumIndices + j] = mesh->mFaces[i].mIndices[j];
            }
        }
        //check which type to draw
        switch(mesh->mFaces[0].mNumIndices) {
            case 1: m_drawMode = (GL_POINTS);break;
            case 2: m_drawMode = (GL_LINES);break;
            case 3: m_drawMode = (GL_TRIANGLES);break;
            case 4: m_drawMode = (GL_QUADS);break;
            default: m_drawMode = (GL_POLYGON);break;
        }
    }
    return true;
}

bool Mesh::setMaterialandTextures(const aiMaterial* mat, const std::map<std::string, GLuint>& textureIdMap, const std::string path)
{
    int texIndex = 0;
    aiReturn texFound;
    aiString texPath;	// filename

    texFound = mat->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath);

    if(texFound == AI_SUCCESS){
        while (texFound == AI_SUCCESS){
            std::stringstream absPath; absPath<<path<<texPath.C_Str();
            m_textureIDs.push_back(textureIdMap.at(absPath.str()));
            ++texIndex;
            texFound = mat->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath);

        }
    }

    aiColor4D tmpcolor;
    float shininess, strength;
    int wireframe;
    unsigned int max;

    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &tmpcolor)){
        m_material.diffuse[0] = tmpcolor.r;
        m_material.diffuse[1] = tmpcolor.g;
        m_material.diffuse[2] = tmpcolor.b;
        m_material.diffuse[3] = tmpcolor.a;
    }
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &tmpcolor)){
        m_material.ambient[0] = tmpcolor.r;
        m_material.ambient[1] = tmpcolor.g;
        m_material.ambient[2] = tmpcolor.b;
        m_material.ambient[3] = tmpcolor.a;
    }
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &tmpcolor)){
        m_material.specular[0] = tmpcolor.r;
        m_material.specular[1] = tmpcolor.g;
        m_material.specular[2] = tmpcolor.b;
        m_material.specular[3] = tmpcolor.a;
    }
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &tmpcolor)){
        m_material.emission[0] = tmpcolor.r;
        m_material.emission[1] = tmpcolor.g;
        m_material.emission[2] = tmpcolor.b;
        m_material.emission[3] = tmpcolor.a;
    }
    unsigned int ret1,ret2;
    max = 1;
    ret1 = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max);
    max = 1;
    ret2 = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
    if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS)){
        m_material.shininess = shininess;
        m_material.shininess_strength = strength;
    } else {
        m_material.shininess = 0.;
        m_material.specular[0] = 0.;m_material.specular[1] = 0.;m_material.specular[2] = 0.;m_material.specular[3] = 0.;
    }
    if(AI_SUCCESS == aiGetMaterialIntegerArray(mat, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
        m_material.fill_mode = wireframe ? GL_LINE : GL_FILL;
    else
        m_material.fill_mode = GL_FILL;

    return true;
}

// bind voas, upload data; drawing only happens in the draw()
bool Mesh::uploadToGPU()
{
	// error handling
    if(m_vertices.size() == 0) {
    	std::cout << "mesh::uploadToGPU -> no vertices found" << std::endl;
    	return false;
    }

	// glGenBuffers(1, &m_vertexID); // old
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // upload vertices
    glGenBuffers(1, &m_vertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexID);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &(m_vertices[0][0]), GL_STATIC_DRAW);
//    std::cout << "m_vertices.size: " << m_vertices.size() << std::endl;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

//    upload colors
    if(m_colors.size() != 0){
//    	std::cout << "has colors"  << std::endl;
        glGenBuffers(1, &m_colorsID);
        glBindBuffer(GL_ARRAY_BUFFER, m_colorsID);
        glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(glm::vec4), &(m_colors[0][0]), GL_STATIC_DRAW);
        // enable location 1 for VAO
        glEnableVertexAttribArray(1);
        // set metadata in VAO (location 1, 4 floats, not normalized, stride = 0, offset = 0)
    	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    } else {
    	// no colors
    	std::cout << "no colors found!"  << std::endl;
    	glGenBuffers(1, &m_colorsID);
    	glBindBuffer(GL_ARRAY_BUFFER, m_colorsID);

    	glEnableVertexAttribArray(1);
    	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }


//    upload normals
    if(m_normals.size() != 0){
//    	std::cout<<"has normals"<<std::endl;
        glGenBuffers(1, &m_normalID);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalID);
        glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &(m_normals[0][0]), GL_STATIC_DRAW);
	    glEnableVertexAttribArray(2);
	    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

//
//    upload texture coords
//    for(std::vector<std::vector<glm::vec2> >::iterator it = m_tCoords.begin(); it != m_tCoords.end(); ++it){
//        GLuint tmpID;
//        glGenBuffers(1, &tmpID);
//        glBindBuffer(GL_ARRAY_BUFFER, tmpID);
//        glBufferData(GL_ARRAY_BUFFER, it->size() * sizeof(glm::vec2), &((*it)[0][0]), GL_STATIC_DRAW);
//        m_tcoordIDs.push_back(tmpID);
//    }
//
//    upload indices
    if(m_indices.size() != 0){
        glGenBuffers(1, &m_indicesID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &(m_indices[0]), GL_STATIC_DRAW);
    }
//
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    return true;
}

// upload selected set of vertices with highlighting to the GPU
bool Mesh::uploadSelectionToGPU()
{
	// uploadToGPU() kopieren, alles mit vertexID rausnehemn
	// glArrayBuffer mit colorsID deleten (glDeleteBuffers)
	// Dinge müssen ggf nicht neu generiert & mit Daten befüllt, sondern nur hochgeladen werden

		// binding buffer for updating
        glBindBuffer(GL_ARRAY_BUFFER, m_colorsID);
        // wirting updated data to the buffer
        glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(glm::vec4), &(m_colors[0][0]), GL_STATIC_DRAW);
        // enable location 1 for VAO
//        glEnableVertexAttribArray(1);
//        // set metadata in VAO (location 1, 4 floats, not normalized, stride = 0, offset = 0)
//    	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
//    }

    glBindBuffer(GL_ARRAY_BUFFER,0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
//    glDisableVertexAttribArray(0);
//    glDisableVertexAttribArray(1);
//    glDisableVertexAttribArray(2);
    return true;
}

void Mesh::draw()
{
//	mat_from_obj *  m_SceneTrafo * m_ObjectTrafo;
//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_material.diffuse);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_material.ambient);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_material.specular);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_material.emission);
//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_material.shininess * m_material.shininess_strength);
//  glPolygonMode(GL_FRONT_AND_BACK, m_material.fill_mode);

//  glPushMatrix();
//  glMultMatrixf(glm::value_ptr(m_ObjectTrafo));
//	glMultMatrixf(glm::value_ptr(m_SceneTrafo));
//
//    if(m_vertexID){
//        glBindBuffer(GL_ARRAY_BUFFER,m_vertexID);
//        glEnableClientState(GL_VERTEX_ARRAY);//!
//        glVertexPointer(3,GL_FLOAT,0,0);//!
//    }
//    if(m_colorsID){
//    	std::cout << "m_colorsID found!" << std::endl;
//    	std::cout << "colorsID" << std::endl;
//        glBindBuffer(GL_ARRAY_BUFFER, m_colorsID);
//        glEnableClientState(GL_COLOR_ARRAY);//!
//        glColorPointer(4,GL_FLOAT,0,0);//!
//    }
//    if(m_normalID){
//    	std::cout << "normalID" << std::endl;
//        glBindBuffer(GL_ARRAY_BUFFER, m_normalID);
//        glEnableClientState(GL_NORMAL_ARRAY);//!
//        glNormalPointer(GL_FLOAT,0,0);//!
//    }
//
//    std::vector<GLuint>::iterator tcIt =  m_tcoordIDs.begin();
//    std::vector<GLuint>::iterator tIt =  m_textureIDs.begin();
//    unsigned int index = 0;

	glBindVertexArray(m_vao);
    if(m_indicesID){
    	// main looped draw call
        glDrawElements(m_drawMode,m_indices.size(), GL_UNSIGNED_INT, (GLvoid*)0);
    } else {
        glDrawArrays(GL_TRIANGLES,0,m_vertices.size());
    }
	glBindVertexArray(0);
//
//
////unbind everything
//    glDisableClientState(GL_TEXTURE_COORD_ARRAY);//!
//    glDisableClientState(GL_VERTEX_ARRAY);//!
//    glDisableClientState(GL_NORMAL_ARRAY);//!
//    glDisableClientState(GL_COLOR_ARRAY);//!
//    glBindTexture(GL_TEXTURE_2D,0);
//
//    glBindBuffer(GL_ARRAY_BUFFER,0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
//    glPopMatrix();

    GLenum err;
    if(m_debug){
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "################## after draw call: OpenGL error: " << err << std::endl;
        }
    }

}

void Mesh::changeAllColors (float color[4])
{
	for (int i=0; i!=m_colors.size();++i) {
		m_colors[i][0] = color[0];
		m_colors[i][2] = color[1];
		m_colors[i][2] = color[2];
	}
//	for (auto c = m_colors.begin(); c != m_colors.end(); ++c) {
//
////		m_colors[*c][0] = color[0];
////		m_colors[*c][1] = color[1];
////		m_colors[*c][2] = color[2];
//	}
}

// change color of vertices
void Mesh::changeColorsAdd (std::set <size_t> &selectionIndices, float color[4])
{
//	std::cout << "changeColors called; length of selection: " << selectionIndices.size() << std::endl;
	std::set<size_t>::iterator it = selectionIndices.begin();
	for (int n=0; n != selectionIndices.size(); ++n) {

		// check if vertex is already highlighted
		if (m_colors[*it][3] == 0.99) {		// already selected
			break;
		} else {
			m_colors[*it][0] = color[0];
			m_colors[*it][1] = color[1];
			m_colors[*it][2] = color[2];
			m_colors[*it][3] = 0.99;		// selected
			std::advance(it, 1);
		}
	}
}

void Mesh::changeColorsRem (std::set <size_t> &selectionIndices, float color[4])
{
//	std::cout << "changeColors called; length of selection: " << selectionIndices.size() << std::endl;
	// reset all vertices back to grey
	resetColors();

	std::set<size_t>::iterator it = selectionIndices.begin();
	for (int n=0; n != selectionIndices.size(); ++n) {
		m_colors[*it][0] = color[0];
		m_colors[*it][1] = color[1];
		m_colors[*it][2] = color[2];
		m_colors[*it][3] = 0.99;		// selected
		std::advance(it, 1);
	}
}

// change color of a single vertex
void Mesh::changeColor (size_t index, float color[4])
{
	m_colors[index][0] = color[0];
	m_colors[index][1] = color[1];
	m_colors[index][2] = color[2];
	m_colors[index][3] = color[3];
}

void Mesh::resetColors (void)
{
	for(int i = 0; i < m_colors.size(); ++i) {
		m_colors[i][0] = 0.89;
		m_colors[i][1] = 0.89;
		m_colors[i][2] = 0.89;
		m_colors[i][3] = 1.0;
	}
}
