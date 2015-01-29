#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <functional>
#include <ctime>
#include <map>
#include <cstdlib>

using namespace std;

#define MAX_PLAYERS 2

double startTime, endTime;
double interval;

class Move {
public:
	int pods;
	int position;
	int next;

	Move() : pods(0), position(0), next(0) {
	}
	Move(int p, int po, int n) : pods(p), position(po), next(n) {
	}
};
class Zone {
public:
	int id;
	vector<Zone*> neighbours;
	int platinum;
	int isVisible;
	bool isExplored;
	int owner;
	int pods[MAX_PLAYERS];
	vector<vector<Zone*>> paths;

	int virtualOwner;
	int virtualIsExplored;
	int virtualPods[MAX_PLAYERS];

	Zone() : id(0), neighbours(), platinum(0), isVisible(0), isExplored(false), owner(-1), virtualOwner(-1), virtualIsExplored(false) {
	}

	bool operator==(const Zone &other) const {
		return (id == other.id);
	}
	int getUnkownNeighbourZones() {
		int ret = 0;
		for (size_t i = 0; i < neighbours.size(); i++) {
			Zone* next = (neighbours)[i];
			ret += !next->virtualIsExplored;
		}
		return ret;
	}
	void updateVirtualOwner() {
		int minElement = *min_element(pods, pods + 2);

		virtualPods[0] -= min(3, minElement);
		virtualPods[1] -= min(3, minElement);

		if ((virtualPods[0] > 0 && virtualPods[1] > 0) || (virtualPods[0] == 0 && virtualPods[1] == 0)) {
			;
		} else if (virtualPods[0] > virtualPods[1]) {
			virtualOwner = 0;
		} else if (virtualPods[1] > virtualPods[0]) {
			virtualOwner = 1;
		}
	}
	void updateVirtualIsExplored() {
		for (size_t i = 0; i < neighbours.size(); i++) {
			Zone* next = (neighbours)[i];
			next->virtualIsExplored = true;
		}
	}
	void resetVirtual() {
		virtualOwner = owner;
		virtualIsExplored = isExplored;
		virtualPods[0] = pods[0];
		virtualPods[1] = pods[1];
	}
};
class ZoneData {
public:
	int id;
	int platinum;
	int unkownNeighbourZones;
	int enemyPods;
	int dist;

	//ZoneData() : id(0), platinum(0), unkownNeighbourZones(0), enemyPods(0) {
	//}
	ZoneData(int id, int platinum, int unkownNeighbourZones, int enemyPods) : id(id), platinum(platinum), unkownNeighbourZones(unkownNeighbourZones), enemyPods(enemyPods) {
	}
	ZoneData(Zone zone, int enemyId) : id(zone.id), platinum(zone.platinum), unkownNeighbourZones(zone.getUnkownNeighbourZones()), enemyPods(zone.virtualPods[enemyId]) {
	}
	ZoneData(Zone *zone, int enemyId) : id(zone->id), platinum(zone->platinum), unkownNeighbourZones(zone->getUnkownNeighbourZones()), enemyPods(zone->virtualPods[enemyId]) {
	}
	ZoneData(Zone zone, int enemyId, int dist) : id(zone.id), platinum(zone.platinum), unkownNeighbourZones(zone.getUnkownNeighbourZones()), enemyPods(zone.virtualPods[enemyId]), dist(dist) {
	}
	ZoneData(Zone *zone, int enemyId, int dist) : id(zone->id), platinum(zone->platinum), unkownNeighbourZones(zone->getUnkownNeighbourZones()), enemyPods(zone->virtualPods[enemyId]), dist(dist) {
	}
};
class Pod {
public:
	int zone;
	bool hasJob;
	int platinum;
	int distanceToBase;
	int destination;

	Pod(int z, bool h, int p, int d) : zone(z), hasJob(h), platinum(p), distanceToBase(d) {
	}

	bool operator < (const Pod& p) const	{
		return (platinum > p.platinum);
	}
};
class LocalSearchData {
public:
	int id;
	int next;
	int nextdistToEnemyBase;
	int nextOwnPods;
	int numOfDest;

	LocalSearchData(int id, int next, int nextdistToEnemyBase, int nextOwnPods, int numOfDest) : id(id), next(next), nextdistToEnemyBase(nextdistToEnemyBase), nextOwnPods(nextOwnPods), numOfDest(numOfDest) {
	}
};
class GlobalSearchData {
public:
	int id;
	int next;
	int destination;
	int pods;

