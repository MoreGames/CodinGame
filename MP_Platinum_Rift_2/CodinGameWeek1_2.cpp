#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <functional>
#include <ctime>
#include <map>

using namespace std;

#define MAX_PLAYERS 4
#define MAX_PLATINUM 6

double startTime, endTime;
double interval;


/********************************************************************
 ********************************************************************
 ** C++ and STL class implementation of the Hungarian algorithm by David Schwarz, 2012
 **
 **
 ** O(n^3) implementation derived from libhungarian by Cyrill Stachniss, 2004
 **
 **
 ** Solving the Minimum Assignment Problem using the 
 ** Hungarian Method.
 **
 ** ** This file may be freely copied and distributed! **
 **
 **
 ** This file is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied 
 ** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 ** PURPOSE.  
 **
 ********************************************************************
 ********************************************************************/
#define verbose (0)
typedef enum {
	HUNGARIAN_MODE_MINIMIZE_COST,
	HUNGARIAN_MODE_MAXIMIZE_UTIL,
} MODE;
typedef enum {
	HUNGARIAN_NOT_ASSIGNED,
	HUNGARIAN_ASSIGNED,
} ASSIGN;
class Hungarian {

public:
	/** This method initialize the hungarian_problem structure and init 
	 *  the  cost matrices (missing lines or columns are filled with 0).
	 *  It returns the size of the quadratic(!) assignment matrix. **/

	Hungarian();
	Hungarian(const vector<vector<int>>&, int, int, MODE);

	int init(const vector<vector<int>>& input_matrix, 
			   int rows, 
			   int cols, 
			   MODE mode);

	/** This method computes the optimal assignment. **/
	bool solve();

	/** Accessor for the cost **/
	int cost() const;

	/** Reference accessor for assignment **/
	vector<vector<int>> assignment();

	/** Print the computed optimal assignment. **/
	void print_assignment();

	/** Print the cost matrix. **/
	void print_cost();

	/** Print cost matrix and assignment matrix. **/
	void print_status();

protected:
	bool check_solution(const vector<int>& row_dec, const vector<int>& col_inc, const vector<int>& col_vertex);
	bool assign_solution(const vector<int>& row_dec, const vector<int>& col_inc, const vector<int>& col_vertex);

private:

