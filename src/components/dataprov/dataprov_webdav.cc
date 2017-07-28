/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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

#include "config.hh"
#include "common.hh"
#include "log.hh"

#include "dataprov_webdav.hh"

#if HAVE_NEON_H
# include <neon/ne_auth.h>
# include <neon/ne_request.h>
# include <neon/ne_session.h>
# include <neon/ne_uri.h>
# include <neon/ne_utils.h>
# define USE_NEON  1
#endif

#if HAVE_DAVIX_HPP
# include <davix/davix.hpp>
# include <davix/file/davfile.hpp>
# define USE_DAVIX  1
using namespace Davix;
#endif


using namespace std;


/**
* class to encapsulate the data of the response from the server.
*/
class scdc_dataprov_webdav_response_data
{
public:
    /**
     * result of the request function
     */
    int dispatch_result;

    /**
     * http status code
     */
    int status_code;

    /**
     * error message send with the http response
     */
    std::string error_message;

    /**
     * body of the response as string
     */
    std::string response_body; //TODO refactor to response_body_as_str
    //TODO add field for responsebody as binary

    /**
     * map of the revieved headers
     */
    std::map<std::string,std::string> response_headers;

    scdc_dataprov_webdav_response_data()
      :dispatch_result(0), status_code(0)
    { }
    ~scdc_dataprov_webdav_response_data()
    { }

    //TODO enable this on DEBUG and return a string
    /*
    void print(){ //just for testing
        std::cout<<"********************************"<< std::endl;
        std::cout<< "dispatch result: "<< dispatch_result << std::endl;
        std::cout<< "status code: "<< status_code<< std::endl;
        std::cout<< "error message: "<< error_message<< std::endl;
        std::cout<< "headers: "<<std::endl;

        std::map<std::string,std::string>::iterator iter = response_headers.begin();

        for(; iter != response_headers.end(); iter++){
            std::cout<<"  "<<iter->first<<": "<<iter->second<<std::endl;
        }

        std::cout<< "body: "<< std::endl;
        std::cout<<response_body<<std::endl;
        std::cout<<"********************************"<< std::endl;
    }
    */
};


/**
 * Interface for various session handlers
 */
class scdc_dataprov_webdav_session_handler
{
protected:
    /**
     * Server URL
     */
    std::string host;
    /**
     * Location of the WebDAV directory on the server.
     */
    std::string base_path;
    /**
     * Protocol to connect to the server. (http or https)
     */
    std::string protocol; //TODO rthis to scheme
    /**
     * Struct to encapsulate the data of the response from the server.
     */
    scdc_dataprov_webdav_response_data* response_data;

public:

    /**
     * Set username and password for login (only basic authentication is supported).
     * @param username
     * @param password
     */
    virtual void set_username_password(std::string username, std::string password) = 0;

    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the set configuration will be created.
     * If an error occurred this method returns false and a description of the error is saved in the scdc_dataprov_webdav_response_data.
     * @return true on success, false otherwise
     * @see scdc_dataprov_webdav_response_data
     */
    virtual bool new_session() = 0;

    /**
     * Requests the content of file with the given url from the host.
     * @param url of the file relative to the base path
     * @return content of the file as string or an empty string if an error occurred. A description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual std::string get_file_content(std::string url) = 0;

    /**
     * Save the data into a file on the host.
     * @param data content of the file as string
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual bool put_file(std::string data, std::string url) = 0;

    /**
     * Removes the file with the given url from the host.
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise; a description of the error is saved in the response_data.
     * @see scdc_dataprov_webdav_response_data
     */
    virtual bool remove_file(std::string url) = 0;

    //basic getter and setter
    virtual std::string get_base_path() const {return base_path;}
    virtual void set_base_path(std::string base_path){this->base_path = base_path;}

    virtual std::string get_host() const {return host;}
    virtual void set_host(std::string host) {this->host = host;}

    virtual std::string get_protocol() const {return protocol;}
    virtual void set_protocol(std::string protocol) {this->protocol = protocol;}

    virtual scdc_dataprov_webdav_response_data* get_response_data() const {return response_data;}
    virtual void set_response_data(scdc_dataprov_webdav_response_data* response_data) {this->response_data = response_data;}
};


