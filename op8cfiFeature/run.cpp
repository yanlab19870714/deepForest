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

#include "../treeServer/Worker.h"


int main(int argc, char** argv){
	if(argc != 14)
	{
		cout << "args: [hdfs_data_dir1]  [num_features1]  [input1_col_factor]  [hdfs_data_dir2]  [num_features2]  [input2_col_factor]  [output_hdfs_dir]  [output_col_factor]  [RF_num]  [ET_num]  [job_config_file]  [model_dir]  [num_threads]" << endl;
		return -1;
	}

    Worker worker;
    worker.convert_CFi(atoi(argv[9]), atoi(argv[10]), argv[1], atoi(argv[2]), atoi(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]), atoi(argv[8]), argv[7], argv[11], argv[12], atoi(argv[13]));
}

// mpiexec -n 4 ./run mnist/3 1440 10 mnist/CF0 40 1 mnist/CF1 1 2 2 mnist/job_CF0.config mnist/CF1_models 8