	int m_cost;
	int m_rows;
	int m_cols;
	vector<vector<int>> m_costmatrix;
	vector<vector<int>> m_assignment;   
};
Hungarian::Hungarian()
{
	//much ado about nothing
	m_rows = 1;
	m_cols = 1;
	m_cost = 0;

	m_costmatrix.resize(m_rows, vector<int>(m_cols,0));
	m_assignment.resize(m_rows, vector<int>(m_cols,0));
}
Hungarian::Hungarian(const vector<vector<int>>& input_matrix, int rows, int cols, MODE mode)
{
  int i,j, org_cols, org_rows;
  int max_cost;
  max_cost = 0;
  
  org_cols = cols;
  org_rows = rows;

  // is the matrix square? 
  // if no, expand with 0-cols / 0-cols

  if(rows!=cols)
  {
	  rows = std::max(cols, rows);
	  cols = rows;
  }
  
  m_rows = rows;
  m_cols = cols;

  m_costmatrix.resize(rows, vector<int>(cols,0));
  m_assignment.resize(rows, vector<int>(cols,0));

  for(i=0; i<m_rows; i++) 
  {
	  for(j=0; j<m_cols; j++) 
	  {
		  m_costmatrix[i][j] =  (i < org_rows && j < org_cols) ? input_matrix[i][j] : 0;
		  m_assignment[i][j] = 0;

		  if (max_cost < m_costmatrix[i][j])
		  {
			  max_cost = m_costmatrix[i][j];
		  }
	  }
  }


  if (mode == HUNGARIAN_MODE_MAXIMIZE_UTIL) 
  {
	  for(i=0; i<m_rows; i++) 
	  {
		  for(j=0; j<m_cols; j++) 
		  {
			  m_costmatrix[i][j] =  max_cost - m_costmatrix[i][j];
		  }
	  }
  }
  else if (mode == HUNGARIAN_MODE_MINIMIZE_COST) 
  {
    // nothing to do
  }
  else 
    fprintf(stderr,"%s: unknown mode. Mode was set to HUNGARIAN_MODE_MINIMIZE_COST !\n", __FUNCTION__);
}
void hungarian_print_matrix(const vector<vector<int>>& C, int rows, int cols) 
{
	int i,j;
	fprintf(stderr , "\n");
	for(i=0; i<rows; i++) 
	{
		fprintf(stderr, " [");
		for(j=0; j<cols; j++) 
		{
		fprintf(stderr, "%5d ",C[i][j]);
		}
		fprintf(stderr, "]\n");
	}
	fprintf(stderr, "\n");
}
void Hungarian::print_assignment() {
  hungarian_print_matrix(m_assignment, m_rows, m_cols) ;
}
void Hungarian::print_cost() {
  hungarian_print_matrix(m_costmatrix, m_rows, m_cols) ;
}
void Hungarian::print_status() 
{
  
  fprintf(stderr,"cost:\n");
  print_cost();

  fprintf(stderr,"assignment:\n");
  print_assignment();
  
}
int Hungarian::init(const vector<vector<int>>& input_matrix, int rows, int cols, MODE mode) 
{

  int i,j, org_cols, org_rows;
  int max_cost;
  max_cost = 0;
  
  org_cols = cols;
  org_rows = rows;

  // is the number of cols  not equal to number of rows ? 
  // if yes, expand with 0-cols / 0-cols
  rows = std::max(cols, rows);
  cols = rows;
  
  m_rows = rows;
  m_cols = cols;

  m_costmatrix.resize(rows, vector<int>(cols,0));
  m_assignment.resize(rows, vector<int>(cols,0));

  for(i=0; i<m_rows; i++) 
  {
    for(j=0; j<m_cols; j++) 
	{
      m_costmatrix[i][j] =  (i < org_rows && j < org_cols) ? input_matrix[i][j] : 0;
      m_assignment[i][j] = 0;

      if (max_cost < m_costmatrix[i][j])
	   max_cost = m_costmatrix[i][j];
    }
  }


  if (mode == HUNGARIAN_MODE_MAXIMIZE_UTIL) {
    for(i=0; i<m_rows; i++) {
      for(j=0; j<m_cols; j++) {
	m_costmatrix[i][j] =  max_cost - m_costmatrix[i][j];
      }
    }
  }
  else if (mode == HUNGARIAN_MODE_MINIMIZE_COST) {
    // nothing to do
  }
  else 
    fprintf(stderr,"%s: unknown mode. Mode was set to HUNGARIAN_MODE_MINIMIZE_COST !\n", __FUNCTION__);
  
  return rows;
}
bool Hungarian::check_solution(const vector<int>& row_dec, const vector<int>& col_inc, const vector<int>& col_vertex)
{
	int k, l, m, n;

	m = m_rows;
	n = m_cols;
	// Begin doublecheck the solution 23
	for (k=0;k<m;k++)
		for (l=0;l<n;l++)
			if (m_costmatrix[k][l]<row_dec[k]-col_inc[l])
			return false;

	for (k=0;k<m;k++)
	{
		l=col_vertex[k];
		if (l<0 || m_costmatrix[k][l]!=row_dec[k]-col_inc[l])
			return false;
	}
	k=0;
	
	for (l=0;l<n;l++)
	{
		if (col_inc[l])
		{
			k++;
		}
	}
	if (k>m)
	{
		return false;
	}

	//everything checks out, then
	return true;
  // End doublecheck the solution 23
}
bool Hungarian::assign_solution(const vector<int>& row_dec,const vector<int>&  col_inc, const vector<int>&  col_vertex)
{
	  // End Hungarian algorithm 18
	int i, j, k, l, m, n;

	m = m_rows;
	n = m_cols;

	for (i=0;i<m;++i)
	{
		m_assignment[i][col_vertex[i]]=HUNGARIAN_ASSIGNED;
		/*TRACE("%d - %d\n", i, col_vertex[i]);*/
	}
	for (k=0;k<m;++k)
	{
		for (l=0;l<n;++l)
		{
		/*TRACE("%d ",m_costmatrix[k][l]-row_dec[k]+col_inc[l]);*/
			m_costmatrix[k][l]=m_costmatrix[k][l]-row_dec[k]+col_inc[l];
		}
		/*TRACE("\n");*/
	}
	for (i=0;i<m;i++)
	{
		m_cost+=row_dec[i];
	}
	for (i=0;i<n;i++)
	{
		m_cost-=col_inc[i];
	}
	if (verbose)
		fprintf(stderr, "Cost is %d\n",m_cost);

	return true;
}
bool Hungarian::solve()
{
	int i, j, m, n, k, l, s, t, q, unmatched, cost;

	m = m_rows;
	n = m_cols;

	int INF = std::numeric_limits<int>::max();

	//vertex alternating paths,
	vector<int> col_vertex(m), row_vertex(n), unchosen_row(m), parent_row(n),
				row_dec(m),  col_inc(n),  slack_row(m),    slack(n);

	cost=0;

	for (i=0;i<m_rows;i++) 
	{
		col_vertex[i]=0;
		unchosen_row[i]=0;
		row_dec[i]=0;
		slack_row[i]=0;
	}

	for (j=0;j<m_cols;j++) 
	{
		row_vertex[j]=0;
		parent_row[j] = 0;
		col_inc[j]=0;
		slack[j]=0;
	}

	//Double check assignment matrix is 0
	m_assignment.assign(m, vector<int>(n, HUNGARIAN_NOT_ASSIGNED));

  // Begin subtract column minima in order to start with lots of zeroes 12
	if (verbose)
	{
		fprintf(stderr, "Using heuristic\n");
	}

	for (l=0;l<n;l++)
	{
		s = m_costmatrix[0][l];

		for (k=1;k<m;k++) 
		{
			if (m_costmatrix[k][l] < s)
			{
				s=m_costmatrix[k][l];
			}
			cost += s;
		}

		if (s!=0)
		{
			for (k=0;k<m;k++)
			{
				m_costmatrix[k][l]-=s;
			}
		}

		//pre-initialize state 16
		row_vertex[l]= -1;
		parent_row[l]= -1;
		col_inc[l]=0;
		slack[l]=INF;
	}
  // End subtract column minima in order to start with lots of zeroes 12

  // Begin initial state 16
	t=0;

	for (k=0;k<m;k++)
	{
		bool row_done = false;
		s=m_costmatrix[k][0];

		for (l=0;l<n;l++)
		{

			if(l > 0)
			{
				if (m_costmatrix[k][l] < s)
				{
					s = m_costmatrix[k][l];
				}
				row_dec[k]=s;
			}

			if (s == m_costmatrix[k][l] && row_vertex[l]<0)
				{
					col_vertex[k]=l;
					row_vertex[l]=k;

					if (verbose)
					{
						fprintf(stderr, "matching col %d==row %d\n",l,k);
					}
					row_done = true;
					break;
				}
		}

		if(!row_done)
		{
			col_vertex[k]= -1;

			if (verbose)
			{
				fprintf(stderr, "node %d: unmatched row %d\n",t,k);
			}
		
			unchosen_row[t++]=k;
		}

	}
  // End initial state 16

	bool checked = false;

  // Begin Hungarian algorithm 18

	//is matching already complete?
	if (t == 0)
	{
		checked = check_solution(row_dec, col_inc, col_vertex);
		if (checked)
		{
			//finish assignment, wrap up and done.
			bool assign = assign_solution(row_dec, col_inc, col_vertex);
			return true;
		}
		else
		{
			if(verbose)
			{
				fprintf(stderr, "Could not solve. Error.\n");
			}
			bool assign = assign_solution(row_dec, col_inc, col_vertex);
			return false;
		}
	}

	unmatched=t;


	while (1)
	{
		if (verbose)
		{
			fprintf(stderr, "Matched %d rows.\n",m-t);
		}
		q=0;
		bool try_matching;
		while (1)
		{
			while (q<t)
			{
			// Begin explore node q of the forest 19
				
				k=unchosen_row[q];
				s=row_dec[k];
				for (l=0;l<n;l++)
				{
					if (slack[l])
					{
						int del;
						del=m_costmatrix[k][l]-s+col_inc[l];
						if (del<slack[l])
						{
							if (del==0)
							{
								if (row_vertex[l]<0)
								{
									goto breakthru;
								}
								slack[l]=0;
								parent_row[l]=k;
								if (verbose){
									fprintf(stderr, "node %d: row %d==col %d--row %d\n",
										t,row_vertex[l],l,k);}
								unchosen_row[t++]=row_vertex[l];
							}
							else
							{
								slack[l]=del;
								slack_row[l]=k;
							}
						}
					}
				}
			// End explore node q of the forest 19
				q++;	
			}
 
	  // Begin introduce a new zero into the matrix 21
		s=INF;
		for (l=0;l<n;l++)
		{
			if (slack[l] && slack[l]<s)
			{
				s=slack[l];
			}
		}
		for (q=0;q<t;q++)
		{
			row_dec[unchosen_row[q]]+=s;
		}
		for (l=0;l<n;l++)
		{
			//check slack
			if (slack[l])
			{
				slack[l]-=s;
				if (slack[l]==0)
				{
					// Begin look at a new zero 22
					k=slack_row[l];
					if (verbose)
					{
						fprintf(stderr, 
						"Decreasing uncovered elements by %d produces zero at [%d,%d]\n",
						s,k,l);
					}
					if (row_vertex[l]<0)
					{
						for (j=l+1;j<n;j++)
							if (slack[j]==0)
							{
								col_inc[j]+=s;
							}

						goto breakthru;
					}
					else
					{
						parent_row[l]=k;
						if (verbose)
							{ fprintf(stderr, "node %d: row %d==col %d--row %d\n",t,row_vertex[l],l,k);}
						unchosen_row[t++]=row_vertex[l];
						
					}
		// End look at a new zero 22
				}
			}
			else
			{
				col_inc[l]+=s;
			}
		}
	// End introduce a new zero into the matrix 21
	}

    breakthru:
      // Begin update the matching 20
		if (verbose)
		{
			fprintf(stderr, "Breakthrough at node %d of %d!\n",q,t);
		}
		while (1)
		{
			j=col_vertex[k];
			col_vertex[k]=l;
			row_vertex[l]=k;
			if (verbose)
			{
				fprintf(stderr, "rematching col %d==row %d\n",l,k);
			}
			if (j<0)
			{
				break;
			}
			k=parent_row[j];
			l=j;
		}
		// End update the matching 20
		if (--unmatched == 0)
		{
			checked = check_solution(row_dec, col_inc, col_vertex);
			if (checked)
			{
				//finish assignment, wrap up and done.
				bool assign = assign_solution(row_dec, col_inc, col_vertex);
				return true;
			}
			else
			{
				if(verbose)
				{
					fprintf(stderr, "Could not solve. Error.\n");
				}
				bool assign = assign_solution(row_dec, col_inc, col_vertex);
				return false;
			}
		}
		
		// Begin get ready for another stage 17
			t=0;
			for (l=0;l<n;l++)
			{
				parent_row[l]= -1;
				slack[l]=INF;
			}
			for (k=0;k<m;k++)
			{
				if (col_vertex[k]<0)
				{
					if (verbose)
					{ fprintf(stderr, "node %d: unmatched row %d\n",t,k);}
					unchosen_row[t++]=k;
				}
			}
		// End get ready for another stage 17
	}// back to while loop
}
//ACCESSORS
int Hungarian::cost() const
{
	return m_cost;
}
vector<vector<int>> Hungarian::assignment()
{
	return m_assignment;
}


