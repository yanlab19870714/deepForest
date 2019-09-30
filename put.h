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
//## CHOWDHURY, Md Mashiur Rahman    (Mashiur)
//## YAN, Da    (Daniel)
//########################################################################

#ifndef PUT_H
#define PUT_H

#include "ydhdfs.h"
#include "global.h"
#include <sstream>

// row file path is xxx/xxx/rows_i
// if output_cols is true, also output:
// - xxx/xxx/rows_i_cols_1
// - xxx/xxx/rows_i_cols_2
// - ...
// - xxx/xxx/rows_i_cols_g
// (g is the number of column groups)


void put_rows(const char* row_file_path, vector<vector<double>> & data_set, vector<string> & y_labels) {
	//### if "y_labels" is empty, do not output y; otherwise, output y as the last column (belonging to a single column group)

	hdfsFS fs = getHdfsFS();
	hdfsFile row_handler = getWHandle(row_file_path, fs);
	//------------------------------------------
	for(size_t index = 0; index < data_set.size(); index++) {
		vector<double> & row_data = data_set[index];
		string line = "";

		for(size_t col_index = 0; col_index < row_data.size(); col_index++) {
			line += to_string(row_data[col_index]);
			if(col_index != row_data.size() - 1) {
				line += ",";
			}
		}
		// ------
		if(y_labels.size() == 0) {
			line += '\n';
		} else {
			line += "," + y_labels[index] + '\n';
		}
		// ------
		int len;
		len = line.length();
		tSize numWritten = hdfsWrite(fs, row_handler, line.c_str(), len);
		if (numWritten == -1) {
			fprintf(stderr, "Failed to write file!\n");
			exit(-1);
		}
		else if (numWritten != len) {
			fprintf(stderr, "File written but content size does not match!\n");
			exit(-1);
		}
	}

	hdfsCloseFile(fs, row_handler);
	hdfsDisconnect(fs);
}


void put_cols(const char* out_folder_and_row_prefix, vector<vector<double>> & data_set, vector<string> & y_labels, int col_factor) {
	//### if "y_labels" is empty, do not output y; otherwise, output y as the last column (belonging to a single column group)
	hdfsFS fs = getHdfsFS();
	// get column number, and group number
    size_t col_num = data_set[0].size();
    size_t N = _num_workers * col_factor;
    int cols_per_group = col_group_size(col_num, col_factor);
    N = update_group_size(col_num, cols_per_group);

    for(int i=0; i<N; i++)
    {
    	string buffer; //will be output finally to avoid writing value by value (could be very slow)
    	size_t col_start = i * cols_per_group;
    	size_t col_end = col_start + cols_per_group;
    	if(col_end > col_num) col_end = col_num;
    	//------
    	for(size_t row_idx = 0; row_idx < data_set.size(); row_idx++)
    	{
    		for(int col_idx = col_start; col_idx < col_end; col_idx++)
			{
    			buffer += to_string(data_set[row_idx][col_idx]);
    			if(col_idx != col_end - 1) {
    				buffer += ',';
				}
			}
    		buffer += '\n';
    	}
    	//------
    	string path = out_folder_and_row_prefix;
    	path += "_cols_" + to_string(i);
    	hdfsFile handle = getWHandle(path.c_str(), fs);
    	tSize numWritten = 0;
		int len = buffer.length();
		const char * pos = buffer.c_str();
		while(len > 0)
		{
			numWritten = hdfsWrite(fs, handle, pos, len);
			if (numWritten == -1) {
				fprintf(stderr, "Failed to write file!\n");
				exit(-1);
			}
			pos += numWritten;
			len -= numWritten;
		}
		hdfsCloseFile(fs, handle);
    }

    if(y_labels.size() > 0) {
    	string buffer;
    	for(size_t row_idx = 0; row_idx < data_set.size(); row_idx++)
		{
			buffer += y_labels[row_idx];
			buffer += '\n';
		}
    	//------
    	string path = out_folder_and_row_prefix;
		path += "_col_y";
		hdfsFile handle = getWHandle(path.c_str(), fs);
		tSize numWritten = 0;
		int len = buffer.length();
		const char * pos = buffer.c_str();
		while(len > 0)
		{
			numWritten = hdfsWrite(fs, handle, pos, len);
			if (numWritten == -1) {
				fprintf(stderr, "Failed to write file!\n");
				exit(-1);
			}
			pos += numWritten;
			len -= numWritten;
		}
		hdfsCloseFile(fs, handle);
    }

    hdfsDisconnect(fs);
}

void put_file(const char* hdfs_file_path, vector<string> & lines) { //used mainly for putting meta file and job.config
    hdfsFS fs = getHdfsFS();
    hdfsFile file_handler = getWHandle(hdfs_file_path, fs);

    string line;
    for(size_t index = 0; index < lines.size(); index++) {
        line = lines[index] + '\n';
        int len = line.length();
        tSize numWritten = hdfsWrite(fs, file_handler, line.c_str(), len);
        if (numWritten == -1) {
            fprintf(stderr, "Failed to write file!\n");
            exit(-1);
        }
        else if (numWritten != len) {
            fprintf(stderr, "File written but content size does not match!\n");
            exit(-1);
        }
    }

    hdfsCloseFile(fs, file_handler);
    hdfsDisconnect(fs);
}

void put_meta(const char* hdfs_file_path, int num_Xcols, bool include_y = false) {
	vector<string> lines;
	set_meta(lines, num_Xcols, include_y);
	put_file(hdfs_file_path, lines);
}

/*
//test with -n 4
int main(int argc, char** argv)
{
	// ====== get argc, argv; set global variables
	init_worker(&argc, &argv);

	if(_my_rank == 0)
	{
		int a[16] = {1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4};
		vector<vector<double> > data_set;
		vector<double> row(16);
		for(int i=0; i<16; i++) row[i] = a[i];
		for(int i=0; i<22; i++) data_set.push_back(row);
		vector<string> ylabels;
		for(int i=0; i<22; i++) ylabels.push_back("YES");

		cout<< "data created" << endl;

		put_rows("yanda2", data_set, ylabels);

		cout<< "rows put" << endl;

		put_cols("yanda2/rows_0", data_set, ylabels);

		cout<< "columns put" << endl;
	}

	worker_finalize();
    return 0;
}
//*/

/*
//test with -n 1
int main(int argc, char** argv)
{
	put_meta("yanda2/meta2.csv", 10, true);

    return 0;
}
//*/

#endif
