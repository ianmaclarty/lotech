/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltpickle)

struct LTPickler {
    int size;
    int capacity;
    unsigned char *data;

    LTPickler();
    virtual ~LTPickler();

    void writeByte(unsigned char b);
    void writeString(const char *val);
    void writeDouble(LTdouble val);
    void writeBool(bool val);
    void writeInt(int val);
    void writeData(void *buf, int n);

    private:
    void ensureBigEnough(int n);
};

struct LTUnpickler {
    int size;
    int pos;
    unsigned char *data;

    // LTUnpickler makes a copy of data.
    LTUnpickler(const void *data, int size);
    virtual ~LTUnpickler();

    bool eof();
    unsigned char readByte();
    // The caller must free the returned string with delete[].
    const char* readString();
    LTdouble readDouble();
    bool readBool();
    int readInt();
    void readData(void *buf, int n);
};
