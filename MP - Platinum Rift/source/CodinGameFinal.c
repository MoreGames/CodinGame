#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define MAX_CONNECTIONS 6
#define WORLD_INFO 8

//#define DEBUG
//#define DMOVE
//#define DINSERT

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)<(y)?(y):(x)) 

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
	short resourceZones[7];
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
	short resourceZones[7];// number of zones with 6 resources,5 resources,4 resources,...
};

short breathFirstSearch(short* graph, int zoneCount, short startZone, short endZone, short* temp1, short* temp2, char all){
	int i, n = 0;
	short zone, next;

	// init queue
	temp1[n++] = startZone;
	// init came_from
	for (i = 0; i<zoneCount; i++){
		temp2[i] = -1;
	}

	for (; n;){
		zone = temp1[--n];

		if (!all&&zone == endZone)
			break;

		for (i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
			if (temp2[next] == -1){
				temp1[n++] = next;
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
// low order priority queue
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


void copyArrays(short* source, short* target, int num){
	for (int i = 0; i<num; i++){
		target[i] = source[i];
	}
}
int cmp0(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[0]<b[0]) return -1;
	if (a[0]>b[0]) return +1;
	return 0;
}
int cmp0Desc(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[0]<b[0]) return +1;
	if (a[0]>b[0]) return -1;
	return 0;
}
int cmp1(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[1]<b[1]) return -1;
	if (a[1]>b[1]) return +1;
	return 0;
}
int cmp1Desc(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[1]<b[1]) return +1;
	if (a[1]>b[1]) return -1;
	return 0;
}
int cmp2(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[2]<b[2]) return -1;
	if (a[2]>b[2]) return +1;
	return 0;
}
int cmp2Desc(const void *pa, const void *pb){
	const short* a = (const short*)pa;
	const short* b = (const short*)pb;
	if (a[2]<b[2]) return +1;
	if (a[2]>b[2]) return -1;
	return 0;
}
int cmp1DescULL(const void *pa, const void *pb){
	const unsigned long long* a = (const unsigned long long*)pa;
	const unsigned long long* b = (const unsigned long long*)pb;
	if (a[1]<b[1]) return +1;
	if (a[1]>b[1]) return -1;
	return 0;
}
int cmp1ULL(const void *pa, const void *pb){
	const unsigned long long* a = (const unsigned long long*)pa;
	const unsigned long long* b = (const unsigned long long*)pb;
	if (a[1]<b[1]) return -1;
	if (a[1]>b[1]) return +1;
	return 0;
}

