/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTIOS

void ltIOSInitGameCenter(); 
void ltIOSTeardownGameCenter();
void ltIOSSubmitGameCenterScore(int score, const char *leaderboard);
void ltIOSShowGameCenterLeaderBoard(const char *leaderboard);
bool ltIOSGameCenterIsAvailable();

#endif