#if USE_NEON

#define SCDC_LOG_PREFIX  "dataprov-webdav-session-handler-neon: "

/**
 * session handler that uses the neon library
 */
class scdc_dataprov_webdav_session_handler_neon : public scdc_dataprov_webdav_session_handler
{
private:

    std::string username;
    std::string password;
    std::string credentials_base64;

    ne_session *session;

    /**
     * Generates the credentials for basic authentication from the username and password.
     */
    void set_credentials_base64();

    /**
     * Sets a number of basic headers for the given request.
     * @param request
     */
    void set_basic_headers(ne_request *request);

    /**
     * Sets the authorization header for a request send via https.
     * username and password must be set.
     * @param request
     */
    void set_HTTPS_headers(ne_request *request);

    /**
     * Callback for the response body.
     * @param userdata pointer to the variable that should contain the content of the response body
     * @param buf buffer
     * @param len buffer size
     * @return
     */
    static int http_response_reader(void *userdata, const char *buf, size_t len);

    /**
     * Returns a new request object with headers set (based on the protocol).
     * @param method WebDAV or HTTP method
     * @param url of the file relative to the base path
     * @return the request object
     */
    ne_request* get_new_request(const char* method, std::string url);

    /**
     * Dispatch the given request.
     * The result and the response from the host is saved in response_data.
     * The request is destroyed after the dispatch.
     * @param request
     * @see scdc_dataprov_webdav_response_data
     */
    void request_dispatch(ne_request* request);

    /**
     * Returns a string containing all methods (separated with commas) that can be used with the given resource.
     * @param url of the resource
     */
    std::string get_options(std::string url);

public:

    scdc_dataprov_webdav_session_handler_neon();
    ~scdc_dataprov_webdav_session_handler_neon();

    //virtual methods from super class
    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the set configuration will be created.
     * @return true on succsess, false otherwise
     */
    virtual bool new_session();

    /**
     * Requests the content of file with the given url from the host.
     * @param url of the file relative to the base path
     * @return content of the file as string
     */
    virtual std::string get_file_content(std::string url);

    /**
     * Save the date into a file on the host.
     * @param data content of the file as string
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise
     */
    virtual bool put_file(std::string data, std::string url);

    /**
     * Removes the file with the given url from the host.
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise
     */
    virtual bool remove_file(std::string url);

    /**
     * Set username and password for login.
     * @param username
     * @param password
     */
    virtual void set_username_password(std::string username, std::string password);

    //basic getter and setter
    std::string get_username(){return username;}
    void set_username(std::string username) {this->username = username;}

    std::string get_password(){return password;}
    void set_password(std::string password) {this->password = password;}

    std::string get_credentials_bas64(){return credentials_base64;}
    void set_credentials_base64(std::string credentials_base64) {this->credentials_base64 = credentials_base64;} //TODO check bas64

    ne_session* get_session(){return session;}
};


scdc_dataprov_webdav_session_handler_neon::scdc_dataprov_webdav_session_handler_neon() {
    response_data = new scdc_dataprov_webdav_response_data();
    session = NULL;

    //init neon socket and check for errors
    if (ne_sock_init() != 0) //ne_sock_init() returns 0 if no error occurred
    {
        SCDC_FAIL("could not initialize neon library");
    };
}

scdc_dataprov_webdav_session_handler_neon::~scdc_dataprov_webdav_session_handler_neon() {
    if (session) {
        ne_session_destroy(session); //destroy existing session
    }

    ne_sock_exit();
}

void scdc_dataprov_webdav_session_handler_neon::set_username_password(std::string username, std::string password) {
    this->username = username;
    this->password = password;

    set_credentials_base64();
}

void scdc_dataprov_webdav_session_handler_neon::set_credentials_base64() {
    string creds = username + ":" + password;

    /* "ne_base64(...) returns malloc-allocated NUL-terminated buffer which the caller must free()." */
    char* tmp = ne_base64((unsigned char*) creds.c_str(), creds.size());

    credentials_base64 = string(tmp);

    free(tmp);
}

