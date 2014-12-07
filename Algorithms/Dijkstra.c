#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define MAX_CONNECTIONS 3

void insertPriorityQueue(int* size, short* queue, short data, short pri){
	int i;
	for (i = *size - 1; i >= 0 && pri>queue[i * 2 + 1]; i--){
		queue[(i + 1) * 2] = queue[i * 2];
		queue[(i + 1) * 2 + 1] = queue[i * 2 + 1];
	}
	queue[(i + 1) * 2] = data;
	queue[(i + 1) * 2 + 1] = pri;

	*size += 1;
}
short popPriorityQueue(int* size, short* queue){
	*size -= 1;
	return queue[*size * 2];
}
// returns the first zone towards target
// http://www.redblobgames.com/pathfinding/a-star/introduction.html
// input:
//        graph - zoneCount * MAX_CONNECTIONS
//        costZone - zoneCount
// input/output: 
//        temp1 - zoneCount * 2
//        temp2 - zoneCount
//        temp3 - zoneCount
// temp1 ... representing a priority queue with maximum zoneCount elements / representing the reconstructed path from end to start on output
// temp2 ... representing the origin zone for the current one (came_from)
// temp3 ... representing the costs for each zone
// all ... if true all zones of the map will be searched
short dijkstra(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, short* temp3, short* costZone, char all){
	int i, n = 0;
	short zone, next;

	// init queue
	insertPriorityQueue(&n, temp1, startZone, 0);
	// init came_from
	for (i = 0; i<zoneCount; i++){
		temp2[i] = -1;
		temp3[i] = -1;
	}
	temp3[startZone] = 0;

	for (; n;){
		zone = popPriorityQueue(&n, temp1);

		if (!all&&zone == endZone)
			break;

		for (i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
			short cost = temp3[zone] + costZone[next];

			if (temp3[next] == -1 || cost<temp3[next]){
				temp3[next] = cost;
				insertPriorityQueue(&n, temp1, next, cost);
				temp2[next] = zone;
			}
		}
	}

	// reconstruct path
	zone = endZone;
	temp1[0] = zone;
	for (i = 1; zone != startZone; i++){
		zone = temp2[zone];
		temp1[i] = zone;
		if (zone == -1) return -1;
	}

	return temp1[i - 2];
}


int main(void) {

	double start, end;
	double interval;

	int zoneCount = 5;

	short* graph;
	short* temp1, *temp2, *temp3;
	short* costZone;
	short zone;

	graph = (short*)malloc(zoneCount * MAX_CONNECTIONS * sizeof(short));

	// some kind of queue and path and cost array
	temp1 = (short*)malloc(zoneCount * sizeof(short));
	// came_from reference
	temp2 = (short*)malloc(zoneCount * sizeof(short));
	// some special kind of priority queue
	temp3 = (short*)malloc(zoneCount * 2 * sizeof(short));

	costZone = (short*)malloc(zoneCount * sizeof(short));

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

	// cost/weight for each zone
	for (int i = 0; i < zoneCount; i++)
		costZone[i] = 1;
	costZone[3] = 5;

	for (int i = 0; i < 10000; i++)
		zone = dijkstra(graph, zoneCount, 4, 0, temp3, temp2, temp1, costZone, 1);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);
	fprintf(stderr, "next zone towards %3hd from %3hd: %3hd \n", 0, 4, zone);

	fprintf(stderr, "path from %3hd back to %3hd \n", 4, 0);
	for (int i = 0; i < zoneCount; i++) {
		fprintf(stderr, "%3d %3hd \n", i, temp1[i]);
	}

}
