#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>

/*
 * A class to read data from a csv file.
 */
class CSVReader
{
	std::string fileName;
	std::string delimeter;

public:
	CSVReader(std::string filename, std::string delm = " ") :
			fileName(filename), delimeter(delm)
	{ }

	// Function to fetch data from a CSV File
	std::vector<std::vector<std::string> > getData();
};

/*
* Parses through csv file line by line and returns the data
* in vector of vector of strings.
*/
std::vector<std::vector<std::string> > CSVReader::getData()
{
	std::ifstream file(fileName);

	std::vector<std::vector<std::string> > dataList;

	std::string line = "";
	// Iterate through each line and split the content using delimeter
	while (getline(file, line))
	{
		std::vector<std::string> vec;
		boost::algorithm::split(vec, line, boost::is_any_of(delimeter));
		dataList.push_back(vec);
	}
	// Close the File
	file.close();

	return dataList;
}
void build_kernel(float sig, int window, float* kernel) 
{
	float data[window];
	float sum = 0;
	for (int i = 0; i < window; i++) 
	{
		float t = i - (window-1)/2;
		float exp = -pow(t,2)/2/pow(sig,2);
		float coef = -t/pow(sig,3)/sqrt(2*M_PI);
		data[i] = coef * pow(M_E,exp);
		sum = sum + abs(data[i]);
	}
	float factor = 1.0/sum;
	for (int i = 0; i < window; i++)
	{
		kernel[i] = data[i]*factor;
	}
}

int main()
{
	// Create out file
	std::ofstream myfile;
	myfile.open ("processed.csv");
	
	// Set up kernel
	int window_size = 40;
	float sig = window_size/8;
	float kernel[window_size];
	build_kernel(sig, window_size, kernel);

	// Creating an object of CSVWriter
	std::string file = std::string(getenv("DATA_FS")) + "/xppc00117_r136_refsub_ipm4_del3.csv";
	CSVReader reader(file);
	
	// Get the data from CSV File
	std::vector<std::vector<std::string> > dataList = reader.getData();

	std::cout << "| Row    | Rising Idx |  Falling Idx |    Volume |    Rising V |    Falling V |" << std::endl;
	// Print the content of row by row on screen
	int row = 0;
	for(std::vector<std::string> vec : dataList)
	{
		if (row < 1000) {
		int best_rising_idx = 0;
		int best_falling_idx = 0;
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
		for(std::string data : vec)
		{
			// Shift in
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
		std::cout << "\t" << row << "\t" << best_rising_idx << "\t" << best_falling_idx << "\t" << (acc_after_rising - acc_after_falling) << "\t"  << best_rising_slope << "\t" << best_falling_slope << std::endl;
		myfile << row << "," << best_rising_idx << "," << best_falling_idx << "," << (acc_after_rising - acc_after_falling) << ","  << best_rising_slope << "," << best_falling_slope << std::endl;
		}
		row = row + 1;
	}
	myfile.close();
	return 0;

}