bool scdc_dataprov_webdav_session_handler_neon::new_session() {

    if (session) {
        ne_session_destroy(session); //destroy existing session
    }

    if (protocol.compare("http") == 0) {

        session = ne_session_create("http", host.c_str(), 80);

    } else if (protocol.compare("https") == 0) {

        session = ne_session_create("https", host.c_str(), 443);
        // trust default set of CA certificates included in the SSL library
        ne_ssl_trust_default_ca(session);

    } else {
        response_data->error_message = protocol + " is not supported!";
        return false;
    }

    //TODO use OPTIONS to check if the server is online and which methods are supported
    string methods = get_options("");

    if (response_data->dispatch_result != NE_OK) {
        return false;
    }

    return true;
}

std::string scdc_dataprov_webdav_session_handler_neon::get_options(std::string url) {
    ne_request* request = get_new_request("OPTIONS", url);
    request_dispatch(request);

    string methods = response_data->response_headers["allow"];

    return methods;
}

int scdc_dataprov_webdav_session_handler_neon::http_response_reader(void *userdata, const char *buf, size_t len) {
    string *str = (string *) userdata;
    str->append(buf, len);
    return 0;
}

ne_request* scdc_dataprov_webdav_session_handler_neon::get_new_request(const char* method, std::string url) {
    ne_request* request = ne_request_create(session, method, (base_path + url).c_str());

    set_basic_headers(request);

    if (protocol.compare("https") == 0) {
        set_HTTPS_headers(request);
    }

    return request;
}

void scdc_dataprov_webdav_session_handler_neon::request_dispatch(ne_request* request) {
    delete response_data;
    response_data = new scdc_dataprov_webdav_response_data();

    //save the response body as string
    ne_add_response_body_reader(request, ne_accept_always, http_response_reader, &(response_data->response_body));

    //send request and process the response
    response_data->dispatch_result = ne_request_dispatch(request);

    void* cursor = NULL;
    const char *name, *value;
    while ((cursor = ne_response_header_iterate(request, cursor, &name, &value)) != NULL) {
        response_data->response_headers.insert(make_pair(string(name), string(value)));
    }

    //get the http code of the response
    response_data->status_code = ne_get_status(request)->code;

    response_data->error_message = ne_get_error(session);

    //TODO enable this on DEBUG and use SCDC_TRACE
    //response_data->print();

    ne_request_destroy(request);
}

void scdc_dataprov_webdav_session_handler_neon::set_basic_headers(ne_request* request) {
    //only support for text at the moment
    ne_add_request_header(request, "Accept", "text/plain, text/html, text/xml");
    ne_add_request_header(request, "Accept-Charset", "utf-8");
    //TODO add some more (important) headers
}

void scdc_dataprov_webdav_session_handler_neon::set_HTTPS_headers(ne_request* request) {
    if (credentials_base64.empty()) {
        set_credentials_base64();
    }

    string header_content = "Basic " + credentials_base64;
    ne_add_request_header(request, "Authorization", header_content.c_str());

    //TODO digest authentication
}

string scdc_dataprov_webdav_session_handler_neon::get_file_content(std::string url) {
    ne_request* request = get_new_request("GET", url);

    request_dispatch(request);

    if (response_data->dispatch_result != NE_OK) {
        return string();
    }

    string response(response_data->response_body);
    return response;
}

bool scdc_dataprov_webdav_session_handler_neon::put_file(std::string data, std::string url) {

    ne_request* request = get_new_request("PUT", url);

    ne_add_request_header(request, "Content-type", "text/xml"); //which content type the response should have

    ne_set_request_body_buffer(request, data.c_str(), data.size()); //set the actual content that should be saved

    request_dispatch(request);

    if (response_data->status_code == 201) {
        return true;
    } else {
        return false;
    }
}

bool scdc_dataprov_webdav_session_handler_neon::remove_file(std::string url) {
    ne_request* request = get_new_request("DELETE", url);

    request_dispatch(request);

    if (response_data->status_code == 204) {
        return true;
    } else {
        return false;
    }
}

#undef SCDC_LOG_PREFIX

#endif /* USE_NEON */


#if USE_DAVIX

#define SCDC_LOG_PREFIX  "dataprov-webdav-session-handler-davix: "

