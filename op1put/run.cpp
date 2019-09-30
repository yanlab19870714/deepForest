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

#include "../ydhdfs.h"

struct RowWriter {
    hdfsFS fs;
    const char* path;
    int rows_per_file;
    int nxtPart;
    int curSize; //how many rows in current file

    hdfsFile curHdl;

    RowWriter(const char* path, hdfsFS fs, int rows_per_file)
        : nxtPart(0)
        , curSize(0)
    {
        this->path = path;
        this->fs = fs;
        this->rows_per_file = rows_per_file;
        curHdl = NULL;
        //before calling the constructor, make sure "path" does not exist
        nextHdl();
    }

    ~RowWriter()
    {
        if (hdfsFlush(fs, curHdl)) {
            fprintf(stderr, "Failed to 'flush' %s\n", path);
            exit(-1);
        }
        hdfsCloseFile(fs, curHdl);
    }

    //internal use only!
    void nextHdl()
    {
        //set fileName
        char fname[20];
        strcpy(fname, "rows_");
        char buffer[10];
        sprintf(buffer, "%d", nxtPart);
        strcat(fname, buffer);
        //flush old file
        if (nxtPart > 0) {
            if (hdfsFlush(fs, curHdl)) {
                fprintf(stderr, "Failed to 'flush' %s\n", path);
                exit(-1);
            }
            hdfsCloseFile(fs, curHdl);
        }
        //open new file
        nxtPart++;
        curSize = 0;
        char* filePath = new char[strlen(path) + strlen(fname) + 2];
        strcpy(filePath, path);
        strcat(filePath, "/");
        strcat(filePath, fname);
        curHdl = getWHandle(filePath, fs);
        delete[] filePath;
    }

    void writeLine(char* line, int num)
    {
        if (curSize > rows_per_file) //file is full
        {
            nextHdl();
        }
        tSize numWritten = hdfsWrite(fs, curHdl, line, num);
        if (numWritten == -1) {
            fprintf(stderr, "Failed to write file!\n");
            exit(-1);
        }
        numWritten = hdfsWrite(fs, curHdl, newLine, 1);
        if (numWritten == -1) {
            fprintf(stderr, "Failed to create a new line!\n");
            exit(-1);
        }
        curSize ++;
    }
};

void put_rows(char* localpath, char* hdfspath, int rows_per_file)
{
    if (dirCheck(hdfspath, false) == -1)
        return;
    hdfsFS fs = getHdfsFS();
    hdfsFS lfs = getlocalFS();

    hdfsFile in = getRHandle(localpath, lfs);
    LineReader* reader = new LineReader(lfs, in);
    RowWriter* writer = new RowWriter(hdfspath, fs, rows_per_file);
    while (true) {
        reader->readLine();
        if (!reader->eof()) {
            writer->writeLine(reader->line, reader->length);
        } else
            break;
    }
    hdfsCloseFile(lfs, in);
    delete reader;
    delete writer;

    hdfsDisconnect(lfs);
    hdfsDisconnect(fs);
}

int main(int argc, char** argv)
{
	if(argc != 5)
	{
		cout << "args:  [local_path]  [hdfs_folder]  [num_rows]  [num_workers]" << endl;
		return -1;
	}

	int num_rows = atoi(argv[3]);
	int num_workers = atoi(argv[4]);
	int rows_per_file = num_rows / num_workers;
	if(num_rows % num_workers != 0) rows_per_file++;
	put_rows(argv[1], argv[2], rows_per_file);
    return 0;
}
