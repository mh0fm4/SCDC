/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2016, 2017 Eric Kunze
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdio>
#include <cstdarg>
#include <string>
#include <map>

#include <sys/stat.h>

#include "config.hh"
#include "common.hh"
#include "log.hh"

#include "dataprov_webdav.hh"

#include <davix/davix.hpp>
#include <davix/file/davfile.hpp>
using namespace Davix;

using namespace std;

/**
 * class to encapsulate the data of the response from the server.
 */
class scdc_dataprov_webdav_response_data {
public:
    string error_message;
    string response_body; //TODO remove because not used with davix

    //this is the original version when neon was used as WebDAV library and it's main purpose was for debugging
    //    /**
    //     * result of the request function
    //     */
    //    int dispatch_result;
    //
    //    /**
    //     * http status code
    //     */
    //    int status_code;
    //
    //    /**
    //     * error message send with the http response
    //     */
    //    string error_message;
    //
    //    /**
    //     * body of the response as string
    //     */
    //    string response_body; //TODO refactor to response_body_as_str
    //    //TODO add field for responsebody as binary
    //
    //    /**
    //     * map of the revieved headers
    //     */
    //    map<string, string> response_headers;
    //
    //    scdc_dataprov_webdav_response_data()
    //    : dispatch_result(0), status_code(0) {
    //    }
    //
    //    ~scdc_dataprov_webdav_response_data() {
    //    }
    //
    //    //TODO enable this on DEBUG and return a string
    //    /*
    //    void print(){ //just for testing
    //        cout<<"********************************"<< endl;
    //        cout<< "dispatch result: "<< dispatch_result << endl;
    //        cout<< "status code: "<< status_code<< endl;
    //        cout<< "error message: "<< error_message<< endl;
    //        cout<< "headers: "<<endl;
    //
    //        map<string,string>::iterator iter = response_headers.begin();
    //
    //        for(; iter != response_headers.end(); iter++){
    //            cout<<"  "<<iter->first<<": "<<iter->second<<endl;
    //        }
    //
    //        cout<< "body: "<< endl;
    //        cout<<response_body<<endl;
    //        cout<<"********************************"<< endl;
    //    }
    //     */
};


#define SCDC_LOG_PREFIX  "dataprov-webdav-session-handler: "

/**
 * Interface for various session handlers
 */
class scdc_dataprov_webdav_session_handler {
protected:
    /**
     * Server URL
     */
    string host;
    /**
     * Location of the WebDAV directory on the server.
     */
    string base_path;
    /**
     * Protocol to connect to the server. (http or https)
     */
    string protocol; //TODO rename his to scheme
    /**
     * Struct to encapsulate the data of the response from the server.
     */
    scdc_dataprov_webdav_response_data* response_data;

public:

    virtual ~scdc_dataprov_webdav_session_handler(){};

    /**
    * Sets the parameter of the session handler.
    * @param config string formated like this:
    *
    * for http "protocol:host:base"
    *
    * for https "protocol:host:base:username:password"
    * @return false if an error occurred, true otherwise
    */
    bool set_session_handler_config(const std::string &conf);

    /**
     * Set username and password for login (only basic authentication is supported).
     * @param username
     * @param password
     */
    virtual void set_username_password(string username, string password) = 0;

    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the set configuration (protocol, host, bas path, username and password) will be created.
     * If an error occurred this method returns false and a description of the error is saved in the scdc_dataprov_webdav_response_data.
     * @return true on success, false otherwise
     * @see scdc_dataprov_webdav_response_data
     */
    virtual bool new_session() = 0;

    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the given parameters will be created.
     * If an error occurred this method returns false and a description of the error is saved in the scdc_dataprov_webdav_response_data.
     * @param protocol http or https
     * @param host WebDAV server address
     * @param base_path directory on the server
     * @param username
     * @param password
     * @return true on success, false otherwise
     */
    virtual bool new_session(string protocol, string host, string base_path, string username, string password) = 0;

    /**
     * Write content of the buffer into a previous opened file.
     * @param buffer content to be written into the file
     * @param count amount of bytes to write
     * @return the number of bytes written or -1 if an error occurred. A description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual scdcint_t write_buffer_to_file(const void* buffer, scdcint_t count) = 0;

    /**
     * Removes the file with the given url from the host.
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual bool remove_file(string url) = 0;

    /**
     * Requests the content of directory with the given url from the host.
     * @param url of the directory relative to the base path
     * @return content of the directory as string formated as name:type:size| <br>
     * If an error occurred an empty string is returned and a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual string read_dir(string url) = 0; //TODO link the type form  SCDC in docu

    /**
     * Checks if the given url is a directory that can be opened.
     * @param url of the directory relative to the base path
     * @return true if given url is an existing directory false if not or an error occurred. A description of the error is saved in the response_data.
     */
    virtual bool is_dir(string url) = 0;

    /**
     * opens the file with the given url and with the given POSIX flag
     * @param url of the file relative to the base path
     * @param open_mode POSIX flag for how to open the file
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    virtual bool open_file(string url, int open_mode) = 0;

    /**
     * closes the previous opened file
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    virtual bool close_file(void) = 0;

    /**
     * Reads buffer_size bytes from a previous opened file into the given buffer.
     * @param buffer containing the contetn after succesful reading
     * @param buffer_size size of the buffer
     * @return number of bytes read if successful. Returns 0 if end of file is reached. Returns a negative number if a error occurred and a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual scdcint_t read_into_buffer(void* buffer, scdcint_t buffer_size) = 0;

    /**
     * Moves the cursor in a previous opened file. Behavior similar to the POSIX lseek function.
     * @param flag symbolic constants \n
     * SEEK_SET => sets the cursor to the given offset \n
     * SEEK_CUR => sets the cursor to the current location plus given offset \n
     * SEEK_END => sets the cursor to the size of the file plus given offset
     * @param offset
     * @return offset in bytes from the beginning of the file if successful -1 otherwise and a description of the error is saved in the response_data.
     */
    virtual scdcint_t lseek(int flag, scdcint_t offset) = 0;

    /**
     * Returns the meta data of the file with the given url. Behavior similar to the POSIX stat function.
     * @param url of the file relative to the base path
     * @param st stuct to fill
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    virtual bool get_stat(string url, struct stat* info) = 0;

    //basic getter and setter

    virtual string get_base_path() const {
        return base_path;
    }

    virtual string get_host() const {
        return host;
    }

    virtual string get_protocol() const {
        return protocol;
    }

    virtual scdc_dataprov_webdav_response_data* get_response_data() const {
        return response_data;
    }

    virtual void set_base_path(string base_path) {
        this->base_path = base_path;
    }

    virtual void set_host(string host) {
        this->host = host;
    }

    virtual void set_protocol(string protocol) {
        this->protocol = protocol;
    }

    virtual void set_response_data(scdc_dataprov_webdav_response_data* response_data) {
        this->response_data = response_data;
    }
};


bool scdc_dataprov_webdav_session_handler::set_session_handler_config(const string & config) {
    stringlist confs(':', config);
    string protocol, tmp;

    confs.front_pop(protocol);
    set_protocol("http"); //default is HTTP

    //Host
    if (confs.front_pop(tmp)) {
        set_host(tmp);
    } else {
        SCDC_ERROR("setting session configuration: no Host specified");
        return false;
    }

    //if protocol is https a username and password is needed
    if (protocol.compare("https") == 0 || protocol.compare("HTTPS") == 0) {
        set_protocol("https");

        string username, password;

        //username
        if (!confs.front_pop(username)) {
            SCDC_ERROR("setting session configuration: no username specified");
            return false;
        }

        //password
        if (!confs.front_pop(password)) {
            SCDC_ERROR("setting session configuration: no password specified");

            return false;
        }

        set_username_password(username, password);

    }
    return true;
}

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-webdav-session-handler-davix: "

/**
 * session handler that uses the Davix library
 */
class scdc_dataprov_webdav_session_handler_davix : public scdc_dataprov_webdav_session_handler {
private:

