#include <ocTree.h>

/**
 * root constructor
 */
ocTree::ocTree(std::vector<Mesh*> &meshesPointer, int maximumVertsPerLeaf, int maximumSplitDepth, bool debugInfo)
{
	m_isLeaf = true;
	m_myMeshes = meshesPointer;
	m_maxVerticesPerNode = maximumVertsPerLeaf;
	m_maxSplitDepth = maximumSplitDepth;
	m_level = 0;
	m_root = this;
	m_debugInfo = debugInfo;

	if (m_debugInfo) {
		std::cout << "root constructor called" << std::endl;
	}

	// initialize identifier
	std::vector<bool> id(m_maxSplitDepth*3);

	for (int i = 0; i <= id.size(); i++) {
		id[i] = false;
	}

	m_identifier = id;

	// calculate and set dimensions
	getRootDimensions();
}

/**
 * subTree constructor
 */
ocTree::ocTree(
		std::vector <std::pair <size_t, glm::vec3> > vertices,
		int parentLevel,
		int vertsMax,
		int splitsMax,
		float parentDimensions[],
		ocTree* root,
		std::vector<bool> &parId,
		std::vector<bool> &splitDirections,
		bool debugInfo)
{
	m_isLeaf = true;
	setDimensions(parentDimensions, splitDirections);
	m_root = root;
	m_maxVerticesPerNode = vertsMax;
	m_maxSplitDepth = splitsMax;
	m_level = parentLevel+1;
	m_parentIdentifier = parId;
	m_debugInfo = debugInfo;

	// initialize identifier
	int identifierSize = m_parentIdentifier.size();
	int levelOffset = m_level*3-3;
	std::vector<bool> id(identifierSize);

	if (m_debugInfo) {
		std::cout << "level: " << m_level << ", identifierBuilt: ";
	}
	for (int i = 0; i < levelOffset; i++) {
		id[i] = parId[i];
		if (m_debugInfo) {
			if (id[i] == true) {
				std::cout << "1";
			} else {
				std::cout << "0";
			}
		}
	}

	// set the latest values of the identifier, the given splitDirections
	id[levelOffset] = splitDirections[0];
	if (m_debugInfo) {
		if (id[levelOffset] == true) {
			std::cout << "1";
		} else {
			std::cout << "0";
		}
	}
	id[levelOffset+1] = splitDirections[1];
	if (m_debugInfo) {
		if (id[levelOffset+1] == true) {
			std::cout << "1";
		} else {
			std::cout << "0";
		}
	}
	id[levelOffset+2] = splitDirections[2];
	if (m_debugInfo) {
		if (id[levelOffset+2] == true) {
			std::cout << "1";
		} else {
			std::cout << "0";
		}
	}

	// set remaining values of identifier after identiferOffset+3
	for (int j = levelOffset+3; j < identifierSize; j++) {
		id[j] = false;
		if (m_debugInfo) {
			if (id[j] == false) {
				std::cout << "0";
			}
			if (id[j] == true) {
				std::cout << "1";
			}
		}
	}
	if (m_debugInfo) {
		std::cout << std::endl;
	}
	m_identifier = id;


	/**
	 * converting the vector to a list for convenience
	 */
	std::list<std::pair<size_t, glm::vec3> > verticesList;
	std::vector <std::pair <size_t, glm::vec3> > verticesVector = vertices;

	// next line is very important
	buildTreeRecursively(verticesVector);
}

// destructor
ocTree::~ocTree() {
	//  @FIXME do something
}

// getter for boolean attribute isLeaf
bool ocTree::getisLeaf() {
	return m_isLeaf;
}

/**
 * get Root dimensions based on all vertices in the ocTree
 */
