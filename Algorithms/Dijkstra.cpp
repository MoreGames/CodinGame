#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <utility>
#include <ctime>
#include <functional>

using namespace std;

class Zone {
public:
	int id;
	int cost;
	vector<Zone*> neighbours;

	Zone() : neighbours() {
	}

	bool operator==(const Zone &other) const {
		return (id == other.id);
	}
};

// http://www.redblobgames.com/pathfinding/a-star/introduction.html
unordered_map<int, Zone*> dijkstra(Zone* graph, Zone* start, Zone* goal, char all) {
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
			int new_cost = cost_so_far[zone->id] + next->cost;
			if (!cost_so_far.count(next->id) || new_cost < cost_so_far[next->id]) {
				cost_so_far[next->id] = new_cost;
				q.emplace(make_pair(new_cost, next));
				came_from[next->id] = zone;
			}
		}
	}
	return came_from;
}

vector<Zone*> reconstructPath(Zone* start, Zone* end, const unordered_map<int, Zone*>& came_from) {
	vector<Zone*> path;
	Zone* current = end;
	path.push_back(current);
	while (!(*current == *start)) {
		try {
			current = came_from.at(current->id);
		} catch (...) {
			return vector<Zone*>();
		}
		path.push_back(current);
	}
	return path;
}

int main(void) {

	double start, end;
	double interval;

	int zoneCount = 5;

	Zone* zones = new Zone[zoneCount];
	for (int i = 0; i < zoneCount; i++) {
		zones[i].id = i;
		zones[i].cost = 1;
	}
	zones[2].cost = 5;

	zones[0].neighbours.push_back(&zones[1]);
	zones[1].neighbours.push_back(&zones[0]); zones[1].neighbours.push_back(&zones[2]); zones[1].neighbours.push_back(&zones[3]);
	zones[2].neighbours.push_back(&zones[1]); zones[2].neighbours.push_back(&zones[4]);
	zones[3].neighbours.push_back(&zones[1]); zones[3].neighbours.push_back(&zones[4]);
	zones[4].neighbours.push_back(&zones[2]); zones[4].neighbours.push_back(&zones[3]);

	// pathfinding with weights
	start = clock();

	unordered_map<int, Zone*> came_from;
	for (int i = 0; i < 1000; i++)
		came_from = dijkstra(zones, &zones[4], &zones[0], 1);

	vector<Zone*> path = reconstructPath(&zones[4], &zones[0], came_from);

	end = clock() - start;
	interval = end / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", end, interval);
	fprintf(stderr, "path \n");
	for (vector<Zone*>::reverse_iterator rit = path.rbegin(); rit != path.rend(); ++rit) {
		fprintf(stderr, "next zone towards %3d from %3d: %3d \n", 0, 4, (*rit)->id);
	}
}
