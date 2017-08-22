#ifndef ocTree_H
#define ocTree_H

#include <glm/glm.hpp>
#include <mesh.h>
#include <set>
#include <string>
#include <vector>


class ocTree{

	private:
		bool m_isLeaf;
		bool m_debugInfo;										// debug additional information if true
		std::vector<Mesh*> m_myMeshes;							// meshes containing vertices

		// vertices within the bounds of this leaf
		std::vector<std::pair<size_t, glm::vec3> > m_verticesInBounds;
		int m_maxVerticesPerNode;								// maximum number of vertices that one oct can hold
		int m_maxSplitDepth;									// maximum split depth
		int m_level;											// current split level
		std::vector<bool> m_identifier;							// boolean representation of this leafs identifier
		std::vector<bool> m_parentIdentifier;					// boolean representation of this leafs parents identifier
		ocTree* m_myChildren [8];							// array of children nodes
		ocTree* m_root;										// pointer to root

		float m_minX, m_maxX, m_meanX;							// limits & mean in x dimension
		float m_minY, m_maxY, m_meanY;							// limits & mean in y dimension
		float m_minZ, m_maxZ, m_meanZ;							// limits & mean in z dimension

		/**
		 * private function to set dimensions of none-root octs based on given split directions
		 * @PARAM float parentDimensions - float array of parents values for min-, max- and mean values in x, y and z- dimension
		 * @PARAM std::vector<bool> splitDirections - split directions given by parent oct
		 *
		 */
		void setDimensions(float parentDimensions[], std::vector<bool> &splitDirections);

		/**
		 * private split function to create 8 new nodes with the calling node as parent
		 * is called when the number of vertices assigned to the current node exceeds the maximum number of vertices per node
		 * in this case, the current ocTree number is deleted and 8 new ones are created
		 *
		 * @RETURN true if splitting was successful, otherwise false
		 */
		bool split();

	public:
		/**
		 * root constructor
		 * to be called from main
		 * @PARAM std::vector<Mesh*> &meshesPointer - vector of pointers to Mesh objects
		 * @PARAM int maximumVertsPerLeaf - maximum number of vertices a Leaf can hold
		 * @PARAM int maximumSplitDepth - maximum number of splits
		 * @PARAM bool debugInfo - if true, print additional information to the console during construction
		 */
		ocTree(
			std::vector<Mesh*> &meshesPointer,
			int maximumVertsPerLeaf,
			int maximumSplitDepth,
			bool debugInfo
		);


		/**
		 * subTree constructor
		 * to be called from parent ocTree object (nodes)
		 *
		 * @PARAM std::vector<std::pair<size_t, glm::vec3>> vertices - vector of indices and vertices (float x,y,z- coordinates)
		 * @PARAM int parentLevel - split-depth (level) of parent-node
		 * @PARAM int maximumVertsPerLeaf - maximum number of vertices a leaf can hold until it has to split and become an ocTree
		 * @PARAM int maximumSplitDepth - maximum split depth (level)
		 * @PARAM float parentDimensions[] - limits and mean values in x,y and z dimensions of parent node
		 * @PARAM ocTree* root - pointer to root node
		 * @PARAM std::vector<bool> &parId - binary representation of the parent node's identifier
		 * @PARAM std::vector<bool> &splitDirections - binary representation of this node's identifier on the current split-depth level
		 * @PARAM bool debugInfo - if true, print additional information to the console during construction
		 */
		ocTree(
				std::vector <std::pair <size_t, glm::vec3> > vertices,
				int parentLevel,
				int vertsMax,
				int splitsMax,
				float parentDimensions[],
				ocTree* root,
				std::vector<bool> &parId,
				std::vector<bool> &splitDirections,
				bool debugInfo
		);

		/**
		 * DESTRUCTOR
		 */
		~ocTree();

		/**
		 * determine whether this is a leaf or not
		 * returns bool variable isLeaf
		 */
		bool getisLeaf();

		/**
		 * get a leaf node via a boolean input array representing its identifier
		 * traverses through an entire tree and will return the node with given identifier and the lowest level
		 * compare has to contain 15 boolean values representing the identifier of a node, i.e.
		 * {false,false,false,false,false,true,false,false,false,false,false,false,false,false,false} = 000001000000000
		 *
		 * @PARAM std::vector<bool> - identifier array
		 * @PARAM bool debugInfo - if this is true, additional information (dimensions etc) about the node will be written to the console
		 * @RETURN null | pointer to matching leaf node
		 */
		ocTree* getNodeByIdentifierArray(std::vector<bool> compare, bool debugInfo = false);