void initGraph(short* graph, int zoneCount, short* links, int linkCount){
	for (int i = 0; i<zoneCount*MAX_CONNECTIONS; i++){
		graph[i] = -1;
	}

	for (int j = 0; j<zoneCount; j++){
		char found = 0;

		for (int i = 0; i<linkCount; i++){
			if (links[i * 2] == j)  {
				graph[j*MAX_CONNECTIONS + found] = links[i * 2 + 1];
				found += 1;
				continue;
			}
			if (links[i * 2 + 1] == j)  {
				graph[j*MAX_CONNECTIONS + found] = links[i * 2];
				found += 1;
			}
		}
	}
}
void initWorld(short* world, int zoneCount, short* resources, player* players, map* map, int playerCount){
	int num = 0, sum = 0;

	for (int i = 0; i<zoneCount; i++){
		world[i*WORLD_INFO] = i;// zoneId
		//world[i*WORLD_INFO+1] // continentId
		world[i*WORLD_INFO + 2] = resources[i];// resources
		world[i*WORLD_INFO + 3] = -1;// ownerId
		//world[i*WORLD_INFO+4] // pods1
		//world[i*WORLD_INFO+5] // pods2
		//world[i*WORLD_INFO+6] // pods3
		//world[i*WORLD_INFO+7] // pods4
	}

	for (int i = 0; i<playerCount; i++){
		players[i].platinum = 200;
		players[i].income = 0;
		players[i].numberOfPods = 0;
		players[i].numberOfZones = 0;
	}

	for (int i = 0; i<7; i++){
		map->resourceZones[i] = 0;
	}
	for (int i = 0; i<zoneCount; i++){
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
int sumResources(short* zones, int zoneCount, short* resources){
	int sum = 0;
	for (int i = 0; i<zoneCount; i++){
		sum += resources[zones[i]];
	}
	return sum;
}
int numberOfResourceZones(short* zones, int zoneCount, short* resources){
	int sum = 0;
	for (int i = 0; i<zoneCount; i++){
		sum += !!resources[zones[i]];
	}
	return sum;
}
int setContinents(continent** continents, short* world, short* resources, int zoneCount, short* graph, short* temp1, short* temp2){
	int i, j, continentNumber = 0;
	short* searched = (short*)malloc(zoneCount*sizeof(short));
	for (i = 0; i<zoneCount; i++){
		searched[i] = -1;
	}

	i = 0;
	do {
		breathFirstSearch(graph, zoneCount, (short)i, (short)(zoneCount - 1), temp1, temp2, (char)1);

		for (j = 0; j<zoneCount; j++){
			if (temp2[j] != -1){
				searched[j] = continentNumber;
			}
		}
		for (; i<zoneCount&&searched[i] != -1; i++);

		continentNumber += 1;
	} while (i<zoneCount);


	*continents = (continent*)malloc(zoneCount*continentNumber*sizeof(continent));
	for (j = 0; j<continentNumber; j++){
		(*continents)[j].zones = (short*)malloc(zoneCount*sizeof(continent));
	}
	for (j = 0; j<continentNumber; j++){
		for (i = 0; i<zoneCount; i++){
			(*continents)[j].zones[i] = -1;
		}
	}
	for (j = 0; j<continentNumber; j++){
		(*continents)[j].numberOfPlayerPods[0] = 0;
		(*continents)[j].numberOfPlayerPods[1] = 0;
		(*continents)[j].numberOfPlayerPods[2] = 0;
		(*continents)[j].numberOfPlayerPods[3] = 0;
		(*continents)[j].numberOfPlayerZones[0] = 0;
		(*continents)[j].numberOfPlayerZones[1] = 0;
		(*continents)[j].numberOfPlayerZones[2] = 0;
		(*continents)[j].numberOfPlayerZones[3] = 0;

		int k = 0;
		for (i = 0; i<zoneCount; i++){
			if (searched[i] == j){
				(*continents)[j].zones[k] = i;
				k++;
			}
		}

		for (i = 0; i<7; i++){
			(*continents)[j].resourceZones[i] = 0;
		}
		for (int i = 0; i<k; i++){
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
		for (j = 0; j<continentNumber; j++){
			temp[j * 2] = j;
			temp[j * 2 + 1] = (*continents)[j].zoneCount;
		}

		qsort(temp, continentNumber, 2 * sizeof(short), cmp1);

		for (j = 0; j<continentNumber; j++){
			(*continents)[temp[j * 2]].rankZones = j + 1;
		}

		for (j = 0; j<continentNumber; j++){
			temp[j * 2] = j;
			temp[j * 2 + 1] = (*continents)[j].sumOfResources;
		}

		qsort(temp, continentNumber, 2 * sizeof(short), cmp1);

		for (j = 0; j<continentNumber; j++){
			(*continents)[temp[j * 2]].rankResources = j + 1;
		}

		free(temp);
	}

	// link world with continents
	for (i = 0; i<zoneCount; i++){
		world[i*WORLD_INFO + 1] = searched[i];
	}

	free(searched);

	return continentNumber;
}

void computeDistanceToAll(short* graph, int zoneCount, short* temp1, short* temp2, short* temp3, short* d){
	short* costZone = (short*)malloc(zoneCount*sizeof(short));
	for (int i = 0; i<zoneCount; i++)
		costZone[i] = 1;

	for (int i = 0; i<zoneCount; i++){
		dijkstra(graph, zoneCount, (short)i, (short)i, temp3, temp2, temp1, costZone, (char)1);

		for (int j = 0; j<zoneCount; j++){
			if (j == i){
				d[i*zoneCount + j] = 0;
			} else{
				// reconstruct path
				short zone = j;
				int k;
				for (k = 0; zone != i; k++){
					zone = temp2[zone];
					if (zone == -1) break;
				}
				if (zone == -1){
					d[i*zoneCount + j] = -1;
				} else{
					d[i*zoneCount + j] = k;
				}
			}
		}
	}

	free(costZone);
}
void computeDistanceResourcesToAll(short* resources, int zoneCount, short* distanceToAll, float* distanceResourcesToAll){
	for (int i = 0; i<zoneCount; i++){
		for (int j = 0; j<zoneCount; j++){
			if (distanceToAll[i*zoneCount + j] == -1){
				distanceResourcesToAll[i*zoneCount + j] = -1;
			} else if (distanceToAll[i*zoneCount + j] == 0){
				distanceResourcesToAll[i*zoneCount + j] = 0;
			} else{
				distanceResourcesToAll[i*zoneCount + j] = ((float)resources[j] + 0.2f) / (float)distanceToAll[i*zoneCount + j];
			}
		}
	}
}

void updateWorldAll(short* world, int zoneCount, short* resources, player* players, continent* continents, int continentCount, int playerCount){
	int num[4] = { 0 }, sum[4] = { 0 }, in[4] = { 0 }, plat[4] = { 0 };

	for (int i = 0; i<zoneCount; i++){
		int zId;// this zone's ID
		scanf("%d%d%d%d%d%d",
			&zId,
			&world[i*WORLD_INFO + 3],	// ownerId
			&world[i*WORLD_INFO + 4],	// pods0
			&world[i*WORLD_INFO + 5],	// pods1
			&world[i*WORLD_INFO + 6],	// pods2
			&world[i*WORLD_INFO + 7]	// pods3
			);
		fgetc(stdin);
	}

	// continent
	for (int j = 0; j<continentCount; j++){
		continents[j].numberOfPlayerPods[0] = 0;
		continents[j].numberOfPlayerPods[1] = 0;
		continents[j].numberOfPlayerPods[2] = 0;
		continents[j].numberOfPlayerPods[3] = 0;
		continents[j].numberOfPlayerZones[0] = 0;
		continents[j].numberOfPlayerZones[1] = 0;
		continents[j].numberOfPlayerZones[2] = 0;
		continents[j].numberOfPlayerZones[3] = 0;
		for (int i = 0; i<continents[j].zoneCount; i++){
			for (int k = 0; k<4; k++){
				continents[j].numberOfPlayerPods[k] += world[continents[j].zones[i] * WORLD_INFO + 4 + k];
			}
			if (world[continents[j].zones[i] * WORLD_INFO + 3] != -1)
				continents[j].numberOfPlayerZones[world[continents[j].zones[i] * WORLD_INFO + 3]] += 1;
		}
	}

	// player
	for (int i = 0; i<zoneCount; i++){
		if (world[i*WORLD_INFO + 3] != -1) num[world[i*WORLD_INFO + 3]] += 1;
		if (world[i*WORLD_INFO + 3] != -1) in[world[i*WORLD_INFO + 3]] += resources[i];
		sum[0] += world[i*WORLD_INFO + 4];
		sum[1] += world[i*WORLD_INFO + 5];
		sum[2] += world[i*WORLD_INFO + 6];
		sum[3] += world[i*WORLD_INFO + 7];
	}
	for (int i = 0; i<playerCount; i++){
		players[i].platinum = (players[i].platinum % 20) + in[i];
		players[i].income = in[i];
		players[i].numberOfPods = num[i];
		players[i].numberOfZones = sum[i];
	}
}

void updateOwnerSingle(short* world, int myId, short zone){
	short temp[8];

	for (int i = 0; i<4; i++){
		temp[i * 2] = i;
		temp[i * 2 + 1] = world[zone*WORLD_INFO + 4 + i];
	}

	qsort((void*)temp, 4, 2 * sizeof(short), cmp1Desc);

	if (temp[0] == 0){
		;// nothing to do
	} else if (temp[0]>0 && temp[2] == 0){
		world[zone*WORLD_INFO + 3] = temp[1];
	} else{ // fighting zone
		for (int j = 0; j<3; j++){
			for (int i = 0; i<4; i++){
				temp[i] = MAX(temp[i] - 1, 0);
			}

			qsort((void*)temp, 4, 2 * sizeof(short), cmp1Desc);

			if (temp[0] == 0){
				break;// nothing to do
			} else if (temp[0]>0 && temp[2] == 0){
				world[zone*WORLD_INFO + 3] = temp[1];
				break;
			}
		}

		if (temp[0]>0 && temp[2]>0){
			world[zone*WORLD_INFO + 3] = -1;
		}

	}
}
void updateOwner(short* world, int myId, short startZone, short endZone){
	updateOwnerSingle(world, myId, startZone);
	updateOwnerSingle(world, myId, endZone);
}
void updateWorldSingle(short* world, int myId, short startZone, short endZone){
	world[startZone*WORLD_INFO + 4 + myId] -= 1;
	world[endZone*WORLD_INFO + 4 + myId] += 1;

	updateOwner(world, myId, startZone, endZone);
}

int getNumberOfPods(continent* continents, int continentCount, int playerId){
	int sum = 0;
	for (int i = 0; i<continentCount; i++){
		sum += continents[i].numberOfPlayerPods[playerId];
	}
	return sum;
}


void computeResourcesOwnNeighbours(short* graph, short* world, int zoneCount, continent* continents, int continentCount, short* r){
	short next;
	for (int i = 0; i<zoneCount; i++){
		r[i * 2] = i;
		r[i * 2 + 1] = world[i*WORLD_INFO + 2];
		for (int j = 0; j<MAX_CONNECTIONS && ((next = graph[i*MAX_CONNECTIONS + j]) != -1); j++){
			r[i * 2 + 1] += world[next*WORLD_INFO + 2];
		}
	}
	for (int i = 0; i<zoneCount; i++){
		r[i * 2 + 1] *= 10;
		r[i * 2 + 1] += (continentCount - continents[world[i*WORLD_INFO + 1]].rankZones);
	}
}
short findResourcesOwnNeighbours(short zone, short* r, int zoneCount){
	for (int i = 0; i<zoneCount; i++){
		if (zone == r[i * 2])
			return r[i * 2 + 1];
	}
	return 0;
}
char isNeighbour(short zone, short* zones, int count, short* graph){
	short next;
	for (int i = 0; i<count; i++){
		for (int j = 0; j<MAX_CONNECTIONS && ((next = graph[zones[i] * MAX_CONNECTIONS + j]) != -1); j++){
			if (zone == next)
				return 1;
		}
	}
	return 0;
}


char numberOfPlayersInZone(short zone, short* world){
	char num = 0;
	for (int j = 4; j<8; j++){
		num += !!world[zone*WORLD_INFO + j];
	}
	return num;
}
char checkIfOwnZone(short zone, short* world, int playerId){
	return (world[zone*WORLD_INFO + 3] == playerId);
}
char checkIfZoneFighting(short zone, short* world){
	if (numberOfPlayersInZone(zone, world)>1){
		return 1;
	} else{
		return 0;
	}
}
char checkIfContinentIsOwned(short zone, short* world, int zoneCount, int playerId){
	short contNum = world[zone*WORLD_INFO + 1];
	for (int i = 0; i<zoneCount; i++){
		if (world[i*WORLD_INFO + 1] == contNum&&world[i*WORLD_INFO + 3] != playerId){
			return 0;
		}
	}
	return 1;
}
char checkIfContinentHasOwnPods(short zone, short* world, int zoneCount, int playerId){
	short contNum = world[zone*WORLD_INFO + 1];
	for (int i = 0; i<zoneCount; i++){
		if (world[i*WORLD_INFO + 1] == contNum&&world[i*WORLD_INFO + 4 + playerId]>0){
			return 1;
		}
	}
	return 0;
}



// 1 if enemy border zone,0 else
char checkEnemyBorderZone(short zone, short* graph, short* world, int playerId){
	char val = checkIfOwnZone(zone, world, playerId);
	short next;

	if (val != 0){
		for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
			if (world[next*WORLD_INFO + 3] != -1 && world[next*WORLD_INFO + 3] != playerId){
				return 1;
			}
		}

		return 0;
	}

	return 0;
}
// zones can be neutral (fighting zones) or enemy
short sumAdjacentEnemyPods(short zone, short* graph, short* world, int playerId){
	short sum = 0;
	short next;

	for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
		if (world[next*WORLD_INFO + 3] != playerId){
			for (int j = 4; j<8; j++){
				sum += world[next*WORLD_INFO + j];
			}
		}
	}

	return sum;
}
// not counting pods in fighting zones (e.g. pods from several players are in the zone)
short maxAdjacentEnemyPods(short zone, short* graph, short* world, int playerId){
	short val = checkEnemyBorderZone(zone, graph, world, playerId);
	short next;

	if (val != 0){
		short temp[8] = { 0, 0, 1, 0, 2, 0, 3, 0 };

		// TODO kleine Ungenauigkeit wegen fighting zones
		for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
			if (world[next*WORLD_INFO + 3] != -1 && world[next*WORLD_INFO + 3] != playerId){
				if (numberOfPlayersInZone(next, world) == 1){
					for (int j = 4; j<8; j++){
						temp[(j - 4) * 2 + 1] += world[next*WORLD_INFO + j];
					}
				}
			}
		}

		qsort((void*)temp, 4, 2 * sizeof(short), cmp1Desc);

		if (temp[0] == playerId){
			val = temp[3];
		} else{
			val = temp[1];
		}
	}

	return val;
}
// zones can be neutral or enemy
short maxAdjacentResourcesEnemy(short zone, short* graph, short* world, int playerId){
	short maxRes = 0;
	short next;

	for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
		if (world[next*WORLD_INFO + 3] != playerId){
			if (maxRes<world[next*WORLD_INFO + 2]){
				maxRes = world[next*WORLD_INFO + 2];
			}
		}
	}

	return maxRes;
}
// every zone is considered
short maxAdjacentResources(short zone, short* graph, short* world){
	short maxRes = 0;
	short next;

	for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
		if (maxRes<world[next*WORLD_INFO + 2]){
			maxRes = world[next*WORLD_INFO + 2];
		}
	}

	return maxRes;
}
// zones can be neutral or enemy
short sumAdjacentResourcesEnemy(short zone, short* graph, short* world, int playerId){
	short sum = 0;
	short next;

	for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
		if (world[next*WORLD_INFO + 3] != playerId){
			sum += world[next*WORLD_INFO + 2];
		}
	}

	return sum;
}
// every zone is considered
short sumAdjacentResources(short zone, short* graph, short* world){
	short sum = 0;
	short next;

	for (int i = 0; i<MAX_CONNECTIONS && ((next = graph[zone*MAX_CONNECTIONS + i]) != -1); i++){
		sum += world[next*WORLD_INFO + 2];
	}

	return sum;
}
char getRandomNumber0To9(){
	return rand() % 10;
}