void ocTree::getRootDimensions(void)
{
	m_minX = 0.;
	m_maxX = 0.;
	m_minY = 0.;
	m_maxY = 0.;
	m_minZ = 0.;
	m_maxZ = 0.;
	m_meanX = 0.;
	m_meanY = 0.;
	m_meanZ = 0.;

	int b = 1;
	for (auto t = m_myMeshes.begin(); t != m_myMeshes.end(); t++) {
		std::cout <<"MESH " << b << std::endl;
		b++;
	}

	int c = 1;
//	std::vector<glm::vec3> vertices = getVerticesOfMesh(0);
	std::vector<glm::vec3> vertices = getVertices();
		for (std::vector<glm::vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
			if ((*it).x < m_minX) {
				m_minX = (*it).x;
			}
			if ((*it).x > m_maxX) {
				m_maxX = (*it).x;
			}
			if ((*it).y < m_minY) {
				m_minY =(*it).y;
			}
			if ((*it).y > m_maxY) {
				m_maxY = (*it).y;
			}
			if ((*it).z < m_minZ) {
				m_minZ =(*it).z;
			}
			if ((*it).z > m_maxZ) {
				m_maxZ = (*it).z;
			}
			c++;
		}

		std::cout <<"vertex count: "<<c<<std::endl;

		m_minX = m_minX - 0.0001;
		m_maxX = m_maxX + 0.0001;
		m_minY = m_minY - 0.0001;
		m_maxY = m_maxY + 0.0001;
		m_minZ = m_minZ - 0.0001;
		m_maxZ = m_maxZ + 0.0001;

		m_meanX = (float)(m_minX + m_maxX)/2;
		m_meanY = (float)(m_minY + m_maxY)/2;
		m_meanZ = (float)(m_minZ + m_maxZ)/2;
}

/**
 * setDimensions function for setting min, max and mean-values for x, y and z direction
 *
 * parDimensions: {[0]minX, [1]maxX, [2]meanX, [3]minY, [4]maxY, [5]meanY, [6]minZ, [7]maxZ, [8]meanZ};
 */
void ocTree::setDimensions(float parentDimensions[], std::vector<bool> &splitDirections)
{
	// splitDirectios::at(0)
	if (splitDirections.at(0) == false) {
		m_minZ = parentDimensions[8];	// minZ = p.meanZ
		m_maxZ = parentDimensions[7];	// maxZ = p.maxZ
	} else {
		m_minZ = parentDimensions[6];	// minZ = p.minZ
		m_maxZ = parentDimensions[8];	// maxZ = p.meanZ
	}

	// splitDirectios::at(1)
	if (splitDirections.at(1) == false) {
		m_minY = parentDimensions[5];	// minY = p.meanY
		m_maxY = parentDimensions[4];	// maxY = p.maxY
	} else {
		m_minY = parentDimensions[3];	// minY = p.minY
		m_maxY = parentDimensions[5];	// maxY = p.meanY
	}

	// splitDirectios::at(2)
	if (splitDirections.at(2) == false) {
		m_minX = parentDimensions[0];	// minX = p.minX
		m_maxX = parentDimensions[2];	// maxX = p.meanX
	} else {
		m_minX = parentDimensions[2];	// minX = p.meanX
		m_maxX = parentDimensions[1];	// maxX = p.maxX
	}

	// mean values:
	m_meanZ = (m_minZ+m_maxZ)/2;
	m_meanY = (m_minY+m_maxY)/2;
	m_meanX = (m_minX+m_maxX)/2;
}