		/**
		 * get a leaf containing a coordinate given via one input coordinate given via glm::vec3 target
		 * recursively traverses through the tree until the correct leaf node (containing target) is found
		 * finds neighbor nodes of target node, searches through all those nodes' vertices
		 * adds vertices within target coordinates + radius to the global vertex election
		 *
		 * @PARAM glm::vec3 target - target coordinates
		 * @PARAM float radius - search radius for vertices to be added to the selection in relationship to given coordinates
		 * if any of the target node's boundaries is within the radius, its neighboring nodes are retrieved
		 * @PARAM std::set<size_t> &intermediateSelection - reference to global set variable which stores selected vertices via their position within the set of vertices held by a node
		 * @PARAM bool debugInfo - if this is true, additional information about the node will be written to the console
		 * @RETURN null | pointer to matching next level node
		 */
		ocTree* addVerticesToSelectionByCoordinates(glm::vec3 target, float radius, std::set<size_t> &intermediateSelection, bool debugInfo);

		/**
		 * recursively traverses through the tree until correct leaf node is found
		 * finds neighbor nodes of target node, searches through all those nodes' vertices
		 * if already within the global selection, removes all vertices from it that lie within target coordinates + radius
		 *
		 * @PARAM glm::vec3 target - target coordinates
		 * @PARAM float radius - search radius for vertices to be removed from the selection in relationship to given coordinates
		 * if any of the target node's boundaries is within the radius, its neighboring nodes are retrieved
		 * @PARAM std::set<size_t> &intermediateSelection - reference to global set variable which stores selected vertices via their position within the set of vertices held by a node
		 * @PARAM bool debugInfo - if this is true, additional information about the node will be written to the console
		 * @RETURN pointer to matching next level node
		 */
		ocTree* removeVerticesFromSelectionByCoordinates (glm::vec3 target, float radius, std::set<size_t> &intermediateSelection, bool debugInfo);

		/**
		 * binary predicate helper function to determine whether a vertex had already been added to the selection or not
		 *
		 * @PARAM glm::vec3 frst - first vertex
		 * @PARAM glm::vec3 scnd - second vertex
		 */
//		bool same_coordinates (glm::vec3 frst, glm::vec3 scnd);

		void getRootDimensions(void);							// calculate min and max values as well as means for x, y and z

		/**
		 * getters
		 */
		std::vector<Mesh*> getMyMeshes(void);					// getter for all meshes
		std::vector<Mesh*>::iterator getMesh (int n);			// getter for mesh at position n
		std::vector<glm::vec3> getVertices(void);				// getter for vertices of all meshes
		std::vector<glm::vec3> getVerticesOfMesh (int n);		// getter for all vertices of mesh at position n
		std::vector<std::pair<size_t, glm::vec3> > getVerticesInBounds (void);
																// return m_verticesInBounds
		int getLevel(void);										// getter for m_level

		// convenience functions

		/**
		 * buildRecursively function
		 * iterates through given vector of vertices, checking if their coordinates are within the bounds of the current node
		 *
		 * @PARAM std::list <std::pair <size_t, glm::vec3> > & subsetVerts - ordered list of vector of vertices
		 */
		void buildTreeRecursively(std::vector <std::pair <size_t, glm::vec3> > & subsetVerts);

		/**
		 * return maximum allowed search radius for queries based on the root dimensions
		 * the maximum radius is defined as minDif / 2^maximum_split_level / 2
		 * where minDif is the minimum difference between max- and min values of the three possible dimensions
		 */
		float getMaximumSearchRadius(void);

		/**
		 * console out functions for testing purposes
		 */
		void saySomething(void);								// helloworld
		void debugTreeInfo(void);								// prints assorted relevant data to console
		void debugFirstVertex(void);							// prints vertex at position [0] of first mesh to console
		void debugAllVertices(void);							// prints all vertices to console
		void debugIdentifierasString(void);						// prints identifier as sequence of binary numbers
};

#endif
