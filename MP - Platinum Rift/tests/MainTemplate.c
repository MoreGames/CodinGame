#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define MAX_CONNECTIONS 6
#define WORLD_INFO 8

#define DEBUG

#define MIN(x, y) ((x)<(y)?(x):(y))
#define MAX(x, y) ((x)<(y)?(y):(x)) 

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef struct continent continent;
struct continent {
	short* zones;
	int zoneCount;
	short rankZones;
	short rankResources;
	int sumOfResources;
	int numberOfResourceZones;
	float averageResourcePerZone;
	float averageResourcePerContinent;
	short resourceZones[7]; // number of zones with 6 resources, 5 resources, 4 resources, ...
	int numberOfPlayerPods[4];
	int numberOfPlayerZones[4];
};

typedef struct player player;
struct player {
	short platinum;
	short income;
	short numberOfPods;
	short numberOfZones;
};

typedef struct map map;
struct map {
	short numberOfResourceZones;
	short sumOfResources;
	float averageResourcePerZone;
	short resourceZones[7]; // number of zones with 6 resources, 5 resources, 4 resources, ...
};

// function headers
void copyArrays(short* source, short* target, int num);
void insertPriorityQueue(int* size, short* queue, short data, short pri);
short popPriorityQueue(int* size, short* queue);
short breathFirstSearch(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, char all);
short dijkstra(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, short* temp3, short* costZone, char all);

int cmp0(const void *pa, const void *pb);
int cmp1(const void *pa, const void *pb);
int cmp1Desc(const void *pa, const void *pb);
int cmp2(const void *pa, const void *pb);
int cmp2Desc(const void *pa, const void *pb);
int cmp1DescULL(const void *pa, const void *pb);

char readFile(short* links, int linkCount, short* resources, int zoneCount) {
	FILE* linksFile = fopen("LinksFinal.in", "r");
	FILE* resourcesFile = fopen("ResourcesLowishAmerica.in", "r");

	if (linksFile == NULL || resourcesFile == NULL) {
		fprintf(stderr, "Error! Could not open file.\n");
		return 0;
	}

	for (int i = 0; i < zoneCount; i++) {
		int zoneId;
		int resource;
		fscanf(resourcesFile, "%d %d", &zoneId, &resource);
		resources[zoneId] = (short)resource;
	}

	for (int i = 0; i < linkCount; i++) {
		int start, end;
		fscanf(linksFile, "%d %d", &start, &end);
		links[i * 2] = (short)start;
		links[i * 2 + 1] = (short)end;
	}

	fclose(linksFile);
	fclose(resourcesFile);

	return 1;
}

