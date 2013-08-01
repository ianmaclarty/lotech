/* Copyright (C) 2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(lthttp)

struct LTHTTPRequest : LTObject {
    LTHTTPRequest();
    virtual ~LTHTTPRequest();

    char *url;
    char *post_data;

    char *write_buf;
    int write_buf_capacity;
    int write_buf_pos;

    char *err_buf;

    CURLM* mcurl; 
    CURL*  curl;

    LTfloat download_total; // may be 0 if total not yet known.
    LTfloat download_now;
    LTbool is_done;

    virtual void init(lua_State *L);
    void poll();
    void cancel();
};