    /**
     * main handle for Davix
     */
    Context context;

    /**
     * container for Davix request options
     */
    RequestParams request_parameters;

    /**
     * Davix file descriptor
     */
    DAVIX_FD* dav_fd;

    /**
     * filename of the open file
     */
    string name_of_open_file;

    /**
     * open flags, similar to the POSIX function open
     */
    int posix_open_mode;

    /**
     * Returns an formated URI "<protocol>://<host><base_path><path>"
     * @param path of the resource relative to the base path
     * @return
     */
    Uri get_davix_uri(string path);

    /**
     * Transfer the information from DavixError to the response data.
     * @param err Davix Error
     */
    void set_error_to_response_data(DavixError* err);

    /**
     * Set error information to the response data
     * @param scope where occurred the error
     * @param msg error messge
     */
    void set_error_to_response_data(string scope, string msg);

    /**
     * Tries to connect to the server with the set configuration.
     * If an error occurred this method returns false and a description of the error is saved in the scdc_dataprov_webdav_response_data.
     * @return true on success, false otherwise
     */
    bool check_configuration();

    /**
     * Requests the content of file with the given url from the host and writes it into the given buffer.
     * @param url of the file relative to the base path
     * @param buffer
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool get_file_content(string url, vector<char> *buffer);

    /**
     * Opens the file with the given url and the given mode.
     * @param url of the file relative to the base path
     * @param open_mode open flags, similar to the POSIX function open
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool open_file_with_davix(string url, int open_mode);

    /**
     * Deletes the file with the given url on the server and opens a new one with the same url and the given mode.
     * @param url of the file relative to the base path
     * @param open_mode open flags, similar to the POSIX function open
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool open_file_to_write_trunc(string url, int open_mode);

    /**
     * Reads the content of the file with the given url. After this the old file is removed from the server.
     * A new file with the same url is created and the old content is copied into this file on the local machine.
     * @param url of the file relative to the base path
     * @param open_mode open flags, similar to the POSIX function open
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool open_file_to_write_append(string url, int open_mode);

    /**
     * Returns the open mode flags (similar to the POSIX function open) as a string
     * @param open_mode open flags, similar to the POSIX function open
     * @return the flags as string
     */
    string get_open_mode_as_string(int open_mode);

public:

    scdc_dataprov_webdav_session_handler_davix();
    ~scdc_dataprov_webdav_session_handler_davix();

    //virtual methods from super class
    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the set configuration will be created.
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool new_session() override;

    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the given parameters will be created.
     * If an error occurred this method returns false and a description of the error is saved in the scdc_dataprov_webdav_response_data.
     * @param protocol http or https
     * @param host WebDAV server address
     * @param base_path directory on the server
     * @param username
     * @param password
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool new_session(string protocol, string host, string base_path, string username = "", string password = "") override;

    /**
     * Set username and password for login.
     * @param username
     * @param password
     */
    void set_username_password(string username, string password) override;

    /**
     * opens the file with the given url ad with the given POSIX flag
     * @param url of the file relative to the base path
     * @param open_mode POSIX flag for how to open the file
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool open_file(string url, int open_mode) override;

    /**
     * closes the previous opened file
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool close_file(void) override;

    /**
     * Requests the content of directory with the given url from the host.
     * @param url of the directory relative to the base path
     * @return content of the directory as string formated as name:type:size| <br>
     * If an error occurred an empty string is returned and a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    string read_dir(string url) override;

    /**
     * Checks if the given url is a directory that can be opened.
     * @param url of the directory relative to the base path
     * @return true if given url is an existing directory false if not or an error occurred. A description of the error is saved in the response_data.
     */
    bool is_dir(string url) override;

    /**
     * Reads buffer_size bytes from a previous opened file into the given buffer.
     * @param buffer containing the contetn after succesful reading
     * @param buffer_size size of the buffer
     * @return number of bytes read if successful. Returns 0 if end of file is reached. Returns a negative number if a error occurred and a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    scdcint_t read_into_buffer(void* buffer, scdcint_t buffer_size) override;

    /**
     * move the cursor in a previous opened file. Behavior similar to the POSIX lseek function.
     * @param flag symbolic constants \n
     * SEEK_SET => sets the cursor to the given offset \n
     * SEEK_CUR => sets the cursor to the current location plus given offset \n
     * SEEK_END => sets the cursor to the size of the file plus given offset
     * @param offset
     * @return offset in bytes from the beginning of the file if successful -1 otherwise and a description of the error is saved in the response_data.
     */
    scdcint_t lseek(int flag, scdcint_t offset) override;

    /**
     * Write content of the buffer into a previous opened file.
     * @param buffer content to be written into the file
     * @param count amount of bytes to write
     * @return the number of bytes written or -1 if an error occurred. A description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    scdcint_t write_buffer_to_file(const void* buffer, scdcint_t count) override;

    /**
     * Removes the file with the given url from the host.
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise
     */
    bool remove_file(string url) override;

    /**
     * Returns the meta data of the file with the given url. Behavior similar to the POSIX stat function.
     * @param url of the file relative to the base path
     * @param st stuct to fill
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     */
    bool get_stat(string url, struct stat* info) override;