int cmp0Desc(const void *pa, const void *pb){
	const int* a = (const int*)pa;
	const int* b = (const int*)pb;
	if (a[0]<b[0]) return +1;
	if (a[0]>b[0]) return -1;
	return 0;
}
bool sortReinforcements(const pair<int, int>& i, const pair<int, int>& j) {
	return (i.first > j.first);
}

class Move {
public:
	int position;
	int next;
	int target;
	int stepsToTarget;

	Move() : position(0), next(0), target(0), stepsToTarget(0) {
	}
	Move(int p, int n, int t, int s) : position(p), next(n), target(t), stepsToTarget(s) {
	}
};

class Zone {
private:

	bool checkIsEnemyBorder(int playerId) {
		if (isOwned) {
			for (size_t i = 0; i < neighbours.size(); i++) {
				Zone* next = neighbours[i];
				if (next->owner != -1 && next->owner != playerId){
					return true;
				}
			}
			return false;
		}
		return false;
	}
	vector<int> checkCanBeConquered(int playerId) {
		vector<int> conquer(4,0);
		for (size_t i = 0; i < neighbours.size(); i++) {
			Zone* next = neighbours[i];
			int numberOfPlayers = 0;
			int numberOfPods = 0;
			for (int i = 0; i < 4; i++) {
				numberOfPlayers += !!next->pods[i];
				numberOfPods += next->pods[i];
			}
			if (next->owner != -1 && next->owner != playerId && numberOfPlayers == 1 && numberOfPods>0) {
				conquer[next->owner] += next->pods[next->owner];
			}
		}
		return conquer;
	}
	bool checkIsInner(int playerId) {
		if (isOwned) {
			for (size_t i = 0; i < neighbours.size(); i++) {
				Zone* next = neighbours[i];
				if (next->owner != playerId){
					return false;
				}
			}
			return true;
		}
		return false;
	}

public:
	// fix
	int id;
	int platinum;
	vector<Zone*> neighbours;
	int value;
	// round info
	int owner; // the player who owns this zone (-1 otherwise)
	int pods[MAX_PLAYERS];
	// update info
	int numberOfPlayers;
	bool isFighting;
	bool isOwned;
	bool isEnemyBorder;
	vector<int> canBeConquered;
	bool isInner;
	vector<Zone*> closeEnemyPods; // enemy pods within 5 steps