void initGraph(short* graph, int zoneCount, short* links, int linkCount) {
	for (int i = 0; i < zoneCount * MAX_CONNECTIONS; i++) {
		graph[i] = -1;
	}

	for (int j = 0; j < zoneCount; j++) {
		char found = 0;

		for (int i = 0; i < linkCount; i++) {
			if (links[i * 2] == j)  {
				graph[j * MAX_CONNECTIONS + found] = links[i * 2 + 1];
				found += 1;
				continue;
			}
			if (links[i * 2 + 1] == j)  {
				graph[j * MAX_CONNECTIONS + found] = links[i * 2];
				found += 1;
			}
		}
	}
}
void initWorld(short* world, int zoneCount, short* resources, player* players, map* map, int playerCount) {
	int num = 0, sum = 0;

	for (int i = 0; i < zoneCount; i++) {
		world[i * WORLD_INFO] = i; // zoneId
		//world[i * WORLD_INFO + 1] // continentId
		world[i * WORLD_INFO + 2] = resources[i]; // resources
		world[i * WORLD_INFO + 3] = -1; // ownerId
		//world[i * WORLD_INFO + 4] // pods1
		//world[i * WORLD_INFO + 5] // pods2
		//world[i * WORLD_INFO + 6] // pods3
		//world[i * WORLD_INFO + 7] // pods4
	}

	for (int i = 0; i < playerCount; i++) {
		players[i].platinum = 200;
		players[i].income = 0;
		players[i].numberOfPods = 0;
		players[i].numberOfZones = 0;
	}

	for (int i = 0; i < 7; i++) {
		map->resourceZones[i] = 0;
	}
	for (int i = 0; i < zoneCount; i++) {
		num += !!resources[i];
		sum += resources[i];
		if (resources[i] == 0) map->resourceZones[0] += 1;
		if (resources[i] == 1) map->resourceZones[1] += 1;
		if (resources[i] == 2) map->resourceZones[2] += 1;
		if (resources[i] == 3) map->resourceZones[3] += 1;
		if (resources[i] == 4) map->resourceZones[4] += 1;
		if (resources[i] == 5) map->resourceZones[5] += 1;
		if (resources[i] == 6) map->resourceZones[6] += 1;
	}
	map->numberOfResourceZones = num;
	map->sumOfResources = sum;
	map->averageResourcePerZone = num ? (float)sum / num : 0;
}
int sumResources(short* zones, int zoneCount, short* resources) {
	int sum = 0;
	for (int i = 0; i < zoneCount; i++) {
		sum += resources[zones[i]];
	}
	return sum;
}
int numberOfResourceZones(short* zones, int zoneCount, short* resources) {
	int sum = 0;
	for (int i = 0; i < zoneCount; i++) {
		sum += !!resources[zones[i]];
	}
	return sum;
}
// return numberOfContinents
// bestimmt die Anzahl der Kontinente, setzt für jeden Kontinent die Zonen und deren Anzahl
// setzt für die Zonen der Welt die Kontinentzuordnung
int setContinents(continent** continents, short* world, short* resources, int zoneCount, short* graph, short* temp1, short* temp2) {
	int i, j, continentNumber = 0;
	short* searched = (short*)malloc(zoneCount * sizeof(short));
	for (i = 0; i < zoneCount; i++) {
		searched[i] = -1;
	}

	i = 0;
	do {
		breathFirstSearch(graph, zoneCount, (short)i, (short)(zoneCount - 1), temp1, temp2, (char)1);

		for (j = 0; j < zoneCount; j++) {
			if (temp2[j] != -1) {
				searched[j] = continentNumber;
			}
		}
		for (; i<zoneCount && searched[i] != -1; i++);

		continentNumber += 1;
	} while (i<zoneCount);


	*continents = (continent*)malloc(zoneCount * continentNumber * sizeof(continent));
	for (j = 0; j < continentNumber; j++) {
		(*continents)[j].zones = (short*)malloc(zoneCount * sizeof(continent));
	}
	for (j = 0; j < continentNumber; j++) {
		for (i = 0; i < zoneCount; i++) {
			(*continents)[j].zones[i] = -1;
		}
	}
	for (j = 0; j < continentNumber; j++) {
		(*continents)[j].numberOfPlayerPods[0] = 0;
		(*continents)[j].numberOfPlayerPods[1] = 0;
		(*continents)[j].numberOfPlayerPods[2] = 0;
		(*continents)[j].numberOfPlayerPods[3] = 0;
		(*continents)[j].numberOfPlayerZones[0] = 0;
		(*continents)[j].numberOfPlayerZones[1] = 0;
		(*continents)[j].numberOfPlayerZones[2] = 0;
		(*continents)[j].numberOfPlayerZones[3] = 0;

		int k = 0;
		for (i = 0; i < zoneCount; i++) {
			if (searched[i] == j) {
				(*continents)[j].zones[k] = i;
				k++;
			}
		}

		for (i = 0; i < 7; i++) {
			(*continents)[j].resourceZones[i] = 0;
		}
		for (int i = 0; i < k; i++) {
			if (resources[(*continents)[j].zones[i]] == 0) (*continents)[j].resourceZones[0] += 1;
			if (resources[(*continents)[j].zones[i]] == 1) (*continents)[j].resourceZones[1] += 1;
			if (resources[(*continents)[j].zones[i]] == 2) (*continents)[j].resourceZones[2] += 1;
			if (resources[(*continents)[j].zones[i]] == 3) (*continents)[j].resourceZones[3] += 1;
			if (resources[(*continents)[j].zones[i]] == 4) (*continents)[j].resourceZones[4] += 1;
			if (resources[(*continents)[j].zones[i]] == 5) (*continents)[j].resourceZones[5] += 1;
			if (resources[(*continents)[j].zones[i]] == 6) (*continents)[j].resourceZones[6] += 1;
		}

		(*continents)[j].zoneCount = k;
		(*continents)[j].sumOfResources = sumResources((*continents)[j].zones, (*continents)[j].zoneCount, resources);
		(*continents)[j].numberOfResourceZones = numberOfResourceZones((*continents)[j].zones, (*continents)[j].zoneCount, resources);
		(*continents)[j].averageResourcePerZone = (*continents)[j].numberOfResourceZones ? (float)(*continents)[j].sumOfResources / (float)(*continents)[j].numberOfResourceZones : 0;
		(*continents)[j].averageResourcePerContinent = (*continents)[j].zoneCount ? (float)(*continents)[j].sumOfResources / (float)(*continents)[j].zoneCount : 0;
	}

	// set continent rankZones: the continent with the highest zoneCount gets the highest number
	// set continent rankResources: the continent with the highest resources gets the highest number
	{
		short* temp = (short*)malloc(continentNumber * 2 * sizeof(short));
		for (j = 0; j < continentNumber; j++) {
			temp[j * 2] = j;
			temp[j * 2 + 1] = (*continents)[j].zoneCount;
		}

		qsort(temp, continentNumber, 2 * sizeof(short), cmp1);

		for (j = 0; j < continentNumber; j++) {
			(*continents)[temp[j * 2]].rankZones = j + 1;
		}

		for (j = 0; j < continentNumber; j++) {
			temp[j * 2] = j;
			temp[j * 2 + 1] = (*continents)[j].sumOfResources;
		}

		qsort(temp, continentNumber, 2 * sizeof(short), cmp1);

		for (j = 0; j < continentNumber; j++) {
			(*continents)[temp[j * 2]].rankResources = j + 1;
		}

		free(temp);
	}

	// link world with continents
	for (i = 0; i < zoneCount; i++) {
		world[i * WORLD_INFO + 1] = searched[i];
	}

	free(searched);

	return continentNumber;
}

// distanceToAll ... zoneCount x zoneCount
// rows ... startZone
// columns ... endZone
void computeDistanceToAll(short* graph, int zoneCount, short* temp1, short* temp2, short* temp3, short* distanceToAll) {
	short* costZone = (short*)malloc(zoneCount * sizeof(short));
	for (int i = 0; i < zoneCount; i++)
		costZone[i] = 1;

	for (int i = 0; i < zoneCount; i++) {
		dijkstra(graph, zoneCount, (short)i, (short)i, temp3, temp2, temp1, costZone, (char)1);

		for (int j = 0; j < zoneCount; j++) {
			if (j == i) {
				distanceToAll[i * zoneCount + j] = 0;
			} else {
				// reconstruct path
				short zone = j;
				int k;
				for (k = 0; zone != i; k++) {
					zone = temp2[zone];
					if (zone == -1) break;
				}
				if (zone == -1) {
					distanceToAll[i * zoneCount + j] = -1;
				} else {
					distanceToAll[i * zoneCount + j] = k;
				}
			}
		}
	}

	free(costZone);
}
void computeDistanceResourcesToAll(short* resources, int zoneCount, short* distanceToAll, float* distanceResourcesToAll) {
	for (int i = 0; i < zoneCount; i++) {
		for (int j = 0; j < zoneCount; j++) {
			if (distanceToAll[i * zoneCount + j] == -1) {
				distanceResourcesToAll[i * zoneCount + j] = -1;
			} else if (distanceToAll[i * zoneCount + j] == 0) {
				distanceResourcesToAll[i * zoneCount + j] = 0;
			} else {
				distanceResourcesToAll[i * zoneCount + j] = (float)resources[j] + 0.2f / (float)distanceToAll[i * zoneCount + j];
			}
		}
	}
}


