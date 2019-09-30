//########################################################################
//## Copyright 2018 Da Yan http://www.cs.uab.edu/yanda
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

//Acknowledgements: the operator overloading is implemented based on pregel-mpi (https://code.google.com/p/pregel-mpi/) by Chuntao Hong.

#ifndef IOSER_HDFS_H
#define IOSER_HDFS_H

#include <string.h> //for memcpy
#include "serialization.h"

#include <vector>
#include <set>
#include <map>
#include <string>

//### newly added to make the file more IO-robust
#include <iostream>
#include "../ydhdfs.h"

#define MAX_FWRITE_TRIALS 8 //how many times fwrite(.) can try to write "membuf"
//######

using namespace std;

#define STREAM_MEMBUF_SIZE 65536 //64k

//-------------------------------------

class HdfsReaderStream {

private:
    vector<char> buffer;
    size_t bufpos = 0;
    size_t bufsize = 0;

    hdfsFS fs;
    hdfsFile file_handler;
    char* pch;

    void read_buffer() {
        buffer.clear();
        bufsize = hdfsRead(fs, file_handler, &buffer[0], STREAM_MEMBUF_SIZE);
        bufpos = 0;
    }

public:

    HdfsReaderStream(const char* path)
    {
        fs = getHdfsFS();
        file_handler = getRHandle(path, fs);
        buffer.resize(STREAM_MEMBUF_SIZE);
    }

    char raw_byte()
    {
        if(bufpos == bufsize) {
            read_buffer();
        }
        return buffer[bufpos++];
    }

    char* raw_bytes(size_t n_bytes)
    {
        if(bufpos == bufsize) {
            read_buffer();
        }

        char* ret = new char[n_bytes]; //this will be deleted outside at operator>>
        size_t ret_pos = 0;

        if(bufpos + n_bytes > bufsize) {
            size_t remain = bufsize - bufpos;
            memcpy(ret + ret_pos, &buffer[0] + bufpos, remain);
            bufpos += remain;
            ret_pos += remain;

            remain = n_bytes - remain;

            while(remain > 0) {
                read_buffer();// bufsize read

                if(remain > bufsize) {
                    memcpy(ret + ret_pos, &buffer[0] + bufpos, bufsize);
                    ret_pos += bufsize;
                    bufpos += bufsize;
                    remain -= bufsize;
                } else {
                    memcpy(ret + ret_pos, &buffer[0] + bufpos, remain);
                    ret_pos += remain;
                    bufpos += remain;
                    remain -= remain;
                }
            }


        } else {
            memcpy(ret, &buffer[0] + bufpos, n_bytes);
            bufpos += n_bytes;
        }

        return ret;
    }

    void close()
    {
        hdfsCloseFile(fs, file_handler);
        hdfsDisconnect(fs);
    }
};