	Zone() : platinum(0), neighbours(), value(0), owner(-1), pods(), numberOfPlayers(0), isFighting(false), isOwned(false), isEnemyBorder(false), canBeConquered(), isInner(false) {
	}

	bool operator==(const Zone &other) const {
		return (id == other.id);
	}
	void updateOwner() {
		int temp[8];

		for (int i = 0; i<4; i++){
			temp[i * 2] = pods[i];
			temp[i * 2 + 1] = i;
		}

		qsort((void*)temp, 4, 2 * sizeof(int), cmp0Desc);

		if (temp[0] == 0){
			;// nothing to do
		} else if (temp[0]>0 && temp[2] == 0){
			owner = temp[1];
		} else{ // fighting zone
			for (int j = 0; j<3; j++){
				for (int i = 0; i<4; i++){
					temp[i] = max(temp[i] - 1, 0);
				}

				qsort((void*)temp, 4, 2 * sizeof(int), cmp0Desc);

				if (temp[0] == 0){
					break;// nothing to do
				} else if (temp[0]>0 && temp[2] == 0){
					owner = temp[1];
					break;
				}
			}

			if (temp[0]>0 && temp[2]>0){
				owner = -1;
			}
		}
	}
	void update(int playerId, Zone* zones, double** distanceToAll, int zoneCount) {
		numberOfPlayers = 0;
		for (int i = 0; i < 4; i++){
			numberOfPlayers += !!pods[i];
		}
		if (numberOfPlayers > 1) 
			isFighting = true;
		else
			isFighting = false;
		if (owner == playerId)
			isOwned = true;
		else
			isOwned = false;
		isEnemyBorder = checkIsEnemyBorder(playerId);
		canBeConquered = checkCanBeConquered(playerId);
		isInner = checkIsInner(playerId);

		if (platinum > 0) {
			closeEnemyPods.clear();
			for (int i = 0; i < zoneCount; i++) {
				if (distanceToAll[id][i] < 6 && distanceToAll[id][i] > 0) {
					int num = 0;
					for (int j = 0; j < 4; j++) {
						if (j == playerId)
							continue;
						num += !!zones[i].pods[j];
					}
					if (num>0)
					closeEnemyPods.push_back(&zones[i]);
				}
			}
		}
	}
	// TODO gehört noch genau überlegt
	int getValue(int playerId) {
		int m = 0;
		for (size_t j = 0; j < neighbours.size(); j++) {
			Zone* next = (neighbours)[j];
			if (next->platinum > m && next->owner != playerId) {
				m = next->platinum;
			}
		}
		return platinum*10 + m;
	}
};

// http://www.redblobgames.com/pathfinding/a-star/introduction.html
unordered_map<int, Zone*> breathFirstSearch(Zone* graph, Zone* start, Zone* goal, char all) {
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
unordered_map<int, Zone*> dijkstra(Zone* graph, Zone* start, Zone* goal, int* costZone, char all) {
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
		} catch (...) {
			return vector<Zone*>();
		}
		path.push_back(current);
	}
	return path;
}

