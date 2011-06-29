/* Copyright (C) 2010 Ian MacLarty */
#ifdef LTIOS
#ifndef LTIOSGAMECENTER_H
#define LTIOSGAMECENTER_H

void ltIOSInitGameCenter(); 
void ltIOSTeardownGameCenter();
void ltIOSSubmitGameCenterScore(int score, const char *leaderboard);
void ltIOSShowGameCenterLeaderBoard(const char *leaderboard);

#endif
#endif
