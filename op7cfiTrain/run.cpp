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
	if(argc != 9)
	{
		cout << "args:  [tree_config_file (local)]  [jobConfig with data_dir1 (hdfs)]  [num_columns1]  [col_factor1]  [data_dir2 (hdfs)]  [num_columns2]  [col_factor2]  [model_dir (hdfs)]" << endl;
		return -1;
	}

	//[job_config_file (hdfs)] now only contains the first src

    WorkerParams params;

    params.job_file_path = argv[2]; //contains the HDFS paths for training data and meta data
    params.tree_file_path = argv[1];

    Worker worker;
    params.column_assignment = MODE_REPLICATE;
    params.replicate = 2;
    worker.run_CFi(params, atoi(argv[3]), atoi(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]), argv[8]);
}

// mpiexec -n 4 ./run tree.config mnist/job_CF0.config 1440 10 mnist/CF0 40 1 mnist/CF1_models