void updateWorldAll(short* world, int zoneCount, short* resources, player* players, continent* continents, int continentCount, int playerCount) {
	int num[4] = { 0 }, sum[4] = { 0 }, in[4] = { 0 }, plat[4] = { 0 };

	for (int i = 0; i < zoneCount; i++) {
		int zId; // this zone's ID
		scanf("%d%d%d%d%d%d",
			&zId,
			&world[i * WORLD_INFO + 3],	// ownerId
			&world[i * WORLD_INFO + 4],	// pods0
			&world[i * WORLD_INFO + 5],	// pods1
			&world[i * WORLD_INFO + 6],	// pods2
			&world[i * WORLD_INFO + 7]	// pods3
			);
		fgetc(stdin);
	}

	// continent
	for (int j = 0; j < continentCount; j++) {
		continents[j].numberOfPlayerPods[0] = 0;
		continents[j].numberOfPlayerPods[1] = 0;
		continents[j].numberOfPlayerPods[2] = 0;
		continents[j].numberOfPlayerPods[3] = 0;
		continents[j].numberOfPlayerZones[0] = 0;
		continents[j].numberOfPlayerZones[1] = 0;
		continents[j].numberOfPlayerZones[2] = 0;
		continents[j].numberOfPlayerZones[3] = 0;
		for (int i = 0; i < continents[j].zoneCount; i++) {
			for (int k = 0; k < 4; k++) {
				continents[j].numberOfPlayerPods[k] += world[continents[j].zones[i] * WORLD_INFO + 4 + k];
			}
			if (world[continents[j].zones[i] * WORLD_INFO + 3] != -1)
				continents[j].numberOfPlayerZones[world[continents[j].zones[i] * WORLD_INFO + 3]] += 1;
		}
	}

	// player
	for (int i = 0; i < zoneCount; i++) {
		if (world[i * WORLD_INFO + 3] != -1) num[world[i * WORLD_INFO + 3]] += 1;
		if (world[i * WORLD_INFO + 3] != -1) in[world[i * WORLD_INFO + 3]] += resources[i];
		sum[0] += world[i * WORLD_INFO + 4];
		sum[1] += world[i * WORLD_INFO + 5];
		sum[2] += world[i * WORLD_INFO + 6];
		sum[3] += world[i * WORLD_INFO + 7];
	}
	for (int i = 0; i < playerCount; i++) {
		players[i].platinum = (players[i].platinum % 20) + in[i];
		players[i].income = in[i];
		players[i].numberOfPods = num[i];
		players[i].numberOfZones = sum[i];
	}
}

// http://pages.ripco.net/~jgamble/nw.html
// http://stackoverflow.com/questions/2786899/fastest-sort-of-fixed-length-6-int-array
// sort array with 4 elements in descending order
void sort4_sorting_network_simple_swap(short* d) {
#define SWAP(x,y) {const short a = MIN(d[x], d[y]); const short b = MAX(d[x], d[y]); d[x] = a; d[y] = b;}
	SWAP(0, 1);
	SWAP(2, 3);
	SWAP(0, 2);
	SWAP(1, 3);
	SWAP(1, 2);
#undef SWAP
}
void updateOwnerSingle(short* world, int myId, short zone) {
	short temp[4];
	copyArrays(&world[zone * WORLD_INFO + 4], temp, 4);
	sort4_sorting_network_simple_swap(temp);

	// neutral zone
	if (temp[0] == temp[1]) {
		world[zone * WORLD_INFO + 3] = -1;
	} else {
		for (int i = 0; i < 4; i++) {
			if (temp[0] == world[zone * WORLD_INFO + 4 + i])
				world[zone * WORLD_INFO + 3] = i;
		}
	}

}
void updateOwner(short* world, int myId, short startZone, short endZone) {
	updateOwnerSingle(world, myId, startZone);
	updateOwnerSingle(world, myId, endZone);
}
void updateWorldSingle(short* world, int myId, short startZone, short endZone) {
	world[startZone * WORLD_INFO + 4 + myId] -= 1;
	world[endZone * WORLD_INFO + 4 + myId] += 1;

	updateOwner(world, myId, startZone, endZone);
}

