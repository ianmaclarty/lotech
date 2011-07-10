#include <string.h>
#include "ltpickle.h"

#define INITIAL_CAPACITY 1024

LTPickler::LTPickler() {
    size = 0;
    capacity = INITIAL_CAPACITY;
    data = (unsigned char*)malloc(capacity);
}

LTPickler::~LTPickler() {
    free(data);
}

void LTPickler::writeByte(unsigned char b) {
    writeData(&b, 1);
}

void LTPickler::writeString(const char *val) {
    int n = strlen(val) + 1;
    writeInt(n);
    writeData((unsigned char*)val, n);
}

void LTPickler::writeDouble(LTdouble val) {
    writeData(&val, sizeof(LTdouble));
}

void LTPickler::writeBool(bool val) {
    writeByte(val ? 1 : 0);
}

void LTPickler::writeInt(int val) {
    writeData(&val, sizeof(int));
}

void LTPickler::writeData(void *buf, int n) {
    ensureBigEnough(n);
    memcpy(&data[size], buf, n);
    size += n;
}

void LTPickler::ensureBigEnough(int n) {
    while (capacity < size + n) {
        capacity *= 2;
        data = (unsigned char*)realloc(data, capacity);
    }
}

LTUnpickler::LTUnpickler(const void *d, int s) {
    size = s;
    data = (unsigned char*)malloc(size);
    memcpy(data, d, s);
    pos = 0;
}

LTUnpickler::~LTUnpickler() {
    free(data);
}

bool LTUnpickler::eof() {
    return pos >= size;
}

unsigned char LTUnpickler::readByte() {
    unsigned char b;
    readData(&b, 1);
    return b;
}

const char* LTUnpickler::readString() {
    int n = readInt();
    char *buf = new char[n];
    readData(buf, n);
    return buf;
}

LTdouble LTUnpickler::readDouble() {
    LTdouble d;
    readData(&d, sizeof(LTdouble));
    return d;
}

bool LTUnpickler::readBool() {
    unsigned char b = readByte();
    return b == 0 ? false : true;
}

int LTUnpickler::readInt() {
    int i;
    readData(&i, sizeof(int));
    return i;
}

void LTUnpickler::readData(void *buf, int n) {
    memcpy(buf, &data[pos], n);
    pos += n;
}