void setMoveTargets(short zone, short* graph, short* world, int zoneCount, continent* continents, int playerId, short* moveTargets, int* numOfMoveTargets){
	short contNum = world[zone*WORLD_INFO + 1];

	*numOfMoveTargets = 0;

	// only neutral or enemy zones
	for (int i = 0; i<continents[contNum].zoneCount; i++){
		short c = continents[contNum].zones[i];
		if (world[c*WORLD_INFO + 3] != playerId){
			moveTargets[*numOfMoveTargets] = c;
			*numOfMoveTargets += 1;
		}
	}
}
void createPodTargetMatrix(short* podList, int numOfPodList, short* moveTargets, int numOfMoveTargets, float* distanceResourcesToAll, int zoneCount, float* podTargetMatrix){
	for (int i = 0; i<numOfPodList; i++){
		for (int j = 0; j<numOfMoveTargets; j++){
			podTargetMatrix[i*numOfMoveTargets + j] = distanceResourcesToAll[podList[i * 2] * zoneCount + moveTargets[j]];
		}
	}
}
void bestPodTargetCombination(short* start, short* target, short* podList, int numOfPodList, short* moveTargets, int numOfMoveTargets, float* podTargetMatrix){
	float best = 0;

#ifdef DMOVE
	for (int i = 0; i<numOfPodList; i++){
		for (int j = 0; j<numOfMoveTargets; j++){
			fprintf(stderr, "%4.2f ", podTargetMatrix[i * numOfMoveTargets + j]);
		}
		fprintf(stderr, "\n");
	}
#endif

	for (int i = 0; i<numOfPodList; i++){
		for (int j = 0; j<numOfMoveTargets; j++){
			if (podTargetMatrix[i*numOfMoveTargets + j]>best){
				best = podTargetMatrix[i*numOfMoveTargets + j];
				*start = podList[i * 2];
				*target = moveTargets[j];
			}
		}
	}
}
void updatePodTargetData(short start, short target, short* podList, int* numOfPodList, short* moveTargets, int* numOfMoveTargets){
	for (int i = 0; i<(*numOfPodList); i++){
		if (podList[i * 2] == start){
			podList[i * 2 + 1] -= 1;
			if (podList[i * 2 + 1] == 0){
				(*numOfPodList) -= 1;
				qsort((void*)podList, (*numOfPodList) + 1, 2 * sizeof(short), cmp1Desc);
			} else{
				qsort((void*)podList, (*numOfPodList), 2 * sizeof(short), cmp1Desc);
			}
			break;
		}
	}

#ifdef DMOVE
	fprintf(stderr, "pod list \n");
	for (int k = 0; k<(*numOfPodList); k++){
		fprintf(stderr, "%3hd %2hd\n", podList[k * 2], podList[k * 2 + 1]);
	}
#endif

	for (int i = 0; i<*numOfMoveTargets; i++){
		if (moveTargets[i] == target){
			moveTargets[i] = -1;
			qsort((void*)moveTargets, *numOfMoveTargets, sizeof(short), cmp0Desc);
			*numOfMoveTargets -= 1;
		}
	}

#ifdef DMOVE
	fprintf(stderr, "targets \n");
	for (int k = 0; k<(*numOfMoveTargets); k++){
		fprintf(stderr, "%3hd\n", moveTargets[k]);
	}
#endif
}

