#pragma once
#include <stdlib.h>
#include <vector> 

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp> //reading writing images 
#include <opencv2/highgui.hpp> //visualize images 

using namespace std;

//does what it says ._.
int GetSignOfInteger(int x);

//Generates an array of 8bit items that represent pixels with the given dimensions (row-major)
uchar* GenerateRandomImage(int image_height, int image_width);



struct DLANode {
	uchar val;
	uint x_pos;
	uint y_pos;
	int parent_connection_idx; //contains an list/vector index corresponding to the DLANode that this node is attached to. (-1 would be root node) 
	//DLANode* parent; //if nullptr then is the root node
	//could have a children field but do we really need it nahhh
	bool is_leaf; //true or false to indicate if this node has none other explicitly connected to it
};


/*
//Run Diffusion Limited Aggregation algorithm
*
* Returns a configuration of nodes, which is a list of DLANodes that have a relationship of connections. (Explicitly, each leaf node is connected to another node higher up in the tree. Connection is via list index and not memory pointer.) 
* 
* Fill threshold indicates at what percentage of nodes / grid area  should the algorithim stop 
* 
* Optional starting configuration (and specified grid dimension of starting tree which may be different than the input grid dimensions)
*	
* This version of the algorithm utilizes collision attachment to attaches node together (other version is Sticky Attachment) 
* 
*/
vector<DLANode> RunDLA_CollisionAttachment(int grid_dimension, float fill_threshold_percentage, vector<DLANode>* initial_configuration, int starting_config_dimensions, bool should_upscale_initial_config);


vector<DLANode> RunDLA_StickyAttachment(int grid_dimension, float fill_threshold_percentage, vector<DLANode>* initial_configuration, int starting_config_dimensions, bool should_upscale_initial_config);


/*
//Helper function used to upscale a given DLAConfiguration to the appropraite new grid dimension
* 
* Attempts to maintain the configuration's shape but increase the amount of nodes to match the increase in grid dimension.
*/
vector<DLANode> DLAUpscalingAlg(int new_grid_dimension, vector<DLANode>* initial_configuration, int starting_config_dimensions);


/*
//Builds a M x M array (dimensions specified in parameter) of pixels.
* 
* Nodes that are further up the DLA configuration tree (closer to the root) will yield higher pixel values (up to a max value of 255) 
*/
uchar* DLAConfigToPixels(vector<DLANode>* dla_configuration, int grid_dimension);


//Combines crisp dla pixeled image with a blurred version and an optional pre-calculated imaged provided in parameter.
cv::Mat BuildDLAHeightmap(uchar* crisp_dla_pixel_image, cv::Mat initial_dla_image);


/*
//returns the index within the list where the DLANode was found (returns -1 if not found)
*
* makes search comparison based on (x,y) coordinates of the DLANode
*/
int FindDLANodeInList(DLANode node, vector<DLANode>* dla_configuration_list);
