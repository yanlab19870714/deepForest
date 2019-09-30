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

#include "../treeServer/tree.h" //load_forest_hdfs
#include "../treeServer/tree_obj.h" //TreeNode, predict
#include "../treeServer/csv.h" //load_data
#include "../treeServer/config.h" //load_job_config


char* model_dir = "mnist/mgs/3/models"; //arg1: hdfs file path for a forest model
char* datafile = "../mnist/train_mgs.csv"; //arg2: local test dataset path: hadoop fs -cat mnist/mgs/3/rows_0 mnist/mgs/3/rows_1 mnist/mgs/3/rows_2 mnist/mgs/3/rows_3 > mnist/train_mgs.csv
char* metafile = "mnist/mgs/3/meta.csv"; //arg3: hdfs metafile path
char* job_config_file = "mnist/mgs/3/job.config"; //arg4: hdfs job-config path

int RF_num = 2;
int ET_num = 2;

/* // moved to Worker
void free_forest(vector<TreeNode*> rootList)
{
	Matrix & data_set = cserver.X;

	for(size_t root_idx = 0; root_idx < rootList.size(); root_idx++) {
		free_tree(rootList[root_idx], y_index, data_set);
	}
}
*/

int main(int argc, char** argv){

	int win_side = 3;
	WIDTH = 8;
	HEIGHT = 8;
	int batch_size = (WIDTH - win_side + 1) * (HEIGHT - win_side + 1);


    Matrix & data_set = cserver.X;
    cserver.load_meta(metafile);
    cout << metafile << " loaded ..." << endl;

    load_data(datafile, data_set);
    cout << datafile << " loaded ..." << endl;

    int N = data_set.col[0]->size;
    cout << N << " rows loaded ..." << endl;

    load_job_config(job_config_file);
    cout << load_job_config << " loaded ..." << endl;

    vector<vector<TreeNode*> > RF;
    vector<vector<TreeNode*> > ET;

    RF.resize(RF_num);
    ET.resize(ET_num);
    int model_id = 0;

    for(int i=0; i<RF_num; i++)
    {
    	string path = model_dir;
    	path += "/model_" + to_string(model_id);
    	RF[i] = load_forest_hdfs(data_set, y_index, path.c_str());
    	cout <<"RF "<<i<<" loaded from model_"<<model_id<<endl;
    	model_id ++;
    }

    for(int i=0; i<ET_num; i++)
	{
		string path = model_dir;
		path += "/model_" + to_string(model_id);
		ET[i] = load_forest_hdfs(data_set, y_index, path.c_str());
		cout <<"ET "<<i<<" loaded from model_"<<model_id<<endl;
		model_id ++;
	}

    //----------------------------------------------------------------

    //one classVec list for each model
    vector<vector<vector<double> > > RF_probVecs;
    vector<vector<vector<double> > > ET_probVecs;

	RF_probVecs.resize(RF_num);
	ET_probVecs.resize(ET_num);

    for(int row_idx = 0; row_idx < N; row_idx++)
    {
    	for(int i=0; i<RF_num; i++)
		{
    		vector<TreeNode*> rootList = RF[i];
    		vector<double> output_vector;
    		predict_forest<string>(rootList, row_idx, output_vector, data_set, INT_MAX);
    		RF_probVecs[i].push_back(output_vector);
		}

		for(int i=0; i<ET_num; i++)
		{
			vector<TreeNode*> rootList = ET[i];
			vector<double> output_vector;
			predict_forest<string>(rootList, row_idx, output_vector, data_set, INT_MAX);
			ET_probVecs[i].push_back(output_vector);
		}

		if(row_idx % 1000 == 0) cout<<row_idx<<" rows processed"<<endl; //report progress
    }

    cout<<endl;

    //batching merge
    int B = N / batch_size;

    vector<vector<double> > merged_probVecs(B); //features are the same as in the paper, but their order is different
    for(int i=0; i<B; i++)
    {
    	vector<double> & cur = merged_probVecs[i];

    	// for simplicity, outer loop is connecting vectors from different slided segments:
    	for(int j=0; j<batch_size; j++)
    	{
    		int row_idx = i * batch_size + j;

    		for(int k=0; k<RF_num; k++)
			{
				vector<vector<double> > & vecs = RF_probVecs[k];
				vector<double> & vec = vecs[row_idx];
				// for simplicity, inner loop is connecting vectors from different models:
				cur.insert(cur.end(), vec.begin(), vec.end());
			}

			for(int k=0; k<ET_num; k++)
			{
				vector<vector<double> > & vecs = ET_probVecs[k];
				vector<double> & vec = vecs[row_idx];
				// for simplicity, inner loop is connecting vectors from different models:
				cur.insert(cur.end(), vec.begin(), vec.end());
			}
    	}
    }

    //print merged_probVecs
	for(int i=0; i<merged_probVecs.size(); i++)
	{
		vector<double> & merged_probVec = merged_probVecs[i];
		for(int j=0; j<merged_probVec.size(); j++)
		{
			cout<<merged_probVec[j]<<",";
		}
		cout<<endl;
	}

    /* ====== deprecated, the case before considering different slided segments
    //concat classVec lists
    vector<vector<double> > concat_probVecs(N);
    for(int row_idx = 0; row_idx < N; row_idx++)
	{
    	vector<double> & cur = concat_probVecs[row_idx];

		for(int i=0; i<RF_num; i++)
		{
			vector<vector<double> > & vecs = RF_probVecs[i];
			vector<double> & vec = vecs[row_idx];
			cur.insert(cur.end(), vec.begin(), vec.end());
		}

		for(int i=0; i<ET_num; i++)
		{
			vector<vector<double> > & vecs = ET_probVecs[i];
			vector<double> & vec = vecs[row_idx];
			cur.insert(cur.end(), vec.begin(), vec.end());
		}
	}

    //print concat_probVecs
    for(int i=0; i<concat_probVecs.size(); i++)
    {
    	vector<double> & concat_probVec = concat_probVecs[i];
    	for(int j=0; j<concat_probVec.size(); j++)
    	{
    		cout<<concat_probVec[j]<<",";
    	}
    	cout<<endl;
    }
    */ //====== deprecated, the case before considering different slided segments

    //----------------------------------------------------------------

    for(int i=0; i<RF_num; i++) free_forest(RF[i]);

	for(int i=0; i<ET_num; i++) free_forest(ET[i]);

}
