import os
import timeit

# todo: now only runs on one machine, need to add hostfile support for mpi command

#////////////////////////////////////////////////////////////
# compile and executable file directories
HOME_DIRECTORY = '.'
OP1PUT_DIRECTORY = '{}/op1put/'.format(HOME_DIRECTORY)
OP2SLIDE_DIRECTORY = '{}/op2slide/'.format(HOME_DIRECTORY)
OP3MGS_TRAIN_DIRECTORY = '{}/op3mgsTrain/'.format(HOME_DIRECTORY)
OP4MGS_FEATURE_DIRECTORY = '{}/op4mgsFeature/'.format(HOME_DIRECTORY)
OP5_CF0TRAIN_DIRECTORY = '{}/op5cf1train/'.format(HOME_DIRECTORY)
OP6_CF0FEATURE_DIRECTORY = '{}/op6cf1feature/'.format(HOME_DIRECTORY)
OP7_CF_TRAIN_DIRECTORY = '{}/op7cfiTrain/'.format(HOME_DIRECTORY)
OP8_CF_FEATURE_DIRECTORY = '{}/op8cfiFeature/'.format(HOME_DIRECTORY)
TREESERVER_DIRECTORY = '{}/treeServer/'.format(HOME_DIRECTORY)
#////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////
# local data directories
LOCAL_TRAINING_DATA = '{}/mnist/train.csv'.format(HOME_DIRECTORY)
#////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////
# HDFS directories
HDFS_HOME_DIRECTORY = 'mnist_demo'
HDFS_TRAINING_DATA_DIRECTORY = '{}/data_train'.format(HDFS_HOME_DIRECTORY) # data is put from LOCAL_DATA_DIRECTORY to here
HDFS_MGS_DIRECTORY = '{}/mgs'.format(HDFS_HOME_DIRECTORY) # phase 1: multi-grained scaling
#////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////
# shared job parameters
num_workers = 4 # we run with "mpiexec -n {num_workers}", and rows are partitioned among these processes
num_compers = 8 # number of processing threads at each machine (process)

# job parameters to set for OP1 and OP2
WIDTH = 8
HEIGHT = 8
num_samples = 1198
y_index = 64 # y_index
window_list = [3, 5] # window side lengths (comma-separated)

# job parameters to set for TreeServer MGS (some are reused by cascading phase later)
max_category_number = 10 # max allowed unique category item in a feature column (for not ignoring it)
mgs_column_factor = 2 # we group columns of X into "[col_factor] * _num_worker" groups
subtree_D = 10000 # subtree_D, threshold to decide on subtree task or col_split task
BFS_PRIORITY_THRESHOLD = 80000
ACTIVE_TREE_THRESHOLD = 200
y_classes = [0,1,2,3,4,5,6,7,8,9]

# mgs tree config parameters
mgs_forests = 1 # number of forests
mgs_forests_ntrees = 20 # number of trees in a forest
mgs_forests_maxdepth = 10
mgs_forests_func = 9 # gini
mgs_forests_minleaf = 10
# ------
mgs_extratrees = 1 # number of extratrees
mgs_extratrees_ntrees = 20 # number of trees in a forest
mgs_extratrees_maxdepth = 10
mgs_extratrees_func = 9 # gini
mgs_extratrees_minleaf = 10

# cascading parameters
cf_column_factor = 10 # we group columns of X into "[col_factor] * _num_worker" groups
# ------
cf_forests = 1 # number of forests
cf_forests_ntrees = 1000 # number of trees in a forest
cf_forests_maxdepth = 2147483647
cf_forests_func = 9 # gini
cf_forests_minleaf = 1
# ------
cf_extratrees = 1 # number of extratrees
cf_extratrees_ntrees = 1000 # number of trees in a forest
cf_extratrees_maxdepth = 2147483647
cf_extratrees_func = 9 # gini
cf_extratrees_minleaf = 1