	GlobalSearchData(int id, int next, int destination, int pods) : id(id), next(next), destination(destination), pods(pods) {
	}
};


// http://www.redblobgames.com/pathfinding/a-star/introduction.html
unordered_map<int, Zone*> breathFirstSearch(Zone* start, Zone* goal, char all) {
	// init queue
	queue<Zone*> q;
	q.push(start);
	// init came_from
	unordered_map<int, Zone*> came_from;
	came_from[start->id] = start;


	for (; !q.empty();) {
		Zone* zone = q.front();
		q.pop();

		if (!all && zone == goal)
			break;

		for (size_t i = 0; i < zone->neighbours.size(); i++) {
			Zone* next = (zone->neighbours)[i];
			if (!came_from.count(next->id)) {
				q.push(next);
				came_from[next->id] = zone;
			}
		}
	}
	return came_from;
}
unordered_map<int, Zone*> dijkstra(Zone* start, Zone* goal, int* costZone, char all) {
	// init queue
	priority_queue<pair<int, Zone*>, vector<pair<int, Zone*>>, greater<pair<int, Zone*>>> q;
	q.emplace(make_pair(0, start));
	// init came_from
	unordered_map<int, Zone*> came_from;
	came_from[start->id] = start;
	// init cost_so_far
	unordered_map<int, int> cost_so_far;
	cost_so_far[start->id] = 0;

	for (; !q.empty();) {
		Zone* zone = q.top().second;
		q.pop();

		if (!all && *zone == *goal)
			break;

		for (size_t i = 0; i < zone->neighbours.size(); i++) {
			Zone* next = (zone->neighbours)[i];
			int new_cost = cost_so_far[zone->id] + costZone[next->id];
			if (!cost_so_far.count(next->id) || new_cost < cost_so_far[next->id]) {
				cost_so_far[next->id] = new_cost;
				q.emplace(make_pair(new_cost, next));
				came_from[next->id] = zone;
			}
		}
	}
	return came_from;
}
vector<Zone*> reconstructPath(Zone* start, Zone* end, unordered_map<int, Zone*>& came_from) {
	vector<Zone*> path;
	Zone* current = end;
	path.push_back(current);
	while (!(*current == *start)) {
		try {
			current = came_from.at(current->id);
		}
		catch (...) {
			return vector<Zone*>();
		}
		path.push_back(current);
	}
	return path;
}

class Game {
private:
	void updateVirtualMove(int pods, int start, int dest) {
		Zone* s = &zones[start];
		s->virtualPods[myId] -= 1;
		Zone* d = &zones[dest];
		d->virtualPods[myId] += 1;
		d->updateVirtualOwner();
		d->updateVirtualIsExplored();
	}
	void showVisibleZones() {
		fprintf(stderr, "visible zones: \n");
		for (int i = 0; i < zoneCount; i++) {
			if (zones[i].isVisible)
				fprintf(stderr, "%d %d %d %d | %d %d\n", zones[i].id, zones[i].platinum, zones[i].isExplored, zones[i].owner, zones[i].virtualIsExplored, zones[i].virtualOwner);
		}
	}
	void showPods() {
		fprintf(stderr, "pods: \n");
		for (auto pod : pods) {
			fprintf(stderr, "%d %d %d\n", pod.zone, pod.hasJob, pod.distanceToBase);
		}
	}

public:
	int turn;
	int playerCount;
	int platinum;
	int myId;
	int enemyId;
	int myBase;
	int enemyBase;
	int mySumPods;
	int enemySumPods;

	int zoneCount;
	int linkCount;
	Zone* zones;

	int** distanceToAll;
	vector<Pod> pods;