    void set_base_path(string base_path) override;
};

scdc_dataprov_webdav_session_handler_davix::scdc_dataprov_webdav_session_handler_davix() {
    base_path = "/";
    posix_open_mode = -1;
    dav_fd = NULL;
    response_data = new scdc_dataprov_webdav_response_data();
}

scdc_dataprov_webdav_session_handler_davix::~scdc_dataprov_webdav_session_handler_davix() {
    if (!close_file()) {
        SCDC_FAIL("Error while closing file: " << response_data->error_message);
    }

    delete response_data;
}

void scdc_dataprov_webdav_session_handler_davix::set_username_password(const string username, const string password) {
    request_parameters.setClientLoginPassword(username, password);
}

Uri scdc_dataprov_webdav_session_handler_davix::get_davix_uri(string path) {
    return Uri(protocol + "://" + host + base_path + path);
}

void scdc_dataprov_webdav_session_handler_davix::set_error_to_response_data(DavixError* err) {
    set_error_to_response_data(err->getErrScope(), err->getErrMsg());
    err->clearError(&err);
}

void scdc_dataprov_webdav_session_handler_davix::set_error_to_response_data(string scope, string msg) {
    delete response_data;
    response_data = new scdc_dataprov_webdav_response_data();
    response_data->error_message = "Scope: " + scope + " | Error message: " + msg;
}

void scdc_dataprov_webdav_session_handler_davix::set_base_path(string path) {
    if (path.find_first_of("/", 0) != 0) { //base path has to start and end with '/'
        path = "/" + path;
    }

    if (path[path.size() - 1] != '/') {
        path += "/";
    }
    base_path = path;
}

bool scdc_dataprov_webdav_session_handler_davix::new_session(string protocol, string host, string base_path, string username, string password) {
    this->protocol = protocol;
    this->host = host;
    this->base_path = base_path;

    if ((!username.empty()) || (!password.empty())) {
        set_username_password(username, password);
    }
    return new_session();
}

bool scdc_dataprov_webdav_session_handler_davix::new_session() {

    if (!close_file()) { //close file if one was opened before
        return false;
    }

    return check_configuration();
}

bool scdc_dataprov_webdav_session_handler_davix::check_configuration() {
    struct stat info;
    return get_stat("", &info); //read info from the base path
}

bool scdc_dataprov_webdav_session_handler_davix::open_file(string path, int open_mode) {
    SCDC_INFO("opening '" << path << "' with " << get_open_mode_as_string(open_mode));

    if (open_mode == 0) { //open file as read-only (open_mode & O_RDONLY) doesn't work because 0 & 0 => 0
        return open_file_with_davix(path, open_mode);

    } else if (open_mode & O_WRONLY || open_mode & O_RDWR) { //open file to write or to read and write

        //check if file exists
        struct stat info;
        if (get_stat(path, &info)) {

            if (open_mode & O_TRUNC) { //open to write an delete previous content
                return open_file_to_write_trunc(path, open_mode);

            } else if (open_mode & O_APPEND) { //append the existing file
                //if O_TRUNC and O_APPEND are both set only O_TRUNC is used!
                return open_file_to_write_append(path, open_mode);

            } else {
                //no O_TRUNC and no O_APPEND is given
                //open file to write and keep its content and set the cursor to the beginning of the file
                if (open_file_to_write_append(path, open_mode)) {
                    if (lseek(SEEK_SET, 0) < 0) { //set cursor to beginning of the file
                        //error message is already set in lseek()
                        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
                        return false;
                    } else {
                        return true;
                    }
                } else {
                    //error message is already set in open_file_to_write_append()
                    return false;
                }
            }
        } else { //file doesn't exist
            if (open_mode & O_CREAT) {
                return open_file_with_davix(path, open_mode);
            } else {
                set_error_to_response_data(__PRETTY_FUNCTION__, "The file '" + path + "' doesn't exist and O_CREAT wasn't set!");
                return false;
            }
        }

    } else { //no mode or the wrong mode was given
        set_error_to_response_data(__PRETTY_FUNCTION__, "invalid open mode for file '" + path + "'");
        return false;
    }
}

string scdc_dataprov_webdav_session_handler_davix::get_open_mode_as_string(int open_mode) {

    string mode_as_string = "";

    if (open_mode == 0)
        mode_as_string = "O_RDONLY"; // (open_mode & O_RDONLY) doesn't work because 0 & 0 => 0
    else if (open_mode & O_WRONLY)
        mode_as_string = "O_WRONLY";
    else if (open_mode & O_RDWR)
        mode_as_string = "O_RDWR";
    else
        mode_as_string = "UNKNOWN";

    mode_as_string += (open_mode & O_CREAT) ? " O_CREAT" : "";
    mode_as_string += (open_mode & O_TRUNC) ? " O_TRUNC" : "";
    mode_as_string += (open_mode & O_APPEND) ? " O_APPEND" : "";

    return mode_as_string;
}

bool scdc_dataprov_webdav_session_handler_davix::open_file_with_davix(string path, int open_mode) {
    DavixError* err = NULL;
    DavPosix dav_posix_file(&context);

    dav_fd = dav_posix_file.open(&request_parameters, get_davix_uri(path).getString(), open_mode, &err);

    if (err == NULL) {
        name_of_open_file = path;
        posix_open_mode = open_mode;
        return true;
    } else {
        set_error_to_response_data(err);
        return false;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::open_file_to_write_trunc(string path, int open_mode) {
    //delete old file because it will get overwritten anyway
    //TODO check if this is practical (in a multi user scenario one could write a file with the same name while the other is creating or deleting it -> LOCK)

    if (!remove_file(path)) {
        //response_data is already set in remove_file()
        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
        return false;
    }

    int tmp_open_mode = open_mode | O_CREAT; // the O_CREAT flag is needed to create a new file

    bool result = open_file_with_davix(path, tmp_open_mode);

    posix_open_mode = open_mode; //set this to the given open mode

    return result;
}

bool scdc_dataprov_webdav_session_handler_davix::open_file_to_write_append(string path, int open_mode) {
    //open the old file and get it's content
    vector<char> buffer;

    if (!get_file_content(path, &buffer)) {
        //response_data is already set in get_file_content()
        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
        return false;
    }

    //remove old file
    if (!remove_file(path)) {
        //response_data is already set in remove_file()
        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
        return false;
    }

    int tmp_open_mode = open_mode | O_CREAT; // the O_CREAT flag is needed to create a new file

    bool result = open_file_with_davix(path, tmp_open_mode); //open new file

    posix_open_mode = open_mode; //set this to the given open mode

    if (!result) {
        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
        return false;
    }

    //write old content to new file
    if (!write_buffer_to_file(buffer.data(), buffer.size())) {
        //error -> response_data is set in write_buffer_to_file()
        response_data->error_message += "\n This error occurred while opening file '" + path + "' with flags: " + get_open_mode_as_string(open_mode);
        response_data->error_message += " The new file is open but the content of the old file is lost.";
        //TODO save the content of the old file in a local file so no data gets lost
        return false;
    }

    return true;
}

bool scdc_dataprov_webdav_session_handler_davix::close_file() {

    if (dav_fd == NULL) {
        return true;
    }

    DavixError* err = NULL;
    DavPosix dav_posix_file(&context);

    dav_posix_file.close(dav_fd, &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {
        dav_fd = NULL;
        name_of_open_file = "";
        posix_open_mode = -1; 
        return true;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::get_file_content(string path, vector<char> *buffer) {
    DavixError* err = NULL;

    if (dav_fd != NULL && path.empty()) { //if a file is already open
        path = name_of_open_file;
    }

    DavFile file(context, request_parameters, get_davix_uri(path));

    file.getFull(NULL, *buffer, &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {

        return true;
    }
}

scdcint_t scdc_dataprov_webdav_session_handler_davix::read_into_buffer(void* buffer, scdcint_t buffer_size) {
    DavixError* err = NULL;

    DavPosix dav_posix_file(&context);
    scdcint_t num_of_bytes = dav_posix_file.read(dav_fd, buffer, buffer_size, &err);

    if (err || num_of_bytes < 0) {
        set_error_to_response_data(err);
        return -1;
    } else {

        return num_of_bytes;
    }
}

scdcint_t scdc_dataprov_webdav_session_handler_davix::lseek(int flag, scdcint_t offset) {
    DavixError* err = NULL;

    DavPosix dav_posix_file(&context);

    scdcint_t num_of_bytes = dav_posix_file.lseek(dav_fd, offset, flag, &err);

    if (err || num_of_bytes < 0) {
        set_error_to_response_data(err);
        return -1;
    } else {

        return num_of_bytes;
    }
}

scdcint_t scdc_dataprov_webdav_session_handler_davix::write_buffer_to_file(const void* buffer, scdcint_t count) {

    //doesn't work when file already exists

    DavixError* err = NULL;

    DavPosix dav_posix_file(&context);

    scdcint_t num_of_bytes = dav_posix_file.write(dav_fd, buffer, count, &err);

    if (err || num_of_bytes < 0) {
        set_error_to_response_data(err);
        return -1;
    } else {

        return num_of_bytes;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::remove_file(string path) {
    DavixError* err = NULL;

    if (dav_fd != NULL && path.empty()) { //if a file is already open
        path = name_of_open_file; 
        close_file();
    }

    DavFile file(context, request_parameters, get_davix_uri(path));
    file.deletion(NULL, &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {

        return true;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::is_dir(string path) {

    if (path.empty())
        return false;

    DavixError* err = NULL;
    DavPosix pos(&context);

    DAVIX_DIR* fd;

    //base path has to start with '/'
    if (path.find_first_of("/", 0) != 0) {
        path = "/" + path;
    }

    //base path has to end with '/'
    if (path[path.size() - 1] != '/') {
        path += "/";
    }

    Uri url(protocol + "://" + host + path);

    fd = pos.opendir(&request_parameters, url.getString(), &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {
        pos.closedir(fd, &err);
        if (err) { //thats unfortunate because the requested directory exists but we have an error while closing :(
            set_error_to_response_data(err);
            return false;
        } else {
            return true;
        }
    }
}

string scdc_dataprov_webdav_session_handler_davix::read_dir(string path) {

    DavixError* err = NULL;
    DavPosix pos(&context);

    DAVIX_DIR* fd;
    struct dirent* entry;
    struct stat info;
    char buffer[1024];
    string res = "";

    SCDC_INFO(get_davix_uri(path).getString());

    //opend dir
    fd = pos.opendir(&request_parameters, get_davix_uri(path).getString(), &err);

    if (err) { //check for errors
        set_error_to_response_data(err);
        return string();
    }

    //read content into string
    while ((entry = pos.readdirpp(fd, &info, &err)) != NULL) {
        res.append(entry->d_name);

        if (S_ISDIR(info.st_mode)) { //directory
            res.append(":d"); //TODO use SCDC types
        } else if (S_ISREG(info.st_mode)) { //regular file
            res.append(":f"); //TODO use SCDC types
            res.append(":");

            snprintf(buffer, 1024, "%jd", (intmax_t) info.st_size);
            res.append(buffer);
        } else {
            res.append(":?");
        }

        res.append("|");
    }

    if (err) { //check for errors
        set_error_to_response_data(err);

        //try close dir
        DavixError* err_close = NULL;
        pos.closedir(fd, &err_close);

        if (err_close) { //closing didn't work which is very unfortunate :(
            response_data->error_message += "\n another error occurred while closing the directory:\n";
            response_data->error_message += err_close->getErrScope() + " Error: " + err_close->getErrMsg();
        }

        return string();
    }

    //close dir
    pos.closedir(fd, &err);

    if (err) { //check for errors
        set_error_to_response_data(err);
        return string();

    } else { //everything went well

        if (res.empty()) { //if the checked directory contains any files or subdirectories
            res = " ";
        }

        return res;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::get_stat(string path, struct stat * info) {
    DavixError* err = NULL;
    DavPosix pos(&context);

    pos.stat(&request_parameters, get_davix_uri(path).getString(), info, &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {

        return true;
    }
}

#undef SCDC_LOG_PREFIX

/**
 * Factory class that has only on static method which returns a session handler according to the selected library (neon or Davix).
 *
 */
class scdc_dataprov_webdav_session_handler_factory {
public:

    /**
     * Returns a new session handler
     * @return session handler
     */
    static scdc_dataprov_webdav_session_handler* get_new_session_hander() {
        return new scdc_dataprov_webdav_session_handler_davix();
    }

    //use this if a implementation with neon (or any other webdav implementation) is available
    //    static scdc_dataprov_webdav_session_handler* get_new_session_hander() {
    //#if USE_DAVIX
    //        return new scdc_dataprov_webdav_session_handler_davix();
    //#elif USE_NEON
    //        return new scdc_dataprov_webdav_session_handler_neon();
    //#else
    //#error no suitable WebDAV implementation available
    //#endif
    //    };
};


#define SCDC_LOG_PREFIX  "dataset-webdav-access: "

class scdc_dataset_webdav_access : public scdc_dataset {
public:

    scdc_dataset_webdav_access(scdc_dataprov *dataprov_) : scdc_dataset(dataprov_) {
    };

    /**
     * Reads meta data of the file with the given url. Behavior similar to the POSIX stat function.
     * If no url is given and a  file was opened with de do_cmd_open method the meta data of this file is read.
     * @param params url of the directory relative to the base path.
     * @param input not used
     * @param output the buf struct points to a big enough buffer to hold a stat struct which is set after success.
     * If an error occurs the buffer contains the error message. The size is checked via the buf.size attribute.
     * @return true on success and false if an error occurred
     */
    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Changes the working directory (base path) on the host.
     * If the given path is invalid the old value is restored.
     * @param params base path
     * @param input not used here
     * @param output the buf struct points to a big enough buffer to save an error message for errors that may occur (the size is checked via the buf.size attribute).
     * @return true on success and false if an error occurred
     */
    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Requests the content of directory with the given url from the WebDAV server.
     * @param params url of the directory relative to the base path
     * if the params string is empty the content of the base path read
     * @param input not used
     * @param output the buf struct points to a big enough buffer (the size is checked via the buf.size attribute) to save content of the directory as string formated as name:type:size| \n
     * : is used as a separator for the attributes of a file or directory\n
     * | is used as a separator for the different files or directories
     * total_size (and the buf.current) attribute contains the length of the string
     * @return true on success and false if an error occurred
     */
    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Writes the content of the given buffer (via input) to the previous opened file (use do_cmd_open).
     * @param params not used
     * @param input the buf struct points to a buffer that contain the data to write.  The next field is used to extract all data if set.
     * On success total_size attributes holds the amount of bytes successfully written to the file.
     * @param output the buf struct points to a big enough buffer to save an error message for errors that may occur (the size is checked via the buf.size attribute).
     * @return true on success and false if an error occurred
     */
    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Reads data from a previous opened file (use do_cmd_open) into the given buffer specified in the input parameter.\n
     * If buf.current is 0 EOF is reached and the next function is set to NULL. \n
     * Even if plain text is read the content written into the buffer is not null terminated!
     * (Use the buf.current attribute to determine how many bytes have been written.) \n
     * Use the next function of the output parameter to download more data (e.g. when buf.current equals the size of the given buffer).
     * The next function returns 1 (SCDC_SUCCESS) on success and 0 (SCDC_FAILURE) if an error occurred.
     * In case of an error a error message is saved in the buf attribute of the output. (Does not work with next function!)
     * @param params no extra parameters used
     * @param input the buf struct points to a buffer with a minimum size of 1 byte and the set buf.size must contain the size of this buffer.
     * After execution the buffer contains the data downloaded from the file.
     * The amount of written bytes to the pointer is saved in buf.current.
     * If the next pointer is not NULL more data from the file can be downloaded and EOF is not reached yet.
     * @param output if set informations about the error that may occur are saved in the given buffer (if the buffer is to small the error message may not be complete).
     * @return SCDC_SUCCESS on success and SCDC_FAILURE if an error occurred and the given buffer (via output) then contains a description of the error.
     * @see use do_cmd_open do_cmd_lseek do_cmd_close
     */
    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Removes the the given file from the WebDAV server.
     * @param params the url of the file relative to the base path as string
     * @param input is not used here
     * @param output contains information about errors that may occur if the buf attribute points to a big enough buffer
     * @return true on success and false if an error occurred
     */
    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Move the cursor in a previous opened file (use do_cmd_open).
     * Behavior similar to the POSIX lseek function.
     * @param params must contain one of those commands as string:\n
     * SEEK_SET => sets the cursor to the given offset \n
     * SEEK_CUR => sets the cursor to the current location plus given offset \n
     * SEEK_END => sets the cursor to the size of the file plus given offset \n
     * and the offset in bytes (can be as big as long long) separated by a whitespace\n
     * example:  SEEK_SET 1100
     * @param input  is not used here
     * @param output if a big enough buffer is set via the buf attribute it contains the resulting offset
       location as measured in bytes from the beginning of the file as a scdcint_t.\n
     * If an error occurred and a buffer is set it contains a description of the error.
     * @return true on success and false if an error occurred
     */
    bool do_cmd_lseek(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Opens a file for read/write operation in a POSIX-like approach.
     * Behavior similar to the POSIX open function.
     * @param params the url of the file relative to the base path as string and the open mode as an integer separated by a whitespace.\n
     * The open mode is similar to the flag of the POSIX function open.
     * @param input not used here
     * @param output contains a description of the error if one occurred and if the buf attribute contains a buffer.
     * @return true on success and false if an error occurred
     */
    bool do_cmd_open(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Closes the previous opened file (use do_cmd_open).
     * @param params not used
     * @param input not used
     * @param output contains a description of the error if one occurred.
     * @return true on success and false if an error occurred
     */
    bool do_cmd_close(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

private:
    /**
     * Sets the given result to the output struct.
     * @param result of the request
     * @param output output struct
     */
    static void set_string_result_to_inout(string result, scdc_dataset_inout_t *output);

    /**
     * Sets the error information from the response data to the output struct.
     * @param dataprov_webdav dataprovider
     * @param output output struct
     */
    static void set_response_error_to_inout(scdc_dataprov_webdav_session_handler *session_handler, scdc_dataset_inout_t *output);

    /**
     * next function for reading a file
     * @param inout dataset
     * @return SCDC_SUCCSES if read was successful, SCDC_FAILURE otherwise and an error information is ste to the given dataset
     */
    static scdcint_t dataset_webdav_read_file_next(scdc_dataset_inout_t *inout);
};

bool scdc_dataset_webdav_access::do_cmd_info(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_info: '" << params << "'");

    if (output == NULL || SCDC_DATASET_INOUT_BUF_PTR(output) == NULL) {
        SCDC_FAIL("do_cmd_info: Output must contain a buffer for the result big enough to hold a struct stat!");
        return SCDC_FAILURE;
    }

    if (static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(output)) < sizeof (struct stat)) {
        SCDC_FAIL("do_cmd_info: The given buffer is to small! Must at least be " << sizeof (struct stat) << " bytes.");
        return SCDC_FAILURE;
    }

    struct stat* st = static_cast<struct stat*> (SCDC_DATASET_INOUT_BUF_PTR(output));

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    if (dataprov_webdav->session_handler->get_stat(params, st)) {
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

bool scdc_dataset_webdav_access::do_cmd_cd(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {

    SCDC_TRACE("do_cmd_cd: '" << params << "'");

    //SCDC_DATASET_OUTPUT_CLEAR(output);

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    if (params.empty()) {
        SCDC_ERROR("do_cmd_cd: the given 'params' are empty! You need to give a directory.");
        return SCDC_FAILURE;
    }

    if (dataprov_webdav->session_handler->is_dir(params)) {
        dataprov_webdav->session_handler->set_base_path(params);
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

bool scdc_dataset_webdav_access::do_cmd_ls(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_ls: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    if (output == NULL || SCDC_DATASET_INOUT_BUF_PTR(output) == NULL) {
        SCDC_FAIL("do_cmd_ls: Output must contain a buffer to store the result!");
        return SCDC_FAILURE;
    }

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    string result = dataprov_webdav->session_handler->read_dir(params);

    if (!result.empty()) {
        if (static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(output)) < result.size()) { //FIXEME implement next function for output for do_cmd_ls
            SCDC_ERROR("do_cmd_ls: the given buffer is to small to hold all the information.");
        }
        set_string_result_to_inout(result, output);
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

bool scdc_dataset_webdav_access::do_cmd_put(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_put: '" << params << "'");

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    if (SCDC_DATASET_INOUT_BUF_PTR(input) == NULL) { //check buffer
        SCDC_FAIL("do_cmd_put: Input must contain pointer to buffer with minimum size 1! (use buf and total_size attributes)");
        return SCDC_FAILURE;
    }

    if (SCDC_DATASET_INOUT_BUF_CURRENT(input) > SCDC_DATASET_INOUT_BUF_SIZE(input)) { //check buffer size
        SCDC_FAIL("do_cmd_put: incorrect input: buf.current is bigger than buf.size!");
        return SCDC_FAILURE;
    }

    scdcint_t total_num_of_bytes = dataprov_webdav->session_handler->write_buffer_to_file(SCDC_DATASET_INOUT_BUF_PTR(input), SCDC_DATASET_INOUT_BUF_CURRENT(input));

    if (total_num_of_bytes < 0) {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        input->total_size = total_num_of_bytes;
        return SCDC_FAILURE;
    }

    //SCDC_INFO("buffer size: " << input->buf.size); //doesn't work! -> request for member ‘size’ in ‘input->_scdc_dataset_inout_t::buf’, which is of non-class type ‘void*

    while (input->next) {
        input->next(input);

        scdcint_t current_num_of_bytes = dataprov_webdav->session_handler->write_buffer_to_file(SCDC_DATASET_INOUT_BUF_PTR(input), SCDC_DATASET_INOUT_BUF_CURRENT(input));

        if (current_num_of_bytes < 0) {
            set_response_error_to_inout(dataprov_webdav->session_handler, output);
            //TODO check the status of the file after this because some bytes may have been written to the tmp-file (at least close it maybe)
            input->total_size = total_num_of_bytes;
            return SCDC_FAILURE;

        } else {
            total_num_of_bytes += current_num_of_bytes;
        }
    }

    input->total_size = total_num_of_bytes;

    return SCDC_SUCCESS;
}

bool scdc_dataset_webdav_access::do_cmd_get(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_get: '" << params << "'");

    if (output == NULL || SCDC_DATASET_INOUT_BUF_PTR(output) == NULL || SCDC_DATASET_INOUT_BUF_SIZE(output) < 1) {
        SCDC_FAIL("do_cmd_get: Output must contain buffer with minimum size of one byte!");
        return false;
    }

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    output->data = static_cast<scdc_dataprov_webdav_access *> (dataprov);
    output->next = dataset_webdav_read_file_next;

    //request the first data from the given file
    if (output->next(output)) { //check for error
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

scdcint_t scdc_dataset_webdav_access::dataset_webdav_read_file_next(scdc_dataset_inout_t * inout) {

    if (inout == NULL) {
        return SCDC_FAILURE;
    }

    if (inout->data == NULL) {
        set_string_result_to_inout("dataset_webdav_read_file_next: the data attribute was NULL (should be a pointer to a scdc_dataprov_webdav_access object)", inout);
        return SCDC_FAILURE;
    }

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (inout->data);

    //read next bytes of data from the file
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = dataprov_webdav->session_handler->read_into_buffer(SCDC_DATASET_INOUT_BUF_PTR(inout), SCDC_DATASET_INOUT_BUF_SIZE(inout));


    if (SCDC_DATASET_INOUT_BUF_CURRENT(inout) <= 0) { //0 is eof, negative number is error
        inout->data = NULL;
        inout->next = NULL;

        //error occurred
        if (SCDC_DATASET_INOUT_BUF_CURRENT(inout) < 0) {
            //this would override the content of the buffer with the error message which is maybe not wanted           
            //set_response_error_to_inout(dataprov_webdav->session_handler, NULL); //uses SCDC_ERROR() 
            //FIXME export error message
            return SCDC_FAILURE;
        }
    }

    //TODO set inout->total_size and inout->total_size_given

    return SCDC_SUCCESS;
}

bool scdc_dataset_webdav_access::do_cmd_rm(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_rm: '" << params << "'");

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    if (dataprov_webdav->session_handler->remove_file(params)) {
        return true;

    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);

        return false;
    }
}

bool scdc_dataset_webdav_access::do_cmd_lseek(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_lseek: '" << params << "'");

    //separate flag and offset from the params
    stringlist confs(' ', params);
    string flag_as_string = confs.front_pop();
    string offset_as_string = confs.front_pop();

    int flag;

    if (flag_as_string.compare("SEEK_SET") == 0) {
        flag = SEEK_SET;
    } else if (flag_as_string.compare("SEEK_CUR") == 0) {
        flag = SEEK_CUR;
    } else if (flag_as_string.compare("SEEK_END") == 0) {
        flag = SEEK_END;
    } else {
        SCDC_FAIL("do_cmd_lseek: the flag '" << flag_as_string << "' is not supported");
        return SCDC_FAILURE;
    }

    scdcint_t offset;
    try {
        offset = stoll(offset_as_string);
    } catch (const std::invalid_argument& ex) {
        SCDC_FAIL("do_cmd_lseek: given offset could not be converted into an integer: " << ex.what());
        return SCDC_FAILURE;
    } catch (const std::out_of_range& ex) {
        SCDC_FAIL("do_cmd_lseek: given offset could not be converted into an integer: " << ex.what());
        return SCDC_FAILURE;
    }

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    scdcint_t result = dataprov_webdav->session_handler->lseek(flag, offset);

    if (result == -1) {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    } else {

        if (output != NULL && SCDC_DATASET_INOUT_BUF_PTR(output) != NULL && static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(output)) >= sizeof (result)) {
            scdcint_t *out = static_cast<scdcint_t*> (SCDC_DATASET_INOUT_BUF_PTR(output));
            *out = result;
        } else {
            SCDC_INFO("do_cmd_lseek: no output or buffer given to save the result.");
        }
    }

    return SCDC_SUCCESS;
}

bool scdc_dataset_webdav_access::do_cmd_close(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_close: '" << params << "'");

    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    if (!dataprov_webdav->session_handler->close_file()) {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);

        return false;
    }

    return true;
}

bool scdc_dataset_webdav_access::do_cmd_open(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_open: '" << params << "'");

    //separate filename and open mode from the params
    stringlist confs(' ', params);
    string filename = confs.front_pop();
    string mode_as_string = confs.front_pop();

    if (filename.find('/') != filename.npos) { //check if given filename contains any '/'
        SCDC_FAIL("do_cmd_open: the given parameters '" << params << "' contain directories but only a filename is allowed. Use do_cmd_cd for changing the base path.");
        return SCDC_FAILURE;
    }

    int mode;
    try {
        mode = stoi(mode_as_string);
    } catch (const std::invalid_argument& ex) {
        SCDC_FAIL("do_cmd_open: given mode could not be converted into an int: " << ex.what());
        return SCDC_FAILURE;
    } catch (const std::out_of_range& ex) {
        SCDC_FAIL("do_cmd_open: given mode could not be converted into an int: " << ex.what());
        return SCDC_FAILURE;
    }


    scdc_dataprov_webdav_access *dataprov_webdav = static_cast<scdc_dataprov_webdav_access *> (dataprov);

    //open file
    if (!dataprov_webdav->session_handler->open_file(filename, mode)) {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);

        return SCDC_FAILURE;
    }

    return SCDC_SUCCESS;
}

void scdc_dataset_webdav_access::set_string_result_to_inout(string result, scdc_dataset_inout_t * inout) {
    SCDC_TRACE("set_string_result_to_inout");
    //FIXEME set_string_result_to_inout: implement next function for inout

    if (inout == NULL) {
        SCDC_INFO("set_string_result_to_inout: inout was NULL -> result information not set");
        return;
    }

    if (SCDC_DATASET_INOUT_BUF_PTR(inout) == NULL) {
        SCDC_INFO("set_string_result_to_inout: no buffer was given -> result information not set");
        return;
    }

    char* buffer = static_cast<char*> (SCDC_DATASET_INOUT_BUF_PTR(inout));

    strncpy(buffer, result.c_str(), SCDC_DATASET_INOUT_BUF_SIZE(inout));

    if (result.size() < static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(inout))) {
        SCDC_DATASET_INOUT_BUF_CURRENT(inout) = result.size();
        inout->total_size = result.size() + 1; //because of '/0';
    } else {
        SCDC_DATASET_INOUT_BUF_CURRENT(inout) = SCDC_DATASET_INOUT_BUF_SIZE(inout);
        inout->total_size = SCDC_DATASET_INOUT_BUF_SIZE(inout);
    }

    inout->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
    strncpy(inout->format, "text", SCDC_FORMAT_MAX_SIZE);
}

void scdc_dataset_webdav_access::set_response_error_to_inout(scdc_dataprov_webdav_session_handler *session_handler, scdc_dataset_inout_t * inout) {
    SCDC_TRACE("set_response_error_to_inout");

    if (session_handler == NULL) {
        SCDC_INFO("set_response_error_to_inout: session_handler was NULL -> error information is lost");
        return;
    }

    string error = session_handler->get_response_data()->error_message;
    string response_body = session_handler->get_response_data()->response_body;

    //combine the error message and the full response from the Server
    if (!response_body.empty()) {
        error += "\nresponse body:\n" + response_body;
    }

    if (inout == NULL || SCDC_DATASET_INOUT_BUF_PTR(inout) == NULL) {
        SCDC_ERROR(error);

    } else {
        char* buffer = static_cast<char*> (SCDC_DATASET_INOUT_BUF_PTR(inout));

        strncpy(buffer, error.c_str(), SCDC_DATASET_INOUT_BUF_SIZE(inout));

        if (error.size() < static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(inout))) { //FIXEME implement next function for output for set_response_error_to_inout
            SCDC_DATASET_INOUT_BUF_CURRENT(inout) = error.size();
            inout->total_size = error.size() + 1; //because of '/0';
        } else {
            SCDC_DATASET_INOUT_BUF_CURRENT(inout) = SCDC_DATASET_INOUT_BUF_SIZE(inout);
            inout->total_size = SCDC_DATASET_INOUT_BUF_SIZE(inout);
        }

        inout->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
        strncpy(inout->format, "text", SCDC_FORMAT_MAX_SIZE);

    }
}

#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-webdav-access: "

scdc_dataprov_webdav_access::scdc_dataprov_webdav_access()
: scdc_dataprov("webdav") {

    this->session_handler = scdc_dataprov_webdav_session_handler_factory::get_new_session_hander();
}

scdc_dataprov_webdav_access::~scdc_dataprov_webdav_access() {
    delete this->session_handler;
}

bool scdc_dataprov_webdav_access::open(const char *conf, scdc_args * args) {
    SCDC_TRACE("open: conf: '" << conf << "'");

    bool ret = true; //return value

    //check if config
    const char *webdav_config;
    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &webdav_config) == SCDC_ARG_REF_NULL) {
        SCDC_ERROR("open: getting WebDAV configuration");
        ret = false;
        goto do_quit;
    }

    SCDC_TRACE("open: webdav conf: '" << webdav_config << "'");

    if (!scdc_dataprov::open(conf, args)) {
        SCDC_FAIL("open: opening base");
        ret = false;

    } else {

        //set config struct
        if (!session_handler->set_session_handler_config(webdav_config)) {
            SCDC_ERROR("open: setting WebDAV configuration");
            ret = false;
            goto do_close;
        }

        //connect to Server
        if (!session_handler->new_session()) {
            SCDC_ERROR("open: Error message: " << session_handler->get_response_data()->error_message);

            ret = false;
            goto do_close;
        }

        //dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_pwd));
        dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_info));
        dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_cd));
        dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_ls));
        dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_put));
        dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_get));
        dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_rm));

        //dataset_cmds_add("get_buffered", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_get_buffered));
        dataset_cmds_add("lseek", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_lseek));
        dataset_cmds_add("open", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_open));
        dataset_cmds_add("close", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_access::do_cmd_close));

        SCDC_TRACE("open: datastore is open");

do_close:
        if (!ret) scdc_dataprov::close();
    }

do_quit:

    return ret;
}


void scdc_dataprov_webdav_access::close() {

    SCDC_TRACE("close:");
    scdc_dataprov::close();
}

scdc_dataset * scdc_dataprov_webdav_access::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t * output) {
    SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

    scdc_dataset *dataset = 0;

    if (config_open(path, path_size, output, &dataset)) return dataset;

    scdc_dataset_webdav_access * dataset_webdav = new scdc_dataset_webdav_access(this);


    //TODO set this to something usefull (if the server is reacheable is alreday checked in scdc_dataprov_webdav_access::scdc_dataprov_webdav_access())

    //    if (path && !dataset_webdav->do_cmd_cd(string(path, path_size).c_str(), NULL, output)) {
    //        SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    //        delete dataset_webdav;
    //        return 0;
    //    }


    SCDC_TRACE("dataset_open: return: '" << dataset_webdav << "'");

    return dataset_webdav;
}

void scdc_dataprov_webdav_access::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t * output) {
    SCDC_TRACE("dataset_close: '" << dataset << "'");

    if (config_close(dataset, output)) return;

    delete dataset;

    SCDC_TRACE("dataset_close: return");
}

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataset-webdav-access: "


#define DIR_STORE_PREFIX  "store_v1_"
#define DIR_ENTRY_PREFIX  "entry_v1_"
#define DIR_ENTRY_SUFFIX  ""


static scdcint_t dir_count_stores()
{
  return 0;
}


static scdcint_t dir_count_entries()
{
  return 0;
}

class scdc_dataset_webdav_store : public scdc_dataset {
public:

