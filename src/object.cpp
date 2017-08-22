#include "object.h"


Object::Object(bool debug) : m_debug(false)
{
}

//Object::Object(int window_x, int window_y) : m_debug(false)
//{
//	old_window_x = window_x;
//	old_window_y = window_y;
//}

Object::Object(const std::string &filename, bool useDefaults)
{
	if (useDefaults) {
		this->loadObject(filename,
			aiProcess_CalcTangentSpace       |
			aiProcess_Triangulate            |
			aiProcess_JoinIdenticalVertices  |
			aiProcess_SortByPType);
	} else {
		this->loadObject(filename);
	}
}

Object::~Object()
{
    for(std::vector<Mesh*>::iterator it = m_Meshes.begin(); it != m_Meshes.end(); ++it){
        delete (*it);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    for(std::map<std::string, GLuint>::iterator it = m_textureIdMap.begin(); it != m_textureIdMap.end(); ++it){
        glDeleteTextures(1,&(it->second));
    }
}


bool Object::saysomethingobjective(void) {
	std::cout << "Object.cpp says hello" << std::endl;
	return true;
}

std::vector<Mesh*> Object::getMeshes(void) {
	return m_Meshes;
}

void Object::reshape(int w, int h) {

}

// basic getters
int Object::getWindowX() {
	return old_window_x;
}
int Object::getWindowY() {
	return old_window_y;
}


FIBITMAP* Object::readTexture(const std::string &filename)
{

    // Determine the format of the image.
    // Note: The second paramter ('size') is currently unused, and we should use 0 for it.
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str() , 0);

    // Image not found? Abort! Without this section we get a 0 by 0 image with 0 bits-per-pixel but we don't abort, which
    // you might find preferable to dumping the user back to the desktop.
    if (format == -1)
    {
        std::cerr << "Could not find image: " << filename << " - Aborting." << std::endl;
        return 0;
    }

    // Found image, but couldn't determine the file format? Try again...
    if (format == FIF_UNKNOWN)
    {
        std::cerr << "Couldn't determine file format - attempting to get from file extension..." << std::endl;

        format = FreeImage_GetFIFFromFilename(filename.c_str());

        // Check that the plugin has reading capabilities for this format (if it's FIF_UNKNOWN,
        // for example, then it won't have) - if we can't read the file, then we bail out =(
        if ( !FreeImage_FIFSupportsReading(format) )
        {
            std::cerr << "Detected image format cannot be read!" << std::endl;
            return 0;
            //             exit(-1);
        }
    }

    // If we're here we have a known image format, so load the image into a bitap
    FIBITMAP* bitmap = FreeImage_Load(format, filename.c_str());

    // How many bits-per-pixel is the source image?
    int bitsPerPixel =  FreeImage_GetBPP(bitmap);
    FIBITMAP* bitmap32;
    if (bitsPerPixel == 32)
    {
        if(m_debug)
            std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Skipping conversion." << std::endl;
        bitmap32 = bitmap;
    }
    else
    {
        if(m_debug)
            std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Converting to 32-bit colour." << std::endl;
        bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
        FreeImage_Unload(bitmap);
    }

    return bitmap32;



}

GLuint Object::uploadTexture(FIBITMAP *tex)
{

    int imageWidth  = FreeImage_GetWidth(tex);
    int imageHeight = FreeImage_GetHeight(tex);


    GLuint texId;
    glGenTextures(1,&texId);
    glBindTexture(GL_TEXTURE_2D,texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,imageWidth,imageHeight,0,GL_BGRA,GL_UNSIGNED_BYTE,tex->data);
    glBindTexture(GL_TEXTURE_2D, 0);
    FreeImage_Unload(tex);
    return texId;

}

bool Object::loadObject(const std::string &filename, unsigned int pFlags)
{
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT,65535);
    const aiScene* scene  = importer.ReadFile(filename, pFlags );

        if(scene){
            //get paths to all textures and save them in a map
            for (unsigned int m=0; m<scene->mNumMaterials; m++)
            {
                int texIndex = 0;
                aiString path;	// filename
                aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);

                std::stringstream absolutePath;
                absolutePath<<filename.substr(0, filename.find_last_of("/")+1)<<path.C_Str();
                while (texFound == AI_SUCCESS)
                {
                    m_textureIdMap[absolutePath.str()] = 0; //fill map with textures, pointers still NULL yet
                    texIndex++;
                    texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
                }
            }

            //read all textures from disk and upload them to gpu memory, save id of buffer in our map
            std::map<std::string, GLuint>::iterator itr = m_textureIdMap.begin();
            for(; itr != m_textureIdMap.end(); ++itr){
                if(itr->first.size() > 0){
                    FIBITMAP* tex = this->readTexture(itr->first);
                    if(tex)
                        itr->second = this->uploadTexture(tex);
                }
            }

            bool retVal = true;

            // load all meshes
            for(int numMesh = 0; numMesh < scene->mNumMeshes; ++numMesh){
                Mesh* mesh = new Mesh;
                retVal &= mesh->loadMesh(scene->mMeshes[numMesh]);
                aiMaterial* mat = scene->mMaterials[scene->mMeshes[numMesh]->mMaterialIndex];
                //give path<->id map and material of this mesh to the mesh
                mesh->setMaterialandTextures(mat,m_textureIdMap,filename.substr(0, filename.find_last_of("/")+1));
                m_Meshes.push_back(mesh);
            } // end for(int numMesh = 0; numMesh < scene->mNumMeshes; ++numMesh)

            //get object transformation matirx
            for(int c = 0; c <4; ++c){
                for(int r = 0; r<4; ++r){
                    //coloumn major                    row major
                    m_ObjectTrafo[c][r] = scene->mRootNode->mTransformation[r][c];
                }
            }
            if(!retVal)
                std::cerr<<"Could not load all meshes of this file"<<std::endl;
            return retVal;
        } else {//end if(scene)
            std::cerr<<"Could not load file "<<filename<<std::endl;
            return false;
        }
}

// draw function
// calls Mesh::draw() for each Mesh belonging to this object
void Object::drawWithoutParams()
{
    for(std::vector<Mesh*>::iterator it = m_Meshes.begin(); it != m_Meshes.end(); ++it) {
        (*it)->draw();
    }
}

// for each mesh belonging to the object, calls its uploadToGPU() function
void Object::uploadAllMeshes()
{
    for(std::vector<Mesh*>::iterator it = m_Meshes.begin(); it != m_Meshes.end(); ++it){
    	if(!(*it)->uploadToGPU()) {
        	std::cout << "couldn't upload meshes to GPU (Object.uploadAllMeshes())" << std::endl;
        }
    }
}

void Object::updateAllMeshes() {
    for(std::vector<Mesh*>::iterator it = m_Meshes.begin(); it != m_Meshes.end(); ++it){
    	if(!(*it)->uploadSelectionToGPU()) {
        	std::cout << "couldn't upload meshes to GPU (Object.uploadAllMeshes())" << std::endl;
        }
    }
}
