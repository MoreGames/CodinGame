#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define MAX_CONNECTIONS 3

// returns the first zone towards target
// http://www.redblobgames.com/pathfinding/a-star/introduction.html
// input:
//        graph - zoneCount * MAX_CONNECTIONS
// input/output: 
//        temp1 - zoneCount
//        temp2 - zoneCount
// temp1 ... representing a queue with maximum zoneCount elements / representing the reconstructed path from end to start on output
// temp2 ... representing the origin zone for the current one (came_from)
// all ... if true all zones of the map will be searched
short breathFirstSearch(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, char all) {
	int i, n = 0;
	short zone, next;

	// init queue
	temp1[n++] = startZone;
	// init came_from
	for (i = 0; i < zoneCount; i++) {
		temp2[i] = -1;
	}

	for (; n;) {
		zone = temp1[--n];

		if (!all && zone == endZone)
			break;

		for (i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
			if (temp2[next] == -1) {
				temp1[n++] = next;
				temp2[next] = zone;
			}
		}
	}

	// reconstruct path
	zone = endZone;
	temp1[0] = zone;
	for (i = 1; zone != startZone; i++) {
		zone = temp2[zone];
		temp1[i] = zone;
		if (zone == -1) return -1;
	}

	// print path
	fprintf(stderr, "Step Zone \n");
	for (int j = i - 2, k = 1; j >= 0; j--, k++) {
		fprintf(stderr, "%4d %4hd \n", k, temp1[j]);
	}

	return temp1[i - 2];
}

int main(void) {

	double start, end;
	double interval;

	int zoneCount = 5;

	short* graph;
	short* temp1, *temp2;
	short zone;

	graph = (short*)malloc(zoneCount * MAX_CONNECTIONS * sizeof(short));

	// some kind of queue and path and cost array
	temp1 = (short*)malloc(zoneCount * sizeof(short));
	// came_from reference
	temp2 = (short*)malloc(zoneCount * sizeof(short));

	for (int i = 0; i < zoneCount * MAX_CONNECTIONS; i++) {
		graph[i] = -1;
	}
	graph[0 * MAX_CONNECTIONS + 0] = 1;
	graph[1 * MAX_CONNECTIONS + 0] = 0; graph[1 * MAX_CONNECTIONS + 1] = 2; graph[1 * MAX_CONNECTIONS + 2] = 3;
	graph[2 * MAX_CONNECTIONS + 0] = 1; graph[2 * MAX_CONNECTIONS + 1] = 4;
	graph[3 * MAX_CONNECTIONS + 0] = 1; graph[3 * MAX_CONNECTIONS + 1] = 4;
	graph[4 * MAX_CONNECTIONS + 0] = 2; graph[4 * MAX_CONNECTIONS + 1] = 3;

	// pathfinding with weights
	start = clock();

	for (int i = 0; i < 1; i++)
		zone = breathFirstSearch(graph, zoneCount, 4, 0, temp1, temp2, 1);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);
	fprintf(stderr, "next zone towards %3hd from %3hd: %3hd \n", 0, 4, zone);
}
