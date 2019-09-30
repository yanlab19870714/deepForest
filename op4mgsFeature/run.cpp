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
	if(argc != 12)
	{
		cout << "args:  [hdfs_data_dir]  [input_col_factor]  [output_hdfs_dir]  [output_col_factor]  [win_side]  [WIDTH]  [HEIGHT]  [RF_num]  [ET_num]  [num_threads]  [model_dir]" << endl;
		return -1;
	}

    Worker worker;
    worker.convert_MGS(atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]), atoi(argv[9]), argv[1], atoi(argv[2]), atoi(argv[4]), argv[3], atoi(argv[10]), argv[11]);
}

// mpiexec -n 4 ./run mnist/mgs/3 2 mnist/3 10 3 8 8 2 2
