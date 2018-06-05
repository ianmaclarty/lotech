/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

#if !defined(LTIOS) && !defined(LTANDROID)
LT_INIT_IMPL(lthttp)

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
    ensure_curl_global_init();

    if (url == NULL) {
        luaL_error(L, "missing URL");
    }

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

LT_REGISTER_TYPE(LTHTTPRequest, "lt.HTTPRequest", "lt.Object")

void LTHTTPRequest::cancel() {
    if (!is_done) {
        cleanup_handles(this);
        is_done = true;
        strcpy(err_buf, "request cancelled");
    }
}

static int do_poll(lua_State *L) {
    LTHTTPRequest *req = lt_expect_LTHTTPRequest(L, 1);
    req->poll();
    return 0;
}

static int do_cancel(lua_State *L) {
    LTHTTPRequest *req = lt_expect_LTHTTPRequest(L, 1);
    req->cancel();
    return 0;
}

static LTbool get_success(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->is_done && req->err_buf[0] == 0;
}

static LTbool get_failure(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->is_done && req->err_buf[0] != 0;
}

static LTstring get_response(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->write_buf;
}

static LTstring get_error(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->err_buf;
}

static LTstring get_url(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->url;
}

static void set_url(LTObject *obj, LTstring url) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    if (req->url != NULL) {
        return; // Can't set url twice.
    }
    if (url == NULL) {
        req->url = NULL;
    } else {
        req->url = new char[strlen(url) + 1];
        strcpy(req->url, url);
    }
}

static LTstring get_data(LTObject *obj) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    return req->post_data;
}

static void set_data(LTObject *obj, LTstring data) {
    LTHTTPRequest *req = (LTHTTPRequest*)obj;
    if (req->post_data != NULL) {
        return; // Can't set post data twice.
    }
    if (data == NULL) {
        req->post_data = NULL;
    } else {
        req->post_data = new char[strlen(data) + 1];
        strcpy(req->post_data, data);
    }
}

LT_REGISTER_PROPERTY_STRING(LTHTTPRequest, url, get_url, set_url)
LT_REGISTER_PROPERTY_STRING(LTHTTPRequest, data, get_data, set_data)
LT_REGISTER_METHOD(LTHTTPRequest, Poll, do_poll)
LT_REGISTER_METHOD(LTHTTPRequest, Cancel, do_cancel)
LT_REGISTER_PROPERTY_BOOL_NOCONS(LTHTTPRequest, success, get_success, NULL)
LT_REGISTER_PROPERTY_BOOL_NOCONS(LTHTTPRequest, failure, get_failure, NULL)
LT_REGISTER_PROPERTY_STRING_NOCONS(LTHTTPRequest, response, get_response, NULL)
LT_REGISTER_PROPERTY_STRING_NOCONS(LTHTTPRequest, error, get_error, NULL)

#endif
