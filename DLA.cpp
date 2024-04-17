#include <iostream>
#include <time.h>
#include <chrono>
#include <cmath>


#include "DLA.h"

using namespace std;



int main(int argc, char* argv[])
{
	//int image_height = 700;
	//int image_width = 1000;

	//uchar* pixel_buffer = GenerateRandomImage(image_height, image_width);



	//seed RANDOM :3
	srand(time(0));

	//timing
	auto start_time = chrono::system_clock::now();

	//initial alg
	int square_dimension = 5;
	printf("started the thingfy\n");
	vector<DLANode> test = RunDLA_CollisionAttachment(square_dimension, 0.15f, nullptr, 0, false);
	//uchar* pixel_buffer = DLAConfigToPixels(&test, square_dimension);

	int target_dimension = 80;
	int cur_dimension = square_dimension;
	float desired_density = 0.12f;
	while (cur_dimension < target_dimension)
	{
		//desired_density /= 2;
		cur_dimension *= 2; //double the dimension size and run algorithm again
		if (cur_dimension >= 160){
			test = RunDLA_CollisionAttachment(cur_dimension, desired_density, &test, cur_dimension / 2, true);
		}
		else {
			test = RunDLA_CollisionAttachment(cur_dimension, desired_density, &test, cur_dimension / 2, false);

		}
	}


	//uchar* pixel_buffer = DLAConfigToPixels(&test, cur_dimension);
	//cv::Mat dla_img(cur_dimension, cur_dimension, CV_8UC1, cv::Scalar(0)); //10x10 1 channel 8bit unsigned matrix, autofilled with pixels with value "0"
	//std::memcpy(dla_img.data, pixel_buffer, pow(cur_dimension, 2) * sizeof(uchar));

	//cv::namedWindow(std::to_string(cur_dimension), cv::WINDOW_FREERATIO);
	//cv::imshow(std::to_string(cur_dimension), dla_img);

	//int response = cv::waitKey(0);
	//if (response == 's')
	//{
	//	cv::imwrite("output.png", dla_img);
	//}


	//test upscaling
	//int medium_square = 160;
	//test = RunDLA_CollisionAttachment(medium_square, 0.0f, &test, cur_dimension, true);
	//cur_dimension = medium_square;


	int bigger_square = 500;
	vector<DLANode> test_bigger_canvas = RunDLA_CollisionAttachment(bigger_square, 0.0f, &test, cur_dimension, true);
	uchar* pixel_buffer = DLAConfigToPixels(&test_bigger_canvas, bigger_square);



	chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start_time;
	printf("finished the thingy\n (elapsed seconds = %f)\n::: displaying output\n", elapsed_seconds.count());


	////openCV imaging stuff
	cv::Mat x(bigger_square, bigger_square, CV_8UC1, cv::Scalar(0)); //10x10 1 channel 8bit unsigned matrix, autofilled with pixels with value "0"
	std::memcpy(x.data, pixel_buffer, pow(bigger_square,2) * sizeof(uchar)); //copy pixel buffer into the cv::Mat datatype so we can display an image
	//cv::resize(x, x, cv::Size(), 2, 2);
	//cv::blur(x, x, cv::Size(15, 15));
	free(pixel_buffer); //empty out pixel buffer assuming we don't need it anymore 



	//cv::Mat resized(target_dimension, target_dimension, CV_8UC1, cv::Scalar(0));
	//std::memcpy(resized.data, DLAConfigToPixels(&test, target_dimension), pow(target_dimension,2) * sizeof(uchar));
	//cv::resize(resized, resized, cv::Size(), 2, 2);
	//cv::blur(resized, resized, cv::Size(15, 15));
	//cv::GaussianBlur(resized, resized, cv::Size(21, 21), 1.5, 1.5);

	cv::namedWindow("Display Window", cv::WINDOW_FREERATIO);
	cv::imshow("Display Window", x);
	int response = cv::waitKey(0);

	//if (response == 's')
	//{
	//	cv::imwrite("output.png", x);
	//}




	//const std::string image_path = "C:/Users/CG/Pictures/download.jpg";
	//cv::Mat img = cv::imread(image_path); //read image (cv::IMREAD_UNCHANGED) 

	//if (img.empty())
	//{
	//	std::cout << "Could not read the image: " << image_path << std::endl;
	//	return -1;
	//}

	//cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
	//cv::imshow("Display window", img);
	//int k = cv::waitKey(0); // Wait for a keystroke in the window

	//if (k == 's')
	//{
	//	cv::imwrite("output.png", img);
	//}




	return 0;
}



