#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <ctime>
#include <cstring>
#include <iomanip>

using namespace std;

double startTime, endTime;
double interval;

#define R 9
#define C 9

// board - cell numbering
// | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 |
// | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 |
// ...

int originGraph[R * C * 4] = { 0 }; // row wise numbered, neighbours orientation: left, top, right, bottom, -1 no neighbour cell
int originWallPositions[(R - 1) * (C - 1) * 2] = { 0 }; // 1 legal wall position, 0 otherwise

void setOrigin() {
	for (int i = 0; i < R * C * 4; i++) {
		originGraph[i] = -1;
	}

	for (int i = 0; i < C; i++) { // columns
		for (int j = 0; j < R; j++) { // rows
			int n = j * C + i;

			if (i != 0) {
				originGraph[n * 4 + 0] = j * C + i - 1;
			}
			if (j != 0) {
				originGraph[n * 4 + 1] = (j - 1) * C + i;
			}
			if (i != C - 1) {
				originGraph[n * 4 + 2] = j * C + i + 1;
			}
			if (j != R - 1) {
				originGraph[n * 4 + 3] = (j + 1) * C + i;
			}
		}
	}

	for (int i = 0; i < (R - 1) * (C - 1) * 2; i++) {
		originWallPositions[i] = 1;
	}
}
/*
class Wall {
public:
int pos;
int orientation; // 0 horizontal, 1 vertical
};
class WalkMove {
public:
int move; // cell number
WalkMove(int m) : move(m) {}
};
*/
class Move {
public:
	int playerId; // wall move ... -1
	int move; // cell number, index of WallPositions array
	double value;
	vector<int> val;

	Move(int p, int m) : playerId(p), move(m) {
	}
};
class Player {
public:
	int position;
	int wallsLeft;
	vector<int> targets;
};
class Board {
public:
	void updateGraphAdd(int x, int y, int dir) {
		int n = y * C + x;

		if (dir == 0) {
			graph[n * 4 + 1] = graph[(n + 1) * 4 + 1] = -1;
			graph[(n - C) * 4 + 3] = graph[(n - C + 1) * 4 + 3] = -1;
		} else if (dir == 1) {
			graph[n * 4 + 0] = graph[(n - 1) * 4 + 2] = -1;
			graph[(n + C) * 4 + 0] = graph[(n + C - 1) * 4 + 2] = -1;
		}
	}
	void setWallPositionAdd(int x, int y, int o) {
		wallPositions[convertWallMoveToNumber(x, y, o)] = 0;
	}
	void updateWallPositionsAdd(int x, int y, int o) {
		setWallPositionAdd(x, y, o);
		if (o == 0) {
			if ((x + 1) < (C - 1)) setWallPositionAdd(x + 1, y, o);
			if ((x - 1) >= 0) setWallPositionAdd(x - 1, y, o);
			setWallPositionAdd(x + 1, y - 1, 1);
		} else if (o == 1) {
			if ((y - 1) >= 0) setWallPositionAdd(x, y - 1, o);
			if ((y + 1) < (R - 1)) setWallPositionAdd(x, y + 1, o);
			setWallPositionAdd(x - 1, y + 1, 0);
		}
	}