void computePodAction(short* podAction, int* numberOfPodAction, short* graph, short* world, int zoneCount, continent* continents, int continentCount, int playerId, int playerCount,
	short* temp1, short* temp2, short* temp3, float* distanceResourcesToAll){

	short* worldCopy = (short *)malloc(zoneCount*WORLD_INFO*sizeof(short));
	for (int i = 0; i<zoneCount*WORLD_INFO; i++){
		worldCopy[i] = world[i];
	}

	*numberOfPodAction = 0;

	for (int i = 0; i<continentCount; i++){
		char isContinentOwned = checkIfContinentIsOwned(continents[i].zones[0], worldCopy, zoneCount, playerId);

		if (!isContinentOwned){
			char continentHasOwnPods = checkIfContinentHasOwnPods(continents[i].zones[0], worldCopy, zoneCount, playerId);

#ifdef DMOVE
			fprintf(stderr, "continent %d %d %d\n", i, continentHasOwnPods, isContinentOwned);
#endif
			if (continentHasOwnPods){

				int numOfMoveTargets = 0;
				short* moveTargets = (short*)malloc(zoneCount*sizeof(short));
				setMoveTargets(continents[i].zones[0], graph, worldCopy, zoneCount, continents, playerId, moveTargets, &numOfMoveTargets);

#ifdef DMOVE
				fprintf(stderr, "continent %d\n", i);
#endif

				if (numOfMoveTargets>0){

					int numOfPodList = 0;
					int podCounter = 0;
					short* podList = (short*)malloc(2000 * 2 * sizeof(short));// zone,ranking
					for (int j = 0; j<continents[i].zoneCount; j++){
						short z = continents[i].zones[j];
						if (world[z*WORLD_INFO + 4 + playerId]>0){
							char isFighting = checkIfZoneFighting(z, worldCopy);
							/*
							short podsWithoutMoveAction=maxAdjacentEnemyPods(start,graph,worldCopy,playerId);

							// no resources nothing to defend
							if(worldCopy[start*WORLD_INFO+2]==0){
							podsWithoutMoveAction=0;
							}
							*/
							if (!isFighting){
								podList[numOfPodList * 2] = z;
								podList[numOfPodList * 2 + 1] = world[z*WORLD_INFO + 4 + playerId];
								numOfPodList += 1;
								podCounter += world[z*WORLD_INFO + 4 + playerId];
							}
						}
					}

					short* costZone = (short*)malloc(zoneCount*sizeof(short));
					for (int j = 0; j<zoneCount; j++){
						if (worldCopy[j*WORLD_INFO + 3] == playerId){
							costZone[j] = 100;
						} else{
							costZone[j] = 99 - worldCopy[j*WORLD_INFO + 2];
						}
					}

					while (podCounter>0){
						float* podTargetMatrix = (float*)malloc(numOfPodList*numOfMoveTargets*sizeof(float));
						short start, next, target;

#ifdef DMOVE
						fprintf(stderr, "%3d %3d %3d\n", numOfPodList, podCounter, numOfMoveTargets);
#endif

						createPodTargetMatrix(podList, numOfPodList, moveTargets, numOfMoveTargets, distanceResourcesToAll, zoneCount, podTargetMatrix);

						bestPodTargetCombination(&start, &target, podList, numOfPodList, moveTargets, numOfMoveTargets, podTargetMatrix);

#ifdef DMOVE
						fprintf(stderr, "%3d %3d %3d %3d %3d\n", start, target, numOfPodList, podCounter, numOfMoveTargets);
#endif

						updatePodTargetData(start, target, podList, &numOfPodList, moveTargets, &numOfMoveTargets);

						next = dijkstra(graph, zoneCount, start, target, temp3, temp2, temp1, costZone, 0);

						updateWorldSingle(worldCopy, playerId, start, next);
						if (worldCopy[next*WORLD_INFO + 3] == playerId){
							costZone[next] = 100;
						} else{
							costZone[next] = 99 - worldCopy[next*WORLD_INFO + 2];
						}

						podAction[(*numberOfPodAction) * 3] = 1;
						podAction[(*numberOfPodAction) * 3 + 1] = start;
						podAction[(*numberOfPodAction) * 3 + 2] = next;
						(*numberOfPodAction) += 1;

						free(podTargetMatrix);

						podCounter--;

						if (numOfMoveTargets == 0){
							setMoveTargets(continents[i].zones[0], graph, worldCopy, zoneCount, continents, playerId, moveTargets, &numOfMoveTargets);
							if (numOfMoveTargets == 0)
								break;
						}
					}

					free(podList);
					free(costZone);
				}

				free(moveTargets);
			}
		}
	}

	free(worldCopy);
}

