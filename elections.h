/*
 * ============================================
 * file: elections.h
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 23/10/2024
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Structure and function declarations
 *        for CS240 Project Phase 1,
 *        Winter Semester 2024-2025
 * ============================================
 */
#ifndef __CS240_ELECTIONS_H__

#define __CS240_ELECTIONS_H__
struct candidate {
	int cid; /* 0 reserved for blanks, 1 for voids */
	int pid;
	int votes;
	int elected;
	struct candidate *next;
	struct candidate *prev;
};

struct party {
	int pid;
	int nelected;
	struct candidate *elected; /* Singly-linked, sorted by decreasing votes */
};

struct parliament {
	struct candidate *members; /* Singly-linked, sorted by decreasing votes */
};

struct voter {
	int vid;
	int voted;
	struct voter *next;
};

struct station {
	int sid;
	int registered; /* Increment on new voter */
	struct voter *voters; /* Singly-linked, unsorted, with sentinel node */
	struct voter *vsentinel;
	struct station *next;
};

struct district {
	int did;
	int seats; /* Total seats given by districts */
	int allotted; /* Seats given to candidates on first vote count */
	int blanks;
	int voids;
	struct station *stations; /* Singly-linked, unsorted */
	struct candidate *candidates; /* Doubly-linked, sorted by decreasing votes */
};

/*
 * Announce Elections - Event A
 * Initialize global structures,
 * i.e., districts array, party array,
 * parliament list.
 */
void announce_elections(void);

/*
 * Create District - Event D
 * Create and initialize a new
 * district with ID=did and
 * total seats=seats. Insert it
 * to global districts array in O(1)
 * time. Returns 0 on success, 1 on
 * failure.
 */
int create_district(int did, int seats);

/*
 * Create Station - Event S
 * Create and initialize a new voting
 * station with ID=sid, and add it to
 * the station list of the district
 * with ID=did. Returns 0 on success, 1 on
 * failure
 */
int create_station(int sid, int did);

/*
 * Create Party - Event P
 * Create and initialize a new party with
 * ID=pid. Add it to the global party array.
 */
void create_party(int pid);

/*
 * Register Candidate - Event C
 * Create and initialize the candidate with ID=cid
 * and party ID=pid. cid may not be any of the
 * reserved values 0 or 1, and pid must be a valid
 * party ID. Add the candidate to the candidate list
 * of district did. Returns 0 on success, 1 on failure
 */
int register_candidate(int cid, int did, int pid);

/*
 * Register voter - Event R
 * Create and initialize a new voter with ID=vid. Add
 * the voter to the voter list of station sid, which is
 * stored in the station list of district did. Returns 0
 * on success, 1 on failure.
 */
int register_voter(int vid, int did, int sid);

/*
 * Unregister voter - Event U
 * Remove voter with ID=vid from the appropriate
 * voter list. Returns 0 on success, 1 on failure.
 */
int unregister_voter(int vid);

/*
 * Delete empty stations - Event E
 * Iterate all voting stations across all districts.
 * Remove from the corresponding station lists any station
 * with 0 registered voters. O(n) time complexity, where
 * n = total voting stations across all districts.
 */
void delete_empty_stations(void);

/*
 * Vote - Event V.
 * Voter vid votes at station sid for candidate cid. It is an error
 * if any input argument is invalid (no voter vid, or no station sid,
 * or cid does not match any registered candidate (and is also not 0 or 1)).
 * Otherwise, add one vote to candidate cid, and swap places with previous
 * candidate in list if candidate cid now has more votes than the previous.
 * Returns 0 on success, 1 on failure.
 */
int vote(int vid, int sid, int cid);

/*
 * Count Votes - Event M
 * Count votes for district with ID=did.
 * Compute the total seats per party, say x, based on total candidate votes.
 * For each party, the first x candidates are removed from the district candidate
 * list, and added to the party's elected candidates list, and the
 * district allotted field is increased by x.
 */
void count_votes(int did);

/*
 * Form Government - Event G
 * Distribute remaining district seats.
 * Start with the first party (by number of elected candidates).
 * For each district, distribute any remaining seat (their total number
 * is seats - allotted) to any remaining candidates of this party, similar
 * to the count votes event. If there are still seats to be distributed and
 * no more candidates of the first party, distribute the remaining seats based
 * on vote count by iterating the candidate list.
 */
void form_government(void);

/*
 * Form Parliament - Event N
 * Form parliament candidate list from party elected
 * candidate lists. Merge the elected candidate list
 * for all parties into one sorted (by decreasing vote
 * count) list. O(n) time complexity, where n = total number
 * of elected candidates.
 */
void form_parliament(void);

/*
 * Print District - Event I
 * Print district information
 * for district with ID=did.
 */
void print_district(int did);

/*
 * Print Station - Event J
 * Print voting station information
 * for station ID=sid, within district
 * with ID=did
 */
void print_station(int sid, int did);

/*
 * Print party - Event K
 * Print party information for party with ID=pid
 */
void print_party(int pid);

/*
 * Print parliament - Event L
 * Print parliament elected candidate list.
 */
void print_parliament(void);
#endif