	void updateGraphRemove(int x, int y, int dir) {
		int n = y * C + x;

		if (dir == 0) {
			graph[n * 4 + 1] = originGraph[n * 4 + 1];
			graph[(n + 1) * 4 + 1] = originGraph[(n + 1) * 4 + 1];
			graph[(n - C) * 4 + 3] = originGraph[(n - C) * 4 + 3];
			graph[(n - C + 1) * 4 + 3] = originGraph[(n - C + 1) * 4 + 3];
		} else if (dir == 1) {
			graph[n * 4 + 0] = originGraph[n * 4 + 0];
			graph[(n - 1) * 4 + 2] = originGraph[(n - 1) * 4 + 2];
			graph[(n + C) * 4 + 0] = originGraph[(n + C) * 4 + 0];
			graph[(n + C - 1) * 4 + 2] = originGraph[(n + C - 1) * 4 + 2];
		}
	}
	/*void setWallPositionRemove(int x, int y, int o) {
	wallPositions[convertWallMoveToNumber(x, y, o)] = 1;
	}*/
	/*void updateWallPositionsRemove(int x, int y, int o) {
	setWallPositionRemove(x, y, o);
	if (o == 0) {
	if ((x + 1) < (C - 1)) setWallPositionRemove(x + 1, y, o);
	if ((x - 1) >= 0) setWallPositionRemove(x - 1, y, o);
	setWallPositionRemove(x + 1, y - 1, 1);
	} else if (o == 1) {
	if ((y - 1) >= 0) setWallPositionRemove(x, y - 1, o);
	if ((y + 1) < (R - 1)) setWallPositionRemove(x, y + 1, o);
	setWallPositionRemove(x - 1, y + 1, 0);
	}
	}*/
	void dfs(int from, int* cameFrom) {
		int q[C*R];
		for (int i = 0; i < C*R; i++) {
			cameFrom[i] = -1;
		}

		int ind = 0;
		q[ind] = from;
		ind += 1;
		cameFrom[from] = from;
		for (; ind > 0;) {
			ind -= 1;
			int cell = q[ind];
			for (int i = 0; i < 4; i++) {
				int next = graph[cell * 4 + i];
				if (next != -1 && cameFrom[next] == -1) {
					q[ind] = next;
					ind += 1;
					cameFrom[next] = cell;
				}
			}
		}
	}

	void bfs(int from, int* cameFrom) {
		for (int i = 0; i < C*R; i++) {
			cameFrom[i] = -1;
		}

		queue<int> q;
		q.push(from);
		cameFrom[from] = from;
		for (; !q.empty();) {
			int cell = q.front();
			q.pop();
			for (int i = 0; i < 4; i++) {
				int next = graph[cell * 4 + i];
				if (next != -1 && cameFrom[next] == -1) {
					q.push(next);
					cameFrom[next] = cell;
				}
			}
		}
	}
	int shortestPath(int from, vector<int> targets) {
		int cameFrom[C*R] = { 0 };
		bfs(from, cameFrom);

		int minD = 0;// (C - 1) - from%C;
		int dist = R*C - 1,
			cell = -1;
		for (size_t i = 0; i < targets.size(); i++) {
			int current = targets[i];
			if (cameFrom[current] != -1) {
				int d;
				for (d = 1; cameFrom[current] != from; d++){
					current = cameFrom[current];
				}

				//cerr << d << " " << i << " " << current << endl;

				if (d <= minD) {
					cell = current;
					dist = d;
					break;
				} else if (d < dist) {
					cell = current;
					dist = d;
				}
			}
		}

		//printCameFrom(cameFrom);
		//cerr << "shortest path from " << from << " to target(s), next cell: " << cell << " (distance: " << dist << ")" << endl;

		return cell;
	}
	vector<int> minDistanceToGoal() { // minimum distance to targets for each player
		vector<int> ret(players.size());
		int cameFrom[C*R] = { 0 };

		for (size_t i = 0; i < players.size(); i++) {
			int from = players[i].position;

			bfs(from, cameFrom);

			int minDist = 0;
			if (i == 0) {
				minDist = (C - 1) - from%C;
			} else if (i == 1) {
				minDist = from%C;
			} else if (i == 2) {
				minDist = (R - 1) - from / C;
			}

			int dist = R*C - 1;
			for (auto current : players[i].targets) {
				if (current == from) {
					dist = 0;
					break;
				} else if (cameFrom[current] != -1) {
					int d;
					for (d = 1; cameFrom[current] != from; d++){
						current = cameFrom[current];
					}

					if (d <= minDist) {
						dist = d;
						break;
					} else if (d < dist) {
						dist = d;
					}
				}
			}
			ret[i] = dist;
		}

		return ret;
	}

