#ifndef TIMEDIFF_STATS_H
#define TIMEDIFF_STATS_H

#include "includes.h"

void StatsInsert(const char *Title, struct timeval *When, double timediff);
void StatsDisplay();


#endif
