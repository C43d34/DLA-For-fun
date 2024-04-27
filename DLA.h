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
	ushort val;
	int x_pos;
	int y_pos;
	int parent_connection_idx; //contains an list/vector index corresponding to the DLANode that this node is attached to. (-1 would be root node) 
	//DLANode* parent; //if nullptr then is the root node
	//could have a children field but do we really need it nahhh
	bool is_leaf; //true or false to indicate if this node has none other explicitly connected to it
};


struct DLALine {
	int x_pos_start;
	int y_pos_start;
	int x_pos_end;
	int y_pos_end;
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
* 
* Returns a tuple containing the resulting configuration after upscaling, and an accompanying Spatial tiling lookup grid that knows the neighborhood of each node (tiling size determiend by parameter) 
*/
tuple<vector<DLANode>, vector<vector<int>>> DLAUpscalingAlg(int new_grid_dimension, vector<DLANode>* initial_configuration, int starting_config_dimensions, int tile_size);



/*
* Takes a given node (usually a leaf node) within a dla configuration and adds value to each parent up the connection.
* Continues to do so until root node of the configuration is reached 
* 
* If current value of parent is already greater than added value, will take the larger value 
*/
void PropagateValueUpParents(vector<DLANode>* dla_configuration, int child_node_index);



/*
* Creates vector of "tiles" that represent 2D subsections of the entire dla configuration's dimension space.
* 
* Within each tile are the indicies of the nodes relative to the given dla_configuration. 
* 
* The tiles are organized in row major order. Meaning tiled 0 takes up the top left corner of the configuration grid. 
* 
* If spatial_tile_dimension = 1/2 config_grid dimension, there would be 4 tiles total in the vector.
* |0 1|
* |2 3|
*/
vector<vector<int>> BuildDLASpatialLookupTable(vector<DLANode> dla_configuration, int config_grid_dimension, int spatial_tile_dimensions);


/*
* Given some node with an x,y position and an index corresponding to its location within a DLA configuration list, 
* assign it to the appropriate spatial tile among a table that encompasses it's neighborhood. 
* Grid dimensions = size of the working space that the node could be (image size)
* Tile size = size of each tile 
*/
void AddNodeToTable(DLANode node, int node_index_inside_config, vector<vector<int>>* existing_spatial_tiling_table, int grid_dimensions, int tile_size);



//could change this to short[4] instead of int[4] i mean no way we will make an image 2mil across lol 
vector<DLALine> BuildDLALineConfiguration(vector<DLALine>* initial_dla_line_configuration);


vector<DLALine> UpscaleDLALineConfiguration(vector<DLALine> dla_line_configuration, double upscale_factor);


vector<DLANode> BuildDLANodesFromLineConfiguration(vector<DLALine> dla_line_configuration);





//Does this node inside the configuration exist inside a cycle?
bool NodeCycleCheck(vector<DLANode> dla_configuration, int starting_node_index);


/*
//Builds a M x M array (dimensions specified in parameter) of pixels.
* 
* Nodes that are further up the DLA configuration tree (closer to the root) will yield higher pixel values (up to a max value of 255) 
*/
uchar* DLAConfigToPixels(vector<DLANode>* dla_configuration, int grid_dimension);


/*
* Build a cv::Mat type image out of DLA configuration 
* - single channel black and while image
* 
* Uses DLAConfigToPixels() to handle the values of each node when converting to pixels 
*/
cv::Mat DLAConfigToCVMAT(vector<DLANode>* dla_configuration, int grid_dimension);


cv::Mat CVMAT_OuterBias(cv::Mat image, int image_dim);


/* 
* //Directly upscales a dla image to the specified upscale dimension using opencv (square images only)
* //Then applies an average filter blur
*/
cv::Mat UpscaleAndBlurDLAImage(cv::Mat crisp_dla_pixel_image, int pixel_image_dim, int upscale_dim, int blur_filter_size);


/*
//returns the index within the list where the DLANode was found (returns -1 if not found)
*
* makes search comparison based on (x,y) coordinates of the DLANode
* 
* (if tiling is present it will use that for search instead)
*/
int FindDLANodeInList(DLANode node, vector<DLANode>* dla_configuration_list);

int FindDLANodeInListUsingTileLookup(DLANode node, vector<DLANode>* dla_configuration_list, vector<vector<int>>* dla_spatial_tiling_lookup_grid, int grid_dimension, int tile_size);



void DisplayDLAConfigAndWait(vector<DLANode> dla_configuration, int dla_configuration_dim);