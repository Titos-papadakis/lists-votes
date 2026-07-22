/*
 * ============================================
 * file: main.c
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 24/10/2023
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Main function
 *        for CS240 Project Phase 1,
 *        Winter Semester 2024-2025
 * @see   Compile using supplied Makefile by running: make
 * ============================================
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "elections.h"
#include "elections.c"

/* Maximum input line size */
#define MAX_LINE 1024

/* 
 * Uncomment the following line to
 * enable debugging prints
 * or comment to disable it
 */
/* #define DEBUG */
#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...)
#endif /* DEBUG */

/*
 * Globals:
 * you may add some here for certain events
 * (such as D and P)
 */
struct district Districts[56];
struct party Parties[5];
struct parliament Parliament;

void destroy_structures(void)
{
	/*
	 * TODO: For a bonus
	 * empty all structures
	 * and free all memory associated
	 * with list nodes here
	 */
}

int main(int argc, char *argv[])
{
	FILE *event_file;
	char line_buffer[MAX_LINE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	event_file = fopen(argv[1], "r");
	if (!event_file) {
		perror("fopen error for event file open");
		exit(EXIT_FAILURE);
	}

	while (fgets(line_buffer, MAX_LINE, event_file)) {
		char *trimmed_line;
		char event;
		int did, sid, vid, cid, pid, seats;
		int ret = 0;
		/*
		 * First trim any whitespace
		 * leading the line.
		 */
		trimmed_line = line_buffer;
		while (trimmed_line && isspace(*trimmed_line))
			trimmed_line++;
		if (!trimmed_line)
			continue;
		/* 
		 * Find the event,
		 * or comment starting with #
		 */
		if (sscanf(trimmed_line, "%c", &event) != 1) {
			fprintf(stderr, "Could not parse event type out of input line:\n\t%s",
					trimmed_line);
			fclose(event_file);
			exit(EXIT_FAILURE);
		}

		switch (event) {
			/* Comment, ignore this line */
			case '#':
				break;
			case 'A':
				announce_elections();
				break;
			case 'D':
				if (sscanf(trimmed_line, "D %d %d", &did, &seats) != 2) {
					fprintf(stderr, "Event D parsing error\n");
					ret = 1;
					break;
				}
				ret = create_district(did, seats);
				break;
			case 'S':
				if (sscanf(trimmed_line, "S %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event S parsing error\n");
					ret = 1;
					break;
				}
				ret = create_station(sid, did);
				break;
			case 'P':
				if (sscanf(trimmed_line, "P %d", &pid) != 1) {
					fprintf(stderr, "Event P parsing error\n");
					ret = 1;
					break;
				}
				create_party(pid);
				break;
			case 'C':
				if (sscanf(trimmed_line, "C %d %d %d", &cid, &did, &pid) != 3) {
					fprintf(stderr, "Event C parsing error\n");
					ret = 1;
					break;
				}
				ret = register_candidate(cid, did, pid);
				break;
			case 'R':
				if (sscanf(trimmed_line, "R %d %d %d", &vid, &did, &sid) != 3) {
					fprintf(stderr, "Event R parsing error\n");
					ret = 1;
					break;
				}
				ret = register_voter(vid, did, sid);
				break;
			case 'U':
				if (sscanf(trimmed_line, "U %d", &vid) != 1) {
					fprintf(stderr, "Event U parsing error\n");
					ret = 1;
					break;
				}
				ret = unregister_voter(vid);
				break;
			case 'E':
				delete_empty_stations();
				break;
			case 'V':
				if (sscanf(trimmed_line, "V %d %d %d", &vid, &sid, &cid) != 3) {
					fprintf(stderr, "Event V parsing error\n");
					ret = 1;
					break;
				}
				ret = vote(vid, sid, cid);
				break;
			case 'M':
				if (sscanf(trimmed_line, "M %d", &did) != 1) {
					fprintf(stderr, "Event M parsing error\n");
					ret = 1;
					break;
				}
				count_votes(did);
				break;
			case 'G':
				form_government();
				break;
			case 'N':
				form_parliament();
				break;
			case 'I':
				if (sscanf(trimmed_line, "I %d", &did) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_district(did);
				break;
			case 'J':
				if (sscanf(trimmed_line, "J %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event J parsing error\n");
					ret = 1;
					break;
				}
				print_station(sid, did);
				break;
			case 'K':
				if (sscanf(trimmed_line, "K %d", &pid) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_party(pid);
				break;
			case 'L':
				print_parliament();
				break;
			default:
				fprintf(stderr, "WARNING: Unrecognized event %c. Continuing...\n",
						event);
				ret = 1;
				break;
		}
		fprintf(stderr, "Event %c %s.\n", event,
				ret ? "failed" : "succeeded");
	}
	fclose(event_file);
	destroy_structures();
	return 0;
}
