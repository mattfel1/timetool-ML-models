#include <sstream>
#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <dirent.h>
typedef std::vector<std::string> stringvec;
using namespace std;


void build_kernel(float sig, int window_size, float* kernel) 
{
	float data[window_size];
	float sum = 0;
	for (int i = 0; i < window_size; i++) 
	{
		float t = i - (window_size-1)/2;
		float exp = -pow(t,2)/2/pow(sig,2);
		float coef = -t/pow(sig,3)/sqrt(2*M_PI);
		data[i] = coef * pow(M_E,exp);
		sum = sum + abs(data[i]);
	}
	float factor = 1.0/sum;
	for (int i = 0; i < window_size; i++)
	{
		kernel[i] = data[i]*factor;
	}
}

// take from http://stackoverflow.com/a/236803/248823
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

void processFibermap(std::string file, vector<int> &keys, vector<float> &delays)
{
    // Create reader
    std::ifstream infile (file);
    std::string line;

    // Dump first 2 lines
    for (int i = 0; i < 2; i++) {
    	std::getline(infile, line);
    }

    while (std::getline(infile, line))
    {
        vector<string> row_values;
        split(line, '\t', row_values);
        keys.push_back(stoi(row_values[0]));
        delays.push_back(stof(row_values[6]));
    }

}

float lookupDelay(std::vector<int> &keys, std::vector<float> &delays, int row)
{
	// No indirection
	return delays[row];

	// // With indirection
	// std::vector<int>::iterator it = std::find(keys.begin(), keys.end(), row);
	// int index = std::distance(keys.begin(), it);
	// return delays[index];
}

void processImage(std::string file, std::ofstream& outfile, float sig, int window_size, float* kernel)
{
	// Collect keys/delays from fibermap
	vector<float> delays;
	vector<int> keys;
	std::string fibermap_file = file;
	fibermap_file.replace(fibermap_file.find("interference"),string("interference").length(),"fibermap");
	std::string fileId(file.substr(file.rfind(".") + 1));
	std::cout << fileId << std::endl;
	processFibermap(getenv("DATA_FS") + fibermap_file, keys, delays);

    // Create reader
    std::ifstream infile (getenv("DATA_FS") + file);
    std::string line;

    // Dump first 7 lines
    for (int i = 0; i < 7; i++) {
    	std::getline(infile, line);
    }

	// std::cout << "| File ID    | Row    | Rising Idx |  Falling Idx |    Volume |    Rising V |    Falling V |     First V |    Last V |    Delay  |" << std::endl;
	// Print the content of row by row on screen
	int row = 0;
    while (std::getline(infile, line))
    {
        vector<string> row_values;
        // split(line, ',', row_values);
        split(line, '\t', row_values);


		int best_rising_idx = 0;
		int best_falling_idx = 0;
		float firstV = 0;
		float lastV = 0;
		float best_rising_slope = 0;
		float best_falling_slope = 0;
		int32_t acc_after_rising = 0;
		int32_t acc_after_falling = 0;

		std::vector<int32_t> window(window_size,0);
		int col = 0;
		// Because of retiming, FPGA resets its acc while a few updates are coming down the line.  Simulate that here
		int reset_delay_interval = 26;
		int rising_reset_cnt = 999;
		int falling_reset_cnt = 999;
		std::vector<int32_t> reset_sr(reset_delay_interval, 0);
        for (auto data: row_values) 
		{
			// Shift in
			if (col == 0) firstV = std::stoi(data);
			if (col == row_values.size()-1) lastV = std::stoi(data);
			for (int i = window_size-1; i > 0; i--) { window[i] = window[i-1]; }
			window[0] = std::stoi(data);
			if (rising_reset_cnt > reset_delay_interval) acc_after_rising = acc_after_rising + std::stoi(data);
			//if (rising_reset_cnt <= reset_delay_interval) std::cout << "suppressing: " <<  std::stoi(data) << " (interval " << rising_reset_cnt << ")" << std::endl;
			//if (rising_reset_cnt > reset_delay_interval) std::cout << "    running count = " <<  acc_after_rising << std::endl;


			if (falling_reset_cnt > reset_delay_interval) acc_after_falling = acc_after_falling + std::stoi(data);
			
			// Reset delay fudging
			for (int i = reset_delay_interval-1; i > 0; i--) { reset_sr[i] = reset_sr[i-1]; }
			reset_sr[0] = std::stoi(data);
			
			// Calculate mac
			float mac = 0;
			for (int i = 0; i < window_size; i++) { mac = mac + window[i] * kernel[i]; }
			// Catch rising/falling edges
			if (mac > best_rising_slope && col > window_size-1) {
				best_rising_idx = col;
				acc_after_rising = 0;
				best_rising_slope = mac;
				rising_reset_cnt = 0;
			}
			if (mac < best_falling_slope && col > window_size-1) {
				best_falling_idx = col;
				acc_after_falling = 0;
				best_falling_slope = mac;
				falling_reset_cnt = 0;
			}
			col = col + 1;
			rising_reset_cnt = rising_reset_cnt + 1;
			falling_reset_cnt = falling_reset_cnt + 1;
		}
		float row_delay = lookupDelay(keys, delays, row);
		float row_key = keys[row];
		//std::cout << "\t" << fileId << "\t" << row << "\t" << best_rising_idx << "\t" << best_falling_idx << "\t" << (acc_after_rising - acc_after_falling) << "\t"  << best_rising_slope << "\t" << best_falling_slope << "\t" << firstV << "\t" << lastV << "\t" << row_delay << std::endl;
		outfile << fileId << "," << row << "," << best_rising_idx << "," << best_falling_idx << "," << (acc_after_rising - acc_after_falling) << ","  << best_rising_slope << "," << best_falling_slope << "," << firstV << "," << lastV << "," << row_delay << std::endl;
		row = row + 1;
    }

}

int main()
{
	// Create out file
	std::ofstream outfile;
	// outfile.open ("processed_baby.csv");
	outfile.open ("processed.csv");
	
	// Set up kernel
	int window_size = 40;
	float sig = window_size/8;
	float kernel[window_size];
	build_kernel(sig, window_size, kernel);

	// Get list of files to process
    std::string path = std::string(getenv("DATA_FS"));
    std::vector<std::string> images;
    DIR* dirp = opendir(path.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
    	std::string filename = (dp->d_name);
        if (filename.find("interference") != std::string::npos) {
        	images.push_back(filename);
        }
    }
    closedir(dirp);
	for ( auto&& item : images) {
	   processImage(item, outfile, sig, window_size, kernel);
	}
	outfile.close();

	return 0;

}