cfout_column_factor = 1 # the number of cf output features are usually small
#////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////
# init window_list string
window_lst_str = ''

for index in range(len(window_list)):
    window_lst_str = window_lst_str + str(window_list[index])
    if index < (len(window_list) - 1):
        window_lst_str = window_lst_str + ','

# init y_classes string
y_classes = [0,1,2,3,4,5,6,7,8,9]
y_classes_str = ''

for index in range(len(y_classes)):
    y_classes_str = y_classes_str + str(y_classes[index])
    if index < (len(y_classes) - 1):
        y_classes_str = y_classes_str + ','
#////////////////////////////////////////////////////////////

#////////////////////////////////////////////////////////////
# assisting functions

def compile():
	print("#################### compile started ####################")
	cmd = 'make -C {} clean;make -C {}'.format(OP1PUT_DIRECTORY, OP1PUT_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line

	cmd  = 'make -C {} clean;make -C {}'.format(OP2SLIDE_DIRECTORY, OP2SLIDE_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd  = 'make -C {} clean;make -C {}'.format(OP3MGS_TRAIN_DIRECTORY, OP3MGS_TRAIN_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd  = 'make -C {} clean;make -C {}'.format(OP4MGS_FEATURE_DIRECTORY, OP4MGS_FEATURE_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
	
	cmd  = 'make -C {} clean;make -C {}'.format(OP5_CF0TRAIN_DIRECTORY, OP5_CF0TRAIN_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
	
	cmd  = 'make -C {} clean;make -C {}'.format(OP6_CF0FEATURE_DIRECTORY, OP6_CF0FEATURE_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd  = 'make -C {} clean;make -C {}'.format(OP7_CF_TRAIN_DIRECTORY, OP7_CF_TRAIN_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
	
	cmd  = 'make -C {} clean;make -C {}'.format(OP8_CF_FEATURE_DIRECTORY, OP8_CF_FEATURE_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("#################### compile ended ####################")
	print("") # empty line


def put():
	print("#################### put training data to HDFS ####################")
	cmd = 'hadoop fs -rmr {}'.format(HDFS_HOME_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'hadoop fs -mkdir {}'.format(HDFS_HOME_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = OP1PUT_DIRECTORY+'run '
	cmd += LOCAL_TRAINING_DATA
	cmd += ' '
	cmd += HDFS_TRAINING_DATA_DIRECTORY
	cmd += ' '
	cmd += str(num_samples)
	cmd += ' '
	cmd += str(num_workers)
	print(cmd)
	os.system(cmd)
	print("") # empty line


def slide():
	print("#################### slide training data ####################")
	cmd = 'hadoop fs -rmr {}'.format(HDFS_MGS_DIRECTORY)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP2SLIDE_DIRECTORY+'run '
	cmd += HDFS_TRAINING_DATA_DIRECTORY
	cmd += ' '
	cmd += HDFS_MGS_DIRECTORY
	cmd += ' '
	cmd += str(y_index)
	cmd += ' '
	cmd += str(WIDTH)
	cmd += ' '
	cmd += str(HEIGHT)
	cmd += ' '
	cmd += str(window_lst_str)
	cmd += ' '
	cmd += str(num_compers)
	cmd += ' '
	cmd += str(mgs_column_factor)
	print(cmd)
	os.system(cmd)
	print("") # empty line


delimiter = '---------------------------------------------------'

def writeFile(file_path, content):
	print(delimiter)
	print(file_path)
	print(delimiter)
	print(content)
	print(delimiter)
	print("") # empty line

	file = open(file_path, "w")
	file.write(content)
	file.close()	
	
def prepare_win_job_config(win):
	data_num = (WIDTH - win + 1) * (HEIGHT - win + 1) * num_samples
	
	print("#################### prepare config files ####################")
	content = str(max_category_number)
	content += ' # max allowed unique category item in a feature column (for not ignoring it)\n'
	content += str(num_compers)
	content += ' # num_compers\n'
	content += str(data_num)
	content += ' # number of samples in dataset\n'
	content += str(subtree_D)
	content += ' # subtree_D, threshold to decide on subtree task or col_split task\n'
	content += '1 # NO_WARNING: 1 = true, 0 = false\n'
	content += HDFS_MGS_DIRECTORY + '/' + str(win)
	content += ' # HDFS path train file\n'
	content += 'NOT_USED # HDFS path meta file, not used in deep forest as it will be reset as train_file/meta.csv # not used in testing\n'
	content += str(win * win)
	content += ' # which column is y? this is useless in deep forest as it will be reset from data source # used in testing\n'
	content += 'job # directory for putting job logs and tree outputs\n'
	content += '0 # SAVE_TREE: 1 = true, 0 = false\n'
	content += '1 # SAVE_TREE_HDFS: 1 = true, 0 = false\n'
	content += str(BFS_PRIORITY_THRESHOLD)
	content += ' # BFS_PRIORITY_THRESHOLD, priority threshold\n'
	content += str(ACTIVE_TREE_THRESHOLD)
	content += ' # ACTIVE_TREES_THRESHOLD\n'
	content += y_classes_str
	
	writeFile('job.config', content)
	print("") # empty line	
	
	hdfs_file_path = HDFS_MGS_DIRECTORY + '/' + str(win) + '/job.config'
	
	cmd = "hadoop fs -rm " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line

	cmd = "hadoop fs -put job.config " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line


def prepare_win_job_configs():
	for win in window_list:
		prepare_win_job_config(win)


def prepare_mgs_tree_config():
	content = ""
	
	for i in range(mgs_forests):
		content += "RF # Random Forest\n"
		content += "0 # 0 is sqrt (default of sklearn)\n"
		content += "0 # 1 = true, 0 = false sample column for each node\n"
		content += str(mgs_forests_ntrees)
		content += ' # number of trees\n'
		content += str(mgs_forests_maxdepth)
		content += ' # MAX_TREE_DEPTH\n'
		content += str(mgs_forests_func)
		content += ' # IMPURITY_FUNC\n'
		content += str(mgs_forests_minleaf)
		content += ' # min-sample-leaf\n\n'
		
	for i in range(mgs_extratrees):
		content += "ET # Completely Random Trees\n"
		content += "-1 # -x means taking x columns\n"
		content += "1 # 1 = true, 0 = false sample column for each node\n"
		content += str(mgs_extratrees_ntrees)
		content += ' # number of trees\n'
		content += str(mgs_extratrees_maxdepth)
		content += ' # MAX_TREE_DEPTH\n'
		content += str(mgs_extratrees_func)
		content += ' # IMPURITY_FUNC\n'
		content += str(mgs_extratrees_minleaf)
		content += ' # min-sample-leaf\n\n'
		
	writeFile('tree.config', content)
	print("") # empty line


def mgs_train_win(win):
	job_config_file = HDFS_MGS_DIRECTORY + '/' + str(win) + '/job.config'
	model_dir = HDFS_MGS_DIRECTORY + '/' + str(win) + '/models'

	print("#################### MGS Training for Win {} ####################".format(win))
	cmd = 'hadoop fs -rmr {}'.format(model_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP3MGS_TRAIN_DIRECTORY+'run '
	cmd += job_config_file
	cmd += ' tree.config '
	cmd += str(win)
	cmd += ' '
	cmd += str(mgs_column_factor)
	cmd += ' '
	cmd += model_dir
	print(cmd)
	os.system(cmd)
	print("") # empty line


def mgs_train_wins():
	for win in window_list:
		mgs_train_win(win)
		

def mgs_convert_win(win):
	hdfs_data_dir = HDFS_MGS_DIRECTORY + '/' + str(win)
	output_hdfs_dir = HDFS_HOME_DIRECTORY + '/' + str(win)
	model_dir = hdfs_data_dir + '/models'

	print("#################### MGS Feature Extraction for Win {} ####################".format(win))
	cmd = 'hadoop fs -rmr {}'.format(output_hdfs_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP4MGS_FEATURE_DIRECTORY+'run '
	cmd += hdfs_data_dir
	cmd += ' '
	cmd += str(mgs_column_factor)
	cmd += ' '
	cmd += output_hdfs_dir
	cmd += ' '
	cmd += str(cf_column_factor)
	cmd += ' '
	cmd += str(win)
	cmd += ' '
	cmd += str(WIDTH)
	cmd += ' '
	cmd += str(HEIGHT)
	cmd += ' '
	cmd += str(mgs_forests)
	cmd += ' '
	cmd += str(mgs_extratrees)
	cmd += ' '
	cmd += str(num_compers)
	cmd += ' '
	cmd += model_dir
	print(cmd)
	os.system(cmd)
	print("") # empty line
	

def mgs_convert_wins():
	for win in window_list:
		mgs_convert_win(win)

def prepare_job_config_CF0(win):
	feature_num = (WIDTH - win + 1) * (HEIGHT - win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)
	
	print("#################### prepare config files ####################")
	content = str(max_category_number)
	content += ' # max allowed unique category item in a feature column (for not ignoring it)\n'
	content += str(num_compers)
	content += ' # num_compers\n'
	content += str(num_samples)
	content += ' # number of samples in dataset\n'
	content += str(subtree_D)
	content += ' # subtree_D, threshold to decide on subtree task or col_split task\n'
	content += '1 # NO_WARNING: 1 = true, 0 = false\n'
	content += HDFS_HOME_DIRECTORY + '/' + str(win)
	content += ' # HDFS path train file\n'
	content += 'NOT_USED # HDFS path meta file, not used in deep forest as it will be reset as train_file/meta.csv # not used in testing\n'
	content += str(feature_num)
	content += ' # which column is y? this is useless in deep forest as it will be reset from data source # used in testing\n'
	content += 'job # directory for putting job logs and tree outputs\n'
	content += '0 # SAVE_TREE: 1 = true, 0 = false\n'
	content += '1 # SAVE_TREE_HDFS: 1 = true, 0 = false\n'
	content += str(BFS_PRIORITY_THRESHOLD)
	content += ' # BFS_PRIORITY_THRESHOLD, priority threshold\n'
	content += str(ACTIVE_TREE_THRESHOLD)
	content += ' # ACTIVE_TREES_THRESHOLD\n'
	content += y_classes_str
	
	writeFile('job.config', content)
	print("") # empty line
	
	hdfs_file_path = HDFS_HOME_DIRECTORY + '/job_CF0.config'
	
	cmd = "hadoop fs -rm " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line

	cmd = "hadoop fs -put job.config " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line


def prepare_cf_tree_config():
	content = ""
	
	for i in range(cf_forests):
		content += "RF # Random Forest\n"
		content += "0 # 0 is sqrt (default of sklearn)\n"
		content += "0 # 1 = true, 0 = false sample column for each node\n"
		content += str(cf_forests_ntrees)
		content += ' # number of trees\n'
		content += str(cf_forests_maxdepth)
		content += ' # MAX_TREE_DEPTH\n'
		content += str(cf_forests_func)
		content += ' # IMPURITY_FUNC\n'
		content += str(cf_forests_minleaf)
		content += ' # min-sample-leaf\n\n'
		
	for i in range(cf_extratrees):
		content += "ET # Completely Random Trees\n"
		content += "-1 # -x means taking x columns\n"
		content += "1 # 1 = true, 0 = false sample column for each node\n"
		content += str(cf_extratrees_ntrees)
		content += ' # number of trees\n'
		content += str(cf_extratrees_maxdepth)
		content += ' # MAX_TREE_DEPTH\n'
		content += str(cf_extratrees_func)
		content += ' # IMPURITY_FUNC\n'
		content += str(cf_extratrees_minleaf)
		content += ' # min-sample-leaf\n\n'
		
	writeFile('tree.config', content)
	print("") # empty line


def cf0_train(win):
	feature_num = (WIDTH - win + 1) * (HEIGHT - win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)

	job_config_file = HDFS_HOME_DIRECTORY + '/job_CF0.config'
	model_dir = HDFS_HOME_DIRECTORY + '/CF0_models'

	print("#################### CF0 Training ####################")
	cmd = 'hadoop fs -rmr {}'.format(model_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
	    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP5_CF0TRAIN_DIRECTORY+'run '
	cmd += job_config_file
	cmd += ' tree.config '
	cmd += str(feature_num)
	cmd += ' '
	cmd += str(cf_column_factor)
	cmd += ' '
	cmd += model_dir
	print(cmd)
	os.system(cmd)
	print("") # empty line
	
def cf0_convert(win):
	feature_num = (WIDTH - win + 1) * (HEIGHT - win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)

	hdfs_data_dir = HDFS_HOME_DIRECTORY + '/' + str(win)
	output_hdfs_dir = HDFS_HOME_DIRECTORY + '/CF0'

	print("#################### CF0 Feature Extraction ####################")
	cmd = 'hadoop fs -rmr {}'.format(output_hdfs_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP6_CF0FEATURE_DIRECTORY+'run '
	cmd += hdfs_data_dir
	cmd += ' '
	cmd += str(cf_column_factor)
	cmd += ' '
	cmd += output_hdfs_dir
	cmd += ' '
	cmd += str(cfout_column_factor)
	cmd += ' '
	cmd += str(feature_num)
	cmd += ' '
	cmd += str(cf_forests)
	cmd += ' '
	cmd += str(cf_extratrees)
	cmd += ' '
	cmd += HDFS_HOME_DIRECTORY + '/job_CF0.config'
	cmd += ' '
	cmd += HDFS_HOME_DIRECTORY + '/CF0_models'
	cmd += ' '
	cmd += str(num_compers)
	print(cmd)
	os.system(cmd)
	print("") # empty line


def cf_train(prev_win, round):
	feature_num1 = (WIDTH - prev_win + 1) * (HEIGHT - prev_win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)
	feature_num2 = len(y_classes) * (cf_forests + cf_extratrees)

	job_config_file = HDFS_HOME_DIRECTORY + '/job_CF' + str(round-1) + '.config'
	model_dir = HDFS_HOME_DIRECTORY + '/CF' + str(round) + '_models'
	file2 = HDFS_HOME_DIRECTORY + '/CF' + str(round-1)

	print("#################### CF{} Training ####################".format(round))
	cmd = 'hadoop fs -rmr {}'.format(model_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
	    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP7_CF_TRAIN_DIRECTORY+'run tree.config '
	cmd += job_config_file
	cmd += ' '
	cmd += str(feature_num1)
	cmd += ' '
	cmd += str(cf_column_factor)
	cmd += ' '
	cmd += file2
	cmd += ' '
	cmd += str(feature_num2)
	cmd += ' '
	cmd += str(cfout_column_factor)
	cmd += ' '
	cmd += model_dir
	print(cmd)
	os.system(cmd)
	print("") # empty line


def cf_convert(prev_win, round):
	feature_num1 = (WIDTH - prev_win + 1) * (HEIGHT - prev_win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)
	feature_num2 = len(y_classes) * (cf_forests + cf_extratrees)

	job_config_file = HDFS_HOME_DIRECTORY + '/job_CF' + str(round-1) + '.config'
	model_dir = HDFS_HOME_DIRECTORY + '/CF' + str(round) + '_models'
	data2dir = HDFS_HOME_DIRECTORY + '/CF' + str(round-1)

	data1dir = HDFS_HOME_DIRECTORY + '/' + str(prev_win)
	output_hdfs_dir = HDFS_HOME_DIRECTORY + '/CF' + str(round)

	print("#################### CF{} Feature Extraction ####################".format(round))
	cmd = 'hadoop fs -rmr {}'.format(output_hdfs_dir)
	print(cmd)
	os.system(cmd)
	print("") # empty line
    
	cmd = 'mpiexec -n ' + str(num_workers) 
	cmd += ' '
	cmd += OP8_CF_FEATURE_DIRECTORY+'run '
	cmd += data1dir
	cmd += ' '
	cmd += str(feature_num1)
	cmd += ' '
	cmd += str(cf_column_factor)
	cmd += ' '
	cmd += data2dir
	cmd += ' '
	cmd += str(feature_num2)
	cmd += ' '
	cmd += str(cfout_column_factor)
	cmd += ' '
	cmd += output_hdfs_dir
	cmd += ' '
	cmd += str(cfout_column_factor)
	cmd += ' '
	cmd += str(cf_forests)
	cmd += ' '
	cmd += str(cf_extratrees)
	cmd += ' '
	cmd += job_config_file
	cmd += ' '
	cmd += model_dir
	cmd += ' '
	cmd += str(num_compers)
	print(cmd)
	os.system(cmd)
	print("") # empty line


def prepare_job_config_CF(win, round):
	feature_num = (WIDTH - win + 1) * (HEIGHT - win + 1) * len(y_classes) * (mgs_forests + mgs_extratrees)
	
	print("#################### prepare config files ####################")
	content = str(max_category_number)
	content += ' # max allowed unique category item in a feature column (for not ignoring it)\n'
	content += str(num_compers)
	content += ' # num_compers\n'
	content += str(num_samples)
	content += ' # number of samples in dataset\n'
	content += str(subtree_D)
	content += ' # subtree_D, threshold to decide on subtree task or col_split task\n'
	content += '1 # NO_WARNING: 1 = true, 0 = false\n'
	content += HDFS_HOME_DIRECTORY + '/' + str(win)
	content += ' # HDFS path train file\n'
	content += 'NOT_USED # HDFS path meta file, not used in deep forest as it will be reset as train_file/meta.csv # not used in testing\n'
	content += str(feature_num)
	content += ' # which column is y? this is useless in deep forest as it will be reset from data source # used in testing\n'
	content += 'job # directory for putting job logs and tree outputs\n'
	content += '0 # SAVE_TREE: 1 = true, 0 = false\n'
	content += '1 # SAVE_TREE_HDFS: 1 = true, 0 = false\n'
	content += str(BFS_PRIORITY_THRESHOLD)
	content += ' # BFS_PRIORITY_THRESHOLD, priority threshold\n'
	content += str(ACTIVE_TREE_THRESHOLD)
	content += ' # ACTIVE_TREES_THRESHOLD\n'
	content += y_classes_str
	
	writeFile('job.config', content)
	print("") # empty line
	
	hdfs_file_path = HDFS_HOME_DIRECTORY + '/job_CF' + str(round) + '.config'
	
	cmd = "hadoop fs -rm " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line

	cmd = "hadoop fs -put job.config " + hdfs_file_path
	print(cmd)
	os.system(cmd)
	print("") # empty line


#////////////////////////////////////////////////////////////
# actual running

#compile() # !!! comment this line after running the script for the first time
put() # !!! comment this line after running the script for the first time
slide()
prepare_win_job_configs()
prepare_mgs_tree_config()
mgs_train_wins()
mgs_convert_wins()
prepare_job_config_CF0(window_list[0])
prepare_cf_tree_config() # reused throughout the CF phase
cf0_train(window_list[0])
cf0_convert(window_list[0])

ROUND = 4 #max round, set by users

round = 1 #current round
pos = 0 # position in window_list
while round<=ROUND:
	prev_win = window_list[pos]
	pos += 1
	if(pos >= len(window_list)): pos = 0
	win = window_list[pos]
	print("#################### CF{} Training with Win {} ####################".format(round, win))
	cf_train(prev_win, round)
	cf_convert(prev_win, round)
	prepare_job_config_CF(win, round)
	round += 1