int getNumberOfPods(continent* continents, int continentCount, int playerId){
	int sum = 0;
	for (int i = 0; i < continentCount; i++) {
		sum += continents[i].numberOfPlayerPods[playerId];
	}
	return sum;
}

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
	//fprintf(stderr, "Step Zone \n");
	//for (int j = i - 2, k = 1; j >= 0; j--, k++) {
	//	fprintf(stderr, "%4d %4hd \n", k, temp1[j]);
	//}

	return temp1[i - 2];
}
// low order priority queue
void insertPriorityQueue(int* size, short* queue, short data, short pri) {
	int i;
	for (i = *size - 1; i >= 0 && pri > queue[i * 2 + 1]; i--) {
		queue[(i + 1) * 2] = queue[i * 2];
		queue[(i + 1) * 2 + 1] = queue[i * 2 + 1];
	}
	queue[(i + 1) * 2] = data;
	queue[(i + 1) * 2 + 1] = pri;

	*size += 1;
}
short popPriorityQueue(int* size, short* queue) {
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
short dijkstra(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, short* temp3, short* costZone, char all) {
	int i, n = 0;
	short zone, next;

	// init queue
	insertPriorityQueue(&n, temp1, startZone, 0);
	// init came_from
	for (i = 0; i < zoneCount; i++) {
		temp2[i] = -1;
		temp3[i] = -1;
	}
	temp3[startZone] = 0;

	for (; n;) {
		zone = popPriorityQueue(&n, temp1);

		if (!all && zone == endZone)
			break;

		for (i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
			short cost = temp3[zone] + costZone[next];

			if (temp3[next] == -1 || cost < temp3[next]) {
				temp3[next] = cost;
				insertPriorityQueue(&n, temp1, next, cost);
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
	//fprintf(stderr, "Step Zone \n");
	//for (int j = i - 2, k = 1; j >= 0; j--, k++) {
	//	fprintf(stderr, "%4d %4hd \n", k, temp1[j]);
	//}

	return temp1[i - 2];
}

void computeResourcesOwnNeighbours(short* graph, short* world, int zoneCount, continent* continents, int continentCount, short* resourcesOwnNeighbours) {
	short next;
	for (int i = 0; i < zoneCount; i++) {
		resourcesOwnNeighbours[i * 2] = i;
		resourcesOwnNeighbours[i * 2 + 1] = world[i * WORLD_INFO + 2];
		for (int j = 0; j < MAX_CONNECTIONS && ((next = graph[i * MAX_CONNECTIONS + j]) != -1); j++) {
			resourcesOwnNeighbours[i * 2 + 1] += world[next * WORLD_INFO + 2];
		}
	}
	for (int i = 0; i < zoneCount; i++) {
		resourcesOwnNeighbours[i * 2 + 1] *= 10;
		resourcesOwnNeighbours[i * 2 + 1] += continentCount - continents[world[i * WORLD_INFO + 1]].rankZones;
	}
}
short findResourcesOwnNeighbours(short zone, short* resourcesOwnNeighbours, int zoneCount) {
	for (int i = 0; i < zoneCount; i++) {
		if (zone == resourcesOwnNeighbours[i* 2])
			return resourcesOwnNeighbours[i * 2 + 1];
	}
	return 0;
}
char isNeighbour(short zone, short* zones, int count, short* graph) {
	short next;
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < MAX_CONNECTIONS && ((next = graph[zones[i] * MAX_CONNECTIONS + j]) != -1); j++) {
			if (zone == next)
				return 1;
		}
	}
	return 0;
}

// wird nur aufgerufen wenn, numberOfReinforcements > 0
void computeReinforcements(short* reinforcementAction, int* numberOfReinforcementAction, int numberOfReinforcements, short* graph, short* world, int zoneCount,
	continent* continents, int continentCount, map* map, int playerId, int round, int playerCount, short* resourcesOwnNeighbours) {
	int numberOfZoneTemp = 0;
	short* zoneTemp = (short*)malloc(zoneCount * 2 * sizeof(short));


	*numberOfReinforcementAction = 0;

	if (round == 0) {
		if (playerCount == 2) {
			short* contTemp = (short*)malloc(continentCount * 2 * sizeof(short));

			for (int i = 0; i < continentCount; i++) {
				contTemp[i * 2] = i;
				contTemp[i * 2 + 1] = (short)(continents[i].averageResourcePerContinent * continents[i].sumOfResources);
			}

			qsort((void*)contTemp, continentCount, 2 * sizeof(short), cmp1Desc);

#ifdef DEBUG
			fprintf(stderr, "continent priority: \n");
			for (int i = 0; i < continentCount; i++) {
				fprintf(stderr, "%2hd %3hd\n", contTemp[i * 2 + 1], continents[contTemp[i * 2]].zoneCount);
			}
#endif

			// fokus on two continents
			int contFocus[2] = { contTemp[0], contTemp[2] };
			float sumR = 0;
			for (int i = 0; i < 2; i++) {
				sumR += continents[contFocus[i]].sumOfResources;
			}


#ifdef DEBUG
			fprintf(stderr, "top continents: \n");
			for (int i = 0; i < 2; i++) {
				fprintf(stderr, "%2hd %3hd\n", contFocus[i], continents[contFocus[i]].zoneCount);
			}
#endif

			// best other resource zones (beside the two continents)
			{
				int numberOfOtherZones = 0;
				short* otherZones = (short*)malloc(zoneCount * 2 * sizeof(short));


				for (int i = 0; i < zoneCount; i++) {
					if (world[i * WORLD_INFO + 1] != contFocus[0] && world[i * WORLD_INFO + 1] != contFocus[1]) {
						if (world[i * WORLD_INFO + 2] >= 3) {
							otherZones[numberOfOtherZones * 2] = i;
							otherZones[numberOfOtherZones * 2 + 1] = findResourcesOwnNeighbours(i, resourcesOwnNeighbours, zoneCount);
							numberOfOtherZones += 1;
						}
					}
				}


				qsort((void*)otherZones, numberOfOtherZones, 2 * sizeof(short), cmp1Desc);

#ifdef DEBUG
				fprintf(stderr, "top other zones: \n");
				for (int i = 0; i < numberOfOtherZones; i++) {
					fprintf(stderr, "%2hd %3hd\n", otherZones[i * 2], otherZones[i * 2 + 1]);
				}
#endif

				// nie auf 6er oder auf die 2 besten
				if (numberOfOtherZones > 0) {
					short exclave = -1;

					for (int i = 0; i < numberOfOtherZones; i++) {
						if (world[otherZones[i * 2] * WORLD_INFO + 2] != 6 && i > 1) {
							exclave = otherZones[i * 2];
						}
					}

					if (exclave == -1) {
						exclave = otherZones[otherZones[(numberOfOtherZones - 1) * 2]];
					}

					if (map->averageResourcePerZone < 2.5) {
						reinforcementAction[0] = 2;

						numberOfReinforcements -= 2;
					} else {
						reinforcementAction[0] = 1;

						numberOfReinforcements -= 1;
					}
					reinforcementAction[1] = exclave;
					*numberOfReinforcementAction += 1;
				}

#ifdef DEBUG
				fprintf(stderr, "chosen top other zones: \n");
				for (int i = 0; i < *numberOfReinforcementAction; i++) {
					fprintf(stderr, "%2hd\n", reinforcementAction[i * 2 + 1]);
				}
#endif

				free(otherZones);
			}

			// best continent resource zones
			for (int j = 0; j < 2; j++) {
				short reinforcementCont = (short)roundf((numberOfReinforcements * continents[contFocus[j]].sumOfResources / sumR));

				int numberOfTopZones = 0;
				short* topZones = (short*)malloc(zoneCount * 2 * sizeof(short));
				int numberOfTopZonesExtended = 0;
				short* topZonesExtended = (short*)malloc(zoneCount * 2 * sizeof(short));

				for (int i = 0; i < continents[contFocus[j]].zoneCount; i++) {
					// TODO für high und highish
					if (world[continents[contFocus[j]].zones[i] * WORLD_INFO + 2] > min((short)(continents[contFocus[j]].averageResourcePerZone * 2), 3)) {
						topZones[numberOfTopZones * 2] = continents[contFocus[j]].zones[i];
						topZones[numberOfTopZones * 2 + 1] = findResourcesOwnNeighbours(continents[contFocus[j]].zones[i], resourcesOwnNeighbours, zoneCount);
						numberOfTopZones += 1;
					}
				}

				if (numberOfTopZones < 3) {
					for (int i = 0; i < continents[contFocus[j]].zoneCount; i++) {
						if (world[continents[contFocus[j]].zones[i] * WORLD_INFO + 2] == min((short)(continents[contFocus[j]].averageResourcePerZone * 2), 3)) {
							topZonesExtended[numberOfTopZonesExtended * 2] = continents[contFocus[j]].zones[i];
							topZonesExtended[numberOfTopZonesExtended * 2 + 1] = findResourcesOwnNeighbours(continents[contFocus[j]].zones[i], resourcesOwnNeighbours, zoneCount);
							numberOfTopZonesExtended += 1;
						}
					}

					// keine Felder angrenzend zu bereits gefunden top zonen
					for (int i = 0; i < numberOfTopZonesExtended; i++) {
						if (isNeighbour(topZonesExtended[i * 2], topZones, numberOfTopZones, graph)) {
							topZonesExtended[i * 2 + 1] = -1;
						}
					}

					if (numberOfTopZonesExtended > 0) {
						qsort((void*)topZonesExtended, numberOfTopZonesExtended, 2 * sizeof(short), cmp1Desc);

#ifdef DEBUG
						fprintf(stderr, "top continent %d zones extended: \n", contFocus[j]);
						for (int i = 0; i < numberOfTopZonesExtended; i++) {
							fprintf(stderr, "%2hd %3hd\n", topZonesExtended[i * 2], topZonesExtended[i * 2 + 1]);
						}
#endif
						topZones[numberOfTopZones * 2] = topZonesExtended[0];
						topZones[numberOfTopZones * 2 + 1] = topZonesExtended[1];
						numberOfTopZones += 1;
					}
				}

				qsort((void*)topZones, numberOfTopZones, 2 * sizeof(short), cmp1Desc);

#ifdef DEBUG
				fprintf(stderr, "top continent %d zones: \n", contFocus[j]);
				for (int i = 0; i < numberOfTopZones; i++) {
					fprintf(stderr, "%2hd %3hd\n", topZones[i * 2], topZones[i * 2 + 1]);
				}
#endif

				// set reinforcement, first zone gets always an additional pod
				reinforcementAction[*numberOfReinforcementAction * 2] = 1;
				reinforcementAction[*numberOfReinforcementAction * 2 + 1] = topZones[0];
				reinforcementCont -= 1;

				for (int i = 0; i < min(numberOfTopZones, reinforcementCont)-1; i++) {
					reinforcementAction[((*numberOfReinforcementAction) + 1) * 2 + i * 2] = 0;
					reinforcementAction[((*numberOfReinforcementAction) + 1) * 2 + i * 2 + 1] = 0;
				}
				for (int n = 0; n < reinforcementCont;) {
					for (int i = 0; i < numberOfTopZones && n < reinforcementCont; i++, n++) {
						reinforcementAction[(*numberOfReinforcementAction) * 2 + i * 2] += 1;
						reinforcementAction[(*numberOfReinforcementAction) * 2 + i * 2 + 1] = topZones[i * 2];
					}
				}

				*numberOfReinforcementAction += max(min(numberOfTopZones, reinforcementCont),1);

				free(topZones);
				free(topZonesExtended);
			}

			free(contTemp);
		} else {

		}

		free(zoneTemp);
	} else {
		if (playerCount == 2) {

		} else {

		}
	}

#ifdef DEBUG
	fprintf(stderr, "reinforcement zones: \n");
	for (int i = 0; i < *numberOfReinforcementAction; i++) {
		fprintf(stderr, "%2hd %2hd\n", reinforcementAction[i * 2], reinforcementAction[i * 2 + 1]);
	}
#endif
}


char numberOfPlayersInZone(short zone, short* world) {
	char num = 0;
	for (int j = 4; j < 8; j++) {
		num += !!world[zone * WORLD_INFO + j];
	}
	return num;
}
char checkIfOwnZone(short zone, short* world, int playerId) {
	return (world[zone * WORLD_INFO + 3] == playerId);
}
char checkIfZoneFighting(short zone, short* world) {
	if (numberOfPlayersInZone(zone, world) > 1) {
		return 1;
	}
	else {
		return 0;
	}
}
char checkIfContinentIsOwned(short zone, short* world, int zoneCount, int playerId) {
	short contNum = world[zone * WORLD_INFO + 1];
	for (int i = 0; i < zoneCount; i++) {
		if (world[i * WORLD_INFO + 1] == contNum && world[i * WORLD_INFO + 3] != playerId) {
			return 0;
		}
	}
	return 1;
}
// 1 if enemy border zone, 0 else
char checkEnemyBorderZone(short zone, short* graph, short* world, int playerId) {
	char val = checkIfOwnZone(zone, world, playerId);
	short next;

	if (val != 0) {
		for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
			if (world[next * WORLD_INFO + 3] != -1 && world[next * WORLD_INFO + 3] != playerId) {
				val = 1;
			}
		}
	}

	return val;
}
// 1 if the zone can be conquered, 0 else
// not counting pods in fighting zones (e.g. pods from several players are in the zone)
char checkAbleToConquer(short zone, short* graph, short* world, int playerId) {
	char val = checkEnemyBorderZone(zone, graph, world, playerId);

	if (val != 0) {
		short sum = 0;
		short next;

		for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
			if (world[next * WORLD_INFO + 3] != -1 && world[next * WORLD_INFO + 3] != playerId) {
				if (numberOfPlayersInZone(next, world) == 1) {
					for (int j = 4; j < 8; j++) {
						sum += world[next * WORLD_INFO + j];
					}
				}
			}
		}
		if (sum > world[zone * WORLD_INFO + 4 + playerId]) {
			val = sum - world[zone * WORLD_INFO + 4 + playerId];
		}
		else {
			val = 0;
		}
	}

	return val;
}
// zones can be neutral or enemy
short maxAdjacentResourcesEnemy(short zone, short* graph, short* world, int playerId) {
	short maxRes = 0;
	short next;

	for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
		if (world[next * WORLD_INFO + 3] != playerId) {
			if (maxRes < world[next * WORLD_INFO + 2]) {
				maxRes = world[next * WORLD_INFO + 2];
			}
		}
	}

	return maxRes;
}
// every zone is considered
short maxAdjacentResources(short zone, short* graph, short* world) {
	short maxRes = 0;
	short next;

	for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
		if (maxRes < world[next * WORLD_INFO + 2]) {
			maxRes = world[next * WORLD_INFO + 2];
		}
	}

	return maxRes;
}
// zones can be neutral or enemy
short sumAdjacentResourcesEnemy(short zone, short* graph, short* world, int playerId) {
	short sum = 0;
	short next;

	for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
		if (world[next * WORLD_INFO + 3] != playerId) {
			sum += world[next * WORLD_INFO + 2];
		}
	}

	return sum;
}
// every zone is considered
short sumAdjacentResources(short zone, short* graph, short* world) {
	short sum = 0;
	short next;

	for (int i = 0; i < MAX_CONNECTIONS && ((next = graph[zone * MAX_CONNECTIONS + i]) != -1); i++) {
		sum += world[next * WORLD_INFO + 2];
	}

	return sum;
}
char getRandomNumber0To9() {
	return rand() % 10;
}
unsigned long long setMoveTargetRanking(short zone, short* graph, short* world, continent* continents, int playerId, short* distanceToAll) {
	short resource = world[zone * WORLD_INFO + 2];
	short distance = distanceToAll[zone];
	short maxAdjRes = maxAdjacentResources(zone, graph, world);
	short sumAdjRes = sumAdjacentResources(zone, graph, world);
	char randomNumber = getRandomNumber0To9();

	unsigned long long num = (unsigned long long)((float)resource / (float)distance * 1000);

	num *= 10ULL;
	num += (unsigned long long)maxAdjRes;

	num *= 100ULL;
	num += (unsigned long long)sumAdjRes;

	num *= 10ULL;
	num += (unsigned long long)randomNumber;

#ifdef DEBUG
	fprintf(stderr, "%3d %2d %2d %1d %2d %1d\n", zone, resource, distance, maxAdjRes, sumAdjRes, randomNumber);
#endif

	return num;
}
void setMoveTargets(short zone, short* graph, short* world, int zoneCount, continent* continents, int playerId, unsigned long long* moveTargets, int* numOfMoveTargets, short* distanceToAll) {
	short contNum = world[zone * WORLD_INFO + 1];

	*numOfMoveTargets = 0;

#ifdef DEBUG
	fprintf(stderr, "zon re di m su r\n");
#endif

	// only neutral or enemy zones
	for (int i = 0; i < continents[contNum].zoneCount; i++) {
		if (world[continents[contNum].zones[i] * WORLD_INFO + 3] != playerId && zone != continents[contNum].zones[i]) {
			moveTargets[*numOfMoveTargets * 2] = (unsigned long long)continents[contNum].zones[i];

			moveTargets[*numOfMoveTargets * 2 + 1] = setMoveTargetRanking(continents[contNum].zones[i], graph, world, continents, playerId, &distanceToAll[zone * zoneCount]);

			*numOfMoveTargets += 1;
		}
	}
}
void computePodAction(short* podAction, int* numberOfPodAction, short* graph, short* world, int zoneCount, continent* continents, int continentCount, int playerId, int playerCount,
	short* temp1, short* temp2, short* temp3, short* distanceToAll) {
	short next, target;
	short* worldCopy = (short *)malloc(zoneCount * WORLD_INFO * sizeof(short));
	for (int i = 0; i < zoneCount*WORLD_INFO; i++) {
		worldCopy[i] = world[i];
	}
	short* costZone = (short*)malloc(zoneCount * sizeof(short));
	for (int i = 0; i < zoneCount; i++) {
		if (worldCopy[i * WORLD_INFO + 3] == playerId) {
			costZone[i] = 100;
		} else {
			costZone[i] = 99 - worldCopy[i * WORLD_INFO + 2];
		}
	}

	int numOfMoveTargets = 0;
	unsigned long long* moveTargets = (unsigned long long*)malloc(zoneCount * 2 * sizeof(unsigned long long));

	int numOfPickedTargets = 0;
	short* pickedTargets = (short*)malloc(2000 * 2 * sizeof(short)); // target, count

	*numberOfPodAction = 0;

	for (int i = 0; i < zoneCount; i++) {
		if (world[i * WORLD_INFO + 4 + playerId] > 0) {
			char isFighting = checkIfZoneFighting(i, worldCopy);

			fprintf(stderr, "zone: %3hd, all pods: %2hd isFighting: %1hhd ", i, worldCopy[i * WORLD_INFO + 4 + playerId], isFighting);

			if (!isFighting) {
				char isContinentOwned = checkIfContinentIsOwned(i, worldCopy, zoneCount, playerId);

				fprintf(stderr, "isContinentOwned: %1hhd ", isContinentOwned);

				if (!isContinentOwned) {
					char isConquerable = checkAbleToConquer(i, graph, worldCopy, playerId);
					short podsWithoutMoveAction = 0;

					if (isConquerable) {
						podsWithoutMoveAction = isConquerable;
					}

					fprintf(stderr, "podsWithoutMoveAction: %1hhd \n", podsWithoutMoveAction);

					for (int j = podsWithoutMoveAction; j < worldCopy[i * WORLD_INFO + 4 + playerId]; j++) {

						// set continent zone ranking - target zone
						setMoveTargets(i, graph, worldCopy, zoneCount, continents, playerId, moveTargets, &numOfMoveTargets, distanceToAll);

						// target
						qsort((void*)moveTargets, numOfMoveTargets, 2 * sizeof(unsigned long long), cmp1DescULL);

#ifdef DEBUG
						fprintf(stderr, "targets for %hd \n", i);
						for (int k = 0; k < numOfMoveTargets; k++) {
							fprintf(stderr, "%3llu %7llu\n", moveTargets[k * 2], moveTargets[k * 2 + 1]);
						}
#endif

						// if target is already chosen, pick next target, or target which was picked lowest
						char isAlreadyPicked;
						for (int k = 0; k < numOfMoveTargets; k++) {
							isAlreadyPicked = 0;
							target = (short)moveTargets[k * 2];

							for (int l = 0; l < numOfPickedTargets; l++) {
								if (pickedTargets[l * 2] == target) {
									isAlreadyPicked = 1;
								}
							}

							if (!isAlreadyPicked)
								break;
						}

						if (isAlreadyPicked) {
							short low = 1000;
							short lowIndex = -1;
							for (int k = 0; k < numOfMoveTargets; k++) {
								target = (short)moveTargets[k * 2];

								for (int l = 0; l < numOfPickedTargets; l++) {
									if (pickedTargets[l * 2] == target) {
										if (pickedTargets[l * 2 + 1] < low) {
											low = pickedTargets[l * 2 + 1];
											lowIndex = l;
										}
										break;
									}
								}
							}

							//pickedTargets[lowIndex * 2] = target;
							pickedTargets[lowIndex * 2 + 1] += 1;
						} else {
							pickedTargets[numOfPickedTargets * 2] = target;
							pickedTargets[numOfPickedTargets * 2 + 1] = 1;
							numOfPickedTargets += 1;
						}

#ifdef DEBUG
						fprintf(stderr, "picked targets \n");
						for (int k = 0; k < numOfPickedTargets; k++) {
							fprintf(stderr, "%3hd %3hd\n", pickedTargets[k * 2], pickedTargets[k * 2 + 1]);
						}
#endif

						// set next step to target
						next = dijkstra(graph, zoneCount, i, target, temp3, temp2, temp1, costZone, 0);

						// updates - world, costZone, etc.
						updateWorldSingle(worldCopy, playerId, i, next);

						if (worldCopy[next * WORLD_INFO + 3] == playerId) {
							costZone[next] = 100;
						} else {
							costZone[next] = 99 - worldCopy[next * WORLD_INFO + 2];
						}

						podAction[(*numberOfPodAction) * 3] = 1;
						podAction[(*numberOfPodAction) * 3 + 1] = i;
						podAction[(*numberOfPodAction) * 3 + 2] = next;
						(*numberOfPodAction) += 1;
					}
				}
			}
		}
	}

	free(worldCopy);
	free(costZone);
	free(moveTargets);
	free(pickedTargets);
}

