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
	if(argc != 6)
	{
		cout << "args:  [job_config_file (hdfs)]  [tree_config_file (local)]  [win_side]  [col_factor]  [model_dir (hdfs)]" << endl;
		return -1;
	}

    WorkerParams params;

    params.job_file_path = argv[1]; //contains the HDFS paths for training data and meta data
    params.tree_file_path = argv[2];

    Worker worker;
    params.column_assignment = MODE_REPLICATE;
    params.replicate = 2;
    worker.run_MGS(params, atoi(argv[3]), atoi(argv[4]), argv[5]);
}