/**
 * session handler that uses the Davix library
 */
class scdc_dataprov_webdav_session_handler_davix : public scdc_dataprov_webdav_session_handler
{
private:

    /**
     * main handle for Davix
     */
    Davix::Context context;

    /**
     * container for Davix request options
     */
    Davix::RequestParams request_parameters;

    /**
     * Returns an formated URI "<protocol>://<host><base_path><path>"
     * @param path of the resource relative to the base path
     * @return
     */
    Davix::Uri get_davix_uri(std::string path);

    /**
     *Transfer the information from DavixError to the response data.
     */
    void set_error_to_response_data(Davix::DavixError* err);

public:

    scdc_dataprov_webdav_session_handler_davix();
    ~scdc_dataprov_webdav_session_handler_davix();

    //virtual methods from super class
    /**
     * Creates a new session.
     * The current session will be destroyed and a new session with the set configuration will be created.
     * @return true on success, false otherwise
     */
    virtual bool new_session();

    /**
     * Requests the content of file with the given url from the host.
     * @param url of the file relative to the base path
     * @return content of the file as string
     */
    virtual std::string get_file_content(std::string url);

    /**
     * Save the date into a file on the host.
     * @param data content of the file as string
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise
     */
    virtual bool put_file(std::string data, std::string url);

    /**
     * Removes the file with the given url from the host.
     * @param url of the file relative to the base path
     * @return true if successful, false otherwise
     */
    virtual bool remove_file(std::string url);

    /**
     * Set username and password for login.
     * @param username
     * @param password
     */
    virtual void set_username_password(std::string username, std::string password);
};


scdc_dataprov_webdav_session_handler_davix::scdc_dataprov_webdav_session_handler_davix() {
    request_parameters = new RequestParams();
    response_data = new scdc_dataprov_webdav_response_data();
}

void scdc_dataprov_webdav_session_handler_davix::set_username_password(const std::string username, const std::string password) {
    request_parameters.setClientLoginPassword(username, password);
}

Davix::Uri scdc_dataprov_webdav_session_handler_davix::get_davix_uri(std::string path) {
    return Uri(protocol + "://" + host + base_path + path);
}

void scdc_dataprov_webdav_session_handler_davix::set_error_to_response_data(Davix::DavixError* err) {
    delete response_data;
    response_data = new scdc_dataprov_webdav_response_data();
    response_data->error_message = err->getErrScope() + " Error: " + err->getErrMsg();
}

bool scdc_dataprov_webdav_session_handler_davix::new_session() {
    DavixError* err = NULL;
    struct stat info;

    TRY_DAVIX{
        DavFile file(context, request_parameters, get_davix_uri(""));

        //query basic file metadata
        file.stat(NULL, &info, &err);
        return true;
    }
    CATCH_DAVIX(&err) {
        set_error_to_response_data(err);
        return false;
    }
}

string scdc_dataprov_webdav_session_handler_davix::get_file_content(std::string path) {
    vector<char> buffer;
    DavixError* err = NULL;

    DavFile file(context, request_parameters, get_davix_uri(path));

    file.getFull(NULL, buffer, &err);

    if (err) {
        set_error_to_response_data(err);
        return string();
    } else {
        return string(buffer.begin(), buffer.end());
    }
}

bool scdc_dataprov_webdav_session_handler_davix::put_file(std::string data, std::string path) {

    DavixError* err = NULL;

    TRY_DAVIX{
        DavFile file(context, request_parameters, get_davix_uri(path));
        file.put(NULL, data.c_str(), data.size());
        return true;
    }
    CATCH_DAVIX(&err) {
        set_error_to_response_data(err);
        return false;
    }
}

bool scdc_dataprov_webdav_session_handler_davix::remove_file(std::string path) {
    DavixError* err = NULL;

    DavFile file(context, request_parameters, get_davix_uri(path));
    file.deletion(NULL, &err);

    if (err) {
        set_error_to_response_data(err);
        return false;
    } else {
        return true;
    }
}

#undef SCDC_LOG_PREFIX

#endif /* USE_DAVIX */


/**
 * Factory class that has only on static method which returns a session handler according to the selected library (neon or Davix).
 *
 */