int main(void) {
	srand((unsigned int)time(NULL));

	char saying[11][100] = { "Upps!",
		"Is that you?",
		"Really?",
		"I'm truly sorry!",
		"That's not nice",
		"You and me",
		"Together we rule!",
		"Narf!", "Fjord!!",
		"I'm not a fridge",
		"We must prepare for tomorrow night" };

	double start, end;
	double interval;

	fprintf(stderr, "%s", saying[0]);

	int playerCount = 2;
	int myId = 0;
	int zoneCount = 154;
	int linkCount = 306;
	int i, j;
	short zone;
	int continentCount;
	int round = 0;

	short* graph;
	short* temp1, *temp2, *temp3;
	continent* continents;
	map map;
	short* world;
	short* resourcesOwnNeighbours;
	short* distanceToAll;
	float* distanceResourcesToAll;

	short* resources = (short*)malloc(zoneCount * sizeof(short));
	short* links = (short*)malloc(linkCount * 2 * sizeof(short));
	player* players = (player*)malloc(playerCount * sizeof(player));

	resourcesOwnNeighbours = (short*)malloc(zoneCount * 2 * sizeof(short));
	distanceToAll = (short*)malloc(zoneCount * zoneCount * sizeof(short));
	distanceResourcesToAll = (float*)malloc(zoneCount * zoneCount * sizeof(float));

	// read input
	if (!readFile(links, linkCount, resources, zoneCount)) {
		return 1;
	}

	start = clock();

	graph = (short*)malloc(zoneCount * MAX_CONNECTIONS * sizeof(short));
	initGraph(graph, zoneCount, links, linkCount);

	world = (short*)malloc(zoneCount * WORLD_INFO * sizeof(short));
	initWorld(world, zoneCount, resources, players, &map, playerCount);

	// some kind of queue and path and cost array
	temp1 = (short*)malloc(zoneCount * sizeof(short));
	// came_from reference
	temp2 = (short*)malloc(zoneCount * sizeof(short));
	// some special kind of priority queue
	temp3 = (short*)malloc(zoneCount * 2 * sizeof(short));
	continentCount = setContinents(&continents, world, resources, zoneCount, graph, temp1, temp2);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "init: clock cycles: %f seconds elapsed: %f \n", end, interval);