/*
class Continent {
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
*/
class World {
private:
	int linkCount;
	int numberOfResourceZones;
	int sumOfResources;
	double averageResourcePerZone;
	int resourceView[MAX_PLATINUM + 1];// number of zones with 0, 1, 2, 3, 4, 5 and 6 resources
	double** distanceToAll;
	double** distanceResourcesToAll;

	void computeDistanceResourcesToAll() {
		for (int i = 0; i < zoneCount; i++) {
			unordered_map<int, Zone*> came_from = breathFirstSearch(zonesOrigin, &zonesOrigin[i], NULL, 1);
			for (int j = 0; j < zoneCount; j++){
				vector<Zone*> path = reconstructPath(&zonesOrigin[i], &zonesOrigin[j], came_from);
				if (path.size() == 0){
					distanceToAll[i][j] = distanceResourcesToAll[i][j] = -1;
				} else if (path.size() == 1){
					distanceToAll[i][j] = distanceResourcesToAll[i][j] = 0;
				} else{
					distanceToAll[i][j] = path.size() - 1;
					distanceResourcesToAll[i][j] = (zonesOrigin[j].platinum + 0.2) / (double)(distanceToAll[i][j]);
				}
			}
		}

		//for (int i = 0; i < 15; i++) {
		//	for (int j = 0; j < 15; j++) {
		//		fprintf(stderr, "%4.2f ", distanceResourcesToAll[i][j]);
		//	}
		//	fprintf(stderr, "\n");
		//}
	}
	// only resource (not owned) or enemy zones
	vector<int> moveTargets(Zone* zones, int id) {
		vector<int> targets(0);
		for (int i = 0; i < zoneCount; i++) {
			Zone z = zones[i];
			if ((z.platinum > 0 && z.owner != id) || (z.owner != -1 && z.owner != id)) {
				int numberOfPods = 0;
				for (int j = 0; j < 4; j++) {
						if (j == id) continue;
						numberOfPods += z.pods[j];
				}
				for (int j = 0; j < numberOfPods+1; j++) {
					targets.push_back(z.id);
				}
			}
		}
		return targets;
	}
	vector<int> availablePods(Zone* zones, int id) {
		vector<int> pods(0);
		for (int i = 0; i < zoneCount && pods.size()<60; i++) {
			Zone z = zones[i];
			if (z.pods[id] > 0) {
				if (!z.isFighting) { // pods from fighting zones never move
					if (z.platinum == 0) { // pods from zones with no resources always move
						for (int j = 0; j < z.pods[id]; j++) {
								pods.push_back(z.id);
						}
					} else {
						// pods with higher neighbour resource zones always move
						vector<int> conquer = z.canBeConquered;
						int m(0);
						for (size_t i = 0; i < z.neighbours.size(); i++) {
							Zone* next = (z.neighbours)[i];
							if (next->platinum > m) {
								m = next->platinum;
							}
						}
						if (m > z.platinum) {
							for (int j = 0; j < z.pods[id]; j++) {
									pods.push_back(z.id);
							}
						} else {
							// zones can be conquered; neccessary amount of pods for defending wont move
							int p = max(z.pods[id] - *max_element(conquer.begin(), conquer.end()), 0); 
							for (int j = 0; j < p; j++) {
									pods.push_back(z.id);
							}
						}
					}
				}
			}
		}
		return pods;
	}
	vector<vector<int>> weightPodsTargets(Zone* zones, vector<int>& pods, vector<int>& targets) {
		vector<vector<int>> weights;
		int rows = pods.size();
		int cols = targets.size();
		int mcols = max((int)targets.size(), rows);
		weights.resize(rows, std::vector<int>(mcols, 0));

		for (int i = 0; i < rows; i++) {
			int j = 0;
			while(j < mcols) {
				for (int n = 0; n < cols && j < mcols; n++, j++) {
					weights[i][j] = (int)(zones[targets[n]].value / distanceToAll[pods[i]][targets[n]] * 100);
				}
			}

		}

		return weights;
	}
	vector<vector<int>> bestPodTargetCombination(Zone* zones, vector<int>& pods, vector<int>& targets) {
		vector<vector<int>> weights = weightPodsTargets(zones, pods, targets);

		Hungarian hungarian(weights, weights.size(), weights[0].size(), HUNGARIAN_MODE_MAXIMIZE_UTIL);

		/* some output */
		//fprintf(stderr, "cost-matrix:");
		//hungarian.print_cost();

		/* solve the assignement problem */
		hungarian.solve();

		/* some output */
		//fprintf(stderr, "assignment:");
		//hungarian.print_assignment();

		return hungarian.assignment();
	}
	void setCost(int playerId, Zone &zone, int &costZone) {
		costZone = 100;
		if (zone.owner == -1) {
			costZone -= 1;
		} else if (zone.owner >= 0 && zone.owner != playerId) {
			costZone -= 2;
		}
		costZone -= zone.platinum;
	}
	void simulateMoves(int playerId, vector<Move> moves) {
		for (size_t i = 0; i < moves.size(); i++) {
			zonesSimulation[moves[i].next].pods[playerId] += 1;
			zonesSimulation[moves[i].position].pods[playerId] -= 1;
		}
	}

