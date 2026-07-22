/*
 * elections.c
 * CS240 - Project Phase 1
 *
 * Implementation of the Greek elections simulation described in the
 * assignment handout. Districts/Parties/Parliament are the global arrays
 * declared in main.c; this file is #included directly into main.c (see
 * Makefile), so it can use them as plain globals.
 */
#include "elections.h"

#define NUMBER_OF_DISTRICTS 56
#define NUMBER_OF_PARTIES 5

#define SUCCESS 0
#define FAILURE 1

/* short type names, just for readability in this file */
typedef struct candidate candidate;
typedef struct party party;
typedef struct parliament parliament;
typedef struct voter voter;
typedef struct station station;
typedef struct district district;

extern district Districts[NUMBER_OF_DISTRICTS];
extern party Parties[NUMBER_OF_PARTIES];
extern parliament Parliament;

/* Districts/Parties are arrays with all valid entries packed at the
 * front (positions 0..count-1), so a new entry always goes to
 * Districts[ndistricts] / Parties[nparties] -> O(1) insertion. */
static int ndistricts = 0;
static int nparties = 0;

/* ---------------------------------------------------------- */
/* helper functions                                            */
/* ---------------------------------------------------------- */

static district *find_district(int did)
{
	int i;
	for (i = 0; i < ndistricts; i++)
		if (Districts[i].did == did)
			return &Districts[i];
	return NULL;
}

static station *find_station(district *d, int sid)
{
	station *s;
	for (s = d->stations; s != NULL; s = s->next)
		if (s->sid == sid)
			return s;
	return NULL;
}

static candidate *find_candidate(district *d, int cid)
{
	candidate *c;
	for (c = d->candidates; c != NULL; c = c->next)
		if (c->cid == cid)
			return c;
	return NULL;
}

static int find_party_index(int pid)
{
	int i;
	for (i = 0; i < nparties; i++)
		if (Parties[i].pid == pid)
			return i;
	return -1;
}

/* unlink a candidate from its district's doubly linked list. Used when
 * a candidate gets elected and moves over to the party's list. */
static void unlink_candidate(district *d, candidate *c)
{
	if (c->prev != NULL)
		c->prev->next = c->next;
	else
		d->candidates = c->next;

	if (c->next != NULL)
		c->next->prev = c->prev;

	c->prev = NULL;
	c->next = NULL;
}

/* insert an elected candidate into the party's elected list, keeping it
 * sorted by votes (descending). The list is singly-linked so prev is
 * left NULL, as required. */
static void insert_into_party(party *p, candidate *c)
{
	candidate *cur;

	c->prev = NULL;

	if (p->elected == NULL || c->votes > p->elected->votes) {
		c->next = p->elected;
		p->elected = c;
		return;
	}

	cur = p->elected;
	while (cur->next != NULL && cur->next->votes >= c->votes)
		cur = cur->next;

	c->next = cur->next;
	cur->next = c;
}

