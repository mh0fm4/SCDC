/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: eric
 *
 * Created on 13. Juli 2017, 17:54
 */

#include <stdlib.h>
#include <typeinfo>
#include <fstream>
#include <list>
#include <chrono>
#include <string.h>
#include "basic_webdav_fstream.h"
#include "basic_webdav_filebuf.h"

using namespace std;

void latency_test_webdav_ifstream() {

    char file[] = "https://webdav.magentacloud.de/webdav/testfiles/txt_10_3";

    char username[] = "eric.kunze@gmx.net";
    char password[] = "erku@mag2webdav";

    unsigned int buffer_size = 10;
    char buffer[buffer_size];
    webdavifstream wfs;

    wfs.rdbuf()->pubsetbuf(buffer, buffer_size);

    wfs.open(file, ios_base::in, username, password);
    string line;

    cout << endl << "~~~~~~~~~~ get line ~~~~~~~~~~~~~~" << endl;
    int i = 0;
    auto start = chrono::steady_clock::now();
    while (getline(wfs, line)) {
        i++;
        cout << i << ": " << line << '\n';

    }
    auto end = chrono::steady_clock::now();
    auto diff = end - start;
    wfs.close();
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    cout << chrono::duration <double, milli> (diff).count() << " ms for " << i << " lines" << endl;

    wfs.open(file, ios_base::in, username, password);
    start = chrono::steady_clock::now();
    i = 0;
    char c = 'x';
    while (wfs.good()) {
        i++;
        c = wfs.get();
    };
    end = chrono::steady_clock::now();
    diff = end - start;
    wfs.close();
    if (c == 'x') cout << "found a x" << endl;
    cout << chrono::duration <double, milli> (diff).count() << " ms for " << i << " chars" << endl;
}

void webdavifstream_test() {

    char file[] = "https://webdav.magentacloud.de/webdav/karl_marx_kapital.txt";

    //TODO set this via parameters
    char username[] = "eric.kunze@gmx.net";
    char password[] = "erku@mag2webdav";

    webdavifstream wfs(file, ios_base::in, username, password);
    fstream fs;
    string line;

    cout << endl << "~~~~~~~~~~ get line ~~~~~~~~~~~~~~" << endl;
    int i = 1;
    while (getline(wfs, line)) {
        cout << i << ": " << line << '\n';
        i++;
    }
    wfs.close();
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;


    cout << endl << "~~~~~~~~~~ seekg, tellg ~~~~~~~~~~" << endl;
    wfs.open(file, ios_base::in, username, password);
    streampos begin, end;
    begin = wfs.tellg();
    wfs.seekg(0, ios::end);
    end = wfs.tellg();
    cout << "size is: " << (end - begin) << " bytes.\n";
    wfs.close();
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;


}

void webdavifstream_test_localhost() {

    char file[] = "http://127.0.0.1/webdav/txt_10_4";

    webdavifstream wfs(file, ios_base::in);
    fstream fs;
    string line;

    cout << endl << "~~~~~~~~~~ get line ~~~~~~~~~~~~~~" << endl;
    int i = 1;
    while (getline(wfs, line)) {
        cout << i << ": " << line << '\n';
        i++;
    }
    wfs.close();
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;


    cout << endl << "~~~~~~~~~~ seekg, tellg ~~~~~~~~~~" << endl;
    wfs.open(file, ios_base::in);
    streampos begin, end;
    begin = wfs.tellg();
    wfs.seekg(0, ios::end);
    end = wfs.tellg();
    cout << "size is: " << (end - begin) << " bytes.\n";
    wfs.close();
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;


}

void test_read() {

    char file[] = "https://webdav.magentacloud.de/webdav/karl_marx_kapital.txt";
    //char file[] = "https://eric.kunze@gmx.net:erku@mag2webdav@webdav.magentacloud.de/webdav/karl_marx_kapital.txt";
    //TODO set this via parameters
    char username[] = "eric.kunze@gmx.net";
    char password[] = "erku@mag2webdav";

    streamsize buffersize = 512;
    streamsize content_buffersize = 1024;

    char* buffer = (char*) malloc(buffersize * sizeof (char));
    char* content_buffer = (char*) malloc(content_buffersize * sizeof (char));

    webdavifstream wfs;

    wfs.rdbuf()->pubsetbuf(buffer, buffersize);

    wfs.open(file, ios_base::in, username, password);

    wfs.get(content_buffer, content_buffersize, '@');

    wfs.close();

    free(content_buffer);
    free(buffer);


}

char read_file(char* file, int buffer_size) {

    webdavifstream wfs;
    char buffer[buffer_size];

    wfs.rdbuf()->pubsetbuf(buffer, buffer_size);
    char username[] = "eric.kunze@gmx.net";
    char password[] = "erku@mag2webdav";
    wfs.open(file, ios_base::in, username, password);

    char c = 'x';
    while (wfs.good()) {
        c = wfs.get();
    };

    wfs.close();

    return c;
}

int kb_to_byte(int kb) {
    return kb * 1024;
}

int mb_to_byte(int mb) {
    return mb * 1024 * 1024;
}

void performance_test() {
    char file[] = "https://webdav.magentacloud.de/webdav/testfiles/txt_10_3";

    auto start = chrono::steady_clock::now();
    read_file(file, mb_to_byte(1));
    auto end = chrono::steady_clock::now();
    auto diff = end - start;
    cout << chrono::duration <double, milli> (diff).count() << " ms" << endl;
}

void test_write() {

    string outfilename = "https://eric.kunze@s2014.tu-chemnitz.de:erku@tuc2cloud@webdav.magentacloud.de/webdav/test.txt";

    webdavofstream ofs(outfilename, ios_base::out | ios_base::trunc);
    string test = "geht immer noch";
    ofs << test;

    ofs.close();

}

void test_getc() {

    char file[] = "https://eric.kunze@s2014.tu-chemnitz.de:erku@tuc2cloud@webdav.magentacloud.de/webdav/txt_10_5";

    streamsize buffersize = 10;

    char buffer[10];

    for (streamsize i = 0; i < buffersize; i++) buffer[i] = 'x';

    webdavifstream wfs;

    wfs.rdbuf()->pubsetbuf(buffer, buffersize);

    wfs.open(file, ios_base::in);
    cout << "Lorem ipsum dolor sit amet" << endl;

    cout<< "buffer after open:"<<endl;
    
    cout << "buffer |";
    for (streamsize i = 0; i < buffersize; i++) {

        cout << buffer[i];

    }

    cout << "|" << endl;
    
    cout << "first get: " << (char) wfs.get() << endl;

    
    cout << "buffer |";
    for (streamsize i = 0; i < buffersize; i++) {

        cout << buffer[i];

    }

    cout << "|" << endl;
    
    char c;

    cout << "getc: |";
    for (streamsize i = 0; i < buffersize - 2; i++) {
        c = wfs.get();
        cout << c;

    }
    cout << "|" << endl;

    cout << "getc at last char: " << (char) wfs.get() << endl;

    cout << "buffer |";
    for (streamsize i = 0; i < buffersize; i++) {

        cout << buffer[i];

    }

    cout << "|" << endl;

    wfs.close();


}

int main(int argc, char** argv) {

    //webdav_explorer_test();
    //webdavifstream_test();
    //performance_test();
    //webdavstream_module_test();

    //latency_test_webdav_ifstream();

    //webdavifstream_test_localhost();
    //test_read();
    //test_write();

    test_getc();
    return 0;
}

