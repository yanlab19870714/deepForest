## Distributed Training of Deep Forest
This project implements a ditributed version of deep forest described in https://www.ijcai.org/proceedings/2017/0497.pdf

We build our model on top of our TreeServer project at https://github.com/yanlab19870714/treeServer

Following divide and conquer, TreeServer treats each tree node to construct as a task, and parallelizes all tasks as much as possible following the idea of divide and conquer. This allows it to use all CPU cores in a cluster to train tree models over big data. We require data to be kept on Hadoop Distributed File System for parallel loading.

For more details on how to run the system, please refer to "train.py" for training and "test.py" for testing. There, we currently runs 4 MPI processes on the local machine; we have not added the host file for MPI to run with multiple machines, but you can change the script to add that by appending "-f {hostfile}" right after "mpiexec -n 4 ".

### Configure the Running Environment
Please follow the documentation here to deploy: https://yanlab19870714.github.io/yanda/gthinker/deploy.html
We additionally require you to install the Boost C++ library. You may need to update the Makefiles to run on your platform.

### Contact
Da Yan: http://www.cs.uab.edu/yanda

Video Demo: https://www.youtube.com/watch?v=LzXkzkk0r_0

Email: yanda@uab.edu

### Contributors
CHOWDHURY, Md Mashiur Rahman    (Mashiur)

YAN, Da    (Daniel)