#ifdef DEBUG
	fprintf(stderr, "graph: \n");
	for (j = 0; j < zoneCount; j++) {
		for (i = 0; i < MAX_CONNECTIONS; i++) {
			fprintf(stderr, "%6hd ", graph[j * MAX_CONNECTIONS + i]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "world: \n");
	for (j = 0; j < zoneCount; j++) {
		for (i = 0; i < WORLD_INFO; i++) {
			fprintf(stderr, "%6hd ", world[j * WORLD_INFO + i]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "continents: \n");
	fprintf(stderr, "rankZ rankR sumR numRZones zCount avgRePerZone avgRePerCont resourceZones 6 5 4 3 2 1 0\n");
	for (j = 0; j < continentCount; j++) {
		fprintf(stderr, "%5hhd %5hhd %4d %9d %6d %12.2f %12.2f %4d %4d %4d %4d %4d %4d %4d\n",
			continents[j].rankZones, continents[j].rankResources, continents[j].sumOfResources, continents[j].numberOfResourceZones, continents[j].zoneCount,
			continents[j].averageResourcePerZone, continents[j].averageResourcePerContinent,
			continents[j].resourceZones[6], continents[j].resourceZones[5], continents[j].resourceZones[4], continents[j].resourceZones[3], continents[j].resourceZones[2], continents[j].resourceZones[1], continents[j].resourceZones[0]);
	}

	fprintf(stderr, "rankZ rankR pods0 pods1 pods2 pods3 zones0 zones1 zones2 zones3");
	for (i = 0; i < zoneCount; i++) {
		fprintf(stderr, "%4d", i);
	}
	fprintf(stderr, "\n");
	for (j = 0; j < continentCount; j++) {
		fprintf(stderr, "%5hhd %5hhd %5d %5d %5d %5d %6d %6d %6d %6d", continents[j].rankZones, continents[j].rankResources,
			continents[j].numberOfPlayerPods[0], continents[j].numberOfPlayerPods[1], continents[j].numberOfPlayerPods[2], continents[j].numberOfPlayerPods[3],
			continents[j].numberOfPlayerZones[0], continents[j].numberOfPlayerZones[1], continents[j].numberOfPlayerZones[2], continents[j].numberOfPlayerZones[3]);
		for (i = 0; i < continents[j].zoneCount; i++) {
			fprintf(stderr, "%4hd", continents[j].zones[i]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "map: \n");
	fprintf(stderr, "number of resource zones  %hd \n", map.numberOfResourceZones);
	fprintf(stderr, "sum of resources          %hd \n", map.sumOfResources);
	fprintf(stderr, "average resource per zone %f \n", map.averageResourcePerZone);
	fprintf(stderr, "resource zones \n", map.averageResourcePerZone);
	for (i = 0; i < 7; i++) {
		fprintf(stderr, "%8d %5hd \n", i, map.resourceZones[i]);
	}

	fprintf(stderr, "player overview: \n");
	fprintf(stderr, "player platinum income numberOfPods numberOfZones \n");
	for (int i = 0; i < playerCount; i++) {
		fprintf(stderr, "%6d ", i);
		fprintf(stderr, "%8hd ", players[i].platinum);
		fprintf(stderr, "%6hd ", players[i].income);
		fprintf(stderr, "%12hd ", players[i].numberOfPods);
		fprintf(stderr, "%13hd \n", players[i].numberOfZones);
	}
#endif

	// sum of resources for zone and neighbours
	start = clock();

	computeResourcesOwnNeighbours(graph, world, zoneCount, continents, continentCount, resourcesOwnNeighbours);
	qsort((void*)resourcesOwnNeighbours, zoneCount, 2 * sizeof(short), cmp1Desc);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);

#ifdef DEBUG
	fprintf(stderr, "sum of resources for zone and neighbours \n");
	for (i = 0; i < min(zoneCount, 15); i++) {
		fprintf(stderr, "%3d %2hd\n", resourcesOwnNeighbours[2 * i], resourcesOwnNeighbours[2 * i + 1]);
	}
#endif

	// distance from all zones to all zones
	start = clock();

	
	computeDistanceToAll(graph, zoneCount, temp1, temp2, temp3, distanceToAll);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);

#ifdef DEBUG
	//fprintf(stderr, "distance from all zones to all zones \n");
	//for (i = 0; i < zoneCount; i++) {
	//	for (j = 0; j < zoneCount; j++) {
	//		fprintf(stderr, "%2d ", distanceToAll[i * zoneCount + j]);
	//	}
	//	fprintf(stderr, "\n");
	//}
#endif

	// resources/distance from all zones to all zones
	start = clock();

	computeDistanceResourcesToAll(resources, zoneCount, distanceToAll, distanceResourcesToAll);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);

#ifdef DEBUG
	fprintf(stderr, "resources/distance from all zones to all zones \n");
	for (i = 0; i < zoneCount; i++) {
		for (j = 0; j < zoneCount; j++) {
			fprintf(stderr, "%6.2f ", distanceResourcesToAll[i * zoneCount + j]);
		}
		fprintf(stderr, "\n");
	}
#endif
	

	// game loop
	{
		int numberOfReinforcements = 0;
		int platinum = 200; // my available Platinum
		//scanf("%d", &platinum); fgetc(stdin);
		fprintf(stderr, "platinum: %d round: %d\n", platinum, round);

		//updateWorldAll(world, zoneCount, resources, players, continents, continentCount, playerCount);

#ifdef DEBUG
		fprintf(stderr, "player overview: \n");
		fprintf(stderr, "player platinum income numberOfPods numberOfZones \n");
		for (int i = 0; i < playerCount; i++) {
			fprintf(stderr, "%6d ", i);
			fprintf(stderr, "%8hd ", players[i].platinum);
			fprintf(stderr, "%6hd ", players[i].income);
			fprintf(stderr, "%12hd ", players[i].numberOfPods);
			fprintf(stderr, "%13hd \n", players[i].numberOfZones);
		}
#endif

		// ------ moving ------
		{
			int numberOfPodAction = 0;
			int numberOfPodsAvailable = getNumberOfPods(continents, continentCount, myId);
			short* podAction = (short*)malloc(numberOfPodsAvailable * 3 * sizeof(short)); // number of pods, startZone, endZone

			computePodAction(podAction, &numberOfPodAction, graph, world, zoneCount, continents, continentCount, myId, playerCount, temp1, temp2, temp3, distanceToAll);

			// move pods
			if (numberOfPodAction == 0) {
				printf("WAIT\n");
			} else {
				for (int i = 0; i < numberOfPodAction; i++) {
					printf("%hd %hd %hd ", podAction[i * 3], podAction[i * 3 + 1], podAction[i * 3 + 2]);
				}
				printf("\n");
			}

			free(podAction);
		}

		// ------ buying ------
		numberOfReinforcements = platinum / 20;
		{
			int numberOfReinforcementAction = 0;
			short* reinforcementAction = (short*)malloc(numberOfReinforcements * 2 * sizeof(short)); // number of pods, zone

			if (numberOfReinforcements != 0) {
				computeReinforcements(reinforcementAction, &numberOfReinforcementAction, numberOfReinforcements, graph, world, zoneCount, continents, continentCount, &map, myId, round, playerCount, 
					resourcesOwnNeighbours);
			}

			// send reinforcements
			if (numberOfReinforcementAction == 0) {
				printf("WAIT\n");
			} else {
				for (int i = 0; i < numberOfReinforcementAction; i++) {
					printf("%hd %hd ", reinforcementAction[i * 2], reinforcementAction[i * 2 + 1]);
				}
				printf("\n");
			}

			free(reinforcementAction);
		}

		round += 1;
	}

	free(resources);
	free(links);

	free(graph);
	free(temp1);
	free(temp2);
	free(temp3);
	free(world);
	free(players);

	for (j = 0; j < continentCount; j++) {
		free(continents[j].zones);
	}
	free(continents);

	free(resourcesOwnNeighbours);
	free(distanceToAll);

	return 0;
}

void copyArrays(short* source, short* target, int num) {
	for (int i = 0; i < num; i++) {
		target[i] = source[i];
	}
}
int cmp0(const void *pa, const void *pb) {
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[0] < b[0]) return -1;
	if (a[0] > b[0]) return +1;
	return 0;
}
int cmp1(const void *pa, const void *pb) {
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[1] < b[1]) return -1;
	if (a[1] > b[1]) return +1;
	return 0;
}
int cmp1Desc(const void *pa, const void *pb) {
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[1] < b[1]) return +1;
	if (a[1] > b[1]) return -1;
	return 0;
}
int cmp2(const void *pa, const void *pb) {
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[2] < b[2]) return -1;
	if (a[2] > b[2]) return +1;
	return 0;
}
int cmp2Desc(const void *pa, const void *pb) {
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[2] < b[2]) return +1;
	if (a[2] > b[2]) return -1;
	return 0;
}
int cmp1DescULL(const void *pa, const void *pb) {
	const unsigned long long* a = (const unsigned long long*)pa;
	const unsigned long long* b = (const unsigned long long*)pb;
	if (a[1] < b[1]) return +1;
	if (a[1] > b[1]) return -1;
	return 0;
}