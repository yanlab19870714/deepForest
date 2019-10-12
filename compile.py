import os
import timeit

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



compile() # !!! comment this line after running the script for the first time