	bool wallBlocking(int wall) {
		int x, y, o;
		if (wall < (C - 1)*(R - 1)) {
			x = wall % (C - 1);
			y = wall / (R - 1) + 1;
			o = 0;
		} else {
			wall -= (C - 1)*(R - 1);
			y = wall % (R - 1);
			x = wall / (C - 1) + 1;
			o = 1;
		}

		updateGraphAdd(x, y, o);
		//printBoard();
		//printWallPositions();

		for (auto& p : players) {
			int from = p.position;
			bool ret = true;

			int cameFrom[C*R] = { 0 };
			dfs(from, cameFrom);

			//printCameFrom(cameFrom);

			for (auto t : p.targets) {
				if (cameFrom[t] != -1) {
					ret = false;
					break;
				}
			}

			if (ret) {
				updateGraphRemove(x, y, o);
				return ret;
			}
		}
		updateGraphRemove(x, y, o);

		return false;
	}

	vector<Move> walkMoves(int playerId) {
		vector<Move> moves;
		int from = players[playerId].position;
		for (int i = 0; i < 4; i++) {
			if (graph[from * 4 + i] != -1) moves.push_back(Move(playerId, graph[from * 4 + i]));
		}
		return moves;
	}
	vector<int> wallPositionsArea(int pos, int d) {
		vector<int> walls;
		int x = pos % C;
		int y = pos / R;

		// H
		for (int i = max(0, x - d); i <= min(C - 2, x + d); i++) {
			for (int j = max(1, y - d); j <= min(R - 1, y + d); j++) {
				walls.push_back(convertWallMoveToNumber(i, j, 0));
			}
		}

		// V
		for (int i = max(1, x - d); i <= min(C - 1, x + d); i++) {
			for (int j = max(0, y - d); j <= min(R - 2, y + d); j++) {
				walls.push_back(convertWallMoveToNumber(i, j, 1));
			}
		}

		return walls;
	}
	vector<Move> wallMoves() {
		vector<Move> moves;
		for (auto& p : players) {
			vector<int> walls = wallPositionsArea(p.position, 2);

			for (auto i : walls) {
				if (wallPositions[i] && !wallBlocking(i)) moves.push_back(Move(-1, i));
			}
		}

		sort(moves.begin(), moves.end(), [](const Move& l, const Move& r) {
			return l.move > r.move;
		});
		auto last = unique(moves.begin(), moves.end(), [](const Move& l, const Move& r) {
			return (l.move == r.move) && (l.playerId == r.playerId);
		});
		moves.erase(last, moves.end());

		//int n = (R - 1) * (C - 1) * 2;
		//for (int i = 0; i < n; i++) {
		//	if (wallPositions[i] && !wallBlocking(i)) moves.push_back(Move(-1, i));
		//	//if (wallPositions[i]) moves.push_back(Move(-1, i));
		//}
		return moves;
	}

public:
	int turn;
	int myId;
	int* graph;
	int* wallPositions;
	vector<Player> players;

	Board(int t, vector<Player> p, int m) : turn(t), players(p), myId(m) {
		graph = new int[R * C * 4];
		wallPositions = new int[(R - 1) * (C - 1) * 2];

		memcpy(graph, originGraph, sizeof(int) * R * C * 4);
		memcpy(wallPositions, originWallPositions, sizeof(int) * (R - 1) * (C - 1) * 2);
	}
	Board(const Board& other) : turn(other.turn), players(other.players), myId(other.myId) {
		graph = new int[R * C * 4];
		wallPositions = new int[(R - 1) * (C - 1) * 2];

		memcpy(graph, other.graph, sizeof(int) * R * C * 4);
		memcpy(wallPositions, other.wallPositions, sizeof(int) * (R - 1) * (C - 1) * 2);
	}

	string convertNumberToWalkMove(int from, int to) {
		if (graph[from * 4 + 0] == to) {
			return "LEFT";
		} else if (graph[from * 4 + 1] == to) {
			return "UP";
		} else if (graph[from * 4 + 2] == to) {
			return "RIGHT";
		} else {
			return "DOWN";
		}
	}
	string convertNumberToWallMove(int n) {
		if (n < (C - 1)*(R - 1)) {
			int a = n % (C - 1);
			int b = n / (R - 1) + 1;
			return (to_string(a) + " " + to_string(b) + " H");
		} else {
			n -= (C - 1)*(R - 1);
			int a = n % (R - 1);
			int b = n / (C - 1) + 1;
			return (to_string(b) + " " + to_string(a) + " V");
		}
	}
	int convertWallMoveToNumber(int x, int y, int o) {
		if (o == 0) {
			return (y - 1)*(C - 1) + x;
		} else {
			return (x - 1)*(R - 1) + y + (R - 1)*(C - 1);
		}
	}

