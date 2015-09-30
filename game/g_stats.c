#include "g_local.h"

//copy some existing data from other vars
void G_StatsInitClient(gclient_t *cl) {
	stats_t *stats = &cl->pers.stats;
	stats->sb.score = cl->ps.persistant[PERS_SCORE];
	stats->sb.accuracy = cl->accuracy_shots ? (cl->accuracy_hits * 100 / cl->accuracy_shots) : 0;
	stats->sb.dmgCaused = cl->pers.damageCaused;
	stats->sb.dmgTaken = cl->pers.damageTaken;

	stats->gt.ctf.fragcarrier = cl->pers.teamState.fragcarrier;
	stats->gt.ctf.flagRets = cl->pers.teamState.flagrecovery;
	stats->gt.ctf.flagHold = cl->pers.teamState.flaghold;

	stats->rw.captures = cl->ps.persistant[PERS_CAPTURES];
	stats->rw.assist = cl->ps.persistant[PERS_ASSIST_COUNT];
	stats->rw.defend = cl->ps.persistant[PERS_DEFEND_COUNT];
	stats->rw.excellent = cl->ps.persistant[PERS_EXCELLENT_COUNT];
	stats->rw.impressive = cl->ps.persistant[PERS_IMPRESSIVE_COUNT];
	stats->rw.humiliation = cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT];
}

void G_StatsInit(void) {
	int i;
	for (i = 0; i < level.maxclients; i++) {
		gentity_t *ent = &g_entities[i];
		if (!ent->client || !ent->inuse)
			continue;
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			continue;
		G_StatsInitClient(ent->client);
	}
}

static char line[1022];

char* G_StatsGetColor(int stats, int best) {
	if (best == stats && best != 0) {
		return S_COLOR_GREEN;
	} else {
		return S_COLOR_WHITE;
	}
}

//*fmt should always come with %d
void G_StatsStrcat(const char *fmt, int stats, int best) {
	char *colour = G_StatsGetColor(stats, best);
	Q_strcat(line, sizeof(line), va("%s%s", colour, fmt, stats));
}

void G_StatsPrintIndent(int nameLenMax, int flags) {
	int i = 0, gt = g_gametype.integer;

	if (gt >= GT_TEAM)
		Q_strcat(line, sizeof(line), S_COLOR_CYAN"---- ");

	for (i = 0; i < nameLenMax; i++)
		Q_strcat(line, sizeof(line), "-");

	if (flags & STATS_SCOREBOARD)
		Q_strcat(line, sizeof(line), " ----- --- ---- ---- ----");

	if (gt == GT_CTF && flags & STATS_GAMETYPE)
		Q_strcat(line, sizeof(line), S_COLOR_RED" ------- ----- -------- -------");

	if (flags & STATS_REWARDS) {
		if (gt == GT_CTF)
			Q_strcat(line, sizeof(line), S_COLOR_CYAN" --- "S_COLOR_YELLOW"--- "S_COLOR_BLUE"---");
		Q_strcat(line, sizeof(line), S_COLOR_YELLOW" --- "S_COLOR_CYAN"--- "S_COLOR_RED"---"S_COLOR_CYAN);
	}
}

void G_StatsPrintHeader(int nameLenMax, int flags) {
	int i = 0, gt = g_gametype.integer;

	if (gt >= GT_TEAM)
		Com_sprintf(line, sizeof(line), S_COLOR_CYAN"TEAM NAME");
	else
		Com_sprintf(line, sizeof(line), S_COLOR_CYAN"NAME");

	for (i = 0; i < nameLenMax - 4; i++)
		Q_strcat(line, sizeof(line), " ");

	if (flags & STATS_SCOREBOARD)
		Q_strcat(line, sizeof(line), " SCORE ACC TIME DCSD DTKN");

	if (gt == GT_CTF && flags & STATS_GAMETYPE)
		Q_strcat(line, sizeof(line), S_COLOR_RED" FCKILLS FRETS FLAGHOLD  TH/TE ");

	if (flags & STATS_REWARDS) {
		if (gt == GT_CTF)
			Q_strcat(line, sizeof(line), " "S_COLOR_CYAN"CAP "S_COLOR_YELLOW"ASS "S_COLOR_BLUE"DEF");
		Q_strcat(line, sizeof(line), " "S_COLOR_YELLOW"EXC "S_COLOR_CYAN"IMP "S_COLOR_RED"HUM");
	}

	Q_strcat(line, sizeof(line), S_COLOR_CYAN "\n");

	G_StatsPrintIndent(nameLenMax, flags);
}

void G_StatsPrintTeamName(team_t t) {
	if (t == TEAM_RED)
		Com_sprintf(line, sizeof(line), S_COLOR_RED"RED  ");
	else if (t == TEAM_BLUE)
		Com_sprintf(line, sizeof(line), S_COLOR_BLUE"BLUE ");
}

void G_StatsPrintName(int nameLenMax, gclient_t *cl) {
	int nameLen;

	nameLen = Q_PrintStrlen(cl->pers.netname);

	if (line[0] == '\0')
		Com_sprintf(line, sizeof(line), S_COLOR_WHITE);
	else
		Q_strcat(line, sizeof(line), S_COLOR_WHITE);

	Q_strcat(line, sizeof(line), cl->pers.netname);

	if (nameLen < nameLenMax) {
		int j, d = nameLenMax - nameLen;
		for (j = 0; j < d; j++)
			Q_strcat(line, sizeof(line), " ");
	}
}

