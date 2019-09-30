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

//====== this is a distributed example
//remember to
// 1. using our column-file put to upload [train_file_on_hdfs]
// 2. upload [job-config_on_hdfs] [meta_file_on_hdfs]

#include "csv.h"
#include "config.h"
#include "column_server.h"

using namespace std;

//* //testing Worker::run(.)
#include "Worker.h"
#include <iostream>
#include <cassert>

int main(int argc, char** argv){ //test with: mpiexec -n 4 ./run
	init_worker(&argc, &argv);

	if(_my_rank == 2)
	{
		vector<data_source> sources;
		cserver.append_meta("yanda2/3", sources);
		assert(sources.size() == 1);

		sources[0].set_params(3, 3, 2);

		cout<<"n_group = "<<sources[0].n_group<<endl;
		cserver.load_column_group(sources[0].n_group-2, sources[0]); //this is the target to test
		cserver.load_y_group(sources[0]); //this is the target to test

		for(int i=0; i<cserver.X.size; i++)
		{
			cout<<"i = "<<i<<": ";
			Column * column = cserver.X.get_column(i);
			cout<<column->size<<endl;
		}

	}

	worker_finalize();
	return 0;
}
//*/