	void printGraph() {
		cerr << "graph: " << endl;
		fprintf(stderr, "x y ix |  0  1  2  3\n");
		fprintf(stderr, "--------------------\n");
		for (int i = 0; i < R * C; i++) {
			fprintf(stderr, "%d %d %2d | %2d %2d %2d %2d\n", i%C, i / R, i, graph[i * 4 + 0], graph[i * 4 + 1], graph[i * 4 + 2], graph[i * 4 + 3]);
		}
	}
	void printWallPositions() {
		int l = 0;
		for (int i = 0; i < (R - 1) * (C - 1) * 2; i++) {
			l += wallPositions[i];
		}

		cerr << "wall positions: " << endl;
		cerr << "    ";
		for (int j = 0; j < (C - 1); j++) {
			fprintf(stderr, "%d ", j);
		}
		fprintf(stderr, "\n-------------------\n");
		for (int i = 0; i < (R - 1); i++) {
			cerr << i << " | ";
			for (int j = 0; j < (C - 1); j++) {
				fprintf(stderr, "%d ", wallPositions[i * (C - 1) + j]);
			}
			cerr << endl;
		}
		fprintf(stderr, "-------------------\n");
		for (int j = 0; j < (C - 1); j++) {
			cerr << j << " | ";
			for (int i = 0; i < (R - 1); i++) {
				fprintf(stderr, "%d ", wallPositions[i * (C - 1) + j + (R - 1) * (C - 1)]);
			}
			cerr << endl;
		}
		cerr << "wall positions left: " << l << endl;
		fprintf(stderr, "    ");
		for (int i = 0; i < C; i++) {
			fprintf(stderr, "%d ", i);
		}
		fprintf(stderr, "\n");
		fprintf(stderr, "---------------------\n");
		for (int j = 0; j < R; j++) {
			fprintf(stderr, "%d | ", j);

			for (int i = 0; i < C; i++) {
				int pos = j*C + i;
				int c = 46;
				for (int p = 0; p < players.size(); p++) {
					if (players[p].position == pos) {
						c = 49 + p;
					}
				}
				fprintf(stderr, "%c", c);
				if (j < R - 1) {
					if (i < C - 1) {
						if (wallPositions[i * (R - 1) + j + (R - 1) * (C - 1)]) {
							fprintf(stderr, " ");
						} else {
							fprintf(stderr, "|");
						}
					}
				} else {
					if (i < C - 1) {
						fprintf(stderr, "X");
					}
				}
			}
			fprintf(stderr, "\n");
			if (j < R - 1) {
				fprintf(stderr, "  | ");
				for (int i = 0; i < C; i++) {
					if (i < C - 1) {
						if (wallPositions[j * (C - 1) + i]) {
							fprintf(stderr, " ");
						} else {
							fprintf(stderr, "-");
						}
					} else {
						fprintf(stderr, "X");
					}
					fprintf(stderr, " ");
				}
				fprintf(stderr, "\n");
			}

		}
	}
	void printBoard() {
		cerr << "board: " << endl;
		fprintf(stderr, "    ");
		for (int i = 0; i < C; i++) {
			fprintf(stderr, "%d ", i);
		}
		fprintf(stderr, "\n");
		fprintf(stderr, "  -------------------\n");
		for (int j = 0; j < R; j++) {
			fprintf(stderr, "%d | ", j);
			for (int i = 0; i < C; i++) {
				int pos = j*C + i;
				int c = 46;
				for (int p = 0; p < players.size(); p++) {
					if (players[p].position == pos) {
						c = 49 + p;
					}
				}
				fprintf(stderr, "%c", c);

				if (graph[pos * 4 + 2] != -1) {
					fprintf(stderr, " ");
				} else if (i == C - 1) {
					;
				} else {
					fprintf(stderr, "|");
				}
			}
			fprintf(stderr, "\n");
			if (j < R - 1) {
				fprintf(stderr, "  | ");
				for (int i = 0; i < C; i++) {
					int pos = j*C + i;
					if (graph[pos * 4 + 3] != -1) {
						fprintf(stderr, " ");
					} else if (i == C - 1) {
						;
					} else {
						fprintf(stderr, "-");
					}

					fprintf(stderr, " ");
				}
				fprintf(stderr, "\n");
			}
		}
	}
	void printCameFrom(int* cf) {
		cerr << "board with cameFrom: " << endl;
		fprintf(stderr, "    ");
		for (int i = 0; i < C; i++) {
			fprintf(stderr, "%d ", i);
		}
		fprintf(stderr, "\n");
		fprintf(stderr, "  -------------------\n");
		for (int j = 0; j < R; j++) {
			fprintf(stderr, "%d | ", j);
			for (int i = 0; i < C; i++) {
				int pos = j*C + i;
				int c = 46;
				for (int p = 0; p < players.size(); p++) {
					if (players[p].position == pos) {
						c = 49 + p;
					}
				}

				cf[pos];
				int from = cf[pos];
				int to = pos;
				if (from == to) {
					fprintf(stderr, ".");
				} else if (graph[from * 4 + 0] == to) {
					fprintf(stderr, ">");
				} else if (graph[from * 4 + 1] == to) {
					fprintf(stderr, "U");
				} else if (graph[from * 4 + 2] == to) {
					fprintf(stderr, "<");
				} else if (graph[from * 4 + 3] == to) {
					fprintf(stderr, "^");
				} else {
					fprintf(stderr, " ");
				}

				if (graph[pos * 4 + 2] != -1) {
					fprintf(stderr, " ");
				} else if (i == C - 1) {
					;
				} else {
					fprintf(stderr, "|");
				}
			}
			fprintf(stderr, "\n");
			if (j < R - 1) {
				fprintf(stderr, "  | ");
				for (int i = 0; i < C; i++) {
					int pos = j*C + i;
					if (graph[pos * 4 + 3] != -1) {
						fprintf(stderr, " ");
					} else if (i == C - 1) {
						;
					} else {
						fprintf(stderr, "-");
					}

					fprintf(stderr, " ");
				}
				fprintf(stderr, "\n");
			}
		}
	}

