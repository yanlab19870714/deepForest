//########################################################################
//## Copyright 2019 Da Yan http://www.cs.uab.edu/yanda
//##
//## Licensed under the Apache License, Version 2.0 (the "License");
//## you may not use this file except in compliance with the License.
//## You may obtain a copy of the License at
//##
//## //http://www.apache.org/licenses/LICENSE-2.0
//##
//## Unless required by applicable law or agreed to in writing, software
//## distributed under the License is distributed on an "AS IS" BASIS,
//## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//## See the License for the specific language governing permissions and
//## limitations under the License.
//########################################################################

//########################################################################
//## Contributors
//## YAN, Da    (Daniel)
//########################################################################

#include "../put.h"
#include <sstream>
#include <omp.h>

char* op1put_hdfs_folder;
char* output_hdfs_folder;
int n_threads;
int col_factor;
vector<int> win_sizes;

int main(int argc, char** argv)
{
	// ====== ====== ====== ====== get argc, argv; set global variables ====== ====== ====== ======
	init_worker(&argc, &argv);
	if(argc != 9)
	{
		cout << "args:  [op1put_hdfs_folder]  [output_hdfs_folder]  [y_index]  [image_width]  [image_height]  [win_side_length_list]  [num_threads]  [col_factor]" << endl;
		// mpiexec -n 4 ./run yanda yanda2 64 8 8 3,5 4 1 2
		return -1;
	}

	op1put_hdfs_folder = argv[1];
	output_hdfs_folder = argv[2];
	y_index = atoi(argv[3]);
	WIDTH = atoi(argv[4]);
	HEIGHT = atoi(argv[5]);

	istringstream win_sides(argv[6]);
	string w;
	while(getline(win_sides, w, ',')) {
		win_sizes.push_back(atoi(w.c_str()));
	}

	// if(_my_rank==1) for(size_t i=0; i<win_sizes.size(); i++) cout<<win_sizes[i]<<endl; //for debugging

	if(_my_rank == 0){
		if(dirCheck(op1put_hdfs_folder, output_hdfs_folder, true, false) == -1) exit(-1);
		for(size_t i=0; i<win_sizes.size(); i++)
		{
			string path = output_hdfs_folder;
			path += "/" + to_string(win_sizes[i]);
			cout<<"Created directory: "<<path<<endl;
			if(dirCheck(op1put_hdfs_folder, path.c_str(), true, false) == -1) exit(-1);
		}
	}

	n_threads = atoi(argv[7]);
	col_factor = atoi(argv[8]);

	if(_my_rank==0) cout <<"Args Parsed"<<endl;

	// ====== ====== ====== ====== read data ====== ====== ====== ======
	string input_file = op1put_hdfs_folder;
	input_file += "/rows_" + to_string(_my_rank);

	hdfsFS fs = getHdfsFS();
	hdfsFile in = getRHandle(input_file.c_str(), fs);
	LineReader reader(fs, in);
	string value; //an item separated by comma
	vector<vector<double>> data_set;
	vector<string> y_labels;
	while(true)
	{
		int col_idx = 0;
		reader.readLine();
		if(reader.eof()) break;
		data_set.resize(data_set.size() + 1);
		istringstream sin(reader.getLine());
		while(getline(sin, value, ',')) {
			if(col_idx == y_index)
			{
				y_labels.push_back(value);
			}
			else
			{
				double numeric_value = boost::lexical_cast<double>(value);
				data_set.back().push_back(numeric_value);
			}
			col_idx++;
		}
	}
	hdfsCloseFile(fs, in);
	hdfsDisconnect(fs);

	/* //debug code:
	if(_my_rank == 2)
	{
		for(int i=0; i<data_set.size(); i++){
			for(int j=0; j<data_set[i].size(); j++)
				cout<<data_set[i][j]<<",";
			cout<<endl;
		}
	}
	*/

	if(_my_rank==0) cout <<"Data Loaded"<<endl;

	// ====== ====== ====== ====== slide ====== ====== ====== ======
	for(int wpos = 0; wpos < win_sizes.size(); wpos++)
	{
		int window_size = win_sizes[wpos];
		vector<vector<vector<double>>> new_dataset(n_threads); // one for each thread, to collect slided X
		vector<vector<string>> new_ylabels(n_threads); // one for each thread, to collect slided y
		//------ parallel fetching of images, on at a time for sliding
		#pragma omp parallel for num_threads(n_threads)
		for(size_t img_idx = 0; img_idx < data_set.size(); img_idx++)
		{
			int thread_index = omp_get_thread_num();
			vector<vector<double>> & result_vector = new_dataset[thread_index];
			vector<string> & y_column = new_ylabels[thread_index];
			//------
			for(int top = 0; top < HEIGHT - window_size + 1; top++) {
				for(int left = 0; left < WIDTH - window_size + 1; left++) {
					vector<double> instance;
					for(int h_idx = 0; h_idx < window_size; h_idx++) { // upto window_size
						int start = (top + h_idx) * WIDTH + left;
						int end = start + window_size;
						instance.insert(instance.end(), data_set[img_idx].begin() + start,
										data_set[img_idx].begin() + end);
					}
					result_vector.push_back(instance);
					//------
					y_column.push_back(y_labels[img_idx]);
				}
			}
		}
		//------ merge results
		vector<vector<double>> merged_dataset;
		vector<string> merged_ylabels;
		for(size_t i = 0; i < new_dataset.size(); i++) {
			vector<vector<double>> & current_X = new_dataset[i];
			merged_dataset.insert(merged_dataset.end(), current_X.begin(), current_X.end());
			vector<string> & current_y = new_ylabels[i];
			merged_ylabels.insert(merged_ylabels.end(), current_y.begin(), current_y.end());
		}
		//------ output to HDFS
		string row_file = output_hdfs_folder;
		row_file += "/" + to_string(window_size);
		row_file += "/rows_" + to_string(_my_rank);
		//put_rows(row_file.c_str(), merged_dataset, merged_ylabels); //no need as using models also reads columns like training models
		put_cols(row_file.c_str(), merged_dataset, merged_ylabels, col_factor);
		//meta
		if(_my_rank == 0)
		{
			string meta_file = output_hdfs_folder;
			meta_file += "/" + to_string(window_size) + "/meta.csv";
			put_meta(meta_file.c_str(), merged_dataset[0].size(), true);
			cout<<"Win "<<window_size<<" done"<<endl;
		}
	}

	// ====== finishing
	worker_finalize();
    return 0;
}
