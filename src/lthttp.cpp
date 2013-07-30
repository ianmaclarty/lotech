/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LTHTTPRequest::LTHTTPRequest() {
    url = NULL;
    post_data = NULL;
    mcurl = NULL;
    curl = NULL;
    is_done = false;

    write_buf_capacity = 512;
    write_buf = new char[write_buf_capacity];
    write_buf[0] = 0;
    write_buf_pos = 0;

    err_buf = new char[CURL_ERROR_SIZE];
    memset(err_buf, 0, CURL_ERROR_SIZE);
}

static void cleanup_handles(LTHTTPRequest *req) {
    if (req->curl != NULL) {
        assert(req->mcurl != NULL);
        curl_multi_remove_handle(req->mcurl, req->curl);
        curl_easy_cleanup(req->curl);
        curl_multi_cleanup(req->mcurl);
        req->curl = NULL;
        req->mcurl = NULL;
    }
}

LTHTTPRequest::~LTHTTPRequest() {
    if (url != NULL) delete[] url;
    if (post_data != NULL) delete[] post_data;
    cleanup_handles(this);
    delete [] write_buf;
    delete [] err_buf;
}

static void ensure_curl_global_init() {
    static bool is_init = false;
    if (!is_init) {
        curl_global_init(CURL_GLOBAL_ALL);
        is_init = true;
    }
}

static size_t write_func(char *ptr, size_t size, size_t nmemb, void *userdata) {
    LTHTTPRequest *req = (LTHTTPRequest*)userdata;
    int nbytes = (int)(size * nmemb);

    int available = req->write_buf_capacity - req->write_buf_pos;
    int new_capacity = req->write_buf_capacity;
    while (available < nbytes + 1) {
        new_capacity *= 2;
        available = new_capacity - req->write_buf_pos;
    }

    if (new_capacity > req->write_buf_capacity) {
        char *new_buf = new char[new_capacity];
        if (req->write_buf_pos > 0) {
            memcpy(new_buf, req->write_buf, req->write_buf_pos);
        }
        delete [] req->write_buf;
        req->write_buf = new_buf;
    }

    memcpy(&req->write_buf[req->write_buf_pos], ptr, nbytes);
    req->write_buf_pos += nbytes;
    req->write_buf[req->write_buf_pos] = 0;

    return nbytes;
}

int static progress_func(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    LTHTTPRequest *req = (LTHTTPRequest*)clientp;
    req->download_total = dltotal;
    req->download_now = dlnow;
    return 0;
}

void LTHTTPRequest::init(lua_State *L) {
    const char *tmps;
    int nargs = ltLuaCheckNArgs(L, 1);

    tmps = lua_tostring(L, 1);
    if (tmps == NULL) {
        luaL_error(L, "First arg must be a URL");
    }
    url = new char[strlen(tmps) + 1];
    strcpy(url, tmps);

    if (nargs > 1) {
        tmps = lua_tostring(L, 2);
        if (tmps == NULL) {
            luaL_error(L, "Second arg must be post data as a string");
        }
        post_data = new char[strlen(tmps) + 1];
        strcpy(post_data, tmps);
    }

    ensure_curl_global_init();

    mcurl = curl_multi_init();
    if (mcurl == NULL) {
        strcpy(err_buf, "mcurl initialisation failed");
        return;
    }
    curl = curl_easy_init();
    if (curl == NULL) {
        strcpy(err_buf, "curl initialisation failed");
        curl_multi_cleanup(mcurl);
        mcurl = NULL;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (post_data != NULL) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);

    curl_multi_add_handle(mcurl, curl);

    poll();
}

void LTHTTPRequest::poll() {
    if (!is_done) {
        int running;
        curl_multi_perform(mcurl, &running);
        if (running == 0) {
            is_done = true;
            cleanup_handles(this);
        }
    }
}

void LTHTTPRequest::cancel() {
    cleanup_handles(this);
}