	void playerMove(int playerId, int to) {
		players[playerId].position = to;
	}
	void addWall(int x, int y, int o) {
		updateGraphAdd(x, y, o);
		updateWallPositionsAdd(x, y, o);
	}
	void addWall(int playerId, int x, int y, int o) {
		addWall(x, y, o);
		players[playerId].wallsLeft -= 1;
	}

	vector<Move> allMoves(int playerId) {
		int initWalls = players.size() == 2 ? 10 : 6;

		if (turn >= 4 && players[playerId].wallsLeft == initWalls) {
			return wallMoves();
		} else if (players[playerId].wallsLeft > 0) {
			vector<Move> moves(walkMoves(playerId));

			vector<Move> wm(wallMoves());
			for (auto& m : wm) {
				moves.push_back(m);
			}

			return moves;
		} else {
			return walkMoves(playerId);
		}
	}
	void makeMove(int playerId, Move move) {
		if (move.playerId == -1) {
			int x, y, o;
			if (move.move < (C - 1)*(R - 1)) {
				x = move.move % (C - 1);
				y = move.move / (R - 1) + 1;
				o = 0;
			} else {
				move.move -= (C - 1)*(R - 1);
				y = move.move % (R - 1);
				x = move.move / (C - 1) + 1;
				o = 1;
			}
			addWall(playerId, x, y, o);
		} else {
			playerMove(move.playerId, move.move);
		}
	}


