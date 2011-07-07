/* Copyright (C) 2010 Ian MacLarty */
#ifdef LTIOS
#ifndef LTGAMECENTER_H
#define LTGAMECENTER_H

void ltIOSInitGameCenter(); 
void ltIOSTeardownGameCenter();
void ltIOSSubmitGameCenterScore(int score, const char *leaderboard);
void ltIOSShowGameCenterLeaderBoard(const char *leaderboard);
bool ltIOSGameCenterIsAvailable();

#endif
#endif