	unordered_map<int, int> reinforcementTargets(Zone* zones, int id) {
		unordered_map<int, int> targets(0);
		for (int i = 0; i < zoneCount; i++) {
			Zone z = zones[i];
			if ((z.platinum > 0 && z.owner != id) || (z.owner != -1 && z.owner != id)) {
				vector<int> conquer = z.canBeConquered;
				targets[z.id] = *max_element(conquer.begin(), conquer.end()) + 1;
			}
		}
		return targets;
	}
	double singleEnemyTarget(Zone* zones, int zone, double dist) {
		Zone z = zones[zone];
		int id = z.owner;
		bool isSingle(true);
		bool hasResourcesNext(false);
		bool isEnemyNext(false);

		for (size_t i = 0; i < z.neighbours.size(); i++) {
			Zone* next = (z.neighbours)[i];
			if (next->owner == id) {
				isSingle = false;
				break;
			}
			if (next->platinum > 3) {
				hasResourcesNext = true;
			}
			if (next->isOwned) {
				isEnemyNext = true;
			}
		}


		if (isSingle && hasResourcesNext && isEnemyNext) {
			return 3.5 / dist;
		} else {
			return 0;
		}
	}
	vector<pair<int, int>> orderReinforcementZones(int id, int numberOfReinforcements) {
		vector<pair<int, int>> reinforcementZones(0);

		// first own resource zones that can be conquered within the next turn
		for (int i = 0; i < zoneCount; i++) {
			Zone zo = zonesOrigin[i];
			Zone zs = zonesSimulation[i];
			if (zo.owner == id && zo.platinum > 0) {
				vector<int> conquer = zo.canBeConquered;
				int p = max(*max_element(conquer.begin(), conquer.end()) - zo.pods[id], 0);
				for (int j = 0; j < p; j++) {
					reinforcementZones.push_back(pair<int, int>(1000000 * zo.platinum * 2, zo.id));
				}
			}
		}

		// second enemy resource zones that can be conquered within the next turn
		for (int i = 0; i < zoneCount; i++) {
			Zone zo = zonesOrigin[i];
			Zone zs = zonesSimulation[i];
			if (zo.owner == id) {
				int m = 0;
				for (size_t j = 0; j < zs.neighbours.size(); j++) {
					Zone* next = (zs.neighbours)[j];
					if (next->platinum > m && next->owner != id) {
						m = next->platinum;
					}
				}
				reinforcementZones.push_back(pair<int, int>(1000000 * m, zo.id));
			}
		}


		if (reinforcementZones.size() >= numberOfReinforcements)
			return reinforcementZones;

		for (int i = 0; i < zoneCount; i++) {
			Zone zo = zonesOrigin[i];
			Zone zs = zonesSimulation[i];
			if (zo.owner == id && zo.platinum > 0) {
				for (size_t j = zs.pods[id]; j < zo.closeEnemyPods.size(); j++) {
					reinforcementZones.push_back(pair<int, int>((int)(1000000 * zo.platinum * 2 / distanceToAll[zo.id][zo.closeEnemyPods[j]->id]), zo.id));
				}
				//vector<int> conquer = z.canBeConquered;
				//int p = max(*max_element(conquer.begin(), conquer.end()) - z.pods[id], 0);
				//for (int j = 0; j < p; j++) {
				//	reinforcementZones.push_back(pair<int, int>(1000000 * z.platinum * 2, z.id));
				//}
			}
		}

		// second enemyBorderZones with minmal distance to resources
		unordered_map<int, int> targets = reinforcementTargets(zonesSimulation, id);
		for (int i = 0; i < zoneCount; i++) {
			Zone zo = zonesOrigin[i];
			Zone zs = zonesSimulation[i];
			if (zo.isOwned && !zo.isInner) {
				for (auto& y : targets) {
					reinforcementZones.push_back(pair<int, int>((int)(1000000 * zo.platinum / distanceToAll[zo.id][y.first]), zo.id));
				}
			}
		}
		return reinforcementZones;
	}
	
	vector<Move> computeMoves(int playerId, Zone* zones, vector<Move> enemyMoves) {
		vector<Move> moves(0);

		vector<int> targets = moveTargets(zones, playerId);
		if (targets.size() > 0) {
			vector<int> pods = availablePods(zones, playerId);

			if (pods.size() > 0) {

				vector<vector<int>> podTarget = bestPodTargetCombination(zones, pods, targets);

				int* costZone = new int[zoneCount];
				for (int i = 0; i < zoneCount; i++) {
					setCost(playerId, zonesOrigin[i], costZone[i]);
				}

				for (size_t i = 0; i < pods.size(); i++) {
					Zone *start, *next, *target;

					start = &zonesOrigin[pods[i]];
					for (size_t j = 0; j < targets.size(); j++) {
						if (podTarget[i][j] == 1) {
							target = &zonesOrigin[targets[j]];
						}
					}

					unordered_map<int, Zone*> came_from = dijkstra(zones, start, target, costZone, 0);
					vector<Zone*> path = reconstructPath(start, target, came_from);
					if (path.size() > 1) {
						next = path[path.size() - 2];

						fprintf(stderr, "%3d %3d %3d\n", start->id, target->id, next->id);

						moves.push_back(Move(start->id, next->id, target->id, path.size() - 1));

						// update costZone
						setCost(playerId, *next, costZone[next->id]);
					}
				}

				delete[] costZone;
			}
		}

		return moves;
	}

public:
	int zoneCount;
	Zone* zonesOrigin;
	Zone* zonesSimulation;
	vector<Move> moves;