void G_StatsPrintStats(stats_t best, gclient_t *cl) {
	stats_t stats = cl->pers.stats;
	Q_strcat(line, sizeof(line), S_COLOR_WHITE" ");
	G_StatsStrcat("%5d ", stats.sb.score, best.sb.score);
	G_StatsStrcat("%3d ", stats.sb.accuracy, best.sb.accuracy);
	Q_strcat(line, sizeof(line), va("%s%4d ", S_COLOR_WHITE, (level.time - cl->pers.enterTime) / 60000));
	G_StatsStrcat("%4d ", stats.sb.dmgCaused, best.sb.dmgCaused);
	G_StatsStrcat("%4d ", stats.sb.dmgTaken, best.sb.dmgTaken);
}

void G_StatsPrintCTFStats(int *stat, playerTeamState_t teamState) {
	int teAmount = 3;

	Q_strcat(line, sizeof(line), va("%s%7d ", G_StatsGetColor(*(stat++), teamState.fragcarrier), teamState.fragcarrier));
	Q_strcat(line, sizeof(line), va("%s%5d ", G_StatsGetColor(*(stat++), teamState.flagrecovery), teamState.flagrecovery));

	int secs = (teamState.flaghold / 1000);
	int mins = (secs / 60);

	if (teamState.flaghold >= 60000) {
		secs %= 60;
		Q_strcat(line, sizeof(line), va("%s%3dm %02ds", G_StatsGetColor(*(stat++), teamState.flaghold), mins, secs));
	}
	else {
		Q_strcat(line, sizeof(line), va("%s%7ds", G_StatsGetColor(*(stat++), teamState.flaghold), secs));
	}

	Q_strcat(line, sizeof(line), va(" %s%3d", G_StatsGetColor(*(stat++), teamState.th), teamState.th));

	if (teamState.te - 100 >= 0)
		teAmount = 3;
	else if (teamState.te - 10 >= 0)
		teAmount = 2;
	else
		teAmount = 1;

	Q_strcat(line, sizeof(line), va(S_COLOR_WHITE"/%s%*d", G_StatsGetColor(*stat, teamState.te), teAmount, teamState.te));

		for (int j = 0; j < 4 - teAmount; j++)
	Q_strcat(line, sizeof(line), " ");
}

void G_StatsPrintRewards(int *stat, int pers[]) {
	if (g_gametype.integer == GT_CTF) {
		Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*(stat++), pers[PERS_CAPTURES]), pers[PERS_CAPTURES]));
		Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*(stat++), pers[PERS_ASSIST_COUNT]), pers[PERS_ASSIST_COUNT]));
		Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*(stat++), pers[PERS_DEFEND_COUNT]), pers[PERS_DEFEND_COUNT]));
	}
	else
		stat += 3;
	Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*(stat++), pers[PERS_EXCELLENT_COUNT]), pers[PERS_EXCELLENT_COUNT]));
	Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*(stat++), pers[PERS_IMPRESSIVE_COUNT]), pers[PERS_IMPRESSIVE_COUNT]));
	Q_strcat(line, sizeof(line), va("%s%3d ", G_StatsGetColor(*stat, pers[PERS_GAUNTLET_FRAG_COUNT]), pers[PERS_GAUNTLET_FRAG_COUNT]));
}

#define STATS_LEN sizeof(stats_t)/sizeof(int)-1 //-start
void G_StatsFindBest(stats_t *best, stats_t *stats) {
	int *givenStats = (&(stats->start))+1;
	int *bestStats = (&(best->start))+1;
	int i, len = STATS_LEN;
	
	for (i = 0; i < len; i++, givenStats++, bestStats++) {
		if (*givenStats > *bestStats)
			*bestStats = *givenStats;
	}
}

void G_StatsPrintTeam(team_t team, int id, int flags) {
	int i, nameLenMax = 0;
	stats_t best;
	gclient_t *cl;

	for (i = 0; i < level.maxclients; i++) {
		if (!g_entities[i].client || !g_entities[i].inuse)
			continue;
		if (g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR)
			continue;

		int nameLen = Q_PrintStrlen(g_entities[i].client->pers.netname);

		if (nameLen > nameLenMax)
			nameLenMax = nameLen;

		if (g_entities[i].client->sess.sessionTeam != team)
			continue;

		G_StatsFindBest(&(g_entities[i].client->pers.stats), &best);
	}

	if (nameLenMax < 4)
		nameLenMax = 4;

	G_StatsPrintHeader(nameLenMax, flags);

	trap_SendServerCommand(id, va("print \"%s\n", line));

	for (i = 0; i < level.numConnectedClients; i++) {

		cl = &level.clients[level.sortedClients[i]];

		if (!cl)
			continue;

		if (cl->sess.sessionTeam != team)
			continue;

		G_StatsPrintTeamName(team);
		G_StatsPrintName(nameLenMax, cl);

		if (flags & STATS_SCOREBOARD)
			G_StatsPrintStats(best, cl);

		if (g_gametype.integer == GT_CTF && flags & STATS_GAMETYPE)
			G_StatsPrintCTFStats(best, cl->pers.teamState);

		if (flags & STATS_REWARDS)
			G_StatsPrintRewards(best, cl->ps.persistant);

		trap_SendServerCommand(id, va("print \"%s\n", line));
	}
	trap_SendServerCommand(id, "print \"\n");
}