class scdc_dataprov_webdav_session_handler_factory
{
public:

    /**
     * Returns a new session handler
     * @return session handler
     */
    static scdc_dataprov_webdav_session_handler* get_new_session_hander(){
#if USE_DAVIX
        return new scdc_dataprov_webdav_session_handler_davix();
#elif USE_NEON
        return new  scdc_dataprov_webdav_session_handler_neon();
#else
# error no suitable WebDAV implementation available
#endif
    };
};


#define SCDC_LOG_PREFIX  "dataset-webdav-store: "

class scdc_dataset_webdav_store : public scdc_dataset {
public:
    scdc_dataset_webdav_store(scdc_dataprov *dataprov_) : scdc_dataset(dataprov_) { };

    /**
     *
     * @param params
     * @param input
     * @param output
     * @return true on success and false if an error occurred
     */
    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     *
     * @param params
     * @param input
     * @param output
     * @return true on success and false if an error occurred
     */
    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     *
     * @param params
     * @param input
     * @param output
     * @return true on success and false if an error occurred
     */
    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Transfer data form the given input struct to the WebDAV server.
     * @param params the url of the file relative to the base path as string
     * @param input struct that contains the data in the buf field (next field is used to extract all data)
     * @param output contains information about errors that may occur
     * @return true on success and false if an error occurred
     */
    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Transfer data from the WebDAV server to the given output struct.
     * @param params the url of the file relative to the base path as string
     * @param input is not used here
     * @param output contains the result or information about errors that may occur
     * @return true on success and false if an error occurred
     */
    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Removes the the given file from the WebDAV server.
     * @param params the url of the file relative to the base path as string
     * @param input is not used here
     * @param output contains information about errors that may occur
     * @return true on success and false if an error occurred
     */
    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    /**
     * Sets the error information from the response data to the output struct.
     * @param dataprov_webdav dataprovider
     * @param output output struct
     */
    void set_response_error_to_output(scdc_dataprov_webdav_store *dataprov_webdav, scdc_dataset_output_t *output);

};


