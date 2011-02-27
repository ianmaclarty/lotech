/* Copyright (C) 2011 Ian MacLarty */
#ifndef LTPROTOCOL_H
#define LTPROTOCOL_H

bool ltAmClient();
void ltClientInit();
void ltClientStep();
void ltClientLog(const char *msg);

bool ltAmServer();
void ltServerInit();
void ltServerStep();
void ltServerUpdateFile(const char *file);
void ltServerReset();

#endif