	void loadInitData() {
		cin >> playerCount >> myId >> zoneCount >> linkCount; cin.ignore();

		enemyId = !myId;

		fprintf(stderr, "ids: %d %d \n", myId, enemyId);
		fprintf(stderr, "zones: %d \n", zoneCount);

		zones = new Zone[zoneCount];

		for (int i = 0; i < zoneCount; i++) {
			int id, p;
			cin >> id >> p; cin.ignore();

			zones[id].id = id;
		}

		for (int i = 0; i < linkCount; i++) {
			int z1, z2;
			cin >> z1 >> z2; cin.ignore();

			Zone* a = &zones[z1];
			Zone* b = &zones[z2];

			a->neighbours.push_back(b);
			b->neighbours.push_back(a);

			//fprintf(stderr, "%d %d\n", z1, z2);
		}

		distanceToAll = new int*[zoneCount];
		for (int i = 0; i < zoneCount; i++) {
			distanceToAll[i] = new int[zoneCount];
		}
		for (int i = 0; i < zoneCount; i++) {
			Zone* z = &zones[i];
			unordered_map<int, Zone*> came_from = breathFirstSearch(z, NULL, 1);
			for (int j = 0; j < zoneCount; j++) {
				Zone* end = &zones[j];
				vector<Zone*> path = reconstructPath(z, end, came_from);
				z->paths.push_back(path);
				if (path.size() == 0){
					distanceToAll[i][j] = -1;
				} else if (path.size() == 1) {
					distanceToAll[i][j] = 0;
				} else {
					distanceToAll[i][j] = path.size() - 1;
				}
			}
		}

		//for (int i = 0; i < 15; i++) {
		//	for (int j = 0; j < 15; j++) {
		//		fprintf(stderr, "%4.2f ", distanceToAll[i][j]);
		//	}
		//	fprintf(stderr, "\n");
		//}
	}