    scdc_dataset_webdav_store(scdc_dataprov *dataprov_) : scdc_dataset(dataprov_), admin(false) {
    };

    /**
     * Reads meta data of the file with the given url. Behavior similar to the POSIX stat function.
     * If no url is given and a  file was opened with de do_cmd_open method the meta data of this file is read.
     * @param params url of the directory relative to the base path.
     * @param input not used
     * @param output the buf struct points to a big enough buffer to hold a stat struct which is set after success.
     * If an error occurs the buffer contains the error message. The size is checked via the buf.size attribute.
     * @return true on success and false if an error occurred
     */
    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Changes the working directory (base path) on the host.
     * If the given path is invalid the old value is restored.
     * @param params base path
     * @param input not used here
     * @param output the buf struct points to a big enough buffer to save an error message for errors that may occur (the size is checked via the buf.size attribute).
     * @return true on success and false if an error occurred
     */
    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Requests the content of directory with the given url from the WebDAV server.
     * @param params url of the directory relative to the base path
     * if the params string is empty the content of the base path read
     * @param input not used
     * @param output the buf struct points to a big enough buffer (the size is checked via the buf.size attribute) to save content of the directory as string formated as name:type:size| \n
     * : is used as a separator for the attributes of a file or directory\n
     * | is used as a separator for the different files or directories
     * total_size (and the buf.current) attribute contains the length of the string
     * @return true on success and false if an error occurred
     */
    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Writes the content of the given buffer (via input) to the previous opened file (use do_cmd_open).
     * @param params not used
     * @param input the buf struct points to a buffer that contain the data to write.  The next field is used to extract all data if set.
     * On success total_size attributes holds the amount of bytes successfully written to the file.
     * @param output the buf struct points to a big enough buffer to save an error message for errors that may occur (the size is checked via the buf.size attribute).
     * @return true on success and false if an error occurred
     */
    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Reads data from a previous opened file (use do_cmd_open) into the given buffer specified in the input parameter.\n
     * If buf.current is 0 EOF is reached and the next function is set to NULL. \n
     * Even if plain text is read the content written into the buffer is not null terminated!
     * (Use the buf.current attribute to determine how many bytes have been written.) \n
     * Use the next function of the output parameter to download more data (e.g. when buf.current equals the size of the given buffer).
     * The next function returns 1 (SCDC_SUCCESS) on success and 0 (SCDC_FAILURE) if an error occurred.
     * In case of an error a error message is saved in the buf attribute of the output. (Does not work with next function!)
     * @param params no extra parameters used
     * @param input the buf struct points to a buffer with a minimum size of 1 byte and the set buf.size must contain the size of this buffer.
     * After execution the buffer contains the data downloaded from the file.
     * The amount of written bytes to the pointer is saved in buf.current.
     * If the next pointer is not NULL more data from the file can be downloaded and EOF is not reached yet.
     * @param output if set informations about the error that may occur are saved in the given buffer (if the buffer is to small the error message may not be complete).
     * @return SCDC_SUCCESS on success and SCDC_FAILURE if an error occurred and the given buffer (via output) then contains a description of the error.
     * @see use do_cmd_open do_cmd_lseek do_cmd_close
     */
    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Removes the the given file from the WebDAV server.
     * @param params the url of the file relative to the base path as string
     * @param input is not used here
     * @param output contains information about errors that may occur if the buf attribute points to a big enough buffer
     * @return true on success and false if an error occurred
     */
    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

private:
    bool admin;