void ocTree:: buildTreeRecursively(std::vector <std::pair <size_t, glm::vec3> > & subsetVerts)
{
	/**
	 * ###########################################################
	 * ######## BEGIN FOR-LOOP THROUGH ALL GIVEN VERTICES ########
	 * ###########################################################
	 */

	// for(std::vector<std::pair <size_t, glm::vec3> >::iterator it = subsetVerts.begin(); it != subsetVerts.end(); ++it) {
	for(int t = 0; t != subsetVerts.size(); ++t) {
		std::pair<size_t, glm::vec3> *it;
		it = &subsetVerts[t];
		// add every vertex to the root node
		if (m_level == 0) {
			m_verticesInBounds.push_back((*it));
		} else {
			/**
			 * if the current node is on a level greater than 0, vertices that share x,y or z-values with
			 * the maximum values of the current node need to be treated differently
			 *
			 * whether a vertex with an 'edgecase' value is assigned to a node or not is determined by the
			 * split directions, which can be retrieved via looking at the last, 2nd-to-last and 3rd-to-last
			 * values of a node's identifier.
			 */

			bool determineZ = m_identifier[m_level];
			bool determineY = m_identifier[m_level+1];
			bool determineX = m_identifier[m_level+2];
			bool eraseVertex = false;

			if (determineZ == false) {									// 3rd-to-last	= false
				if (determineY == false) {									// 2nd-to-last	= false
					if (determineX == false) {									// identifier = 000
						if (it->second.x >= m_minX && it->second.x < m_maxX) {
							if (it->second.y >= m_minY && it->second.y < m_maxY) {
								if (it->second.z >= m_minZ && it->second.z < m_maxZ) {
									m_verticesInBounds.push_back(*it);			// add to vertices of this node
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					} else {													// identifier = 001
						if (it->second.x >= m_minX && it->second.x <= m_maxX) {
							if (it->second.y >= m_minY && it->second.y <= m_maxY) {
								if (it->second.z >= m_minZ && it->second.z < m_maxZ) {
							 		m_verticesInBounds.push_back(*it);
							 		if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
							 			eraseVertex = true;
									}
								}
							}
						}
					}
				} else {
					if (determineX == false) {									// identifier = 010
						if (it->second.x >= m_minX && it->second.x < m_maxX) {
							if (it->second.y >= m_minY && it->second.y <= m_maxY) {
								if (it->second.z >= m_minZ && it->second.z <= m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					} else {													// identifier = 011
						if (it->second.x >= m_minX && it->second.x <= m_maxX) {
							if (it->second.y >= m_minY && it->second.y <= m_maxY) {
								if (it->second.z >= m_minZ && it->second.z <= m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					}
				}
			} else {													// 3rd-to-last = true
				if (determineY == false) {									// 2nd-to-last = false
					if (determineX == false) {									// identifier = 100
						if (it->second.x >= m_minX && it->second.x < m_maxX) {
							if (it->second.y >= m_minY && it->second.y < m_maxY) {
								if (it->second.z >= m_minZ && it->second.z < m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					} else {													// identifier = 101
						if (it->second.x >= m_minX && it->second.x <= m_maxX) {
							if (it->second.y >= m_minY && it->second.y < m_maxY) {
								if (it->second.z >= m_minZ && it->second.z < m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					}
				} else {
					if (determineX == false) {									// identifier = 110
						if (it->second.x >= m_minX && it->second.x < m_maxX) {
							if (it->second.y >= m_minY && it->second.y < m_maxY) {
								if (it->second.z >= m_minZ && it->second.z <= m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}
								}
							}
						}
					} else {													// identifier = 111
						if (it->second.x >= m_minX && it->second.x <= m_maxX) {
							if (it->second.y >= m_minY && it->second.y < m_maxY) {
								if (it->second.z >= m_minZ && it->second.z <= m_maxZ) {
									m_verticesInBounds.push_back(*it);
									if (it->second.x==m_minX||it->second.x==m_maxX||it->second.y==m_minY||it->second.y==m_maxY||it->second.z==m_minZ||it->second.z==m_maxZ){
										eraseVertex = true;
									}

								}
							}
						}
					}
				}
			}
			if(eraseVertex) {
				// it = subsetVerts.erase(it);
			} else {
				++it;
			}
		}
	}

	// std::cout << "subVerts size AFTER: " << subsetVerts.size() << std::endl;
	if (m_debugInfo) {
		std::cout << "I contain " << m_verticesInBounds.size() << " vertices." << std::endl;
	}

	/**
	 * #########################################################
	 * ######## END FOR-LOOP THROUGH ALL GIVEN VERTICES ########
	 * #########################################################
	 */
	if (m_verticesInBounds.size() > m_maxVerticesPerNode) {
		if (m_level < m_maxSplitDepth) {
			split();
		} else {
			// @FIXME do something
		}
	}
	std::vector<bool> check = {false,false,true,false,false,false,false,false,false};	// 001000000
}

bool ocTree::split() {
	if (m_debugInfo) {
		std::cout << "split() called at level " << m_level << std::endl;
	}
	float parDimensions[9] = {m_minX, m_maxX, m_meanX, m_minY, m_maxY, m_meanY, m_minZ, m_maxZ, m_meanZ};
	std::vector<bool> split0 = {false,	false,	false};		// 000
	std::vector<bool> split1 = {false,	false,	true};		// 001
	std::vector<bool> split2 = {false,	true,	false};		// 010
	std::vector<bool> split3 = {false,	true,	true};		// 011
	std::vector<bool> split4 = {true,	false,	false};		// 100
	std::vector<bool> split5 = {true,	false,	true};		// 101
	std::vector<bool> split6 = {true,	true,	false};		// 110
	std::vector<bool> split7 = {true,	true,	true};		// 111

	ocTree* chidlLeaf0 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split0, m_debugInfo);
	ocTree* chidlLeaf1 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split1, m_debugInfo);
	ocTree* chidlLeaf2 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split2, m_debugInfo);
	ocTree* chidlLeaf3 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split3, m_debugInfo);
	ocTree* chidlLeaf4 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split4, m_debugInfo);
	ocTree* chidlLeaf5 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split5, m_debugInfo);
	ocTree* chidlLeaf6 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split6, m_debugInfo);
	ocTree* chidlLeaf7 = new ocTree(m_verticesInBounds, m_level, m_maxVerticesPerNode, m_maxSplitDepth, parDimensions, m_root, m_identifier, split7, m_debugInfo);

	m_myChildren[0] = chidlLeaf0;
	m_myChildren[1] = chidlLeaf1;
	m_myChildren[2] = chidlLeaf2;
	m_myChildren[3] = chidlLeaf3;
	m_myChildren[4] = chidlLeaf4;
	m_myChildren[5] = chidlLeaf5;
	m_myChildren[6] = chidlLeaf6;
	m_myChildren[7] = chidlLeaf7;

	m_isLeaf = false;

//	~ocTree();
	return true;
}

float ocTree::getMaximumSearchRadius(void) {
	float minDif = fabs(m_maxX - m_minX);
	if (fabs(m_maxY - m_minY < minDif)) {
		minDif = fabs(m_maxY - m_minY);
	}
	if (fabs(m_maxZ - m_minZ < minDif)) {
		fabs(minDif = m_maxZ - m_minZ);
	}

	return minDif/pow(2, m_maxSplitDepth)*0.95;
}

std::vector<Mesh*> ocTree::getMyMeshes (void)
{
	return m_myMeshes;
}

std::vector<Mesh*>::iterator ocTree::getMesh (int n)
{
	int count = 0;
	for(std::vector<Mesh*>::iterator it = m_myMeshes.begin(); it != m_myMeshes.end(); ++it) {
		if (count == n) {
			return(it);
		}
	}
}

std::vector <glm::vec3> ocTree::getVertices(void)
{
	std::vector<glm::vec3> result;
	for(std::vector<Mesh*>::iterator it = m_myMeshes.begin(); it != m_myMeshes.end(); ++it) {
		std::vector<glm::vec3> itVerts = (*it)->get_m_vertices(); 		// get vertices of current Mesh
		result.insert(result.end(), itVerts.begin(), itVerts.end());	// insert vertices into result vector
	}
	return result;
}

std::vector<glm::vec3> ocTree::getVerticesOfMesh (int n)
{
	int count = 0;
	for(std::vector<Mesh*>::iterator it = m_myMeshes.begin(); it != m_myMeshes.end(); ++it) {
		if (count == n) {
			std::vector<glm::vec3> result = (*it)->get_m_vertices();
			return result;;
		}
	}
}

std::vector<std::pair<size_t, glm::vec3> > ocTree::getVerticesInBounds (void)
{
	return m_verticesInBounds;
}

int ocTree::getLevel(void) {
	return m_level;
}

void ocTree::saySomething(void)
{
	std::cout << "hello, I am a node in an ocTree" << std::endl;
}

// @FIXME: keep this up to date!
void ocTree::debugTreeInfo(void)
{
		std::cout << "=================[ocTree INFO]=================" << std::endl;
		std::cout << "total number of vertices in this ocTree:" << (*getMesh(0))->getNumVertices() << std::endl;
		std::cout << "maximum number of vertices per leaf node: " << m_maxVerticesPerNode << std::endl;
		std::cout << "maximum split depth: " << m_maxSplitDepth << std::endl;
		std::cout << "dimension x: " << m_minX << " to " << m_maxX << ", mean value: " << m_meanX << std::endl;
		std::cout << "dimension y: " << m_minY << " to " << m_maxY << ", mean value: " << m_meanY << std::endl;
		std::cout << "dimension z: " << m_minZ << " to " << m_maxZ << ", mean value: " << m_meanZ << std::endl;
		std::cout << "total number of nodes: [@FIXME]" << std::endl;
		std::cout << "total number of leafs: [@FIXME]" << std::endl;
		std::cout << "average number of vertices / leaf: [@FIXME]" << std::endl;
		std::cout << "highest number of vertices in one leaf: [@FIXME]" << std::endl;
		std::cout << "lowest number of vertices in one leaf: [@FIXME]" << std::endl;
		std::cout << "===============[ocTree INFO END]===============" << std::endl;
}

void ocTree::debugFirstVertex(void)
{
	std::vector<Mesh*>::iterator firstMesh = getMesh(0);
	std::vector<glm::vec3> vertices = (*firstMesh)->get_m_vertices();
	std::cout << "First vertex(x,y,z): " << vertices[0].x <<", "<< vertices[0].y <<", "<< vertices[0].z <<", "<< std::endl;
}

void ocTree::debugAllVertices(void)
{
	for(std::vector<Mesh*>::iterator it = m_myMeshes.begin(); it != m_myMeshes.end(); ++it) {
		std::vector<glm::vec3> vertices = (*it)->get_m_vertices();
		for (int i=0; i <= (*it)->getNumVertices(); i++) {
			std::cout << "x, y, z of vertex #" << i <<": "
			<< vertices[i].x <<", "
			<< vertices[i].y <<", "
			<< vertices[i].z
			<< std::endl;
		}
	}
}

void ocTree::debugIdentifierasString(void)
{
	if (m_level == 0) {
		std::cout << "I am root." << std::endl;
	} else {
		std::cout << "my level: " << m_level << std::endl;
		std::cout << "parentIdentifier: ";
		for (int i = 0; i != m_parentIdentifier.size(); i++) {
			if (m_parentIdentifier[i] == false) {
				std::cout << "0";
			} else {
				std::cout << "1";
			}
		}
		std::cout << std::endl;
		std::cout << "my identifier: ";
		for (int i = 0; i != m_identifier.size(); i++) {
			if (m_identifier[i] == false) {
				std::cout << "0";
			} else {
				std::cout << "1";
			}
		}
		std::cout << std::endl;
	}
}

ocTree* ocTree::getNodeByIdentifierArray(std::vector<bool> compare, bool debugInfo)
{
	ocTree *result;
	if (this->getisLeaf() == true) {
		if (debugInfo == true) {
			std::cout << "node found via getNodeByIdentifierArray()" << std::endl;
			debugIdentifierasString();
		}
		 result = this;
		return result;
	}

	int levelOffset = m_level*3;
	// pass query down to the correct child node
	if (compare[levelOffset] == false) {
		if (compare[levelOffset+1] == false) {
			if (compare[levelOffset+2] == false) {
				result = m_myChildren[0]->getNodeByIdentifierArray(compare, debugInfo);	// 000
			} else {
				result = m_myChildren[1]->getNodeByIdentifierArray(compare, debugInfo);	// 001
			}
		} else {
			if (compare[levelOffset+2] == false) {
				result = m_myChildren[2]->getNodeByIdentifierArray(compare, debugInfo);	// 010
			} else {
				result = m_myChildren[3]->getNodeByIdentifierArray(compare, debugInfo);	// 011
			}
		}
	} else {
		if (compare[levelOffset+1] == false) {
			if (compare[levelOffset+2] == false) {
				result = m_myChildren[4]->getNodeByIdentifierArray(compare, debugInfo);	// 100
			} else {
				result = m_myChildren[5]->getNodeByIdentifierArray(compare, debugInfo);	// 101
			}
		} else {
			if (compare[levelOffset+2] == false) {
				result = m_myChildren[6]->getNodeByIdentifierArray(compare, debugInfo);	// 110
			} else {
				result = m_myChildren[7]->getNodeByIdentifierArray(compare, debugInfo);	// 111
			}
		}
	}
	return result;
}

ocTree* ocTree::addVerticesToSelectionByCoordinates(glm::vec3 target, float radius, std::set<size_t> &intermediateSelection, bool debugInfo)
{
//	std::cout << "Coordinates passed (x,y,z): " << target.x << ", " << target.y <<", " << target.z << std::endl;
	if (debugInfo) {
		std::cout << "addVerticesToSelectionByCoordinates() called" << std::endl;
		std::cout << "Coordinates passed: x, y & z: " << target.x << ", " << target.y <<", " << target.z << std::endl;
	}
	// given coordinate is out of bounds; return NULL
	ocTree *result;
	// check if given coordinates are out of bounds. if so @FIXME do something
	if (target.x < m_minX || target.x > m_maxX || target.y < m_minY || target.y > m_maxY || target.z < m_minZ || target.z > m_maxZ) {
//		std::cout << "everything is out of bounds" << std::endl;
//		return NULL;
	}

	// right node found; end recursion; return current node
	if (this->getisLeaf() == true) {
		// debug additional information if wanted
		if (debugInfo == true) {
			std::cout << "node found via getNodeByCoordinates()" << std::endl;
			if (radius == 0) {
				if (debugInfo) {
					std::cout <<"I am also most likely a neighbor" << std::endl;
				}
			}
			if (debugInfo) {
			debugIdentifierasString();
				std::cout << "minX: " << m_minX << ", meanX: " << m_meanX << ",maxX: " << m_maxX << std::endl;
				std::cout << "minY: " << m_minY << ", meanY: " << m_meanY << ",maxY: " << m_maxY << std::endl;
				std::cout << "minZ: " << m_minZ << ", meanZ: " << m_meanZ << ",maxZ: " << m_maxZ << std::endl;
				std::cout << "I contain " << m_verticesInBounds.size() << " vertices" << std::endl;
			}
		}
		result = this;

		std::vector<std::pair<size_t, glm::vec3> > verticesToCheck;

		for(size_t t = 0; t < result->m_verticesInBounds.size();++t){
			verticesToCheck.push_back(result->m_verticesInBounds[t]);
		}
		if (debugInfo) {
			std::cout << "verticesToCheck size (without neighbors): " << verticesToCheck.size() << std::endl;
		}

		/**
		 * if this is the original request, check if radius extends over the boundaries of this node
		 * do not check if radius == 0 (aka if this is a neighbor)
		 */
		if (radius != 0) {
			glm::vec3 neighborCo;
			// crossing minX
			if (target.x - radius < m_minX) {
				if (debugInfo) {
					std::cout << "crossing minX" << std::endl;
				}
				neighborCo = {target.x-radius, target.y, target.z};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );

				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			// crossing maxX
			if (target.x + radius > m_maxX) {
				if (debugInfo) {
					std::cout << "crossing maxX" << std::endl;
				}
				neighborCo = {target.x+radius,target.y,target.z};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			// crossing minY
			if (target.y - radius < m_minY) {
				if (debugInfo) {
					std::cout << "crossing minY" << std::endl;
				}
				neighborCo = {target.x,target.y-radius,target.z};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			// crossing maxY
			if (target.y + radius > m_maxY) {
				if (debugInfo) {
					std::cout << "crossing maxY" << std::endl;
				}
				neighborCo = {target.x,target.y+radius,target.z};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			// crossing minZ
			if (target.z - radius < m_minZ) {
				if (debugInfo) {
					std::cout << "crossing minZ" << std::endl;
				}
				neighborCo = {target.x,target.y,target.z-radius};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			// crossing maxZ
			if (target.z + radius > m_maxZ) {
				if (debugInfo) {
					std::cout << "crossing maxZ" << std::endl;
				}
				neighborCo = {target.x,target.y,target.z+radius};
				ocTree * neighbor = m_root->addVerticesToSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}
		}

		/**
		 * finding vertices in verticesToCheck that are within the given radius of search coordinates (x,y,z)
		 * considers all vertices within the target node as well as every one of its neighboring nodes that contain (x,y,z)+|radius|
		 * vertices are added to the selection if they are within (x,y,z)+|radius|
		 */
		std::vector<std::pair<size_t, glm::vec3>>::const_iterator it;
		for (it = verticesToCheck.begin(); it != verticesToCheck.end(); ++it) {
			// checking whether any of the vertices in verticesToCheck are within the target coordinates + radius
			if (it->second.x < target.x+radius && it->second.x > target.x-radius && it->second.y < target.y+radius && it->second.y > target.y-radius && it->second.z < target.z+radius && it->second.z > target.z-radius) {
				intermediateSelection.insert(it->first);
				if (debugInfo) {
					std::cout << "vertex "<< it->first << " added; x,y,z: " << it->second.x << ", " << it->second.y << ", " << it->second.z << ", "<< std::endl;
				}
			}
		}
		if(debugInfo) {
			std::cout << "addCo coordinate passed: "<<target[0] <<", "<<target[1]<<", "<<target[2] << "; number of vertices selected: " << intermediateSelection.size() << std::endl;
		}
		return result;
	}

	// redirecting the query to correct child node
	if (target.x < m_meanX) {
		if (target.y < m_meanY) {
			if (target.z < m_meanZ) {
				result = m_myChildren[6]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 110
			} else {
				result = m_myChildren[2]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 010
			}
		} else {
			if (target.z < m_meanZ) {
				result = m_myChildren[4]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 100
			} else {
				result = m_myChildren[0]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 000
			}
		}
	} else {
		if (target.y < m_meanY) {
			if (target.z < m_meanZ) {
				result = m_myChildren[7]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 111
			} else {
				result = m_myChildren[3]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection,debugInfo);	// 011
			}
		} else {
			if (target.z < m_meanZ) {
				result = m_myChildren[5]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 101
			} else {
				result = m_myChildren[1]->addVerticesToSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 001
			}
		}
	}
	return result;
}

ocTree* ocTree::removeVerticesFromSelectionByCoordinates(glm::vec3 target, float radius, std::set<size_t> &intermediateSelection, bool debugInfo)
{
	if (debugInfo) {
		std::cout << "removeVerticesFromSelectionByCoordinates() called" << std::endl;
		std::cout << "Coordinates passed: x, y & z: " << target.x << ", " << target.y <<", " << target.z << std::endl;
	}
	ocTree *result;
	if (target.x < m_minX || target.x > m_maxX || target.y < m_minY || target.y > m_maxY || target.z < m_minZ || target.z > m_maxZ) {
//		std::cout << "everything is out of bounds" << std::endl;
//		return NULL;
	}


	if (this->getisLeaf() == true) {
		if (debugInfo == true) {
			std::cout << "node found via getNodeByCoordinates()" << std::endl;
			if (radius == 0) {
				if (debugInfo) {
					std::cout <<"I am also most likely a neighbor" << std::endl;
				}
			}
			if (debugInfo) {
			debugIdentifierasString();
				std::cout << "minX: " << m_minX << ", meanX: " << m_meanX << ",maxX: " << m_maxX << std::endl;
				std::cout << "minY: " << m_minY << ", meanY: " << m_meanY << ",maxY: " << m_maxY << std::endl;
				std::cout << "minZ: " << m_minZ << ", meanZ: " << m_meanZ << ",maxZ: " << m_maxZ << std::endl;
				std::cout << "I contain " << m_verticesInBounds.size() << " vertices" << std::endl;
			}
		}
		result = this;

		std::vector<std::pair<size_t, glm::vec3> > verticesToCheck;

		for(size_t t = 0; t < result->m_verticesInBounds.size();++t){
			verticesToCheck.push_back(result->m_verticesInBounds[t]);
		}
		if (debugInfo) {
			std::cout << "verticesToCheck size (without neighbors): " << verticesToCheck.size() << std::endl;
		}

		if (radius != 0) {
			glm::vec3 neighborCo;
			if (target.x - radius < m_minX) {
				if (debugInfo) {
					std::cout << "crossing minX" << std::endl;
				}
				neighborCo = {target.x-radius,target.y,target.z};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );

				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			if (target.x + radius > m_maxX) {
				if (debugInfo) {
					std::cout << "crossing maxX" << std::endl;
				}
				neighborCo = {target.x+radius,target.y,target.z};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			if (target.y - radius < m_minY) {
				if (debugInfo) {
					std::cout << "crossing minY" << std::endl;
				}
				neighborCo = {target.x,target.y-radius,target.z};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			if (target.y + radius > m_maxY) {
				if (debugInfo) {
					std::cout << "crossing maxY" << std::endl;
				}
				glm::vec3 neighborCo = {target.x,target.y+radius,target.z};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			if (target.z - radius < m_minZ) {
				if (debugInfo) {
					std::cout << "crossing minZ" << std::endl;
				}
				glm::vec3 neighborCo = {target.x,target.y,target.z-radius};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}

			if (target.z + radius > m_maxZ) {
				if (debugInfo) {
					std::cout << "crossing maxZ" << std::endl;
				}
				glm::vec3 neighborCo = {target.x,target.y,target.z+radius};
				ocTree * neighbor = m_root->removeVerticesFromSelectionByCoordinates(neighborCo,0,intermediateSelection,debugInfo);
				verticesToCheck.insert(verticesToCheck.end(), neighbor->m_verticesInBounds.begin(), neighbor->m_verticesInBounds.end() );
				if (debugInfo) {
					std::cout << "neighbor LEVEL: " << neighbor->m_level << std::endl;
					std::cout << "verticesToCheck size (with neighbors): " << verticesToCheck.size() << std::endl;
				}
			}
		}

		std::vector<std::pair<size_t, glm::vec3>>::const_iterator it;
		for (it = verticesToCheck.begin(); it != verticesToCheck.end(); ++it) {
			if (it->second.x < target.x+radius && it->second.x > target.x-radius && it->second.y < target.y+radius && it->second.y > target.y-radius && it->second.z < target.z+radius && it->second.z > target.z-radius) {
				// check if vertex is already selected (element of global selection set)
				for (std::set<size_t>::iterator i = intermediateSelection.begin(); i != intermediateSelection.end(); ++i)
				{
				    if ((*i) == it->first) {
				    	float color[4] = {0.89, 0.89, 0.89, 1.0};
				    	if (debugInfo) {
							std::cout << "color array built" << std::endl;
				    	}
				    	m_root->m_myMeshes[0]->changeColor(it->first, color);
						intermediateSelection.erase(it->first);
						if (debugInfo) {
							std::cout << "vertex "<< it->first << " removed; x,y,z: " << it->second.x << ", " << it->second.y << ", " << it->second.z << ", "<< std::endl;
						}
				    }
				}
			}
		}
		return result;
	}

	if (target.x < m_meanX) {
		if (target.y < m_meanY) {
			if (target.z < m_meanZ) {
				result = m_myChildren[6]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 110
			} else {
				result = m_myChildren[2]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 010
			}
		} else {
			if (target.z < m_meanZ) {
				result = m_myChildren[4]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 100
			} else {
				result = m_myChildren[0]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 000
			}
		}
	} else {
		if (target.y < m_meanY) {
			if (target.z < m_meanZ) {
				result = m_myChildren[7]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 111
			} else {
				result = m_myChildren[3]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection,debugInfo);	// 011
			}
		} else {
			if (target.z < m_meanZ) {
				result = m_myChildren[5]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 101
			} else {
				result = m_myChildren[1]->removeVerticesFromSelectionByCoordinates(target, radius, intermediateSelection, debugInfo);	// 001
			}
		}
	}
	return result;
}