unsigned long long setReinforcementRanking(short zone, short* graph, short* world, continent* continents, player* players, int playerCount, int playerId){

	short resource = world[zone*WORLD_INFO + 2];

	char isOwnZone = checkIfOwnZone(zone, world, playerId);
	char isConquerable = maxAdjacentEnemyPods(zone, graph, world, playerId)>world[zone*WORLD_INFO + 4 + playerId];
	char hasAdjResEnemy = !!sumAdjacentResourcesEnemy(zone, graph, world, playerId);
	char hasAdjEnemyPods = !!sumAdjacentEnemyPods(zone, graph, world, playerId);
	char isEnemyBorderZone = checkEnemyBorderZone(zone, graph, world, playerId);

	// hier ev in abhängigkeit vom continentRank oder der Anzahl der Resourcen die Resourcenfelder gewichten
	// roundUp
	//short continentRank=continents[world[zone*WORLD_INFO+1]].rankResources;
	//short maxAdjRes=maxAdjacentResources(zone,graph,world,playerId);
	short maxAdjResEnemy = maxAdjacentResourcesEnemy(zone, graph, world, playerId);
	short sumAdjResEnemy = sumAdjacentResourcesEnemy(zone, graph, world, playerId);
	short maxAdjEnemyPods = maxAdjacentEnemyPods(zone, graph, world, playerId);
	short sumAdjEnemyPods = sumAdjacentEnemyPods(zone, graph, world, playerId);
	char randomNumber = getRandomNumber0To9();

#ifdef DINSERT
	fprintf(stderr, "%3d %1d %1d %1d %1d %1d %1d %1d %2d %1d %2d %1d\n",
		zone, resource, isOwnZone, isConquerable, hasAdjResEnemy, hasAdjEnemyPods, isEnemyBorderZone,
		maxAdjResEnemy, sumAdjResEnemy, maxAdjEnemyPods, sumAdjEnemyPods, randomNumber);
#endif

	unsigned long long num =
		resource * 100000000ULL +
		isConquerable * 10000000ULL +
		maxAdjResEnemy * 1000000ULL +
		sumAdjResEnemy * 10000ULL +
		maxAdjEnemyPods * 1000ULL +
		sumAdjEnemyPods * 10ULL +
		randomNumber * 1ULL;

	if (isOwnZone&&isConquerable&&resource>3){ // eigene Felder (4,5,6) die erobert werden können
		return 10000000000000000ULL + num * 10000000ULL;
	} else if ((!isOwnZone&&resource>3) || (isOwnZone&&isConquerable&&resource>0)){ // neutrale Felder (4,5,6) und eigene Felder (1,2,3) die erobert werden können
		return 1000000000000000ULL + num * 1000000ULL;
	} else if ((!isOwnZone&&resource>0) || (hasAdjResEnemy&&maxAdjResEnemy>3 && !hasAdjEnemyPods)){ // neutrale Felder (1,2,3) und Felder wo in einem angrenzenden gegnerischen Feld Resourcen (4,5,6) vorhanden sind,aber keine umgebenden pods
		unsigned long long num =
			(resource>3 ? resource : resource * 2) * 100000000ULL +
			isConquerable * 10000000ULL +
			maxAdjResEnemy * 1000000ULL +
			sumAdjResEnemy * 10000ULL +
			maxAdjEnemyPods * 1000ULL +
			sumAdjEnemyPods * 10ULL +
			randomNumber * 1ULL;

		return 100000000000000ULL + num * 100000ULL;
	} else if (hasAdjResEnemy&&maxAdjResEnemy>0 && !hasAdjEnemyPods){ // Felder wo in einem angrenzenden gegnerischen Feld Resourcen (1,2,3) vorhanden sind,aber keine umgebenden pods
		return 10000000000000ULL + num * 10000ULL;
	} else if (hasAdjResEnemy){ // Felder wo in einem angrenzenden gegnerischen Feld Resourcen (1,2,3,4,5,6) vorhanden sind,aber gegnerische pods vorhanden sind
		return 1000000000000ULL + num * 1000ULL;
	} else if (!isOwnZone){ // Neutrale Felder
		unsigned long long num =
			100000000ULL +
			isConquerable * 10000000ULL +
			maxAdjResEnemy * 1000000ULL +
			sumAdjResEnemy * 10000ULL +
			maxAdjEnemyPods * 1000ULL +
			sumAdjEnemyPods * 10ULL +
			randomNumber * 1ULL;

		return 100000000000ULL + num * 100ULL;
	} else if (isEnemyBorderZone){ // Wenn es keine Felder mehr gibt,sondern nur mehr eigene oder gegnerische dann immer Grenzfelder statt eigene Felder
		unsigned long long num =
			100000000ULL +
			isConquerable * 10000000ULL +
			maxAdjResEnemy * 1000000ULL +
			sumAdjResEnemy * 10000ULL +
			maxAdjEnemyPods * 1000ULL +
			sumAdjEnemyPods * 10ULL +
			randomNumber * 1ULL;

		return 10000000000ULL + num * 10ULL;
	} else{ // Neutrale Felder 
		return 1000000000ULL + num * 1ULL;
	}
}
void computeReinforcementsHelp(int contNum, continent* continents, short* reinforcementAction, int* numberOfReinforcementAction, short* world, int numOfInsertReinf) {
	short* contRes = (short*)malloc(continents[contNum].zoneCount * 2 * sizeof(short));

	for (int i = 0; i<continents[contNum].zoneCount; i++){
		contRes[i * 2] = continents[contNum].zones[i];
		contRes[i * 2 + 1] = world[continents[contNum].zones[i] * WORLD_INFO + 2];
	}

	qsort((void*)contRes, continents[contNum].zoneCount, 2 * sizeof(short), cmp1Desc);

	int n = 0;
	while (n < numOfInsertReinf) {
		for (int i = 0; contRes[i * 2 + 1]>0 && n < numOfInsertReinf; i++) {
			reinforcementAction[(*numberOfReinforcementAction) * 2] = 1;
			reinforcementAction[(*numberOfReinforcementAction) * 2 + 1] = contRes[i * 2];
			(*numberOfReinforcementAction) += 1;
			n += 1;
		}
	}

	free(contRes);
}
void computeReinforcements(short* reinforcementAction, int* numberOfReinforcementAction, int numberOfReinforcements, short* graph, short* world, int zoneCount,
	continent* continents, int continentCount, map* map, int playerId, int round, int playerCount, short* resourcesOwnNeighbours, player* players,
	short* podAction, int numberOfPodAction){
	int numberOfZoneTemp = 0;
	short* zoneTemp = (short*)malloc(zoneCount * 2 * sizeof(short));

	*numberOfReinforcementAction = 0;

	if (round == 0){
#ifndef S1
		if (playerCount == 2){
			short* contTemp = (short*)malloc(continentCount * 2 * sizeof(short));

			for (int i = 0; i<continentCount; i++){
				contTemp[i * 2] = i;
				contTemp[i * 2 + 1] = (short)(continents[i].averageResourcePerContinent*continents[i].sumOfResources);
			}

			qsort((void*)contTemp, continentCount, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
			fprintf(stderr, "continent priority: \n");
			for (int i = 0; i<continentCount; i++){
				fprintf(stderr, "%2hd %3hd\n", contTemp[i * 2 + 1], continents[contTemp[i * 2]].zoneCount);
			}
#endif

			// fokus on two continents
			int contFocus[2] = { contTemp[0], contTemp[2] };
			float sumR = 0;
			for (int i = 0; i<2; i++){
				sumR += continents[contFocus[i]].sumOfResources;
			}


#ifdef DINSERT
			fprintf(stderr, "top continents: \n");
			for (int i = 0; i<2; i++){
				fprintf(stderr, "%2hd %3hd\n", contFocus[i], continents[contFocus[i]].zoneCount);
			}
#endif

			// best other resource zones (beside the two continents)
			{
				int numberOfOtherZones = 0;
				short* otherZones = (short*)malloc(zoneCount * 2 * sizeof(short));


				for (int i = 0; i<zoneCount; i++){
					if (world[i*WORLD_INFO + 1] != contFocus[0] && world[i*WORLD_INFO + 1] != contFocus[1]){
						if (world[i*WORLD_INFO + 2] >= 3){
							otherZones[numberOfOtherZones * 2] = i;
							otherZones[numberOfOtherZones * 2 + 1] = findResourcesOwnNeighbours(i, resourcesOwnNeighbours, zoneCount);
							numberOfOtherZones += 1;
						}
					}
				}


				qsort((void*)otherZones, numberOfOtherZones, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
				fprintf(stderr, "top other zones: \n");
				for (int i = 0; i<numberOfOtherZones; i++){
					fprintf(stderr, "%2hd %3hd\n", otherZones[i * 2], otherZones[i * 2 + 1]);
				}
#endif

				// nie auf 6er oder auf die 2 besten
				if (numberOfOtherZones>0){
					short exclave = -1;

					for (int i = 0; i<numberOfOtherZones; i++){
						if (world[otherZones[i * 2] * WORLD_INFO + 2] != 6 && i>1){
							exclave = otherZones[i * 2];
						}
					}

					if (exclave == -1){
						exclave = otherZones[otherZones[(numberOfOtherZones - 1) * 2]];
					}

					if (map->averageResourcePerZone<2.5){
						reinforcementAction[0] = 2;

						numberOfReinforcements -= 2;
					} else{
						reinforcementAction[0] = 1;

						numberOfReinforcements -= 1;
					}
					reinforcementAction[1] = exclave;
					*numberOfReinforcementAction += 1;
				}

#ifdef DINSERT
				fprintf(stderr, "chosen top other zones: \n");
				for (int i = 0; i<*numberOfReinforcementAction; i++){
					fprintf(stderr, "%2hd\n", reinforcementAction[i * 2 + 1]);
				}
#endif

				free(otherZones);
			}

			// best continent resource zones
			for (int j = 0; j<2; j++){
				short reinforcementCont = (short)roundf((numberOfReinforcements*continents[contFocus[j]].sumOfResources / sumR));

				int numberOfTopZones = 0;
				short* topZones = (short*)malloc(zoneCount * 2 * sizeof(short));
				int numberOfTopZonesExtended = 0;
				short* topZonesExtended = (short*)malloc(zoneCount * 2 * sizeof(short));

				for (int i = 0; i<continents[contFocus[j]].zoneCount; i++){
					if (world[continents[contFocus[j]].zones[i] * WORLD_INFO + 2]>MIN((short)(continents[contFocus[j]].averageResourcePerZone * 2), 3)){
						topZones[numberOfTopZones * 2] = continents[contFocus[j]].zones[i];
						topZones[numberOfTopZones * 2 + 1] = findResourcesOwnNeighbours(continents[contFocus[j]].zones[i], resourcesOwnNeighbours, zoneCount);
						numberOfTopZones += 1;
					}
				}

				if (numberOfTopZones<3){
					for (int i = 0; i<continents[contFocus[j]].zoneCount; i++){
						if (world[continents[contFocus[j]].zones[i] * WORLD_INFO + 2] == MIN((short)(continents[contFocus[j]].averageResourcePerZone * 2), 3)){
							topZonesExtended[numberOfTopZonesExtended * 2] = continents[contFocus[j]].zones[i];
							topZonesExtended[numberOfTopZonesExtended * 2 + 1] = findResourcesOwnNeighbours(continents[contFocus[j]].zones[i], resourcesOwnNeighbours, zoneCount);
							numberOfTopZonesExtended += 1;
						}
					}

					// keine Felder angrenzend zu bereits gefunden top zonen
					for (int i = 0; i<numberOfTopZonesExtended; i++){
						if (isNeighbour(topZonesExtended[i * 2], topZones, numberOfTopZones, graph)){
							topZonesExtended[i * 2 + 1] = -1;
						}
					}
					if (numberOfTopZonesExtended>0){
						qsort((void*)topZonesExtended, numberOfTopZonesExtended, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
						fprintf(stderr, "top continent %d zones extended: \n", contFocus[j]);
						for (int i = 0; i<numberOfTopZonesExtended; i++){
							fprintf(stderr, "%2hd %3hd\n", topZonesExtended[i * 2], topZonesExtended[i * 2 + 1]);
						}
#endif
						topZones[numberOfTopZones * 2] = topZonesExtended[0];
						topZones[numberOfTopZones * 2 + 1] = topZonesExtended[1];
						numberOfTopZones += 1;
					}
				}

				qsort((void*)topZones, numberOfTopZones, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
				fprintf(stderr, "top continent %d zones: \n", contFocus[j]);
				for (int i = 0; i<numberOfTopZones; i++){
					fprintf(stderr, "%2hd %3hd\n", topZones[i * 2], topZones[i * 2 + 1]);
				}
#endif

				// set reinforcement,first zone gets always an additional pod
				reinforcementAction[*numberOfReinforcementAction * 2] = 1;
				reinforcementAction[*numberOfReinforcementAction * 2 + 1] = topZones[0];
				reinforcementCont -= 1;

				for (int i = 0; i<MIN(numberOfTopZones, reinforcementCont) - 1; i++){
					reinforcementAction[(*numberOfReinforcementAction + 1) * 2 + i * 2] = 0;
					reinforcementAction[(*numberOfReinforcementAction + 1) * 2 + i * 2 + 1] = 0;
				}
				for (int n = 0; n<reinforcementCont;){
					for (int i = 0; i<numberOfTopZones&&n<reinforcementCont; i++, n++){
						reinforcementAction[*numberOfReinforcementAction * 2 + i * 2] += 1;
						reinforcementAction[*numberOfReinforcementAction * 2 + i * 2 + 1] = topZones[i * 2];
					}
				}

				*numberOfReinforcementAction += MAX(MIN(numberOfTopZones, reinforcementCont), 1);

				free(topZones);
				free(topZonesExtended);
			}

			free(contTemp);
#endif
		} else if (playerCount == 3){
#ifndef S3
			short* contTemp = (short*)malloc(continentCount * 2 * sizeof(short));

			for (int i = 0; i < continentCount; i++){
				contTemp[i * 2] = i;
				contTemp[i * 2 + 1] = (short)(continents[i].averageResourcePerContinent * 100);
			}

			qsort((void*)contTemp, continentCount, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
			fprintf(stderr, "continent priority: \n");
			for (int i = 0; i<continentCount; i++){
				fprintf(stderr, "%2hd %2hd %3hd\n", contTemp[i * 2], contTemp[i * 2 + 1], continents[contTemp[i * 2]].zoneCount);
			}
#endif

			int contNum = contTemp[0];
			if (contNum == 3 || contNum == 4) {
				computeReinforcementsHelp(contNum, continents, reinforcementAction, numberOfReinforcementAction, world, 1);
				numberOfReinforcements -= 1;
				contNum = contTemp[2];
				if (contNum == 3 || contNum == 4) {
					contNum = contTemp[4];
				}
			}

			computeReinforcementsHelp(contNum, continents, reinforcementAction, numberOfReinforcementAction, world, numberOfReinforcements);

			free(contTemp);
#endif
		} else if (playerCount == 4){
#ifndef S4
			short* contTemp = (short*)malloc(continentCount * 2 * sizeof(short));

			for (int i = 0; i < continentCount; i++){
				contTemp[i * 2] = i;
				contTemp[i * 2 + 1] = (short)(continents[i].averageResourcePerContinent * 100);
			}

			qsort((void*)contTemp, continentCount, 2 * sizeof(short), cmp1Desc);

#ifdef DINSERT
			fprintf(stderr, "continent priority: \n");
			for (int i = 0; i<continentCount; i++){
				fprintf(stderr, "%2hd %2hd %3hd\n", contTemp[i * 2], contTemp[i * 2 + 1], continents[contTemp[i * 2]].zoneCount);
			}
#endif

			int contNum = contTemp[0];
			if (contNum == 3 || contNum == 4) {
				computeReinforcementsHelp(contNum, continents, reinforcementAction, numberOfReinforcementAction, world, 1);
				numberOfReinforcements -= 1;
				contNum = contTemp[2];
				if (contNum == 3 || contNum == 4) {
					contNum = contTemp[4];
				}
			}


			computeReinforcementsHelp(contNum, continents, reinforcementAction, numberOfReinforcementAction, world, numberOfReinforcements);

			free(contTemp);
#endif
		}

		free(zoneTemp);
	} else
	{
		// make world clone and execute pod moves but dont update
		// Wie man daraus einen Nutzen (ohne Nachteile) ziehen kann ist noch nicht klar,wird noch nicht verwendet
		// TODO 
		short* worldCopy = (short *)malloc(zoneCount*WORLD_INFO*sizeof(short));
		for (int i = 0; i<zoneCount*WORLD_INFO; i++){
			worldCopy[i] = world[i];
		}
		for (int i = 0; i<numberOfPodAction; i++){
			updateWorldSingle(worldCopy, playerId, podAction[i * 2 + 1], podAction[i * 2 + 2]);
		}

		if (playerCount > 0){
			int numberOfReinforcementZones = 0;
			unsigned long long* rankingReinforcementZones = (unsigned long long*)malloc(zoneCount * 2 * sizeof(unsigned long long));// zone,ranking

#ifdef DINSERT
			//fprintf(stderr,"zone res isC isN hasAdjR hasAdjE isE cR maxAdjR maxNeu sumAdjRes rN\n");
#endif

			for (int i = 0; i<zoneCount; i++){
				if (world[i*WORLD_INFO + 3] == playerId || world[i*WORLD_INFO + 3] == -1){
					if (!checkIfContinentIsOwned(i, world, zoneCount, playerId)){
						rankingReinforcementZones[numberOfReinforcementZones * 2] = i;
						rankingReinforcementZones[numberOfReinforcementZones * 2 + 1] = setReinforcementRanking(i, graph, world, continents, players, playerCount, playerId);
						numberOfReinforcementZones += 1;
					}
				}
			}

			qsort((void*)rankingReinforcementZones, numberOfReinforcementZones, 2 * sizeof(unsigned long long), cmp1DescULL);
			// TODO
#ifdef DINSERT
			fprintf(stderr, "reinforcement zones \n");
			fprintf(stderr, "zone ranking \n");
			for (int i = 0; i<MIN(numberOfReinforcementZones, 15); i++){
				fprintf(stderr, "%3d %3llu %llu \n", i, rankingReinforcementZones[i * 2], rankingReinforcementZones[i * 2 + 1]);
			}
			fprintf(stderr, "reinforcements: %d \n", numberOfReinforcements);
#endif

			(*numberOfReinforcementAction) = MIN(numberOfReinforcementZones, numberOfReinforcements);
			for (int i = 0; i<(*numberOfReinforcementAction); i++){
				reinforcementAction[i * 2] = 0;
				reinforcementAction[i * 2 + 1] = 0;
			}

			for (int n = 0; n<numberOfReinforcements;){
				for (int i = 0; i<numberOfReinforcementZones&&n<numberOfReinforcements; i++, n++){
					reinforcementAction[i * 2] += 1;
					reinforcementAction[i * 2 + 1] = (short)rankingReinforcementZones[i * 2];
				}
			}

			free(rankingReinforcementZones);
		} else{

		}

		free(worldCopy);
	}

#ifdef DINSERT
	fprintf(stderr, "reinforcement zones: \n");
	for (int i = 0; i<*numberOfReinforcementAction; i++){
		fprintf(stderr, "%2hd %2hd\n", reinforcementAction[i * 2], reinforcementAction[i * 2 + 1]);
	}
#endif
}


int main(void){
	char saying[20][100] = {"Upps!",
		"Is that you?",
		"Really?",
		"I'm truly sorry!",
		"That's not nice",
		"You and me",
		"Together we rule!",
		"Narf!",
		"Fjord!!",
		"I'm not a fridge",
		"We must prepare for tonight!",
		"What are we gonna do tonight?",
		"If I could reach you ...",
		"You make my head hurt.",
		"Brilliant plan",
		"I'm with you,",
		"You astound me",
		"That's a simple task",
		"Let me think",
		"Oh, one quick question"
};

	int playerCount;
	int myId;
	int zoneCount;
	int linkCount;

	int i, j;
	short zone;
	int continentCount;
	int round = 0;

	short* resources, *links, *world;
	short* graph;
	short* temp1, *temp2, *temp3;
	player* players;
	continent* continents;
	map map;
	short* resourcesOwnNeighbours;
	short* distanceToAll;
	float* distanceResourcesToAll;

	srand((unsigned int)time(NULL));

	scanf("%d%d%d%d", &playerCount, &myId, &zoneCount, &linkCount); fgetc(stdin);

	resources = (short*)malloc(zoneCount*sizeof(short));
	links = (short*)malloc(linkCount * 2 * sizeof(short));
	players = (player*)malloc(playerCount*sizeof(player));

	resourcesOwnNeighbours = (short*)malloc(zoneCount * 2 * sizeof(short));
	distanceToAll = (short*)malloc(zoneCount*zoneCount*sizeof(short));
	distanceResourcesToAll = (float*)malloc(zoneCount*zoneCount*sizeof(float));

	for (int i = 0; i<zoneCount; i++){
		int zoneId;
		int resource;
		scanf("%d%d", &zoneId, &resource); fgetc(stdin);
		resources[zoneId] = (short)resource;
		//fprintf(stderr,"%d %d\n",zoneId,resource);
	}

	for (int i = 0; i<linkCount; i++){
		int start, end;
		scanf("%d%d", &start, &end); fgetc(stdin);
		links[i * 2] = (short)start;
		links[i * 2 + 1] = (short)end;
		//fprintf(stderr,"%d %d\n",start,end);
	}

	graph = (short*)malloc(zoneCount*MAX_CONNECTIONS*sizeof(short));
	initGraph(graph, zoneCount, links, linkCount);

	world = (short*)malloc(zoneCount*WORLD_INFO*sizeof(short));
	initWorld(world, zoneCount, resources, players, &map, playerCount);

	// some kind of queue and path and cost array
	temp1 = (short*)malloc(zoneCount*sizeof(short));
	// came_from reference
	temp2 = (short*)malloc(zoneCount*sizeof(short));
	// some special kind of priority queue
	temp3 = (short*)malloc(zoneCount * 2 * sizeof(short));
	continentCount = setContinents(&continents, world, resources, zoneCount, graph, temp1, temp2);

#ifdef DEBUG
	fprintf(stderr, "continents: \n");
	fprintf(stderr, "rankZ rankR sumR numRZones zCount avgRePerZone avgRePerCont resourceZones 6 5 4 3 2 1 0\n");
	for (j = 0; j<continentCount; j++){
		fprintf(stderr, "%5hhd %5hhd %4d %9d %6d %12.2f %12.2f %4d %4d %4d %4d %4d %4d %4d\n",
			continents[j].rankZones, continents[j].rankResources, continents[j].sumOfResources, continents[j].numberOfResourceZones, continents[j].zoneCount,
			continents[j].averageResourcePerZone, continents[j].averageResourcePerContinent,
			continents[j].resourceZones[6], continents[j].resourceZones[5], continents[j].resourceZones[4], continents[j].resourceZones[3], continents[j].resourceZones[2], continents[j].resourceZones[1], continents[j].resourceZones[0]);
	}

	fprintf(stderr, "rankZ rankR pods0 pods1 pods2 pods3 zones0 zones1 zones2 zones3");
	for (i = 0; i<zoneCount; i++){
		fprintf(stderr, "%4d", i);
	}
	fprintf(stderr, "\n");
	for (j = 0; j<continentCount; j++){
		fprintf(stderr, "%5hhd %5hhd %5d %5d %5d %5d %6d %6d %6d %6d", continents[j].rankZones, continents[j].rankResources,
			continents[j].numberOfPlayerPods[0], continents[j].numberOfPlayerPods[1], continents[j].numberOfPlayerPods[2], continents[j].numberOfPlayerPods[3],
			continents[j].numberOfPlayerZones[0], continents[j].numberOfPlayerZones[1], continents[j].numberOfPlayerZones[2], continents[j].numberOfPlayerZones[3]);
		for (i = 0; i<continents[j].zoneCount; i++){
			fprintf(stderr, "%4hd", continents[j].zones[i]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "map: \n");
	fprintf(stderr, "number of resource zones  %hd \n", map.numberOfResourceZones);
	fprintf(stderr, "sum of resources          %hd \n", map.sumOfResources);
	fprintf(stderr, "average resource per zone %f \n", map.averageResourcePerZone);
	fprintf(stderr, "resource zones \n", map.averageResourcePerZone);
	for (i = 0; i<7; i++){
		fprintf(stderr, "%8d %5hd \n", i, map.resourceZones[i]);
	}

	fprintf(stderr, "player overview: \n");
	fprintf(stderr, "player platinum income numberOfPods numberOfZones \n");
	for (int i = 0; i<playerCount; i++){
		fprintf(stderr, "%6d ", i);
		fprintf(stderr, "%8hd ", players[i].platinum);
		fprintf(stderr, "%6hd ", players[i].income);
		fprintf(stderr, "%12hd ", players[i].numberOfPods);
		fprintf(stderr, "%13hd \n", players[i].numberOfZones);
	}
#endif

	computeResourcesOwnNeighbours(graph, world, zoneCount, continents, continentCount, resourcesOwnNeighbours);
	qsort((void*)resourcesOwnNeighbours, zoneCount, 2 * sizeof(short), cmp1Desc);

	computeDistanceToAll(graph, zoneCount, temp1, temp2, temp3, distanceToAll);
	computeDistanceResourcesToAll(resources, zoneCount, distanceToAll, distanceResourcesToAll);

	while (1){
		int numberOfReinforcements = 0;
		int platinum;// my available Platinum
		scanf("%d", &platinum); fgetc(stdin);
		//fprintf(stderr, "platinum: %d round: %d\n", platinum, round);

		updateWorldAll(world, zoneCount, resources, players, continents, continentCount, playerCount);

#ifdef DEBUG
		fprintf(stderr, "player overview: \n");
		fprintf(stderr, "player platinum income numberOfPods numberOfZones \n");
		for (int i = 0; i<playerCount; i++){
			fprintf(stderr, "%6d ", i);
			fprintf(stderr, "%8hd ", players[i].platinum);
			fprintf(stderr, "%6hd ", players[i].income);
			fprintf(stderr, "%12hd ", players[i].numberOfPods);
			fprintf(stderr, "%13hd \n", players[i].numberOfZones);
		}
#endif

		numberOfReinforcements = platinum / 20;
		{
			// ------ moving ------

			int numberOfPodAction = 0;
			int numberOfPodsAvailable = getNumberOfPods(continents, continentCount, myId);
			short* podAction = (short*)malloc(numberOfPodsAvailable * 3 * sizeof(short));// number of pods,startZone,endZone
			int numberOfReinforcementAction = 0;
			short* reinforcementAction = (short*)malloc(numberOfReinforcements * 2 * sizeof(short));// number of pods,zone

			computePodAction(podAction, &numberOfPodAction, graph, world, zoneCount, continents, continentCount, myId, playerCount, temp1, temp2, temp3, distanceResourcesToAll);

			// move pods
			if (numberOfPodAction == 0){
				printf("WAIT\n");
			} else{
				for (int i = 0; i<numberOfPodAction; i++){
					printf("%hd %hd %hd ", podAction[i * 3], podAction[i * 3 + 1], podAction[i * 3 + 2]);
				}
				if ((round % 3) == (rand() % 3)) {
					int ind = rand() % 20;
					printf("'%s", saying[ind]);
				}
				printf("\n");
			}

			// ------ buying ------

			if (numberOfReinforcements != 0){
				computeReinforcements(reinforcementAction, &numberOfReinforcementAction, numberOfReinforcements, graph, world, zoneCount, continents, continentCount, &map, myId, round, playerCount,
					resourcesOwnNeighbours, players, podAction, numberOfPodAction);
			}

			// send reinforcements
			if (numberOfReinforcementAction == 0){
				printf("WAIT\n");
			} else{
				for (int i = 0; i<numberOfReinforcementAction; i++){
					printf("%hd %hd ", reinforcementAction[i * 2], reinforcementAction[i * 2 + 1]);
				}
				printf("\n");
			}

			free(podAction);
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

	for (j = 0; j<continentCount; j++){
		free(continents[j].zones);
	}
	free(continents);

	free(resourcesOwnNeighbours);
	free(distanceToAll);

	return 0;
}