int GetSignOfInteger(int x)
{
	if (x > 0) {
		return 1;
	}
	else if (x < 0) {
		return -1;
	}
	return 0;
}



uchar* GenerateRandomImage(int image_height, int image_width)
{
	//allocate space for the image 
	uchar* image_pixels_buffer = (uchar*)malloc((image_height * image_width)*sizeof(uchar));

	//begin random data population
	for (int i = 0; i < image_height; i++)
	{
		for (int j = 0; j < image_width; j++)
		{
			image_pixels_buffer[(i * image_width) + j] = rand() % 256; //assign random pixel value from 0 - 255
		}
	}

	return image_pixels_buffer;
}



vector<DLANode> RunDLA_CollisionAttachment(int grid_dimension, float fill_threshold_percentage, vector<DLANode>* initial_configuration = nullptr, int starting_config_dimensions = 0, bool should_upscale_initial_config = false)
{
	vector<DLANode> output_configuration;

	//Use initial configuration as a starting place for the algorithm
	if (initial_configuration != nullptr)
	{
		//Should initial configuration be upscaled to current grid dimension, or use as is?
		if (should_upscale_initial_config)
		//upscaling. Attempt to maintain the configuration's shape but increase the amount of nodes to match the increase in grid dimension. 
		{
			output_configuration = DLAUpscalingAlg(grid_dimension, initial_configuration, starting_config_dimensions);
		}
		else 
		//no upscaling, only need to update configuration positions to fit the new grid 
		{
			output_configuration = *initial_configuration; //copy the configuration (dereferencing the pointer to get the actual vector) 
			int position_shift_amount = (grid_dimension - starting_config_dimensions)/2; 

			//shift all nodes positions by current position + shift amount 
			for (int i = 0; i < output_configuration.size(); i++)
			{
				DLANode temp_node = output_configuration[i];
				temp_node.x_pos += position_shift_amount;
				temp_node.y_pos += position_shift_amount;
				output_configuration[i] = temp_node;
			}
		}
	}
	//Initialize DLA from scratch with given grid dimension 
	else
	{
	//pick a midpoint within the grid to place the first node
		DLANode first_node = {(uchar)0, (uint)(grid_dimension / 2),(uint)(grid_dimension / 2), -1, false };
		output_configuration.push_back(first_node);
	}

//main alg loop ::: is fill threshold percentage satisfied?
	uint x_pos;
	uint y_pos;
	uchar traversal_sweep_factor = 1; //use this variable to scale how much we move the nodes when looking for places to put it 
	
	//debugs
	int total_iteration_accum = 0; //total number of iterations it took for all nodes to form a connection 
	int nodes_added = 1;
	
	while (output_configuration.size() < fill_threshold_percentage * pow(grid_dimension, 2))
	{

		//if avg number of iterations to find collisions is over a certain amount, start sweeping the node across the grid a greater distance 
		if (total_iteration_accum / nodes_added > 2500) {
			traversal_sweep_factor = traversal_sweep_factor < 8 ? traversal_sweep_factor * 2 : traversal_sweep_factor; //dont let sweep factor get too big or else it will go crazy wacko mode 
			//traversal_sweep_factor = 1;

			//reset accumulators so that the average doesn't become desensitized to further changes
			//total_iteration_accum = 0;
			//nodes_added = 1;
		}
		else if (traversal_sweep_factor > 1){
			//start reducing the sweep distance if it's getting easier to find good spots to put the thingy
			traversal_sweep_factor = traversal_sweep_factor / 2;
			total_iteration_accum = 0;
			nodes_added = 1;
		}
		
		
		//randomly generate a starting position in the grid (make sure it doesn't overlap with anything)
		x_pos = rand() % grid_dimension;
		y_pos = rand() % grid_dimension;

		bool valid_starting_position = false;
		while (!valid_starting_position) //test if starting position is valid
		{
			valid_starting_position = true; //assume starting position is valid now
			for (int i = 0; i < output_configuration.size(); i++)
			{
				if (output_configuration[i].x_pos == x_pos && output_configuration[i].y_pos == y_pos) {
					valid_starting_position = false;
					break; // position not valid, try to find a different position 
				}
			}

			if (valid_starting_position) {
				break;
			}
			else { // position not valid, try to find a different position 
				x_pos = rand() % grid_dimension;
				y_pos = rand() % grid_dimension;
			}
		}

		//intialize a new node at the starting position
		DLANode new_node = { (uchar)0, x_pos, y_pos, -1, true }; //true because this node will be a leaf node

		//loop until found a spot for this node to connect to
		uchar desired_move_direction;       //(0 = north 1 = south 2 = east 3 = west)
		bool x_axis_move_direction; 
		char direction_sign; 
		uchar sweep_range; //amount to sweep node across an axis when checking for collisions
		bool connection_formed = false;
		int closest_node_index = -1; 

		//debugs
		int per_node_iteration_counter = 0; //number of iterations it takes for the node to form a connection 
		
		while (!connection_formed)
		{
			//generate direction for node to move (0=north 1=south 2=east =3west) 
			desired_move_direction = rand() % 4;
			sweep_range = ((rand() % traversal_sweep_factor) + 1);

			//check for collision in given direction
				//x,y position where we will check for something present. If there is something at this location, then our new node will collide into it
			int looking_for_collision_at_x = new_node.x_pos;
			int looking_for_collision_at_y = new_node.y_pos;


			switch (desired_move_direction)
			{
			case 0: //north
				looking_for_collision_at_y -= 1 * sweep_range;
				x_axis_move_direction = false;
				direction_sign = -1;
				break;
			case 1: //south
				looking_for_collision_at_y += 1 * sweep_range;
				x_axis_move_direction = false;
				direction_sign = 1;
				break;
			case 2: //east
				looking_for_collision_at_x += 1 * sweep_range;
				x_axis_move_direction = true;
				direction_sign = 1;
				break;
			case 3: //west
				looking_for_collision_at_x -= 1 * sweep_range;
				x_axis_move_direction = true;
				direction_sign = -1;
				break;
			default:
				printf("HUH? How did you fuck up generating a number from 0 to 3 ");
				break;
			}


			//Sweep along a given direction and look for all nodes along the sweep that could cause a collision 
			if (x_axis_move_direction)
			{
				for (int i = 0; i < output_configuration.size(); i++)
				{
					if (direction_sign * (output_configuration[i].x_pos - looking_for_collision_at_x) <= 0 && output_configuration[i].y_pos == looking_for_collision_at_y)
						//collision found along path we were sweeping in
					{
					//Test if this node is the closest possible node that we can collide with

						if (direction_sign * (output_configuration[i].x_pos - new_node.x_pos) == 1)
							//collided node is only 1 space away from the new node, so it has to be da one
						{
							closest_node_index = i;
							break; //break from collision search early because we are certain this is it

						}
						else if (closest_node_index > -1)
							//node *could* be the closest possible node so we will keep track of it incase it is. 
						{
							if (direction_sign * (output_configuration[closest_node_index].x_pos - output_configuration[i].x_pos) > 0)
								//found node that is closer collision but there could possibly be a node even closer that we haven't found along the sweep, so keep searchinbg
							{
								closest_node_index = i;
							}
						}
						else {
							closest_node_index = i;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < output_configuration.size(); i++)
				{
					if (output_configuration[i].x_pos == looking_for_collision_at_x && direction_sign * (output_configuration[i].y_pos - looking_for_collision_at_y) <= 0)
						//collision found along path we were sweeping in 
					{
					//Test if this node is the closest possible node that we can collide with

						if (direction_sign * (output_configuration[i].y_pos - new_node.y_pos) == 1)
							//collided node is only 1 space away from the new node, so it has to be da one
						{
							closest_node_index = i;
							break; //break from collision search early because we are certain this is it
						
						}
						else if (closest_node_index > -1)
							//node *could* be the closest possible node so we will keep track of it incase it is. 
						{
							if (direction_sign * (output_configuration[closest_node_index].x_pos - output_configuration[i].x_pos) > 0)
								//found node that is closer collision but there could possibly be a node even closer that we haven't found along the sweep, so keep searchinbg
							{
								closest_node_index = i;
							}
						}
						else {
							closest_node_index = i;
						}
					}
				}
			}


			//OLD COLLISION CHECK ONLY CONSIDERING MOVEMENT 1 SPACE AT A TIME INSTEAD OF SWEEP
			////if collides, create attachment (add to output configuration) 
			//for (int i = 0; i < output_configuration.size(); i++)
			//{
			//	if (output_configuration[i].x_pos == looking_for_collision_at_x && output_configuration[i].y_pos == looking_for_collision_at_y)
			//		//collision found because something is present in the location we were trying to move in
			//	{
			//		new_node.parent_connection_idx = i;
			//		output_configuration.push_back(new_node);
			//		connection_formed = true;

			//		//make sure connected node is no longer treated as a leaf node
			//		output_configuration[i].is_leaf = false;
			//		break;
			//	}
			//}


			//TERMINATION CONDITION (connection formed)
			// Pick the closest node to collide with found and form connection (no node was found if closest node index is -1 ) 
			if (closest_node_index > -1)
			{
				new_node.parent_connection_idx = closest_node_index;
				new_node.x_pos = x_axis_move_direction ? output_configuration[closest_node_index].x_pos - (direction_sign) : new_node.x_pos;
				new_node.y_pos = !x_axis_move_direction ? output_configuration[closest_node_index].y_pos - (direction_sign) : new_node.y_pos;
				output_configuration.push_back(new_node);

				//make sure connected node is no longer treated as a leaf node
				output_configuration[closest_node_index].is_leaf = false;

				connection_formed = true; //terminates loop
			}
			else
			{
				//otherwise, move the node in the earlier given direction (as long as it is inside grid boundaries btw) 
				if (looking_for_collision_at_x >= 0 && looking_for_collision_at_x < grid_dimension) {
					new_node.x_pos = looking_for_collision_at_x;
				}
				if (looking_for_collision_at_y >= 0 && looking_for_collision_at_y < grid_dimension) {
					new_node.y_pos = looking_for_collision_at_y;
				}

				per_node_iteration_counter++;
			}

		}

		total_iteration_accum += per_node_iteration_counter;
		nodes_added++; 
		//printf("DLA list so far....\n");
		printf("size:%d, nodes:%d, fill perc:%.2f, iters this node: %*d ", grid_dimension, (int)output_configuration.size(), output_configuration.size()/pow(grid_dimension, 2), 7, per_node_iteration_counter);
		printf("Running avg... %d::: traversal factor... %d::: \n", total_iteration_accum / nodes_added, traversal_sweep_factor);

	}

	if (nodes_added > 0) {
		printf("Average number of iteration it took for a node to form a connection %d \n", total_iteration_accum / nodes_added);
	}

	return output_configuration;
}



vector<DLANode> RunDLA_StickyAttachment(int grid_dimension, float fill_threshold_percentage, vector<DLANode>* initial_configuration, int starting_config_dimensions, bool should_upscale_initial_config)
{
	vector<DLANode> output_configuration;

	//Use initial configuration as a starting place for the algorithm
	if (initial_configuration != nullptr)
	{
		//Should initial configuration be upscaled to current grid dimension, or use as is?
		if (should_upscale_initial_config)
			//upscaling. Attempt to maintain the configuration's shape but increase the amount of nodes to match the increase in grid dimension. 
		{
			output_configuration = DLAUpscalingAlg(grid_dimension, initial_configuration, starting_config_dimensions);
		}
		else
			//no upscaling, only need to update configuration positions to fit the new grid 
		{
			output_configuration = *initial_configuration; //copy the configuration (dereferencing the pointer to get the actual vector) 
			int position_shift_amount = (grid_dimension - starting_config_dimensions) / 2;

			//shift all nodes positions by current position + shift amount 
			for (int i = 0; i < output_configuration.size(); i++)
			{
				DLANode temp_node = output_configuration[i];
				temp_node.x_pos += position_shift_amount;
				temp_node.y_pos += position_shift_amount;
				output_configuration[i] = temp_node;
			}
		}
	}
	//Initialize DLA from scratch with given grid dimension 
	else
	{
		//pick a midpoint within the grid to place the first node
		DLANode first_node = { (uchar)0, (uint)(grid_dimension / 2),(uint)(grid_dimension / 2), -1, false };
		output_configuration.push_back(first_node);
	}

	//main alg loop ::: is fill threshold percentage satisfied?
	uint x_pos;
	uint y_pos;
	while (output_configuration.size() < fill_threshold_percentage * pow(grid_dimension, 2))
	{
		//randomly generate a position in the grid (make sure it doesn't overlap with anything)
		x_pos = rand() % grid_dimension;
		y_pos = rand() % grid_dimension;

		bool valid_starting_position = false;
		while (!valid_starting_position) //test if starting position is valid
		{
			valid_starting_position = true; //assume starting position is valid now
			for (int i = 0; i < output_configuration.size(); i++)
			{
				if (output_configuration[i].x_pos == x_pos && output_configuration[i].y_pos == y_pos) {
					valid_starting_position = false;
					break; // position not valid, try to find a different position 
				}
			}

			if (valid_starting_position) {
				break;
			}
			else { // position not valid, try to find a different position 
				x_pos = rand() % grid_dimension;
				y_pos = rand() % grid_dimension;
			}
		}

		//intialize a new node at the starting position
		DLANode new_node = { (uchar)0, x_pos, y_pos, -1, true }; //true because this node will be a leaf node


		//loop until found a spot for this node to connect to
		uchar desired_move_direction;       //(0 = north 1 = south 2 = east 3 = west)
		vector<int> nearby_sticky_candidates_node_indexes; //contains the indexes of the nodes (inside the output configuration) that are possible for this new node to stick to
		bool connection_formed = false;
		while (!connection_formed)
		{

		//Check for adjacent nodes nearby. If true, this node might "stick" to it
			//Sticky (radius of 1, not including diagonals). 
			int y_offset = 1;
			for (int k = 0; k < 2; k++) { //test y+1 and y-1 (north and south)
				for (int i = 0; i < output_configuration.size(); i++)
				{
					if (output_configuration[i].x_pos == new_node.x_pos && output_configuration[i].y_pos == new_node.y_pos + y_offset)
						//new node might stick to this node because its position is within the radius
					{
						nearby_sticky_candidates_node_indexes.push_back(i);
						break;  //stop checking in this direction and test a different one
					}
				}
				y_offset = -1;
			}
			int x_offset = 1;
			for (int k = 0; k < 2; k++) { //test x+1 and x-1 (east and west)
				for (int i = 0; i < output_configuration.size(); i++)
				{

					if (output_configuration[i].x_pos == new_node.x_pos + x_offset && output_configuration[i].y_pos == new_node.y_pos)
						//new node might stick to this node because its position is within the radius
					{
						nearby_sticky_candidates_node_indexes.push_back(i);
						break; //stop checking in this direction and test a different one
					}
				}
				x_offset = -1;
			}

			//Determine which node to stick to if any nearby candidates were detected 
			if (nearby_sticky_candidates_node_indexes.size() > 0) 
			{
				uchar chosen_stickiest_node = (uchar)(rand() % (int)nearby_sticky_candidates_node_indexes.size());
				new_node.parent_connection_idx = nearby_sticky_candidates_node_indexes[chosen_stickiest_node];
				output_configuration.push_back(new_node);
				connection_formed = true;

				//make sure connected node is no longer treated as a leaf node
				output_configuration[nearby_sticky_candidates_node_indexes[chosen_stickiest_node]].is_leaf = false;
				
				nearby_sticky_candidates_node_indexes.clear();
			}
			else
			//nothing to stick to, so lets move the node somewhere else (make sure within grid boundaries)
			{
				//generate direction for node to move (0=north 1=south 2=east =3west) 
				desired_move_direction = rand() % 4;
				uint x_move_location = new_node.x_pos;
				uint y_move_location = new_node.y_pos;
				switch (desired_move_direction)
				{
				case 0: //north
					y_move_location -= 1;
					break;
				case 1: //south
					y_move_location += 1;
					break;
				case 2: //east
					x_move_location += 1;
					break;
				case 3: //west
					x_move_location -= 1;
					break;
				default:
					printf("HUH? How did you fuck up generating a number from 0 to 3 ");
					break;
				}

				if (x_move_location >= 0 && x_move_location < (uint)grid_dimension) {
					new_node.x_pos = x_move_location;
				}
				if (y_move_location >= 0 && y_move_location < (uint)grid_dimension) {
					new_node.y_pos = y_move_location;
				}
			}
		}

		//printf("DLA list so far....\n");
		//printf("%d, %f\n", (int)output_configuration.size(), output_configuration.size() / pow(grid_dimension, 2));
	}


	return output_configuration;
}



vector<DLANode> DLAUpscalingAlg(int new_grid_dimension, vector<DLANode>* initial_configuration, int starting_config_dimensions)
{

	vector<DLANode> output_configuration;

//place initial configuration nodes, scaled up to fit the new grid
	vector<DLANode> temp_output_configuration = *initial_configuration;
	int upscale_factor = (int)ceil(new_grid_dimension / starting_config_dimensions);

	for (int i = 0; i < initial_configuration->size(); i++)
	{
		DLANode temp_node = initial_configuration->at(i);
		temp_node.x_pos *= upscale_factor;
		temp_node.y_pos *= upscale_factor;

		temp_output_configuration[i] = temp_node;
	}

	output_configuration.push_back(temp_output_configuration[0]); //first node is the root node and has no connections so we can just add this 


//then connect the gaps between the nodes that form a "line" (loop)
	// (add nodes in the direction of the connection in a sort of mini DLA algorithim.)
	for (int i = 1; i < temp_output_configuration.size(); i++) //there should be n-1 connections between all the current nodes (each node is only connected to 1 other node)
	{
		//these two nodes form a "line" that needs to be populated inside the new upscaled grid
		DLANode start_node = temp_output_configuration[i];
		DLANode dest_node = temp_output_configuration[start_node.parent_connection_idx];


	//Start from the destination node and add nodes up to the start node (mini DLA algorithm) ((we do it in reverse since the start node is a child node and the desination node is the parent node in reality. We only have connection from child to parent which is why we identify the "start" as the child)) 

		int index_of_prev_node = FindDLANodeInList(dest_node, &output_configuration); //index of the last node we added to the line (relative to overall configuration) 
		if (index_of_prev_node == -1)
			//if parent node of this "line" isn't yet in output configuration, put it in and get its index. 
		{
			output_configuration.push_back(dest_node);
			index_of_prev_node = (int)output_configuration.size() - 1;
		}

		//we want these to be either 0 or +/-1 to show the direction that the link will move in (north, south, east, or west, depending on combination of x and y)
		char x_direction_of_link = GetSignOfInteger(start_node.x_pos - dest_node.x_pos);
		char y_direction_of_link = GetSignOfInteger(start_node.y_pos - dest_node.y_pos);
		
		uint delta_x = abs((int)(start_node.x_pos - dest_node.x_pos)); //number of spaces to move in x direction for a given line
		uint delta_y = abs((int)(start_node.y_pos - dest_node.y_pos)); //number of spaces to move in y direction for a given line

		//special situation if the line is "flat" (a cardinal direction) 
		int jitter_shift_amount; //variable to apply to a horizontal or straight line configuration to slightly shift it's middle points to make it more interesting. If zero, will not make any change to the line.
		if (delta_x == 0) {
			//line formed is vertically straight 
			//pick a random amount to shift the middle point of the line  (at most should create an isosolesce triangle with diagonals of slope (1,1)) 
			jitter_shift_amount = (2 * (rand() % (int)floor(delta_y / 2))) - (int)floor(delta_y / 2);
								//random value in from negative to pos range. 
								/*
								* for instance, if we want a range from -4 to 4. We would generate a number (x) "4" and then multiply by 2 to equal a max of 8 and then subtract 4.
								*	if generated max value = 4 then 2(4) - 4 = 4 max range
								*	if generated min value = 0 then 2(0) - 4 = -4 min range
								*/
		}
		else if (delta_y == 0) {
			//line formed is horizontally straight 
			jitter_shift_amount = (2 * (rand() % (int)floor(delta_x / 2))) - (int)floor(delta_x / 2);
		}
		else {
			//line is diagonal
			jitter_shift_amount = 0; //do not involve jitter into horizontal lines I don't wanna think about it thanks
		}

		//current position as we step along the line and place nodes (start at destination and step towards start as noted in earlier comment)  
		uint x_pos = dest_node.x_pos;
		uint y_pos = dest_node.y_pos;
		bool gap_filled = false;
		uint node_inline_index = 1; //ignore first node position because that node already exists (it's the starting node of this line) 
		while (!gap_filled)
		{
			//depending on which direction has more distance will change how we iterate to next node (if they are the same though it's chilling) 
			if (delta_x >= delta_y)
			{
				//step 1 space at a time in x direction, but only step y direction when slope calls for it 
				x_pos += x_direction_of_link;
				
				if (abs(jitter_shift_amount) > 0) //shift y position of elements in this line by some amount
					//assuming we are working with horizontal lines cause if we aren't then this will get kind of weird 
				{
					double slope = (double)jitter_shift_amount / (double)delta_x;
					int line_middle_index = (int)floor(delta_x / 2);

					y_pos = dest_node.y_pos + (int)(slope * abs(abs((int)(line_middle_index - (node_inline_index))) - line_middle_index));
				}
				else //shifting y position a predetermined diagonal amount because this line is diagonal
				{
					y_pos = dest_node.y_pos + y_direction_of_link * (int)floor(node_inline_index * (1 + (delta_y / delta_x)));
				}
			}
			else
			{
				//step 1 space at a time in x direction, but only step y direction when slope calls for it 
				y_pos += y_direction_of_link;

				if (abs(jitter_shift_amount) > 0) //shift x position of elements in this line by some amount
					//assuming we are working with horizontal lines cause if we aren't then this will get kind of weird 
				{
					double slope = (double)jitter_shift_amount / (double)delta_y;
					int line_middle_index = (int)floor(delta_y / 2);

					x_pos = dest_node.x_pos + (int)(slope * abs(abs((int)(line_middle_index - (node_inline_index))) - line_middle_index));
				}
				else //shifting y position a predetermined diagonal amount because this line is diagonal
				{
					x_pos = dest_node.x_pos + x_direction_of_link * (int)floor(node_inline_index * (1 + (delta_x / delta_y)));
				}
			}
			
			DLANode new_node = { 0, x_pos, y_pos, index_of_prev_node, false };

			if (x_pos == start_node.x_pos && y_pos == start_node.y_pos)
				//we have reached the end of the line between the two nodes. 
			{
				gap_filled = true;

				//now just need to make sure we dont end up re-adding this node to the list (since it may have been identified earlier when we were upscaling) 
				int last_node_of_the_line = FindDLANodeInList(start_node, &output_configuration);
				if (last_node_of_the_line != -1)
				{
					output_configuration[last_node_of_the_line].parent_connection_idx = index_of_prev_node; //update the parent node index value of this final node)

					break;
				}
			}

			output_configuration.push_back(new_node);
			index_of_prev_node = (int)output_configuration.size() - 1;
			node_inline_index++;

		}
	}

	return output_configuration;
}



uchar* DLAConfigToPixels(vector<DLANode>* dla_configuration, int grid_dimension)
{
	//initializing the array 
	uchar* array_of_pixels = (uchar*)malloc(sizeof(uchar) * (int)pow(grid_dimension, 2));
	for (int i = 0; i < (int)pow(grid_dimension, 2); i++) {
		array_of_pixels[i] = (uchar)0;
	}

	//adding pixels to the array with some value (0-255) 
	int pixel_x_pos;
	int pixel_y_pos;
	for (int i = 0; i < dla_configuration->size(); i++)
	{
		pixel_x_pos = dla_configuration->at(i).x_pos;
		pixel_y_pos = dla_configuration->at(i).y_pos;

		array_of_pixels[(pixel_y_pos * grid_dimension) + pixel_x_pos] = (uchar)255; //or ->at(i).val; but we aren't really using that just yet (TODO)
	}

	return array_of_pixels;
}



cv::Mat BuildDLAHeightmap(uchar* crisp_dla_pixel_image, cv::Mat initial_dla_image)
{
	return cv::Mat();
}




int FindDLANodeInList(DLANode node, vector<DLANode>* dla_configuration_list)
{
	uint x_pos = node.x_pos;
	uint y_pos = node.y_pos;
	for (int i = 0; i < dla_configuration_list->size(); i++)
	{
		if (dla_configuration_list->at(i).x_pos == x_pos && dla_configuration_list->at(i).y_pos == y_pos) {
			return i;
		}
	}
	return -1;
}