	bool isGameOver() {
		for (auto& p : players) {
			for (auto t : p.targets) {
				if (t == p.position) {
					return true;
				}
			}
		}

		return false;
	}
	double boardValue(int playerId) {
		vector<int> dist = minDistanceToGoal();
		int sum = 0;
		for (auto i : dist) {
			sum += i;
		}


		for (size_t i = 0; i < dist.size(); i++) {
			if (i == playerId && dist[i] == 0) {
				return 1000;
			} else if (dist[i] == 0) {
				return -1000;
			}
		}
		/*
		int val = dist[playerId];

		dist.erase(dist.begin() + playerId);
		int minVal = 1000;
		for (size_t i = 0; i < dist.size(); i++) {
		if (dist[i] < minVal) {
		minVal = dist[i];
		}
		}

		return minVal - val;
		*/
		//return sum-2*dist[playerId] + players[playerId].wallsLeft;
		//return 20-dist[playerId] + 2*players[playerId].wallsLeft;
		return sum - 2 * dist[playerId];
		//return (81. - dist[playerId]) / 81.*0.75 + 0.25*players[playerId].wallsLeft / 10.;
	}
	vector<int> boardValue3P(int playerId) {
		vector<int> dist = minDistanceToGoal();
		for (size_t i = 0; i < dist.size(); i++) {
			if (i == playerId && dist[i] == 0) {
				dist[i] = 1000;
			} else if (dist[i] == 0) {
				dist[i] = -1000;
			} else {
				dist[i] = 81 - dist[i];
			}
		}

		return dist;
	}

	/*
	string nextMove() {
	fprintf(stderr, "turn: %2d\n", turn);
	//printGraph();
	//printWallPositions();
	//printBoard();
	int from = players[myId].position;
	int to = shortestPath(from, players[myId].targets);

	//removeWall(0, 8, 0);
	//printBoard();
	//to = shortestPath(players[myId].position, players[myId].targets);

	int wall = convertWallMoveToNumber(2, 7, 1);
	bool blocking = wallBlocking(wall);

	vector<Move> moves = wallMoves();

	for (auto move : moves) {
	cerr << move << " " << convertNumberToWallMove(move) << endl;
	}

	return convertNumberToWalkMove(from , to);
	}
	*/

	~Board() {
		delete[] graph;
		delete[] wallPositions;
	}
};

class Strategy2Player {
public:
	Board board;

	Strategy2Player(const Board& other) : board(other) {
		//board.printBoard();
	}

	string nextMove() {
		int from = board.players[board.myId].position;

		Move to = alphaBeta(2);

		if (to.playerId == -1) {
			return board.convertNumberToWallMove(to.move);
		} else {
			return board.convertNumberToWalkMove(from, to.move);
		}
	}

	Move alphaBeta(int depth) {
		int playerId = board.myId;
		double cur = board.boardValue(playerId);
		vector<Move> moves = board.allMoves(playerId);

		double alpha = -1000;
		double beta = 1000;
		Move maxMove(-1, 0);
		maxMove.value = -1000;
		for (auto& move : moves) {
			Board cb(board);
			cb.makeMove(playerId, move);
			//cb.printBoard();
			move.value = -negamax(cb, (playerId + 1) % cb.players.size(), depth - 1, -beta, -alpha);

			if (move.value > maxMove.value) {
				maxMove = move;
			}
			if (move.value > alpha) {
				alpha = move.value;
			}
			if (alpha >= beta) {
				//for (int i = 0; i < depth; i++) {
				//	cout << "  ";
				//}
				//if (move.playerId == -1) {
				//	cout << "<-- cutoff " << move.value << " " << board.convertNumberToWallMove(move.move) << endl;
				//} else {
				//	cout << "<-- cutoff " << move.value << " " << board.convertNumberToWalkMove(board.players[board.myId].position, move.move) << endl;
				//}

				return move;
			}

			//for (int i = 0; i < depth; i++) {
			//	cout << "  ";
			//}
			//if (move.playerId == -1) {
			//	cout << "<-- " << move.value << " " << board.convertNumberToWallMove(move.move) << endl;
			//} else {
			//	cout << "<-- " << move.value << " " << board.convertNumberToWalkMove(board.players[board.myId].position, move.move) << endl;
			//}
		}

		sort(moves.begin(), moves.end(), [playerId](const Move& l, const Move& r) {
			return l.value > r.value;
		});

		int from = board.players[board.myId].position;
		int next = board.shortestPath(from, board.players[board.myId].targets);
		Move shortMove(moves[0]);
		for (auto& move : moves) {
			if (move.playerId != -1 && move.move == next) {
				shortMove = move;
			}
		}

		return ((int)(shortMove.value * 1000) == (int)(moves[0].value * 1000)) ? shortMove : moves[0];
	}