HdfsReaderStream & operator>>(HdfsReaderStream & m, size_t & i)
{
    char* ret =  m.raw_bytes(sizeof(size_t));
    i = *(size_t*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, bool & i)
{
    char* ret = m.raw_bytes(sizeof(bool));
    i = *(bool*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, short & i)
{
    char* ret = m.raw_bytes(sizeof(short));
    i = *(short*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, int & i)
{
    char* ret = m.raw_bytes(sizeof(int));
    i = *(int*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, float & i)
{
    char* ret = m.raw_bytes(sizeof(float));
    i = *(float*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, double & i)
{
    char* ret = m.raw_bytes(sizeof(double));
    i = *(double*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, long long & i)
{
    char* ret = m.raw_bytes(sizeof(long long));
    i = *(long long*) ret;
    delete[] ret;
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, char & c)
{
    c = m.raw_byte();
    return m;
}

template <class T>
HdfsReaderStream & operator>>(HdfsReaderStream & m, T* & p)
{
    p = new T;
    return m >> (*p);
}

template <class T>
HdfsReaderStream & operator>>(HdfsReaderStream & m, vector<T> & v)
{
    size_t size;
    m >> size;
    v.resize(size);
    for (typename vector<T>::iterator it = v.begin(); it != v.end(); ++it) {
        m >> *it;
    }
    return m;
}

template <>
HdfsReaderStream & operator>>(HdfsReaderStream & m, vector<int> & v)
{
    size_t size;
    m >> size;
    char* ret = m.raw_bytes(sizeof(int) * size);
    int* data = (int*) ret;
    v.insert(v.begin(), data, data + size);
    delete[] ret;
    return m;
}

template <>
HdfsReaderStream & operator>>(HdfsReaderStream & m, vector<double> & v)
{
    size_t size;
    m >> size;
    char* ret = m.raw_bytes(sizeof(double) * size);
    double* data = (double*) ret;
    v.insert(v.begin(), data, data + size);
    delete[] ret;
    return m;
}

template <class T>
HdfsReaderStream & operator>>(HdfsReaderStream & m, set<T> & v)
{
    size_t size;
    m >> size;
    for (size_t i = 0; i < size; i++) {
        T tmp;
        m >> tmp;
        v.insert(v.end(), tmp);
    }
    return m;
}

HdfsReaderStream & operator>>(HdfsReaderStream & m, string & str)
{
    size_t length;
    m >> length;
    str.clear();
    char* data = m.raw_bytes(length);
    str.append(data, length);
    delete[] data;

    return m;
}

template <class KeyT, class ValT>
HdfsReaderStream & operator>>(HdfsReaderStream & m, map<KeyT, ValT> & v)
{
    size_t size;
    m >> size;
    for (size_t i = 0; i < size; i++) {
        KeyT key;
        m >> key;
        m >> v[key];
    }
    return m;
}

template <class KeyT, class ValT>
HdfsReaderStream & operator>>(HdfsReaderStream & m, unordered_map<KeyT, ValT> & v)
{
    size_t size;
    m >> size;
    for (size_t i = 0; i < size; i++) {
        KeyT key;
        m >> key;
        m >> v[key];
    }
    return m;
}

template <class T>
HdfsReaderStream & operator>>(HdfsReaderStream & m, unordered_set<T> & v)
{
    size_t size;
    m >> size;
    for (size_t i = 0; i < size; i++) {
        T key;
        m >> key;
        v.insert(key);
    }
    return m;
}

class HdfsWriterStream {
private:
    vector<char> buffer;
    size_t curr_buffer_size = 0;
    hdfsFS fs;
    string path;
    hdfsFile handler;

    void flush() {
        size_t len = buffer.size();
        tSize numWritten = hdfsWrite(fs, handler, &buffer[0], len);

        if (numWritten == -1) {
            cout << "Failed to write file! File = " << __FILE__ << ", Line = " << __LINE__ << endl;
            exit(-1);
        }
        else if (numWritten != len) {
            cout << "File written but content size does not match!, File = " << __FILE__
                 << ", Line = " << __LINE__ << endl;
            exit(-1);
        }

        if (hdfsFlush(fs, handler)) {
            cout << "Error : Failed to flush : " << path << endl;
            exit(-1);
        }
    }

public:

    HdfsWriterStream(string forest_path) {
        fs = getHdfsFS();
        path = forest_path;
        handler = getWHandle(path.c_str(), fs);
    }

    void raw_byte(char c)
    {
        if(curr_buffer_size >= STREAM_MEMBUF_SIZE) {
            flush();
            buffer.clear();
            curr_buffer_size = 0;
        }

        buffer.push_back(c);
        curr_buffer_size += 1;
    }

    void raw_bytes(const void* ptr, size_t size)
    {
        if(curr_buffer_size + size >= STREAM_MEMBUF_SIZE) {
            flush();
            buffer.clear();
            curr_buffer_size = 0;
        }

        char * cptr = (char *)ptr;
        buffer.insert(buffer.end(), cptr, cptr + size);
        curr_buffer_size += size;
    }

    ~HdfsWriterStream() {
        flush();
        hdfsCloseFile(fs, handler);
        hdfsDisconnect(fs);
    }
};

HdfsWriterStream & operator<<(HdfsWriterStream & m, size_t i)
{
    m.raw_bytes(&i, sizeof(size_t));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, bool i)
{
    m.raw_bytes(&i, sizeof(bool));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, short i)
{
    m.raw_bytes(&i, sizeof(short));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, int i)
{
    m.raw_bytes(&i, sizeof(int));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, long long i)
{
    m.raw_bytes(&i, sizeof(long long));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, float i)
{
    m.raw_bytes(&i, sizeof(float));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, double i)
{
    m.raw_bytes(&i, sizeof(double));
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, char c)
{
    m.raw_byte(c);
    return m;
}

template <class T>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const T* p)
{
    return m << *p;
}

template <class T>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const vector<T>& v)
{
    m << v.size();
    for (typename vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        m << *it;
    }
    return m;
}

template <>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const vector<int> & v)
{
    m << v.size();
    m.raw_bytes(&v[0], v.size() * sizeof(int));
    return m;
}

template <>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const vector<double> & v)
{
    m << v.size();
    m.raw_bytes(&v[0], v.size() * sizeof(double));
    return m;
}

template <class T>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const set<T> & v)
{
    m << v.size();
    for(typename set<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        m << *it;
    }
    return m;
}

HdfsWriterStream & operator<<(HdfsWriterStream & m, const string & str)
{
    m << str.length();
    m.raw_bytes(str.c_str(), str.length());
    return m;
}

template <class KeyT, class ValT>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const map<KeyT, ValT> & v)
{
    m << v.size();
    for (typename map<KeyT, ValT>::const_iterator it = v.begin(); it != v.end(); ++it) {
        m << it->first;
        m << it->second;
    }
    return m;
}

template <class KeyT, class ValT>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const unordered_map<KeyT, ValT> & v)
{
    m << v.size();
    for (typename unordered_map<KeyT, ValT>::const_iterator it = v.begin(); it != v.end(); ++it) {
        m << it->first;
        m << it->second;
    }
    return m;
}

template <class T>
HdfsWriterStream & operator<<(HdfsWriterStream & m, const unordered_set<T> & v)
{
    m << v.size();
    for (typename unordered_set<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        m << *it;
    }
    return m;
}

//-------------------------------------

#endif