	//Continent* continents;

	World(int z, int l) : zoneCount(z), linkCount(l), resourceView() {
		zonesOrigin = new Zone[zoneCount];
		zonesSimulation = new Zone[zoneCount];
		distanceResourcesToAll = new double*[zoneCount];
		distanceToAll = new double*[zoneCount];
		for (int i = 0; i < zoneCount; i++) {
			distanceResourcesToAll[i] = new double[zoneCount];
			distanceToAll[i] = new double[zoneCount];
		}


		for (int i = 0; i < zoneCount; i++) {
			int id, platinum;
			cin >> id >> platinum; cin.ignore();

			zonesOrigin[id].id = id;
			zonesOrigin[id].platinum = platinum;
			
			//fprintf(stderr, "%d %d\n", id, platinum);
		}

		for (int i = 0; i < linkCount; i++) {
			int zone1, zone2;
			cin >> zone1 >> zone2; cin.ignore();

			zonesOrigin[zone1].neighbours.push_back(&zonesOrigin[zone2]);
			zonesOrigin[zone2].neighbours.push_back(&zonesOrigin[zone1]);

			//fprintf(stderr, "%d %d\n", zone1, zone2);
		}

		for (int i = 0; i < zoneCount; i++) {
			Zone zone = zonesOrigin[i];
			int m = 0;
			for (size_t j = 0; j < zone.neighbours.size(); j++) {
				Zone* next = (zone.neighbours)[j];
				if (next->platinum > m) {
					m = next->platinum;
				}
			}
			zonesOrigin[i].value = zone.platinum*10 + m;
		}

		int num = 0, sum = 0;
		for (int i = 0; i<zoneCount; i++){
			int p = zonesOrigin[i].platinum;
			num += !!p;
			sum += p;
			resourceView[p] += 1;
		}
		numberOfResourceZones = num;
		sumOfResources = sum;
		averageResourcePerZone = num ? (double)sum / num : 0;

		computeDistanceResourcesToAll();
	}
	~World() {
		delete[] zonesOrigin;
		delete[] zonesSimulation;

		for (int i = 0; i < zoneCount; i++) {
			delete[] distanceResourcesToAll[i];
			delete[] distanceToAll[i];
		}
		delete[] distanceResourcesToAll;
		delete[] distanceToAll;
	}

	void loadWorldData() {
		for (int i = 0; i < zoneCount; i++) {
			int id, owner, p0, p1, p2, p3;
			cin >> id;
			cin >> owner >> p0 >> p1 >> p2 >> p3; cin.ignore();

			//if (owner != -1 || p0 != 0 || p1 != 0 || p2 != 0 || p3 != 0)
			//	fprintf(stderr, "%d %d %d %d %d %d \n", id, owner, p0, p1, p2, p3);

			zonesOrigin[id].owner = owner;
			zonesOrigin[id].pods[0] = p0;
			zonesOrigin[id].pods[1] = p1;
			zonesOrigin[id].pods[2] = p2;
			zonesOrigin[id].pods[3] = p3;
		}
	}
	void update(int playerCount, int myId) {
		for (int i = 0; i < zoneCount; i++) {
			zonesOrigin[i].update(myId, zonesOrigin, distanceToAll, zoneCount);
		}
	}
	void status() {
		fprintf(stderr, "number of resource zones  %d \n", numberOfResourceZones);
		fprintf(stderr, "sum of resources          %d \n", sumOfResources);
		fprintf(stderr, "average resource per zone %f \n", averageResourcePerZone);
		fprintf(stderr, "resource view \n");
		for (int i = 0; i<(MAX_PLATINUM + 1); i++){
			fprintf(stderr, "%8d %5hd \n", i, resourceView[i]);
		}
	}