	double negamax(Board cp, int playerId, int depth, double alpha, double beta) {
		if (board.isGameOver() || depth == 0) {
			return cp.boardValue(playerId);
		} else {
			double max = -100000;

			vector<Move> moves = cp.allMoves(playerId);
			for (auto& move : moves) {
				Board cb(cp);
				cb.makeMove(playerId, move);
				//cb.printBoard();
				double x = -negamax(cb, (playerId + 1) % cb.players.size(), depth - 1, -beta, -alpha);
				if (x > max) {
					max = x;
				}
				if (x > alpha) {
					alpha = x;
				}
				if (alpha >= beta) {
					//for (int i = 0; i < depth; i++) {
					//	cout << "  ";
					//}
					//if (move.playerId == -1) {
					//	cout << "<-- cutoff " << x << " " << board.convertNumberToWallMove(move.move) << endl;
					//} else {
					//	cout << "<-- cutoff " << x << " " << board.convertNumberToWalkMove(board.players[board.myId].position, move.move) << endl;
					//}

					return alpha;
				}


				//for (int i = 0; i < depth; i++) {
				//	cout << "  ";
				//}
				//if (move.playerId == -1) {
				//	cout << "<-- " << x << " " << cb.convertNumberToWallMove(move.move) << endl;
				//} else {
				//	cout << "<-- " << x << " " << cb.convertNumberToWalkMove(cp.players[playerId].position, move.move) << endl;
				//}
			}

			return max;
		}

		//playerId = (playerId + 1) % board.players.size();
	}

};

class Strategy3Player {
public:
	Board board;

	Strategy3Player(const Board& other) : board(other) {
		//board.printBoard();
	}

	string nextMove() {
		int from = board.players[board.myId].position;

		Move to = alphaBeta(3);

		if (to.playerId == -1) {
			return board.convertNumberToWallMove(to.move);
		} else {
			return board.convertNumberToWalkMove(from, to.move);
		}
	}

	Move alphaBeta(int depth) {
		int playerId = board.myId;
		double cur = board.boardValue(playerId);
		vector<Move> moves = board.allMoves(playerId);

		for (auto& move : moves) {
			Board cb(board);
			cb.makeMove(playerId, move);
			//cb.printBoard();
			move.val = negamax(cb, (playerId + 1) % cb.players.size(), depth - 1);

			//for (int i = 0; i < depth; i++) {
			//	cout << "  ";
			//}
			//if (move.playerId == -1) {
			//	cout << "<-- " << move.value << " " << board.convertNumberToWallMove(move.move) << endl;
			//} else {
			//	cout << "<-- " << move.value << " " << board.convertNumberToWalkMove(board.players[board.myId].position, move.move) << endl;
			//}
		}

		sort(moves.begin(), moves.end(), [playerId](const Move& l, const Move& r) {
			return l.val[playerId] > r.val[playerId];
		});

		int from = board.players[board.myId].position;
		int next = board.shortestPath(from, board.players[board.myId].targets);
		Move shortMove(moves[0]);
		for (auto& move : moves) {
			if (move.playerId != -1 && move.move == next) {
				shortMove = move;
			}
		}

		return ((int)(shortMove.val[board.myId] * 1000) == (int)(moves[0].val[board.myId] * 1000)) ? shortMove : moves[0];
	}