	void loadRoundData() {
		cin >> platinum; cin.ignore();
		//fprintf(stderr, "platinum: %d\n", platinum);

		for (int i = 0; i < zoneCount; i++) {
			int id, o, p0, p1, v, p;
			cin >> id >> o >> p0 >> p1 >> v >> p; cin.ignore();

			//if (o != -1 || p0 != 0 || p1 != 0 || v != 0 || p != 0)
			//if (id > 10 && id < 70)
			//	fprintf(stderr, "%d %d %d %d %d %d \n", id, o, p0, p1, v, p);

			Zone* z = &zones[id];
			if (v) z->owner = o;
			z->pods[0] = p0;
			z->pods[1] = p1;
			z->isVisible = v;
			if (v) z->isExplored = true;
			if (v) z->platinum = p;
		}

		// reset virtual zones
		for (int i = 0; i < zoneCount; i++) {
			zones[i].resetVirtual();
		}

		// set pods
		pods.clear();
		for (int i = 0; i < zoneCount; i++) {
			for (int j = 0; j < zones[i].pods[myId]; j++) {
				pods.push_back(Pod(i, false, zones[i].platinum, distanceToAll[i][myBase]));
			}
		}
		// sort depending on resources
		sort(pods.begin(), pods.end());

		//showPods();

		// set bases
		if (turn == 0) {
			for (int i = 0; i < zoneCount; i++) {
				if (zones[i].owner == myId) {
					myBase = zones[i].id;
					continue;
				}
				if (zones[i].owner == enemyId) {
					enemyBase = zones[i].id;
				}
			}
			//fprintf(stderr, "bases: %d %d \n", myBase, enemyBase);
		}

		mySumPods += zones[myBase].pods[myId];
		enemySumPods += zones[enemyBase].pods[enemyId];
		fprintf(stderr, "pods: %d %d \n", mySumPods, enemySumPods);

		//for (int i = 0; i < zoneCount; i++) {
		//	if (i > 10 && i < 70)
		//		fprintf(stderr, "%d | %d %d | %d %d %d %d\n", zones[i].id, zones[i].owner, zones[i].virtualOwner, zones[i].pods[0], zones[i].pods[1], zones[i].virtualPods[0], zones[i].virtualPods[1]);
		//}

	}
	void moving() {
		vector<Move> moves = computeMoves();

		if (moves.size() == 0){
			printf("WAIT\n");
		}
		else{
			for (size_t i = 0; i < moves.size(); i++){
				printf("%d %d %d ", moves[i].pods, moves[i].position, moves[i].next);
			}
			printf("\n");
		}
	}
	vector<Move> computeMoves() {
		vector<Move> moves(0);

		//checkNumberOfNewPods -- CheckDraw -- ActivateNoResourceZoneConquer
		//baseDefend();
		fighting();
		attackHigher(moves);
		defendOwn();
		defendNeighbour(moves);
		resource(moves);
		explore(moves);
		mission(moves);

		return moves;
	}
	// pods from fighting zones never move
	void fighting() {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				Zone* z = &zones[pod.zone];
				if (z->pods[0] > 0 && z->pods[1] > 0) {
					pod.hasJob = true;
				}
			}
		}
	}
	// pods with higher (or equal?) neighbour resource zones always move, move to the zone with minimum enemy pods and highest unknown neighbours
	void attackHigher(vector<Move> &moves) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				vector<ZoneData> zoneData;

				// pods with higher neighbour resource zones always move
				for (auto next : zones[pod.zone].neighbours) {
					if (next->virtualOwner != myId && next->platinum > pod.platinum) {
						zoneData.push_back(ZoneData(next, enemyId));
					}
				}

				// pick zone with highest resources and lowest enemy pods and highest unknown neighbours
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						if (l.platinum == r.platinum) {
							if (l.enemyPods == r.enemyPods) {
								return l.unkownNeighbourZones > r.unkownNeighbourZones;
							} else {
								return l.enemyPods < r.enemyPods;
							}
						} else {
							return l.platinum > r.platinum;
						}
					});

					//for (auto zd : zoneData) {
					//	fprintf(stderr, "%d %d %d %d %d \n", pod.zone, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					//}

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, zoneData[0].id));
					updateVirtualMove(1, pod.zone, zoneData[0].id);

					fprintf(stderr, "attackHigher %d %3d %3d \n", 1, pod.zone, zoneData[0].id);
				}
			}
		}
	}
	// dont move with pods on resource zones that can be conquered next turn and have no higher neighbour resource zone
	void defendOwn() {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				if (pod.platinum > 0) {
					Zone* z = &zones[pod.zone];
					for (auto next : z->neighbours) {
						if (next->virtualOwner == enemyId && next->virtualPods[enemyId] > 0) {
							z->virtualPods[myId] -= 1;
							next->virtualPods[enemyId] -= 1;

							pod.hasJob = true;

							fprintf(stderr, "defendOwn %3d \n", pod.zone);

							break;
						}
					}
				}
			}
		}
	}
	// move and defend own neighbour resource zones that can be conquered next turn?
	void defendNeighbour(vector<Move> &moves) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				vector<ZoneData> zoneData;

				for (auto next : zones[pod.zone].neighbours) {
					if (next->virtualOwner == myId && next->platinum > 0) {
						for (auto n : next->neighbours) {
							if (n->virtualOwner == enemyId && n->virtualPods[enemyId] > next->virtualPods[myId]) {
								zoneData.push_back(ZoneData(next, enemyId));
							}
						}
					}
				}

				// pick zone with highest resources and highest unknown neighbours
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						if (l.platinum == r.platinum) {
							return l.unkownNeighbourZones > r.unkownNeighbourZones;
						} else {
							return l.platinum > r.platinum;
						}
					});

					//for (auto zd : zoneData) {
					//	fprintf(stderr, "%d %d %d %d %d \n", pod.zone, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					//}

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, zoneData[0].id));
					updateVirtualMove(1, pod.zone, zoneData[0].id);

					fprintf(stderr, "defendNeighbour %d %3d %3d \n", 1, pod.zone, zoneData[0].id);
				}
			}
		}
	}
	// move to neighbour zone with highest platinum not owned
	void resource(vector<Move> &moves) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				vector<ZoneData> zoneData;

				for (auto next : zones[pod.zone].neighbours) {
					if (next->virtualOwner != myId && next->platinum > 0) {
						zoneData.push_back(ZoneData(next, enemyId));
					}
				}

				// pick zone with highest resources and highest unknown neighbours
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						if (l.platinum == r.platinum) {
							return l.unkownNeighbourZones > r.unkownNeighbourZones;
						} else {
							return l.platinum > r.platinum;
						}
					});

					//for (auto zd : zoneData) {
					//	fprintf(stderr, "%d %d %d %d %d \n", pod.zone, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					//}

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, zoneData[0].id));
					updateVirtualMove(1, pod.zone, zoneData[0].id);

					fprintf(stderr, "platinum %d %3d %3d \n", 1, pod.zone, zoneData[0].id);
				}
			}
		}
	}
	// move to neighbour zone with most unexplored neighbour zones
	void explore(vector<Move> &moves) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				vector<ZoneData> zoneData;

				for (auto next : zones[pod.zone].neighbours) {
					if (next->getUnkownNeighbourZones()) {
						zoneData.push_back(ZoneData(next, enemyId));
					}
				}

				// pick zone with highest unknown neighbours
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						return l.unkownNeighbourZones > r.unkownNeighbourZones;
					});

					//for (auto zd : zoneData) {
					//	fprintf(stderr, "%d %d %d %d %d \n", pod.zone, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					//}

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, zoneData[0].id));
					updateVirtualMove(1, pod.zone, zoneData[0].id);

					fprintf(stderr, "explore %d %3d %3d \n", 1, pod.zone, zoneData[0].id);
				}
			}
		}
	}
	void mission(vector<Move> &moves) {
		if (turn == 0) {
			vector<pair<int, int>> z(0);
			z.push_back(pair<int, int>(myBase, 1000));
			for (auto move : moves) {
				int steps = zones[move.next].paths[enemyBase].size() - 1;
				if (steps < z[0].second) {
					z.clear();
					z.push_back(pair<int, int>(move.next, steps));
				} else if (steps == z[0].second) {
					z.push_back(pair<int, int>(move.next, steps));
				}
			}

			for (auto &p : z) {
				p.second = 0;
			}

			int podsBase = zones[myBase].virtualPods[myId];
			for (int n = 0; n < podsBase; ) {
				for (size_t i = 0; i < z.size() && n < podsBase; i++, n++) {
					z[i].second += 1;
				}
			}

			for (auto &p : z) {
				moves.push_back(Move(p.second, myBase, p.first));
				updateVirtualMove(p.second, myBase, p.first);
				fprintf(stderr, "mission %d %3d %3d \n", p.second, myBase, p.first);
			}
		} else {
			vector<ZoneData> borders = findBorderZones(&zones[enemyBase], myId);


			/* 
			sort(pods.begin(), pods.end(), [](const Pod& l, const Pod& r) {
				if (l.distanceToBase == r.distanceToBase) {
					return l.platinum > r.platinum;
				} else {
					return l.distanceToBase < r.distanceToBase;
				}
			});
			*/
			localResourceSearch(moves, 3);
			localEnemyUnexploredSearch(moves, 3, borders);

			

			globalBorderSearch(moves, borders);

			for (auto &pod : pods) {
				if (!pod.hasJob) {
					Zone* z = &zones[pod.zone];

					Zone* next = z->paths[enemyBase][z->paths[enemyBase].size() - 2];

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, next->id));
					updateVirtualMove(1, pod.zone, next->id);

					fprintf(stderr, "mission %d %3d %3d \n", 1, pod.zone, next->id);
				}
			}

		}
	}

	// search within a local area ('steps' steps from pod zone) for resources 
	void localResourceSearchOld(vector<Move> &moves, int steps) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				vector<ZoneData> zoneData;
				for (int i = 0; i < zoneCount; i++) {
					int d = distanceToAll[pod.zone][i];
					if (d > 0 && d <= steps) {
						if (zones[i].virtualOwner != myId && zones[i].platinum > 0) {
							zoneData.push_back(ZoneData(zones[i], enemyId, d));
						}
					}
				}

				// pick zone with lowest resources to distance ratio and lowest enemy pods and highest unknown neighbours
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						int a = l.platinum / l.dist;
						int b = r.platinum / r.dist;
						if (a == b) {
							if (l.enemyPods == r.enemyPods) {
								return l.unkownNeighbourZones > r.unkownNeighbourZones;
							} else {
								return l.enemyPods < r.enemyPods;
							}
						} else {
							return a < b;
						}
					});

					for (auto zd : zoneData) {
						fprintf(stderr, "%d %d %d %d %d \n", pod.zone, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					}

					Zone* next = zones[pod.zone].paths[zoneData[0].id][zones[pod.zone].paths[enemyBase].size() - 2];

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, next->id));
					updateVirtualMove(1, pod.zone, next->id);

					fprintf(stderr, "localResourceSearch %d %3d %3d \n", 1, pod.zone, next->id);
				}
			}
		}
	}

	// search within a local area ('steps' steps from resource zone) for pods 
	void localResourceSearch(vector<Move> &moves, int steps) {
		for (int i = 0; i < zoneCount; i++) {
			// relevant resource zone
			if (zones[i].virtualOwner != myId && zones[i].platinum > 0) {
				vector<ZoneData> zoneData;

				// pod lookup
				for (int j = 0; j < zoneCount; j++) {
					int d = distanceToAll[i][j];
					if (d > 0 && d <= steps) {
						if (zones[j].pods[myId]>0) {
							for (auto &pod : pods) {
								if (pod.zone == j && !pod.hasJob) {
									zoneData.push_back(ZoneData(zones[j], enemyId, d));
								}
							}
						}
					}
				}
				if (zoneData.size() > 0) {
					sort(zoneData.begin(), zoneData.end(), [](const ZoneData& l, const ZoneData& r) {
						return r.dist < r.dist;
					});

					for (auto zd : zoneData) {
						fprintf(stderr, "%d %d %d %d %d \n", zones[i].id, zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
					}

					Zone* next = zones[zoneData[0].id].paths[zones[i].id][zones[zoneData[0].id].paths[zones[i].id].size() - 2];

					for (auto &pod : pods) {
						if (pod.zone == zoneData[0].id && !pod.hasJob) {
							pod.hasJob = true;
							break;
						}
					}

					moves.push_back(Move(1, zoneData[0].id, next->id));
					updateVirtualMove(1, zoneData[0].id, next->id);

					fprintf(stderr, "localResourceSearch %d %3d %3d \n", 1, zoneData[0].id, next->id);
				}
			}
		}
	}

	// pods on border zones search within a local area ('steps' steps from pod zone) for unexplored or enemy zones 
	void localEnemyUnexploredSearch(vector<Move> &moves, int steps, vector<ZoneData> borders) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				bool isBorderPod = false;

				for (auto border : borders) {
					if (pod.zone == border.id) {
						isBorderPod = true;
					}
				}

				if (isBorderPod) {
					vector<LocalSearchData> zoneData;

					for (auto next : zones[pod.zone].neighbours) {
						int found = 0;

						for (int i = 0; i < zoneCount; i++) {
							int d = distanceToAll[next->id][i];
							if (d > 0 && d <= (steps-1)) {
								if (!zones[i].virtualIsExplored || zones[i].virtualOwner == enemyId) {
									found += 1;
								}
							}
						}

						if (found > 0) {
							zoneData.push_back(LocalSearchData(pod.zone, next->id, distanceToAll[next->id][enemyBase],next->virtualPods[myId],found));
						}
					}

					if (zoneData.size() > 0) {
						sort(zoneData.begin(), zoneData.end(), [](const LocalSearchData& l, const LocalSearchData& r) {
							if (l.nextOwnPods > 0 && r.nextOwnPods > 0) {
								if (l.numOfDest == r.numOfDest) {
									return l.nextdistToEnemyBase < r.nextdistToEnemyBase;
								} else {
									return l.numOfDest > r.numOfDest;
								}
							} else {
								return l.nextOwnPods < r.nextOwnPods;
							}
						});

						for (auto zd : zoneData) {
							fprintf(stderr, "%d %d %d %d %d\n", zd.id, zd.next, zd.nextOwnPods, zd.numOfDest, zd.nextdistToEnemyBase);
						}

						pod.hasJob = true;
						moves.push_back(Move(1, zoneData[0].id, zoneData[0].next));
						updateVirtualMove(1, zoneData[0].id, zoneData[0].next);

						fprintf(stderr, "localEnemyUnexploredSearch %d %3d %3d \n", 1, zoneData[0].id, zoneData[0].next);
					}
				}
			}
		}
	}

	vector<ZoneData> findBorderZones(Zone* start, int id) {
		queue<Zone*> q;
		q.push(start);
		unordered_map<int, Zone*> came_from;
		came_from[start->id] = start;
		vector<ZoneData> zoneData;

		for (; !q.empty();) {
			Zone* zone = q.front();
			q.pop();

			//if (zone->virtualOwner == id) {
			if (zone->owner == id) {
				zoneData.push_back(ZoneData(zone, enemyId));
			} else {
				for (auto next : zone->neighbours) {
					if (!came_from.count(next->id)) {
						q.push(next);
						came_from[next->id] = zone;
					}
				}
			}
		}

		for (auto zd : zoneData) {
			fprintf(stderr, "border %d %d %d %d \n", zd.id, zd.platinum, zd.enemyPods, zd.unkownNeighbourZones);
		}

		return zoneData;
	}

	// auch innenliegende Felder die angrenzend zu gegnerischen Felder sind mit einbeziehen
	// Frage was ist mit unexplored Felder die weder Border noch innerhalb der local searches liegen
	void globalBorderSearchOld(vector<Move> &moves, vector<ZoneData> borders) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				bool isBorderPod = false;

				for (auto border : borders) {
					if (pod.zone == border.id) {
						isBorderPod = true;
					}
				}

				if (!isBorderPod) {
					vector<ZoneData> minBorder;

					for (auto next : zones[pod.zone].neighbours) {
						vector<ZoneData> minBorderNext;

						for (auto border : borders) {
							int d = distanceToAll[next->id][border.id];
							minBorderNext.push_back(ZoneData(next, enemyId, d));
						}

						sort(minBorderNext.begin(), minBorderNext.end(), [](const ZoneData& l, const ZoneData& r) {
							return l.dist < r.dist;
						});

						minBorder.push_back(minBorderNext[0]);
					}


					sort(minBorder.begin(), minBorder.end(), [](const ZoneData& l, const ZoneData& r) {
						return l.dist < r.dist;
					});

					int minDist = minBorder[0].dist;

					vector<ZoneData> temp;

					for (auto mb : minBorder) {
						if (mb.dist == minDist) {
							temp.push_back(mb);
						}
					}

					for (auto t : temp) {
						fprintf(stderr, "%d %d\n", t.id, t.dist);
					}

					int r = rand()%temp.size();

					pod.hasJob = true;
					moves.push_back(Move(1, pod.zone, temp[r].id));
					updateVirtualMove(1, pod.zone, temp[r].id);

					fprintf(stderr, "globalBorderSearch %d %d \n", r, temp.size());
					fprintf(stderr, "globalBorderSearch %d %3d %3d \n", 1, pod.zone, temp[r].id);
				}
			}
		}
	}

	int getPodsZoneToBase(int zone) {
		int num = 0;

		vector<Zone*> path = zones[zone].paths[myBase];
		for (auto z : path) {
			num += z->virtualPods[myId] - z->virtualPods[enemyId];		
		}

		return num;
	}
	int getNext(int start, int destination) {
		return zones[start].paths[destination][zones[start].paths[destination].size()-2]->id;
	}
	bool pathContainsBase(int start, int destination) {
		vector<Zone*> path = zones[start].paths[destination];
		path.erase(path.begin(),path.begin()+1);
		path.erase(path.end()-1,path.end());
		for (auto z : path) {
			if (z->id == myBase) {
				return true;
			}
		}

		return false;
	}

	void globalBorderSearch(vector<Move> &moves, vector<ZoneData> borders) {
		for (auto &pod : pods) {
			if (!pod.hasJob) {
				bool isBorderPod = false;

				for (auto border : borders) {
					if (pod.zone == border.id) {
						isBorderPod = true;
					}
				}

				if (!isBorderPod) {
					vector<GlobalSearchData> zoneData;
					for (auto border : borders) {
						if (!pathContainsBase(pod.zone, border.id)) {
							int p = getPodsZoneToBase(border.id);
							zoneData.push_back(GlobalSearchData(pod.zone, getNext(pod.zone, border.id), border.id, p));
						}
					}

					if (zoneData.size() > 0) {
						sort(zoneData.begin(), zoneData.end(), [](const GlobalSearchData& l, const GlobalSearchData& r) {
							return l.pods < r.pods;
						});

						int oldDist = distanceToAll[pod.zone][myBase];
						int newDist = distanceToAll[zoneData[0].next][myBase];

						if (newDist >= oldDist) {
							pod.hasJob = true;
							moves.push_back(Move(1, pod.zone, zoneData[0].next));
							updateVirtualMove(1, pod.zone, zoneData[0].next);

							fprintf(stderr, "globalBorderSearch %d %3d %3d \n", 1, pod.zone, zoneData[0].next);
						}
					}
				}
			}
		}
	}

	void buying() {
		printf("WAIT\n");
	}

	Game() : turn(0), playerCount(2), mySumPods(0), enemySumPods(0) {
		startTime = clock();
		loadInitData();
		endTime = clock() - startTime;
		interval = endTime / (double)CLOCKS_PER_SEC;
		fprintf(stderr, "init clock cycles: %f seconds elapsed: %f \n", endTime, interval);
	}
	~Game() {
		delete[] zones;
		delete[] distanceToAll;
	}

	void play() {
		while (1) {
			loadRoundData();

			startTime = clock();

			moving();

			endTime = clock() - startTime;
			interval = endTime / (double)CLOCKS_PER_SEC;
			fprintf(stderr, "move clock cycles: %f seconds elapsed: %f \n", endTime, interval);

			buying();

			turn++;
		}
	};

};

int main() {
	srand((unsigned)time(NULL));

	Game game;
	game.play();
}