bool scdc_dataset_webdav_store::do_cmd_info(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {
    SCDC_TRACE("do_cmd_info: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    return false;
}

bool scdc_dataset_webdav_store::do_cmd_cd(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {

    SCDC_TRACE("do_cmd_cd: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    bool ret = scdc_dataset::do_cmd_cd(params, input, output);

    SCDC_TRACE("do_cmd_cd: return: " << ret);

    return ret;

}

bool scdc_dataset_webdav_store::do_cmd_ls(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {
    SCDC_TRACE("do_cmd_ls: '" << params << "'");

    SCDC_DATASET_OUTPUT_CLEAR(output);

    return false;
}

bool scdc_dataset_webdav_store::do_cmd_put(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {
    SCDC_TRACE("do_cmd_put: '" << params << "'");

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    string data = "";

    //current_size is set to the number of bytes written into the buffer
    data.append((char*) input->buf, input->current_size);

    while (input->next) {

        input->next(input);

        data.append((char*) input->buf, input->current_size);
    }

    if (dataprov_webdav->session_handler->put_file(data, params)) {
        return true;

    } else {

        set_response_error_to_output(dataprov_webdav, output);
        return false;
    }
}

bool scdc_dataset_webdav_store::do_cmd_get(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {
    SCDC_TRACE("do_cmd_get: '" << params << "'");

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    string result = dataprov_webdav->session_handler->get_file_content(params);

    if (!result.empty()) {
        //set result to output
        output->buf = strdup(result.c_str());
        output->buf_size = result.size();
        output->total_size = result.size();
        output->current_size = result.size();

        return true;

    } else {
        set_response_error_to_output(dataprov_webdav, output);
        return false;
    }
}

bool scdc_dataset_webdav_store::do_cmd_rm(const std::string& params, scdc_dataset_input_t* input, scdc_dataset_output_t* output) {
    SCDC_TRACE("do_cmd_rm: '" << params << "'");

    scdc_dataprov_webdav_store *dataprov_webdav = static_cast<scdc_dataprov_webdav_store *> (dataprov);

    if (dataprov_webdav->session_handler->remove_file(params)) {
        return true;

    } else {
        set_response_error_to_output(dataprov_webdav, output);
        return false;
    }
}

void scdc_dataset_webdav_store::set_response_error_to_output(scdc_dataprov_webdav_store* dataprov_webdav, scdc_dataset_output_t* output) {
    SCDC_TRACE("set_response_error_to_output");

    string error = dataprov_webdav->session_handler->get_response_data()->error_message;
    string response_body = dataprov_webdav->session_handler->get_response_data()->response_body;

    //combine the error message and the full response from the Server
    if(!response_body.empty()){
        error += "\nresponse body:\n"+response_body;
    }

    output->buf = strdup(error.c_str());
    output->buf_size = error.size();
    output->total_size = error.size();
    output->current_size = error.size();
}


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-webdav-store: "

scdc_dataprov_webdav_store::scdc_dataprov_webdav_store()
: scdc_dataprov("webdav") {
    this->session_handler = scdc_dataprov_webdav_session_handler_factory::get_new_session_hander();
}

/**
 * Open new webdav store.
 *
 * @param conf
 * @param args
 * @return false if an error occurred otherwise true
 */
bool scdc_dataprov_webdav_store::open(const char *conf, scdc_args *args) {
    SCDC_TRACE("open: conf: '" << conf << "'");

    bool ret = true; //return value

    //check if config
    const char *webdav_config;
    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &webdav_config) == SCDC_ARG_REF_NULL) {
        SCDC_ERROR("open: getting WebDAV configuration");
        ret = false;
        goto do_quit;
    }

    if (!scdc_dataprov::open(conf, args)) {
        SCDC_FAIL("open: opening base");
        ret = false;

    } else {

        //set config struct
        if (!set_session_handler_config(webdav_config)) {
            SCDC_ERROR("open: setting WebDAV configuration");
            ret = false;
            goto do_close;
        }

        //connect to Server
        if (!session_handler->new_session()) {
            SCDC_ERROR("open: connect to server");
            ret = false;
            goto do_close;
        }

        dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_pwd));
        dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_info));
        dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_cd));
        dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_ls));
        dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_put));
        dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_get));
        dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f> (&scdc_dataset_webdav_store::do_cmd_rm));

do_close:
        if (!ret) scdc_dataprov::close();
    }

do_quit:
    return ret;
}

bool scdc_dataprov_webdav_store::set_session_handler_config(const string& config) {
    stringlist confs(':', config);
    string protocol, tmp;

    confs.front_pop(protocol);
    session_handler->set_protocol("http"); //default is HTTP

    //Host
    if (confs.front_pop(tmp)) {
        session_handler->set_host(tmp);
    } else {
        SCDC_ERROR("setting session configuration: no Host specified");
        return false;
    }


    //base_path
    if (confs.front_pop(tmp)) {
        session_handler->set_base_path(tmp);
    } else {
        SCDC_ERROR("setting session configuration: no base path specified");
        return false;
    }

    //if protocol is https a username and password is needed
    if (protocol.compare("https") == 0 || protocol.compare("HTTPS") == 0) {
        session_handler->set_protocol("https");

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

        session_handler->set_username_password(username, password);

    }
    return true;
}

void scdc_dataprov_webdav_store::close() {
    SCDC_TRACE("close:");
    scdc_dataprov::close();
}

scdc_dataset *scdc_dataprov_webdav_store::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output) {
    SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

    scdc_dataset *dataset = 0;

    if (config_open(path, path_size, output, &dataset)) return dataset;

    scdc_dataset_webdav_store *dataset_webdav = new scdc_dataset_webdav_store(this);

    if (path && !dataset_webdav->do_cmd_cd(string(path, path_size).c_str(), NULL, output)) {
        SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
        delete dataset_webdav;
        return 0;
    }

    SCDC_TRACE("dataset_open: return: '" << dataset_webdav << "'");

    return dataset_webdav;
}

void scdc_dataprov_webdav_store::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output) {
    SCDC_TRACE("dataset_close: '" << dataset << "'");

    if (config_close(dataset, output)) return;

    delete dataset;

    SCDC_TRACE("dataset_close: return");
}

#undef SCDC_LOG_PREFIX
