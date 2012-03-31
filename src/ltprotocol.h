/* Copyright (C) 2011 Ian MacLarty */
#ifndef LTMINGW

bool ltAmClient();
void ltClientInit();
void ltClientShutdown();
void ltClientStep();
void ltClientLog(const char *msg);
bool ltClientIsTryingToConnect();
bool ltClientIsReady();

bool ltAmServer();
void ltServerInit();
void ltServerStep();
bool ltServerIsReady();
void ltServerClientUpdateFile(const char *file);
void ltServerClientReset();
void ltServerClientSuspend();
void ltServerClientResume();
void ltServerShutdown();
const char* ltServerStateStr();

// Returns NULL if no more logs.  Free the returned string with delete[].
char *ltPopClientLog();

#endif