	vector<Move> computeMoves(int myId) {
		for (int i = 0; i < zoneCount; i++) {
			zonesSimulation[i] = zonesOrigin[i];
		}

		// estimate enemy moves
		vector<Move> enemyMoves = computeMoves(myId == 0 ? 1 : 0, zonesOrigin, vector<Move>(0));
		simulateMoves(myId == 0 ? 1 : 0, enemyMoves);

		fprintf(stderr, "---------");

		// compute own moves
		vector<Move> ownMoves = computeMoves(myId, zonesOrigin, enemyMoves);
		simulateMoves(myId, ownMoves);

		moves.clear();

		for (int i = 0; i < zoneCount; i++) {
			zonesSimulation[i].updateOwner();
			zonesSimulation[i].update(myId, zonesSimulation, distanceToAll, zoneCount);
		}

		moves = ownMoves;

		return moves;
	}
	vector<pair<int, int>> computeReinforcements(int numberOfReinforcements, int myId) {
		if (numberOfReinforcements == 0) {
			return vector<pair<int, int>>(0);
		}
		vector<pair<int, int>> reinforcementZones = orderReinforcementZones(myId, numberOfReinforcements);

		sort(reinforcementZones.begin(), reinforcementZones.end(), sortReinforcements);

		fprintf(stderr, "reinforcement zones %d\n", reinforcementZones.size());
		if (reinforcementZones.size()>15) {
			fprintf(stderr, "zone ranking \n");
			for (vector<pair<int, int>>::iterator it = reinforcementZones.begin(); it != reinforcementZones.begin() + 15; it++) {
				fprintf(stderr, "%lld %3d\n", (*it).first, (*it).second);
			}
			fprintf(stderr, "numberOfReinforcements: %d \n", numberOfReinforcements);
		}

		if (reinforcementZones.size() == 0) {
			return vector<pair<int, int>>(0);
		}

		vector<pair<int, int>> reinforcements(min((int)reinforcementZones.size(), numberOfReinforcements));
		for (size_t i = 0; i < reinforcements.size(); i++) {
			reinforcements[i].first = 0;
			reinforcements[i].second = 0;
		}
		for (int n = 0; n < numberOfReinforcements; ){
			int i = 0; 
			for (vector<pair<int, int>>::iterator it = reinforcementZones.begin(); it != reinforcementZones.end() && n < numberOfReinforcements; it++, i++, n++) {
				reinforcements[i].first += 1;
				reinforcements[i].second = (*it).second;
			}
		}

		return reinforcements;
	}

};

class Player {
public:
	int id;
	int platinum;
	int income;
	int allPlatinum;
	int numberOfPods;
	int numberOfZones;
	int platinumNext; // estimated platinum available next turn

	Player() : platinum(0), platinumNext(0), income(0), allPlatinum(0), numberOfPods(0), numberOfZones(0) {
	}

	void update(World* world) {
		int pods = 0, in = 0, zones = 0;

		// player
		for (int i = 0; i < world->zoneCount; i++) {
			Zone z = world->zonesOrigin[i];
			if (z.owner == id)  {
				in += z.platinum;
				zones += 1;
			}
			pods += z.pods[id];
		}

		platinum = (platinum % 20) + in;
		income = in;
		allPlatinum += in;
		numberOfPods = pods;
		numberOfZones = zones;
	}
	void status() {
		fprintf(stderr, "%4d %8d %6d %7d %9d %10d \n", platinum, platinumNext, income, allPlatinum, numberOfPods, numberOfZones);
	}
	void computePlatinumNext(World* world) {
		int in = 0;

		for (int i = 0; i < world->zoneCount; i++) {
			Zone z = world->zonesSimulation[i];
			if (z.owner == id)  {
				in += z.platinum;
			}
		}

		platinumNext = (platinum % 20) + in;
	}
};

class Game {
private:
	int turn;
	int playerCount;
	int myId;

	World* world;
	Player* players;

	void loadInitData() {
		int zoneCount;
		int linkCount;
		cin >> playerCount >> myId >> zoneCount >> linkCount; cin.ignore();

		world = new World(zoneCount, linkCount);
		players = new Player[playerCount];
		for (int i = 0; i < playerCount; i++) {
			players[i].id = i;
		}
	}
	void loadRoundData() {
		int platinum;
		cin >> platinum; cin.ignore();
		//fprintf(stderr, "platinum: %d\n", platinum);
		world->loadWorldData();
	}
	void update() {
		world->update(playerCount, myId);
		for (int i = 0; i < playerCount; i++) {
			players[i].update(world);
		}
	}
	void status() {
		if (!turn) world->status();

		fprintf(stderr, "player overview: \n");
		fprintf(stderr, "player plat platNext income allPlat numOfPods numOfZones \n");
		for (int i = 0; i < playerCount; i++) {
			fprintf(stderr, "%6d ", players[i].id);
			players[i].status();
		}
	}
	void moving() {
		vector<Move> moves = world->computeMoves(myId);

		if (moves.size() == 0){
			printf("WAIT\n");
		} else{
			for (size_t i = 0; i < moves.size(); i++){
				printf("1 %d %d ", moves[i].position, moves[i].next);
			}
			printf("\n");
		}

		for (int i = 0; i < playerCount; i++) {
			players[i].computePlatinumNext(world);
		}
	}

	void buying() {
		int numberOfReinforcements = players[myId].numberOfPods>50 ? 0 : (players[myId].platinum / 20);
		vector<pair<int, int>> reinforcements = world->computeReinforcements(numberOfReinforcements, myId);

		if (reinforcements.size() == 0){
			printf("WAIT\n");
		} else{
			for (size_t i = 0; i < reinforcements.size(); i++){
				printf("%d %d ", reinforcements[i].first, reinforcements[i].second);
			}
			printf("\n");
		}
	}

public:
	Game() : turn(0) {
		loadInitData();
	}
	~Game() {
		delete world;
		delete[] players;
	}

	void play() {
		while (1) {
			loadRoundData();
			update();

			status();

			startTime = clock();

			moving();

			endTime = clock() - startTime;
			interval = endTime / (double)CLOCKS_PER_SEC;
			fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", endTime, interval);

			startTime = clock();

			buying();

			endTime = clock() - startTime;
			interval = endTime / (double)CLOCKS_PER_SEC;
			fprintf(stderr, "clock cycles: %f seconds elapsed: %f \n", endTime, interval);

			turn++;
		}
	};

};

int main() {
	Game game;
	game.play();
}