    /**
     * Sets the given result to the output struct.
     * @param result of the request
     * @param output output struct
     */
    static void set_string_result_to_inout(string result, scdc_dataset_inout_t *output);

    /**
     * Sets the error information from the response data to the output struct.
     * @param dataprov_webdav dataprovider
     * @param output output struct
     */
    static void set_response_error_to_inout(scdc_dataprov_webdav_session_handler *session_handler, scdc_dataset_inout_t *output);

    /**
     * next function for reading a file
     * @param inout dataset
     * @return SCDC_SUCCSES if read was successful, SCDC_FAILURE otherwise and an error information is ste to the given dataset
     */
    static scdcint_t dataset_webdav_read_file_next(scdc_dataset_inout_t *inout);
};

bool scdc_dataset_webdav_store::do_cmd_info(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output)
{
    SCDC_TRACE("do_cmd_info: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    // scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    scdcint_t stores = dir_count_stores();

    SCDC_DATASET_OUTPUT_PRINTF(output, "admin: %s, stores: %" scdcint_fmt, (admin?"yes":"no"), stores);

    return true;
}

bool scdc_dataset_webdav_store::do_cmd_cd(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {

    SCDC_TRACE("do_cmd_cd: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    if (params == "ADMIN")
    {
      admin = true;
      return scdc_dataset::do_cmd_cd(params, input, output);
    }

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    if (!params.empty())
    {
      string store_path = DIR_STORE_PREFIX + string(params) + "/";

      SCDC_TRACE("do_cmd_cd: store_path: '" << store_path << "'");

      if (!dataprov_webdav->session_handler->is_dir(params))
      {
        SCDC_FAIL("do_cmd_cd: store '" << params << "' does not exist");
        SCDC_DATASET_OUTPUT_PRINTF(output, "store does not exist");
        return false;
      }

      dataprov_webdav->session_handler->set_base_path(params);
    }

    admin = false;

    return scdc_dataset::do_cmd_cd(params, input, output);
}

bool scdc_dataset_webdav_store::do_cmd_ls(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_ls: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    if (output == NULL || SCDC_DATASET_INOUT_BUF_PTR(output) == NULL) {
        SCDC_FAIL("do_cmd_ls: Output must contain a buffer to store the result!");
        return SCDC_FAILURE;
    }

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    string result = dataprov_webdav->session_handler->read_dir(params);

    if (!result.empty()) {
        if (static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(output)) < result.size()) { //FIXEME implement next function for output for do_cmd_ls
            SCDC_ERROR("do_cmd_ls: the given buffer is to small to hold all the information.");
        }
        set_string_result_to_inout(result, output);
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

bool scdc_dataset_webdav_store::do_cmd_put(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_put: '" << params << "'");

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    if (SCDC_DATASET_INOUT_BUF_PTR(input) == NULL) { //check buffer
        SCDC_FAIL("do_cmd_put: Input must contain pointer to buffer with minimum size 1! (use buf and total_size attributes)");
        return SCDC_FAILURE;
    }

    if (SCDC_DATASET_INOUT_BUF_CURRENT(input) > SCDC_DATASET_INOUT_BUF_SIZE(input)) { //check buffer size
        SCDC_FAIL("do_cmd_put: incorrect input: buf.current is bigger than buf.size!");
        return SCDC_FAILURE;
    }

    scdcint_t total_num_of_bytes = dataprov_webdav->session_handler->write_buffer_to_file(SCDC_DATASET_INOUT_BUF_PTR(input), SCDC_DATASET_INOUT_BUF_CURRENT(input));

    if (total_num_of_bytes < 0) {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        input->total_size = total_num_of_bytes;
        return SCDC_FAILURE;
    }

    //SCDC_INFO("buffer size: " << input->buf.size); //doesn't work! -> request for member ‘size’ in ‘input->_scdc_dataset_inout_t::buf’, which is of non-class type ‘void*

    while (input->next) {
        input->next(input);

        scdcint_t current_num_of_bytes = dataprov_webdav->session_handler->write_buffer_to_file(SCDC_DATASET_INOUT_BUF_PTR(input), SCDC_DATASET_INOUT_BUF_CURRENT(input));

        if (current_num_of_bytes < 0) {
            set_response_error_to_inout(dataprov_webdav->session_handler, output);
            //TODO check the status of the file after this because some bytes may have been written to the tmp-file (at least close it maybe)
            input->total_size = total_num_of_bytes;
            return SCDC_FAILURE;

        } else {
            total_num_of_bytes += current_num_of_bytes;
        }
    }

    input->total_size = total_num_of_bytes;

    return SCDC_SUCCESS;
}

bool scdc_dataset_webdav_store::do_cmd_get(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_get: '" << params << "'");

    if (output == NULL || SCDC_DATASET_INOUT_BUF_PTR(output) == NULL || SCDC_DATASET_INOUT_BUF_SIZE(output) < 1) {
        SCDC_FAIL("do_cmd_get: Output must contain buffer with minimum size of one byte!");
        return false;
    }

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    output->data = static_cast<scdc_dataprov_webdav_store *> (dataprov);
    output->next = dataset_webdav_read_file_next;

    //request the first data from the given file
    if (output->next(output)) { //check for error
        return SCDC_SUCCESS;
    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);
        return SCDC_FAILURE;
    }
}

scdcint_t scdc_dataset_webdav_store::dataset_webdav_read_file_next(scdc_dataset_inout_t * inout) {

    if (inout == NULL) {
        return SCDC_FAILURE;
    }

    if (inout->data == NULL) {
        set_string_result_to_inout("dataset_webdav_read_file_next: the data attribute was NULL (should be a pointer to a scdc_dataprov_webdav_store object)", inout);
        return SCDC_FAILURE;
    }

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (inout->data);

    //read next bytes of data from the file
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = dataprov_webdav->session_handler->read_into_buffer(SCDC_DATASET_INOUT_BUF_PTR(inout), SCDC_DATASET_INOUT_BUF_SIZE(inout));


    if (SCDC_DATASET_INOUT_BUF_CURRENT(inout) <= 0) { //0 is eof, negative number is error
        inout->data = NULL;
        inout->next = NULL;

        //error occurred
        if (SCDC_DATASET_INOUT_BUF_CURRENT(inout) < 0) {
            //this would override the content of the buffer with the error message which is maybe not wanted
            set_response_error_to_inout(dataprov_webdav->session_handler, NULL); //uses SCDC_ERROR()
            //FIXME export error message
            return SCDC_FAILURE;
        }
    }

    //TODO set inout->total_size and inout->total_size_given

    return SCDC_SUCCESS;
}

bool scdc_dataset_webdav_store::do_cmd_rm(const string& params, scdc_dataset_input_t* input, scdc_dataset_output_t * output) {
    SCDC_TRACE("do_cmd_rm: '" << params << "'");

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    if (dataprov_webdav->session_handler->remove_file(params)) {
        return true;

    } else {
        set_response_error_to_inout(dataprov_webdav->session_handler, output);

        return false;
    }
}

void scdc_dataset_webdav_store::set_string_result_to_inout(string result, scdc_dataset_inout_t * inout) {
    SCDC_TRACE("set_string_result_to_inout");
    //FIXEME set_string_result_to_inout: implement next function for inout

    if (inout == NULL) {
        SCDC_INFO("set_string_result_to_inout: inout was NULL -> result information not set");
        return;
    }

    if (SCDC_DATASET_INOUT_BUF_PTR(inout) == NULL) {
        SCDC_INFO("set_string_result_to_inout: no buffer was given -> result information not set");
        return;
    }

    char* buffer = static_cast<char*> (SCDC_DATASET_INOUT_BUF_PTR(inout));

    strncpy(buffer, result.c_str(), SCDC_DATASET_INOUT_BUF_SIZE(inout));

    if (result.size() < static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(inout))) {
        SCDC_DATASET_INOUT_BUF_CURRENT(inout) = result.size();
        inout->total_size = result.size() + 1; //because of '/0';
    } else {
        SCDC_DATASET_INOUT_BUF_CURRENT(inout) = SCDC_DATASET_INOUT_BUF_SIZE(inout);
        inout->total_size = SCDC_DATASET_INOUT_BUF_SIZE(inout);
    }

    inout->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
    strncpy(inout->format, "text", SCDC_FORMAT_MAX_SIZE);
}

void scdc_dataset_webdav_store::set_response_error_to_inout(scdc_dataprov_webdav_session_handler *session_handler, scdc_dataset_inout_t * inout) {
    SCDC_TRACE("set_response_error_to_inout");

    if (session_handler == NULL) {
        SCDC_INFO("set_response_error_to_inout: session_handler was NULL -> error information is lost");
        return;
    }

    string error = session_handler->get_response_data()->error_message;
    string response_body = session_handler->get_response_data()->response_body;

    //combine the error message and the full response from the Server
    if (!response_body.empty()) {
        error += "\nresponse body:\n" + response_body;
    }

    if (inout == NULL || SCDC_DATASET_INOUT_BUF_PTR(inout) == NULL) {
        SCDC_ERROR(error);

    } else {
        char* buffer = static_cast<char*> (SCDC_DATASET_INOUT_BUF_PTR(inout));

        strncpy(buffer, error.c_str(), SCDC_DATASET_INOUT_BUF_SIZE(inout));

        if (error.size() < static_cast<size_t>(SCDC_DATASET_INOUT_BUF_SIZE(inout))) { //FIXEME implement next function for output for set_response_error_to_inout
            SCDC_DATASET_INOUT_BUF_CURRENT(inout) = error.size();
            inout->total_size = error.size() + 1; //because of '/0';
        } else {
            SCDC_DATASET_INOUT_BUF_CURRENT(inout) = SCDC_DATASET_INOUT_BUF_SIZE(inout);
            inout->total_size = SCDC_DATASET_INOUT_BUF_SIZE(inout);
        }

        inout->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
        strncpy(inout->format, "text", SCDC_FORMAT_MAX_SIZE);

    }
}

#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-webdav-access: "

scdc_dataprov_webdav_store::scdc_dataprov_webdav_store()
: scdc_dataprov("webdav") {

    this->session_handler = scdc_dataprov_webdav_session_handler_factory::get_new_session_hander();
}

scdc_dataprov_webdav_store::~scdc_dataprov_webdav_store() {
    delete this->session_handler;
}

bool scdc_dataprov_webdav_store::open(const char *conf, scdc_args * args) {
    SCDC_TRACE("open: conf: '" << conf << "'");

    bool ret = true; //return value

    //check if config
    const char *webdav_config;
    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &webdav_config) == SCDC_ARG_REF_NULL) {
        SCDC_ERROR("open: getting WebDAV configuration");
        ret = false;
        goto do_quit;
    }

    SCDC_TRACE("open: webdav conf: '" << webdav_config << "'");

    if (!scdc_dataprov::open(conf, args)) {
        SCDC_FAIL("open: opening base");
        ret = false;

    } else {

        //set config struct
        if (!session_handler->set_session_handler_config(webdav_config)) {
            SCDC_ERROR("open: setting WebDAV configuration");
            ret = false;
            goto do_close;
        }

        //connect to Server
        if (!session_handler->new_session()) {
            SCDC_ERROR("open: Error message: " << session_handler->get_response_data()->error_message);

            ret = false;
            goto do_close;
        }

        //dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_pwd));
        dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_info));
        dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_cd));
        dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_ls));
        dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_put));
        dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_get));
        dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_rm));

        SCDC_TRACE("open: datastore is open");

do_close:
        if (!ret) scdc_dataprov::close();
    }

do_quit:

    return ret;
}

void scdc_dataprov_webdav_store::close() {

    SCDC_TRACE("close:");
    scdc_dataprov::close();
}

scdc_dataset * scdc_dataprov_webdav_store::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t * output) {
    SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

    scdc_dataset *dataset = 0;

    if (config_open(path, path_size, output, &dataset)) return dataset;

    scdc_dataset_webdav_store * dataset_webdav = new scdc_dataset_webdav_store(this);


    //TODO set this to something usefull (if the server is reacheable is alreday checked in scdc_dataprov_webdav_store::scdc_dataprov_webdav_store())

    //    if (path && !dataset_webdav->do_cmd_cd(string(path, path_size).c_str(), NULL, output)) {
    //        SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    //        delete dataset_webdav;
    //        return 0;
    //    }


    SCDC_TRACE("dataset_open: return: '" << dataset_webdav << "'");

    return dataset_webdav;
}

void scdc_dataprov_webdav_store::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t * output) {
    SCDC_TRACE("dataset_close: '" << dataset << "'");

    if (config_close(dataset, output)) return;

    delete dataset;

    SCDC_TRACE("dataset_close: return");
}

#undef SCDC_LOG_PREFIX