/* ---------------------------------------------------------- */
/* Event A                                                     */
/* ---------------------------------------------------------- */
void announce_elections(void)
{
	int i;

	for (i = 0; i < NUMBER_OF_DISTRICTS; i++) {
		Districts[i].did = -1;
		Districts[i].seats = -1;
		Districts[i].allotted = -1;
		Districts[i].blanks = -1;
		Districts[i].voids = -1;
		Districts[i].stations = NULL;
		Districts[i].candidates = NULL;
	}

	for (i = 0; i < NUMBER_OF_PARTIES; i++) {
		Parties[i].pid = -1;
		Parties[i].nelected = -1;
		Parties[i].elected = NULL;
	}

	Parliament.members = NULL;

	ndistricts = 0;
	nparties = 0;

	printf("A\n");
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event D                                                     */
/* ---------------------------------------------------------- */
int create_district(int did, int seats)
{
	int i;

	if (ndistricts >= NUMBER_OF_DISTRICTS)
		return FAILURE;
	if (find_district(did) != NULL)
		return FAILURE;

	Districts[ndistricts].did = did;
	Districts[ndistricts].seats = seats;
	Districts[ndistricts].allotted = 0;
	Districts[ndistricts].blanks = 0;
	Districts[ndistricts].voids = 0;
	Districts[ndistricts].stations = NULL;
	Districts[ndistricts].candidates = NULL;
	ndistricts++;

	printf("D %d %d\n", did, seats);
	printf("  Districts = ");
	for (i = 0; i < ndistricts; i++)
		printf(i == 0 ? "%d" : ", %d", Districts[i].did);
	printf("\n");
	printf("DONE\n");

	return SUCCESS;
}

/* ---------------------------------------------------------- */
/* Event S                                                     */
/* ---------------------------------------------------------- */
int create_station(int sid, int did)
{
	district *d = find_district(did);
	station *new_s, *s;
	voter *sentinel;
	int first;

	if (d == NULL)
		return FAILURE;
	if (find_station(d, sid) != NULL)
		return FAILURE;

	sentinel = malloc(sizeof(voter));
	sentinel->vid = -1;
	sentinel->voted = -1;
	sentinel->next = NULL;

	new_s = malloc(sizeof(station));
	new_s->sid = sid;
	new_s->registered = 0;
	new_s->voters = sentinel;    /* empty list = just the guard node */
	new_s->vsentinel = sentinel;
	new_s->next = d->stations;
	d->stations = new_s;

	printf("S %d %d\n", sid, did);
	printf("  Stations = ");
	first = 1;
	for (s = d->stations; s != NULL; s = s->next) {
		printf(first ? "%d" : ", %d", s->sid);
		first = 0;
	}
	printf("\n");
	printf("DONE\n");

	return SUCCESS;
}

/* ---------------------------------------------------------- */
/* Event P                                                     */
/* ---------------------------------------------------------- */
void create_party(int pid)
{
	int i;

	if (nparties >= NUMBER_OF_PARTIES)
		return;
	if (find_party_index(pid) >= 0)
		return;

	Parties[nparties].pid = pid;
	Parties[nparties].nelected = 0;
	Parties[nparties].elected = NULL;
	nparties++;

	printf("P %d\n", pid);
	printf("  Parties = ");
	for (i = 0; i < nparties; i++)
		printf(i == 0 ? "%d" : ", %d", Parties[i].pid);
	printf("\n");
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event C                                                     */
/* ---------------------------------------------------------- */
int register_candidate(int cid, int did, int pid)
{
	district *d;
	candidate *new_c, *c;
	int first;

	if (cid == 0 || cid == 1)
		return FAILURE;
	if (find_party_index(pid) < 0)
		return FAILURE;

	d = find_district(did);
	if (d == NULL)
		return FAILURE;
	if (find_candidate(d, cid) != NULL)
		return FAILURE;

	new_c = malloc(sizeof(candidate));
	new_c->cid = cid;
	new_c->pid = pid;
	new_c->votes = 0;
	new_c->elected = 0;
	new_c->next = NULL;
	new_c->prev = NULL;

	/* every new candidate starts at 0 votes, so appending at the tail
	 * keeps the list sorted by votes (descending) */
	if (d->candidates == NULL) {
		d->candidates = new_c;
	} else {
		c = d->candidates;
		while (c->next != NULL)
			c = c->next;
		c->next = new_c;
		new_c->prev = c;
	}

	printf("C %d %d %d\n", cid, did, pid);
	printf("  Candidates = ");
	first = 1;
	for (c = d->candidates; c != NULL; c = c->next) {
		printf(first ? "%d" : ", %d", c->cid);
		first = 0;
	}
	printf("\n");
	printf("DONE\n");

	return SUCCESS;
}

/* ---------------------------------------------------------- */
/* Event R                                                     */
/* ---------------------------------------------------------- */
int register_voter(int vid, int did, int sid)
{
	district *d;
	station *s;
	voter *new_v, *v;
	int first;

	d = find_district(did);
	if (d == NULL)
		return FAILURE;
	s = find_station(d, sid);
	if (s == NULL)
		return FAILURE;

	new_v = malloc(sizeof(voter));
	new_v->vid = vid;
	new_v->voted = 0;
	new_v->next = s->voters;  /* push to front, sentinel stays at the end */
	s->voters = new_v;
	s->registered++;

	printf("R %d %d %d\n", vid, did, sid);
	printf("  Voters = ");
	first = 1;
	for (v = s->voters; v != s->vsentinel; v = v->next) {
		printf(first ? "%d" : ", %d", v->vid);
		first = 0;
	}
	printf("\n");
	printf("DONE\n");

	return SUCCESS;
}

/* ---------------------------------------------------------- */
/* Event U                                                     */
/* ---------------------------------------------------------- */
int unregister_voter(int vid)
{
	int i, first;
	district *d;
	station *s;
	voter *prev, *cur;

	for (i = 0; i < ndistricts; i++) {
		d = &Districts[i];
		for (s = d->stations; s != NULL; s = s->next) {
			prev = NULL;
			cur = s->voters;
			while (cur != s->vsentinel) {
				if (cur->vid == vid) {
					if (prev == NULL)
						s->voters = cur->next;
					else
						prev->next = cur->next;
					free(cur);
					s->registered--;

					printf("U %d\n", vid);
					printf("  %d %d\n", d->did, s->sid);
					printf("  Voters = ");
					first = 1;
					for (cur = s->voters; cur != s->vsentinel; cur = cur->next) {
						printf(first ? "%d" : ", %d", cur->vid);
						first = 0;
					}
					printf("\n");
					printf("DONE\n");

					return SUCCESS;
				}
				prev = cur;
				cur = cur->next;
			}
		}
	}

	return FAILURE;
}

/* ---------------------------------------------------------- */
/* Event E                                                     */
/* ---------------------------------------------------------- */
void delete_empty_stations(void)
{
	int i;
	district *d;
	station *cur, *prev, *to_delete;

	printf("E\n");

	for (i = 0; i < ndistricts; i++) {
		d = &Districts[i];
		prev = NULL;
		cur = d->stations;
		while (cur != NULL) {
			if (cur->registered == 0) {
				to_delete = cur;
				if (prev == NULL)
					d->stations = cur->next;
				else
					prev->next = cur->next;
				cur = cur->next;

				printf("  %d %d\n", to_delete->sid, d->did);
				free(to_delete->vsentinel);
				free(to_delete);
			} else {
				prev = cur;
				cur = cur->next;
			}
		}
	}

	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event V                                                     */
/* ---------------------------------------------------------- */
static void print_vote_result(district *d, int vid, int sid, int cid)
{
	candidate *c;
	int first;

	printf("V %d %d %d\n", vid, sid, cid);
	printf("  District = %d\n", d->did);
	printf("  Candidate votes = ");
	first = 1;
	for (c = d->candidates; c != NULL; c = c->next) {
		printf(first ? "(%d, %d)" : ", (%d, %d)", c->cid, c->votes);
		first = 0;
	}
	printf("\n");
	printf("  Blanks = %d\n", d->blanks);
	printf("  Voids = %d\n", d->voids);
	printf("DONE\n");
}

int vote(int vid, int sid, int cid)
{
	int i;
	district *d = NULL;
	station *s = NULL;
	voter *v;
	candidate *c = NULL;

	for (i = 0; i < ndistricts; i++) {
		s = find_station(&Districts[i], sid);
		if (s != NULL) {
			d = &Districts[i];
			break;
		}
	}
	if (s == NULL)
		return FAILURE;

	for (v = s->voters; v != s->vsentinel; v = v->next)
		if (v->vid == vid)
			break;
	if (v == s->vsentinel || v->voted == 1)
		return FAILURE;

	if (cid == 0) {
		d->blanks++;
	} else if (cid == 1) {
		d->voids++;
	} else {
		c = find_candidate(d, cid);
		if (c == NULL)
			return FAILURE;
		c->votes++;
		/* with ties in the list, one extra vote can outrank more than
		 * just the previous node, so keep swapping up until sorted */
		while (c->prev != NULL && c->votes > c->prev->votes) {
			candidate *p = c->prev;
			int tcid = c->cid, tpid = c->pid, tvotes = c->votes, telected = c->elected;
			c->cid = p->cid; c->pid = p->pid; c->votes = p->votes; c->elected = p->elected;
			p->cid = tcid; p->pid = tpid; p->votes = tvotes; p->elected = telected;
			c = p;
		}
	}

	v->voted = 1;
	print_vote_result(d, vid, sid, cid);

	return SUCCESS;
}

/* ---------------------------------------------------------- */
/* Event M                                                     */
/* ---------------------------------------------------------- */
void count_votes(int did)
{
	district *d = find_district(did);
	int pvotes[NUMBER_OF_PARTIES] = { 0 };
	int quota[NUMBER_OF_PARTIES] = { 0 };
	int total_votes = 0;
	int i, idx;
	candidate *c, *next_c;

	if (d == NULL)
		return;

	/* pass 1/2: total votes per party, needed for the electoral quotient */
	for (c = d->candidates; c != NULL; c = c->next) {
		idx = find_party_index(c->pid);
		if (idx >= 0)
			pvotes[idx] += c->votes;
		total_votes += c->votes;
	}

	if (total_votes > 0) {
		for (i = 0; i < nparties; i++)
			quota[i] = (int) ((long long) pvotes[i] * d->seats / total_votes);
	}

	printf("M %d\n", did);
	printf("  Seats =\n");

	/* pass 2/2: the list is already sorted by votes, so scanning it once
	 * and electing while a party still has quota left automatically picks
	 * that party's top-voted candidates first */
	c = d->candidates;
	while (c != NULL) {
		next_c = c->next;
		idx = find_party_index(c->pid);
		if (idx >= 0 && quota[idx] > 0) {
			unlink_candidate(d, c);
			c->elected = 1;
			insert_into_party(&Parties[idx], c);
			Parties[idx].nelected++;
			d->allotted++;
			quota[idx]--;

			printf("    %d %d %d\n", c->cid, c->pid, c->votes);
		}
		c = next_c;
	}

	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event G                                                     */
/* ---------------------------------------------------------- */
void form_government(void)
{
	int i, idx, first, remaining;
	district *d;
	candidate *c, *next_c;

	printf("G\n");
	printf("  Seats =\n");

	/* the party with the most elected seats overall gets priority on
	 * whatever seats are left over in each district */
	first = -1;
	for (i = 0; i < nparties; i++)
		if (first == -1 || Parties[i].nelected > Parties[first].nelected)
			first = i;

	for (i = 0; i < ndistricts; i++) {
		d = &Districts[i];
		remaining = d->seats - d->allotted;
		if (remaining <= 0)
			continue;

		/* first give the leading party its own remaining candidates */
		c = d->candidates;
		while (c != NULL && remaining > 0) {
			next_c = c->next;
			if (first != -1 && c->pid == Parties[first].pid) {
				unlink_candidate(d, c);
				c->elected = 1;
				insert_into_party(&Parties[first], c);
				Parties[first].nelected++;
				d->allotted++;
				remaining--;

				printf("    %d %d %d\n", d->did, c->cid, c->votes);
			}
			c = next_c;
		}

		/* still seats left over: take whoever has the most votes next,
		 * regardless of party (the list is still sorted by votes) */
		while (remaining > 0 && d->candidates != NULL) {
			c = d->candidates;
			unlink_candidate(d, c);
			c->elected = 1;

			idx = find_party_index(c->pid);
			if (idx >= 0) {
				insert_into_party(&Parties[idx], c);
				Parties[idx].nelected++;
			}
			d->allotted++;
			remaining--;

			printf("    %d %d %d\n", d->did, c->cid, c->votes);
		}
	}

	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event N                                                     */
/* ---------------------------------------------------------- */
void form_parliament(void)
{
	candidate *heads[NUMBER_OF_PARTIES];
	candidate *tail, *copy, *c;
	int i, best;

	for (i = 0; i < nparties; i++)
		heads[i] = Parties[i].elected;

	Parliament.members = NULL;
	tail = NULL;

	/* 5-way merge of the (already sorted) party lists. Only 5 parties, so
	 * picking the best head each time is still O(1) per element -> O(n) total.
	 * A candidate stays elected for its party too, so we copy it into the
	 * parliament list instead of relinking the original node. */
	while (1) {
		best = -1;
		for (i = 0; i < nparties; i++) {
			if (heads[i] != NULL && (best == -1 || heads[i]->votes > heads[best]->votes))
				best = i;
		}
		if (best == -1)
			break;

		copy = malloc(sizeof(candidate));
		copy->cid = heads[best]->cid;
		copy->pid = heads[best]->pid;
		copy->votes = heads[best]->votes;
		copy->elected = heads[best]->elected;
		copy->prev = NULL;
		copy->next = NULL;

		if (tail == NULL)
			Parliament.members = copy;
		else
			tail->next = copy;
		tail = copy;

		heads[best] = heads[best]->next;
	}

	printf("N\n");
	printf("  Members =\n");
	for (c = Parliament.members; c != NULL; c = c->next)
		printf("    %d %d %d\n", c->cid, c->pid, c->votes);
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event I                                                     */
/* ---------------------------------------------------------- */
void print_district(int did)
{
	district *d = find_district(did);
	candidate *c;
	station *s;
	int first;

	if (d == NULL)
		return;

	printf("I %d\n", did);
	printf("  Seats = %d\n", d->seats);
	printf("  Blanks = %d\n", d->blanks);
	printf("  Voids = %d\n", d->voids);
	printf("  Candidates =\n");
	for (c = d->candidates; c != NULL; c = c->next)
		printf("    %d %d %d\n", c->cid, c->pid, c->votes);
	printf("  Stations = ");
	first = 1;
	for (s = d->stations; s != NULL; s = s->next) {
		printf(first ? "%d" : ", %d", s->sid);
		first = 0;
	}
	printf("\n");
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event J                                                     */
/* ---------------------------------------------------------- */
void print_station(int sid, int did)
{
	district *d = find_district(did);
	station *s;
	voter *v;

	if (d == NULL)
		return;
	s = find_station(d, sid);
	if (s == NULL)
		return;

	printf("J %d\n", sid);
	printf("  Registered = %d\n", s->registered);
	printf("  Voters =\n");
	for (v = s->voters; v != s->vsentinel; v = v->next)
		printf("    %d %d\n", v->vid, v->voted);
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event K                                                     */
/* ---------------------------------------------------------- */
void print_party(int pid)
{
	int idx = find_party_index(pid);
	candidate *c;

	if (idx < 0)
		return;

	printf("K %d\n", pid);
	printf("  Elected =\n");
	for (c = Parties[idx].elected; c != NULL; c = c->next)
		printf("    %d %d\n", c->cid, c->votes);
	printf("DONE\n");
}

/* ---------------------------------------------------------- */
/* Event L                                                     */
/* ---------------------------------------------------------- */
void print_parliament(void)
{
	candidate *c;

	printf("L\n");
	printf("  Members =\n");
	for (c = Parliament.members; c != NULL; c = c->next)
		printf("    %d %d %d\n", c->cid, c->pid, c->votes);
	printf("DONE\n");
}