	vector<int> negamax(Board cp, int playerId, int depth) {
		if (board.isGameOver() || depth == 0) {
			return cp.boardValue3P(playerId);
		} else {
			double max = -100000;

			vector<Move> moves = cp.allMoves(playerId);
			for (auto& move : moves) {
				Board cb(cp);
				cb.makeMove(playerId, move);
				//cb.printBoard();
				move.val = negamax(cb, (playerId + 1) % cb.players.size(), depth - 1);

				//for (int i = 0; i < depth; i++) {
				//	cout << "  ";
				//}
				//if (move.playerId == -1) {
				//	cout << "<-- " << move.val[playerId] << " " << cb.convertNumberToWallMove(move.move) << endl;
				//} else {
				//	cout << "<-- " << move.val[playerId] << " " << cb.convertNumberToWalkMove(cp.players[playerId].position, move.move) << endl;
				//}
			}

			sort(moves.begin(), moves.end(), [playerId](const Move& l, const Move& r) {
				return l.val[playerId] > r.val[playerId];
			});

			return moves[0].val;
		}

		//playerId = (playerId + 1) % board.players.size();
	}

};

class Game {
public:
	int turn;
	int playerCount;
	int myId;
	vector<Player> players;

	void loadInitData() {
		int w, h;
		cin >> w >> h >> playerCount >> myId; cin.ignore();
		//cerr << w << " " << h << " " << playerCount << " " << myId << endl;

		for (int i = 0; i < playerCount; i++) {
			players.push_back(Player());
			if (i == 0) {
				for (int j = 0; j < h; j++) {
					players[i].targets.push_back(j * C + C - 1);
				}
			} else if (i == 1) {
				for (int j = 0; j < h; j++) {
					players[i].targets.push_back(j * C);
				}
			} else if (i == 2) {
				for (int j = 0; j < w; j++) {
					players[i].targets.push_back((R - 1) * C + j);
				}
			}
		}
	}
	void loadRoundData(Board& board) {
		for (int i = 0; i < playerCount; i++) {
			int x;
			int y;
			int wallsLeft;
			cin >> x >> y >> wallsLeft; cin.ignore();
			//cerr << x << " " << y << " " << wallsLeft << endl;

			board.players[i].position = y*C + x;
			board.players[i].wallsLeft = wallsLeft;
		}
		int wallCount;
		cin >> wallCount; cin.ignore();
		for (int i = 0; i < wallCount; i++) {
			int wallX, wallY;
			char wallOrientation; // wall orientation ('H' or 'V')
			cin >> wallX >> wallY >> wallOrientation; cin.ignore();
			//cerr << wallX << " " << wallY << " " << wallOrientation << endl;

			board.addWall(wallX, wallY, wallOrientation == 'H' ? 0 : 1);
		}
	}

	void action(Board& board) {
		string move;
		if (board.players[myId].wallsLeft == 0) {
			int from = board.players[myId].position;
			int to = board.shortestPath(from, board.players[myId].targets);
			move = board.convertNumberToWalkMove(from, to);
		} else {
			if (playerCount == 2) {
				board.printBoard();
				Strategy2Player strategy(board);
				move = strategy.nextMove();
			} else {
				board.printBoard();
				Strategy3Player strategy(board);
				move = strategy.nextMove();
			}
		}


/*
		if (turn == 0) {
			move = "1 0 V";
		} else if (turn == 1) {
			move = "1 2 V";
		} else if (turn == 2) {
			move = "1 4 V";
		} else if (turn == 3) {
			move = "1 6 V";
		} else if (turn == 4) {
			move = "0 8 H";
		} else {
			if (playerCount == 2) {

				board.printBoard();
				Strategy2Player strategy(board);
				move = strategy.nextMove();
			} else {


			}

		}
*/

		cout << move << endl;
	}


	Game() : turn(0) {
		loadInitData();
	}
	~Game() {
	}

	void play() {
		while (1) {
			Board board(turn, players, myId);

			loadRoundData(board);

			startTime = clock();

			action(board);

			endTime = clock() - startTime;
			interval = endTime / (double)CLOCKS_PER_SEC;
			fprintf(stderr, "action clock cycles: %f seconds elapsed: %f \n", endTime, interval);

			turn++;
		}
	};
};

int main() {
	setOrigin();

	Game game;
	game.play();
}