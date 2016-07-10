'use strict';

// constants

const debug = 0;

const bustersPerPlayer = parseInt(readline()); // the amount of busters you control
const ghostCount = parseInt(readline()); // the amount of ghosts on the map
const myTeamId = parseInt(readline()); // if this is 0, your base is on the top left of the map, if it is one, on the bottom right

const fieldWidth = 16001;
const fieldHeight = 9001;

let home = {x: 0, y: 0},
    ehome = {x: fieldWidth-1, y: fieldHeight-1};
if (myTeamId === 1) home = {x: fieldWidth-1, y: fieldHeight-1};
if (myTeamId === 1) ehome = {x: 0, y: 0};
 
const maxMove = 800;

const ghostBustMin = 1760;
const ghostBustMax = 900;
const ghostMove = 400;
const ghostRec = 2200;

const stunRange = 1760;

const rangeOfVision = 2200;

const releaseMin = 1600;

const PI = 3.14159265359;

// variables

let ghosts = new Map();
let busters = new Map();

let ghostsInVision = new Set();
let ghostsOutOfVision = new Set();
let ghostsCaptured = new Set();
let ownBusters = new Set();
let bustersInVision = new Set();

let availableGhosts = new Set();
let availableBusters = new Set();

let turn = 0;
let gridFactor = 10;



// classes

class Entity {
    constructor (id, x, y, state, value, turn) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.state = state; // for busters: 0=idle, 1=carrying a ghost, for ghosts: stamina
        this.value = value; // number of busters attempting to trap this ghost
        this.turn = turn; // last update at
    }
    
    get pos () {
        return {'x': this.x, 'y': this.y};
    }
  
    set pos (p) {
        this.x = p.x;
        this.y = p.y;
    }
}
class Ghost extends Entity {
    constructor (id, x, y, state, value, turn) {
        super(id, x, y, state, value, turn);
        
        // set later on
        this.xEst = x;
        this.yEst = y;
        
        // not used
        this.catchedByBuster = null; // Id
    }
    
    getBustersTrapping() {
        let own = 0,
            enemy = 0;
            
        for (let id of ownBusters) {
            let b = busters.get(id);
            if (b.state === 3 && b.value === this.id) {
                own += 1;
            }
        }
        
        enemy = this.value - own;
        
        return own-enemy;
        
        //return {own: own, enemy: enemy};
    }
    
    get posEst () {
        return {x: this.xEst, y: this.yEst};
    }
    
    isSearched() {
       for (let id of ownBusters) {
           let b = busters.get(id);
           if (b.mode === 'searching') {
               return true;
           }
       }
       return false; 
    }
    
    isTrappedByOwnBusterNoHelpNeeded () {
       for (let id of ownBusters) {
           let b = busters.get(id);
           if (b.mode === 'busting' && b.targetId === this.id) {
               if (this.getBustersTrapping() <= 0 && this.value !== 0) {
                   
                   //printErr('isTrappedByOwnBusterNoHelpNeeded: g '+this.id+' '+this.getBustersTrapping()+' '+this.value);
                   return false;
               }
               return true;
           }
       }
       return false;
    }
    
    isCatchedByOwnBuster () {
       for (let id of ownBusters) {
           let b = busters.get(id);
           if (b.mode === 'catching' && b.targetId === this.id) {
               return true;
           }
       }
       return false;
    }
    
    busterShouldInVision () {
    	for (let id of ownBusters) {
    		let b = busters.get(id);
    		let dist = distance(this.posEst, b.pos);
    		//printErr('busterShouldInVision g:'+this.id+' b:'+b.id+' dist:'+dist);
    		if (Math.round(dist) <= ghostRec-100) return true;
    	}
    	return false;
    }
    
    ownBustersWithinBustRange () {
        let oB = new Set();
    	for (let id of ownBusters) {
    		let b = busters.get(id);
    		let dist = distance(this.pos, b.pos);
    		if (dist < ghostBustMin && dist > ghostBustMax) oB.add(id);
    	}
    	return oB;
    }
    
    withinEnemyBaseRange(range) {
        let center = {x: fieldWidth-1, y: fieldHeight-1}
        if (myTeamId === 1) center = {x: 0, y: 0};
        let dist = distance(this.pos,center);
        if (dist < range) {
            return true;
        }
        return false;
    }
        
    
    estimatedNextPosition(bustersInVision) {
        // based on all current busters known, estimate the next position
        // 400 steps away from closest buster
        // find nearest buster within 2200
        let minDist = ghostRec;
        let minB = -1;
        for (let id of bustersInVision) {
            let b = busters.get(id);
            if (b.hasMoved()) {
                let dist = distance(b.pos, this.pos);
                if (dist > 0 && dist < minDist) {
                    minDist = dist;
                    minB = b;
                }
            }
        }
        
        // compute buster ghost vector
        if (minB !== -1) {
            let factor = (minDist+ghostMove)/minDist;
            
            let diffX = (this.x - minB.x) * factor;
            let diffY = (this.y - minB.y) * factor;
            
            this.xEst = minB.x + Math.round(diffX);
            this.yEst = minB.y + Math.round(diffY);
            
            // limit move within area
            if (this.xEst < 0) this.xEst = 0;
            if (this.xEst > (fieldWidth-1)) this.xEst = (fieldWidth-1);
            if (this.yEst < 0) this.yEst = 0;
            if (this.yEst > (fieldHeight-1)) this.yEst = (fieldHeight-1);
        } else {
            this.xEst = this.x;
            this.yEst = this.y;
        }
        
        //printErr('xEst:' + this.xEst + ' yEst:' + this.yEst);
    }
    
    toString() {
        return `g id:${this.id} x,y:${this.x},${this.y} xe,ye:${this.xEst},${this.yEst} state:${this.state} value:${this.value} turn:${this.turn}`;
    }
}
class Buster extends Entity {
    constructor (id, x, y, team, state, value, turn) {
        super(id, x, y, state, value, turn);
        this.team = team;   // the team id if it is a buster
        
        // possible modes:
        // exploring, catching, busting, returning, releasing, unknown
        /*
        mode = searching auf der Suche nach einem ghosts der in ghostsOutOfVision
        mode = stunning immer wenn ein anderer buster einen ghost hat
        mode = stunned
        */       
        this.mode = team===myTeamId?'exploring':'unknown';
        this.pathHistory = []; // path history of own busters
        
        // set later on
        this.targetId = null;
        this.targetType = null;
        this.stunTimer = 20;
    }
    
    hasMoved () {
        if (this.team === myTeamId) {
            let lastPos = this.pathHistory[this.pathHistory.length-1];
            if (typeof lastPos === 'undefined') lastPos = home;
            
            let diffX = lastPos.x - this.x,
                diffY = lastPos.y - this.y;
                
            if (diffX === 0 && diffY === 0) {
                return false
            }
            
            return true;
        } else {
            return true;
        }
    }
    
    enemyBusterInStunRange() {
       for (let id of bustersInVision) {
           let b = busters.get(id);
           if (!ownBusters.has(b.id) && b.state !== 2) {
               let dist = distance(this.pos, b.pos);
               if (dist < stunRange) {
                   
                    let possible = true;
                    for (let oId of ownBusters) {
                        let bb = busters.get(oId);
                        if (bb.mode === 'stunning' && bb.targetId === b.id) {
                            possible = false;
                        }
                    }
                   
                    if (possible) {
                        return b.id;
                    }
               }
            }
        }
        return -1;
    }
    
    stunAvailable() {
        return (this.stunTimer === 20);
    }
    
    updateHistory () {
        this.pathHistory.push({x:this.x, y:this.y});
        
        //let output = '';
        //for (let i = this.pathHistory.length-1, j = 9; j >= 0 && i >= 0; j--,i--) {
        //    output +=' x:'+this.pathHistory[i].x+' y:'+this.pathHistory[i].y;
        //}
        //printErr(output);
    }
    
    reset () {
    	this.mode = 'exploring';
    	this.targetId = null;
    	this.targetType = null;
    }
    
    distToHome() {
        return distance(this.pos, home);  
    }
    
    enemyBustersInVision() {
       for (let id of bustersInVision) {
           let b = busters.get(id);
           if (!ownBusters.has(b.id)) {
               let dist = distance(this.pos, b.pos);
               if (dist < rangeOfVision) {
                   return true;
               }
            }
        }
        return false;
    }
    
    ghostsInRange() { // sorted
        let g = [];
        for (let id of ghostsInVision) {
            let dist = distance(this.pos, ghosts.get(id).pos);
            if (dist < rangeOfVision+1)
                g.push({id:id, dist:dist});
        }
        g.sort((a,b) => a.dist-b.dist); // minimal distance first
        return g;
    }
    
    ghostsInBustRange() { // sorted
        let g = [];
        for (let id of ghostsInVision) {
            let ghost = ghosts.get(id);
            let dist = distance(ghost.pos, this.pos);
            if (dist < ghostBustMin && dist > ghostBustMax)
                g.push({id:id, dist:dist});
        }
        g.sort((a,b) => a.dist-b.dist); // minimal distance first
        return g;
    }
    
    getNearestFreeGhostsInVision() {
        let gd = [];
        for (let id of ghostsInVision) {
            let g = ghosts.get(id);
            if (g.catchedByBuster === null)
                gd.push({id:id,dist:distance(this.pos, g.posEst)});
        }
        gd.sort((a,b) => a.dist-b.dist);
        return gd[0];
    }
    
    nextMove () {
        // TODO
        if (this.mode === 'auto') {
            let move = step(this.pos, {x:8000, y:4500}, maxMove);
            return 'MOVE '+Math.round(move.x)+' '+Math.round(move.y);
        }
        
        if (this.mode === 'stunning') {
            this.stunTimer = 0;    
            return 'STUN '+this.targetId;
        }
        if (this.mode === 'stunned') return 'MOVE '+this.x+' '+this.y;
        if (this.mode === 'releasing') {
            ghostsCaptured.add(this.targetId);
            if (ghostsInVision.has(this.targetId)) ghostsInVision.delete(this.targetId);
            if (ghostsOutOfVision.has(this.targetId)) {
                ghostsOutOfVision.delete(this.targetId);
                printErr('nm releasing del:'+this.targetId);
            }
            return 'RELEASE';  
        }
        if (this.mode === 'busting') return 'BUST '+this.targetId;
        
        // TODO check RELEASE?
        if (this.mode === 'returning') {
            if(this.winningGhost()) printErr('returning winningGhost:'+this.winningGhost()+','+this.state);
            
            if (this.winningGhost() && this.state === 1) {
                return 'MOVE '+Math.round(8000)+' '+Math.round(home.y);
            }
                
            
            // release
            let dist = distance(this.pos, home);
            //printErr('dist:'+dist);
            if (dist < releaseMin) return 'RELEASE';
            // or move home
            dist = dist-releaseMin+2; // just for rounding issues
            
            let move;
            if (this.enemyBustersInVision()) {
                move = step(this.pos, home, maxMove);
            } else {
                move = step(this.pos, home, dist<maxMove?dist:maxMove);
            }
            //printErr('dist:'+dist);
            return 'MOVE '+Math.round(move.x)+' '+Math.round(move.y);
        }
        if ((this.mode === 'catching' || this.mode === 'searching') && !this.winningGhost()) { // only called if not within bust range, maybe to close to bust
            let id = this.targetId;
            let type = this.targetType;
            
            let move;
            if (type === 'Ghost') {
                let ghost = ghosts.get(this.targetId);
                let dist = distance(this.pos, ghost.posEst);
                
                if (dist === 0) {
                    move = step(this.pos, ehome, maxMove);
                } else {
                    dist -= (ghostBustMax+2);
                    move = step(this.pos, ghost.posEst, Math.min(dist, maxMove));
                }
            } else {
                let e = busters.get(id);
                let moveEst = step(e.pos, ehome, maxMove);
                move = step(this.pos, moveEst, maxMove);
            }
            
            return 'MOVE '+Math.round(move.x)+' '+Math.round(move.y);
        }
        
        
        let output;
        //if (this.mode === 'exploring') {
            //output = this.findBestExploringMove();
            //gameGrid.setVisitedPoints(this.pos, gameGrid.previewGrid);
        //}
        
        output = this.findBestExploringMove();
        gameGrid2.setVisitedPoints(this.pos, gameGrid2.previewGrid);
        
        //printErr('nextMove:'+output.x+','+output.y);
        

        
        return 'MOVE '+output.x+' '+output.y;
        
        
    }
    
    winningGhost() {
        if (ghostsCaptured.size === Math.floor(ghostCount/2)) {
            let winningGhostCaptured = false;
            for (let id of ownBusters) {
                let b = busters.get(id);
                if (b.state === 1) {
                    winningGhostCaptured = true;
                    break;
                }
            }
            if (winningGhostCaptured) print('winningGhost()');
            return winningGhostCaptured;
        }
        
        return false;
    }
    
    findBestExploringMove() {
        if (ghostsCaptured.size === Math.floor(ghostCount/2)) {
            let winningGhostCaptured = false;
            let winningBuster = -1;
            for (let id of ownBusters) {
                let b = busters.get(id);
                if (this.id !== b.id && b.state === 1) {
                    winningGhostCaptured = true;
                    winningBuster = id;
                    break;
                }
            }
            
            if (winningGhostCaptured) {
                let move = step(this.pos, busters.get(winningBuster).pos, maxMove);
                
                printErr('winningGhost!');
                
                return {x: Math.round(move.x), y: Math.round(move.y)};
            }
        }
        
        let moveDistance = [800, 1600, 3200, 4800, 6400, 8000];
        let intervals = [8, 18, 18, 36, 36, 72];
        for (let step = 0; step < 6; step++) {
            let points = getExplorePositions(this.pos, moveDistance[step], intervals[step]);
            
            let maxUnknown = 0;
            let maxPoint = -1;
            for (let p of points) {
                let unknown = unknownTerritoryEstimate(p);
                
                //printErr(this.id+' p:'+p.x+','+p.y+' unknown:'+unknown);
                
                if (unknown > maxUnknown) {
                    maxUnknown = unknown;
                    maxPoint = p;
                }
            }
            
            if (maxUnknown>0) {
                return {x: Math.round(maxPoint.x), y: Math.round(maxPoint.y)};
            }
        }
        
        // default case
        // wait in front of enemy base
        if (typeof output === 'undefined') {
            let temp = {x: fieldWidth-1, y: fieldHeight-1};
            if (myTeamId === 1) temp = {x: 0, y: 0};
            
            return {x: Math.abs(temp.x - stunRange), y: Math.abs(temp.y - stunRange)};
        }
        
        
 /*  
        
        
        let lastPos = this.pathHistory[this.pathHistory.length-1];
        if (typeof lastPos === 'undefined') lastPos = home;

        let diffX = lastPos.x - this.x,
            diffY = lastPos.y - this.y;
        let dist = distance(lastPos, this.pos);
        
        //printErr('diffX:'+diffX+' diffY:'+diffY+' dist:'+dist);
        
        let alpha = 0,
            start = 5.9341194567807, // 340 grad
            end = 6.6322511575785; // 380 grad
        if (dist !== 0) {
            alpha = Math.acos(diffX/dist);
                
            if (diffY > 0) alpha = 2*PI - alpha;
    		
    		let backFactor = 3; // dont move back, skip the moving area (-PI/backFactor, +PI/backFactor]), which is centered around the last move vector
            start = alpha - PI/backFactor;
            end = alpha + PI/backFactor;
        }
        
		//printErr('alpha:'+alpha/(2*PI)*360+' start:'+start/(2*PI)*360+' end:'+end/(2*PI)*360);
		
		let intervals = 8; // possible move points
		let intervalPoints = new Array(intervals+1);
		for (let i = 0; i <= intervals; i++) {
			let a = start - i * (2*PI-end+start) / intervals;
			let pos = {x: this.x + Math.cos(a)*maxMove, y: this.y - Math.sin(a)*maxMove};
			intervalPoints[i] = {pos: pos, area: monteCarloEstimate (pos)};
			
			//printErr('alpha:'+a/(2*PI)*360+' x:'+pos.x+' y:'+pos.y+' value:'+intervalPoints[i].area);
		}
		
		intervalPoints.sort(function(a, b) {
			return b.area - a.area;
		});
		
		//printErr('next move id:'+this.id+' x:'+intervalPoints[0].pos.x+' y:'+intervalPoints[0].pos.y+' dist: '+distance(intervalPoints[0].pos, this.pos));
		
		return {x: Math.round(intervalPoints[0].pos.x), y: Math.round(intervalPoints[0].pos.y)};
		*/
    }
    
    toString() {
        if (debug) if (this.mode === 'busting') printErr('dist:'+distance(this.pos, ghosts.get(this.targetId).pos));
        if (debug) if (this.mode === 'releasing') printErr('dist:'+distance(this.pos, home));
        return `b id:${this.id} x,y:${this.x},${this.y} mode:${this.mode} targetId:${this.targetId} stunTimer:${this.stunTimer} state:${this.state} value:${this.value} turn:${this.turn}`;
    }
}
/*
// at each turn the game grid will be updated and the preview game grid will be resetted
// to compute buster moves, the preview game grid should be used and modified
class GameGrid {
    constructor(orgWidth, orgHeight, gridFactor) {
        this.gridFactor = gridFactor;
        this.gridWidth = Math.floor(orgWidth / gridFactor) + 1;
        this.gridHeight = Math.floor(orgHeight / gridFactor) + 1;
        this.grid = new Array(this.gridWidth);
        for (let i = 0; i < this.gridWidth; i++) {
            this.grid[i] = new Array(this.gridHeight);
            for (let j = 0; j < this.gridHeight; j++) {
                this.grid[i][j] = 1;
            }
        }

        // preview for buster moves
        this.previewGrid = new Array(this.gridWidth);
        for (let i = 0; i < this.gridWidth; i++) {
            this.previewGrid[i] = new Array(this.gridHeight);
            for (let j = 0; j < this.gridHeight; j++) {
                this.previewGrid[i][j] = 1;
            }
        }
    }

    previewGridPoint(orgPos) {
        let gridPoint = this.findNearestGridPoint(orgPos);
        return this.previewGrid[gridPoint.x][gridPoint.y];
    }
    findNearestGridPoint(orgPos) {
        return { x: Math.round(orgPos.x / gridFactor), y: Math.round(orgPos.y / gridFactor) };
    }
    setVisitedPoints(posBuster, grid) {
        let center = gameGrid.findNearestGridPoint(posBuster);
        
        let p = getBoundingGridPoints(posBuster);
        let p1 = p.p1,
            p2 = p.p2;
        
        for (let i = p1.x; i <= p2.x; i++) {
            for (let j = p1.y; j <= p2.y; j++) {
                if (this.isWithinRangeOfVision({x:i , y:j}, center)) {
                    grid[i][j] = 0;
                }
            }
        }
    }
    updateVisitedPoints() {
        for (let id of ownBusters) {
            this.setVisitedPoints(busters.get(id).pos, this.grid);
        }

        // reset previewGrid
        for (let i = 0; i < this.gridWidth; i++) {
            for (let j = 0; j < this.gridHeight; j++) {
                this.previewGrid[i][j] = this.grid[i][j];
            }
        }
    }
    
    // returns whether a point is in range of vision
    isWithinRangeOfVision (point, center) { 
        let diffX = point.x-center.x;
        let diffY = point.y-center.y;
        return (diffX*diffX + diffY*diffY <= 48400);
    }
    
    getUnknownTerritoryPercentage() {
        let unknown = 0;
        let all = 0;
        for (let i = 0; i < this.gridWidth; i++) {
            for (let j = 0; j < this.gridHeight; j++) {
                all += 1;
                unknown += this.grid[i][j];
            }
        }
        return unknown/all;
    }
    
    stillUnknownTerritoryAroundHome() {
        let radius = 920;
        
        let p1, p2, center;
        if (myTeamId === 0) {// home 0,0
            p1 = home;
            p2 = {x:home.x+radius, y:Math.min(home.y+radius,900)};
            center = home;
        } else {
            p1 = {x:home.x/10-radius, y:Math.max(home.y/10-radius, 0)};
            p2 = {x:home.x/10, y:home.y/10};
            center = {x:home.x/10, y:home.y/10};
        }
         
        let unknown = 0;
        for (let i = p1.x; i <= p2.x; i++) {
            for (let j = p1.y; j <= p2.y; j++) {
                if (this.isWithinRangeOfVision({x:i , y:j}, center)) {
                    unknown += this.grid[i][j];
                }
            }
        }
        
        if (debug) printErr('p:'+p1.x+','+p1.y+' '+p2.x+','+p2.y+' center:'+center.x+','+center.y+' '+unknown);
        
        return (unknown>0);
    }
}
var gameGrid = new GameGrid(fieldWidth, fieldHeight, gridFactor);
*/
class GameGrid2 {
    constructor(orgWidth, orgHeight, gridFactor) {
        this.gridFactor = gridFactor;
        this.gridWidth = Math.floor(orgWidth / gridFactor) + 1;
        this.gridHeight = Math.floor(orgHeight / gridFactor) + 1;
        this.grid = new Array(this.gridWidth);
        
        let center = home;
        if (myTeamId === 1) center = {x:home.x/10, y:home.y/10};
        
        for (let i = 0; i < this.gridWidth; i++) {
            this.grid[i] = new Array(this.gridHeight);
            for (let j = 0; j < this.gridHeight; j++) {
                if (this.isWithinRange({x:i, y:j}, center, 1000)) {
                    this.grid[i][j] = 1;
                } else {
                    this.grid[i][j] = 0;
                }
            }
        }

        // preview for buster moves
        this.previewGrid = new Array(this.gridWidth);
        for (let i = 0; i < this.gridWidth; i++) {
            this.previewGrid[i] = new Array(this.gridHeight);
            for (let j = 0; j < this.gridHeight; j++) {
                this.previewGrid[i][j] = this.grid[i][j];
            }
        }
    }
    
    

    previewGridPoint(orgPos) {
        let gridPoint = this.findNearestGridPoint(orgPos);
        return this.previewGrid[gridPoint.x][gridPoint.y];
    }
    findNearestGridPoint(orgPos) {
        return { x: Math.round(orgPos.x / gridFactor), y: Math.round(orgPos.y / gridFactor) };
    }
    setVisitedPoints(posBuster, grid) {
        let center = this.findNearestGridPoint(posBuster);
        
        let p = getBoundingGridPoints(posBuster);
        let p1 = p.p1,
            p2 = p.p2;
        
        for (let i = p1.x; i <= p2.x; i++) {
            for (let j = p1.y; j <= p2.y; j++) {
                if (this.isWithinRange({x:i , y:j}, center, 220)) {
                    grid[i][j] = 0;
                }
            }
        }
    }
    updateVisitedPoints() {
        for (let id of ownBusters) {
            this.setVisitedPoints(busters.get(id).pos, this.grid);
        }

        // reset previewGrid
        for (let i = 0; i < this.gridWidth; i++) {
            for (let j = 0; j < this.gridHeight; j++) {
                this.previewGrid[i][j] = this.grid[i][j];
            }
        }
    }
    
    isWithinRange (point, center, range) { 
        let diffX = point.x-center.x;
        let diffY = point.y-center.y;
        return (diffX*diffX + diffY*diffY <= range*range);
    }
    
    getUnknownTerritoryPercentage() {
        let unknown = 0;
        let all = 0;
        for (let i = 0; i < this.gridWidth; i++) {
            for (let j = 0; j < this.gridHeight; j++) {
                all += 1;
                unknown += this.grid[i][j];
            }
        }
        
        //printErr('unknown:'+unknown);
        
        return unknown/all;
    }
}
var gameGrid2 = new GameGrid2(fieldWidth, fieldHeight, gridFactor);

// general functions

// not used
function willAlreadyBeCatchedByABuster (ghost) {
    for (let id of ownBusters) {
        let b = busters.get(id);
        if (b.targetId === ghost.id) return true;
    }  
    return false;
}
// not used - updateCatching
function getMinimalDistanceBusters(ghost, availableBusters) {
    let busts = [];
    for (let id of availableBusters) {
        let b = busters.get(id);
        if (b.mode === 'catching' || b.mode === 'exploring') {
            busts.push({id:id,dist:distance(b.pos, ghost.posEst)});
        }
    }
    busts.sort((a,b) => a.dist-b.dist);
    return busts[0];
}
// not used
function updateCatching () {
    let bustList = [];
    let availableBusters = new Set();
    let availableGhosts = new Set(ghostsInVision); // availableGhosts != ghostsInVision see busting
    for (let id of ghostsInVision) {
        if (ghosts.get(id).catchedByBuster === null) {
            availableGhosts.add(id);
        }
    }
    
    for (let id of ownBusters) {
        let b = busters.get(id);
        if (b.mode === 'catching' || b.mode === 'exploring') {
            bustList.push({id:id, numGhostsInRange: b.ghostsInRange()});
            availableBusters.add(id);
        }
    }
    bustList.sort((a,b) => b.numGhostsInRange.length-a.numGhostsInRange.length); // busters with more ghosts first
    
    for (let i = 0; i < bustList.length; i++) {
        let bust = bustList[i];
        let b = busters.get(bust.id);
        
        if (bust.numGhostsInRange.length > 0) {
            // pick closest ghost
            let g = ghosts.get(bust.numGhostsInRange[0].id);
  
            // reset TODO rethink connection ghost - buster
            if (b.targetId !== g.id && b.targetId !== null) {
                ghosts.get(b.targetId).catchedByBuster = null;
            }
            
            //new
            b.mode = 'catching';
            b.targetId = g.id;
            b.targetType = 'Ghost';
            g.catchedByBuster = b.id;
            
            // remove from available lists
            availableBusters.delete(b.id);
            availableGhosts.delete(g.id);
        } else {
    
            //newnewnew
/*
            // pick nearest free ghost
            let gd = b.getNearestFreeGhostsInVision();
            let g = ghosts.get(gd.id);
            // but only if close than catching ghost which maybe hidden
            if (b.mode === 'catching') {
                if (gd.dist < distance(b.pos, ghosts.get(b.targetId).posEst)) {
                    // reset
                    ghosts.get(b.targetId).catchedByBuster = null;
                    // new
                    b.targetId = gd.id;
                    g.catchedByBuster = b.id;
                }
            } else {
                b.mode = 'catching';
                b.targetId = gd.id;
                b.targetType = 'Ghost';
                g.catchedByBuster = b.id;
            }
*/
        }  
    }

    // for all remaining ghosts find free buster with minimal distance
    for (let id of availableGhosts) {
        let g = ghosts.get(id);
        let bd = getMinimalDistanceBusters(g, availableBusters);
        // maybe no busters are available
        if (typeof bd !== 'undefined') {
            let b = busters.get(bd.id);
            // but only if close than catching ghost which maybe hidden
            if (b.mode === 'catching' && b.targetId === ghosts.get(b.targetId).catchedByBuster) {
                if (bd.dist < distance(b.pos, ghosts.get(b.targetId).posEst)) {
                    // reset
                    ghosts.get(b.targetId).catchedByBuster = null;
                    
                    // new
                    b.targetId = g.id;
                    g.catchedByBuster = b.id;
                }
            } else {
                b.mode = 'catching';
                b.targetId = g.id;
                b.targetType = 'Ghost';
                g.catchedByBuster = b.id;
            } 
        }
    }
}

// check if any ghost out of vision should be visible (posEst within buster range of vision)
// if so, then this ghost maybe captured by the opponent
function checkGhostsOutOfVision () {
	let gOoV = new Set(ghostsOutOfVision);

	for (let id of gOoV) {
		let g = ghosts.get(id);
		if (g.busterShouldInVision()) {
		    
		    let found = false;
		    for (let bId of bustersInVision) {
		        if (busters.get(bId).value === id) {
		            found = true;
		        }
		    }
		    
		    if (!found) {
    			g.xEst = null;
    			g.yEst = null;
    			
    			if (debug) printErr('ghostsOutOfVision delete:'+id);
    			ghostsOutOfVision.delete(id);
    			
                // reset any catching or searching buster with this targetId
    			for (let id of ownBusters) {
    				let b = busters.get(id);
    				if (b.mode === 'catching' || b.mode === 'searching') { 
    				    if (b.targetId === g.id) {
    				        b.reset();
    				    }
    				}
    			}
		    }
		}
	}
}
function busterWithinBustRangeNew(stamina = 40) {
    for (let gId of ghostsInVision) {
        let g = ghosts.get(gId);
        if (g.state <= stamina) {
            let state = g.state;
            let oB = g.ownBustersWithinBustRange();
            
            let minDistToHome = 1e6
            let minDistToHomeId = -1;
            for (let bId of oB) {
                let b = busters.get(bId);
                if (availableBusters.has(b.id) && state >= 0) {
                    let distToHome = b.distToHome();
                    if (distToHome < minDistToHome) {
                        minDistToHome = distToHome;
                        minDistToHomeId = b.id;
                    }
                }
            }
            
            if (minDistToHomeId !== -1) {
                let b = busters.get(minDistToHomeId);
    
                b.mode = 'busting';
                b.targetId = g.id;
                b.targetType = 'Ghost';
                
                state -= 1;
                
                if (debug) printErr('bust:'+b.id+' state:'+state);
    
                if (availableGhosts.has(g.id)) availableGhosts.delete(g.id);
                if (availableBusters.has(b.id)) availableBusters.delete(b.id);
            }
        }
    }
}

// not used
function busterWithinBustRange() {
    
    let ghostsInBustRange = [];
    for (let id of ghostsInVision) {
        let g = ghosts.get(id);
        for (let i of ownBusters) {
            let b = busters.get(i);
            let dist = distance(g.pos, b.pos);
            if (dist < ghostBustMin && dist > ghostBustMax) {
                let distToHome = b.distToHome();
                let index = ghostsInBustRange.findIndex(
                    function(e) {
                        if (e.gId === id && e.bId === i && distToHome<e.dist) return true;
                        return false;                    
                    });
                if (index !== -1) {
                    ghostsInBustRange[index] = {gId: id, bId: i, dist: distToHome}
                } else {
                    ghostsInBustRange.push({gId: id, bId: i, dist: distToHome});
                }
            }
        }
    }
    
    for (let e of ghostsInBustRange) {
        let g = ghosts.get(e.gId);
        let b = busters.get(e.bId);

        b.mode = 'busting';
        b.targetId = g.id;
        b.targetType = 'Ghost';
        
        if (availableGhosts.has(g.id)) availableGhosts.delete(g.id);
        if (availableBusters.has(b.id)) availableBusters.delete(b.id);
    }
    
/*
    ghostsInBustRange[0].bId = 0;
    ghostsInBustRange[0].bId = 0;
    
    
    
    
    let otherBusters = [];
    let freeGhosts = new Set(ghostsInVision);
    // only catching busters
    for (let id of ownBusters) {
        let b = busters.get(id);
        if (b.mode === 'catching') {
            let g = ghosts.get(b.targetId);
            let dist = distance(b.pos, g.pos);
            if (dist < ghostBustMin && dist > ghostBustMax) {
                b.mode = 'busting';
                freeGhosts.delete(g.id);
            }
        } else {
           otherBusters.push({id: b.id, distToHome: distance(b.pos, home)});
        }
    }
    
    otherBusters.sort(function(a,b) {
        return a.distToHome-b.distToHome;
    });
    
    // all other busters, starting with the closest to home
    for (let buster of otherBusters) {
        let b = busters.get(buster.id);
        if (b.state !== 1) { // only busters without carrying a ghost
            for (let ghost of freeGhosts) {
                let g = ghosts.get(ghost);
                let dist = distance(b.pos, g.pos);
                if (dist < ghostBustMin && dist > ghostBustMax) {
                    b.mode = 'busting';
                    b.targetId = ghost;
                    b.targetType = 'Ghost';
                    freeGhosts.delete(ghost);
                    g.catchedByBuster = b.id;
                    continue;
                }  
            }
        }
    }
*/
}
function busterWithinHomeRange() {
    for (let id of ownBusters) {
        let b = busters.get(id);
        if (/*b.mode === 'returning' && */ b.state === 1) { // first part not necessary
            let dist = distance(b.pos, home);
            if (dist < releaseMin) b.mode = 'releasing';
        }
    }
}
function updateAllBusters() {
    for (let id of ownBusters) {
        let b = busters.get(id);
        if ((b.mode === 'releasing' || b.mode === 'returning') && b.state === 0) { // releasing a ghost, or lost a ghost while returning home
            b.reset();
        } else if (b.mode === 'stunning') {
            b.reset();
        } else if (b.state === 1) {
            b.mode = 'returning';
            b.targetType = 'Home';
        } else if (b.state === 2) {
            b.mode = 'stunned';
            b.targetId = null;
            b.targetType = null;
        } else if (b.state === 0 && b.mode === 'stunned') {
            b.reset();
        } else if (b.mode === 'catching' || b.mode === 'busting') {
            b.reset();
        }
    }
}
function getMinOfArray(arr, rows, columns){
    let min = {row: 0, col: 0, val: arr[0][0]};
    for(let i = 0; i < rows; i++) {
        for (let j = 0; j < columns; j++) {
            if(arr[i][j] < min.val) {
                min = {row: i, col: j, val: arr[i][j]};
            }
        }
    }
    return min;
}
function updateCatchingTargets(stamina = 40) {
    let freeBusters = Array.from(availableBusters);
    let freeGhosts = [];
    
    for (let id of availableGhosts) {
        let g = ghosts.get(id);
        if (g.state <= stamina) {
            freeGhosts.push(id);
        }
    }
    for (let id of ghostsOutOfVision) {
        let g = ghosts.get(id);
        if (g.state <= stamina) {
            freeGhosts.push(id);
        }
    }
    
    //printArray('freeBusters', freeBusters);
    //printArray('freeGhosts', freeGhosts);
    if (freeGhosts.length > 0) {
        let rows = freeBusters.length,
            columns = freeGhosts.length;
        let arr = new Array(rows);
        for (let i = 0; i < rows; i++) {
            arr[i] = new Array(columns);
            
            let output = freeBusters[i]+':';
            for (let j = 0; j < columns; j++) {
                let b = busters.get(freeBusters[i]);
                let g = ghosts.get(freeGhosts[j]);
                
                let distToGhost = distance(b.pos, g.posEst);
                let distToHome = distance(g.posEst, home);
                let distFromStamina = g.state * 800;
                
                if (debug) printErr('g'+g.id+' distToGhost:'+distToGhost+' distToHome:'+distToHome+' distFromStamina:'+distFromStamina);
                
                let add = 0;
                if (ghostsOutOfVision.has(freeGhosts[j])) add = 800;
                arr[i][j] = distToGhost+distToHome+distFromStamina;
                
                output += ' '+arr[i][j];
            }
            if (debug) printErr(output);
        }
    
        for ( ; rows>0 && columns>0; ) {
            let min = getMinOfArray(arr, rows, columns);
            
            if (debug) printErr('row:'+min.row+' col:'+min.col+' val:'+min.val);
            
            // go catching
            let b = busters.get(freeBusters[min.row]);
            b.mode = ghostsOutOfVision.has(freeGhosts[min.col])?'searching':'catching';
            b.targetId = freeGhosts[min.col];
            b.targetType = 'Ghost';
            
            // reduce arrays
            freeBusters.splice(min.row, 1);
            freeGhosts.splice(min.col, 1);
            arr.splice(min.row, 1);
            rows--;
            for (let i = 0; i < rows; i++) {
                arr[i].splice(min.col, 1);
            }
            columns--;
        }
    }
}

function checkStun() {
    for (let id of ownBusters) {
        let b = busters.get(id);
        if (b.stunAvailable() && b.state !== 2) {
            let eId = b.enemyBusterInStunRange();
            if (eId !== -1) {
                //if (b.state === 2 || eId.state === 2) 
                {
                    b.mode = 'stunning';
                    b.targetId = eId;
                    b.targetType = 'Buster';
                
                    if (availableBusters.has(b.id)) availableBusters.delete(b.id);
                }
            }
        }
    }
}

function updateCatchingTargets(stamina = 40) {
    let freeBusters = Array.from(availableBusters);
    let freeGhosts = [];
    
    for (let id of availableGhosts) {
        let g = ghosts.get(id);
        if (g.state <= stamina) {
            freeGhosts.push(id);
        }
    }
    for (let id of ghostsOutOfVision) {
        let g = ghosts.get(id);
        if (g.state <= stamina) {
            freeGhosts.push(id);
        }
    }
    
    //printArray('freeBusters', freeBusters);
    //printArray('freeGhosts', freeGhosts);
    if (freeGhosts.length > 0) {
        let rows = freeBusters.length,
            columns = freeGhosts.length;
        let arr = new Array(rows);
        for (let i = 0; i < rows; i++) {
            arr[i] = new Array(columns);
            
            let output = freeBusters[i]+':';
            for (let j = 0; j < columns; j++) {
                let b = busters.get(freeBusters[i]);
                let g = ghosts.get(freeGhosts[j]);
                
                let distToGhost = distance(b.pos, g.posEst);
                let distToHome = distance(g.posEst, home);
                let distFromStamina = g.state * 800;
                
                if (debug) printErr('g'+g.id+' distToGhost:'+distToGhost+' distToHome:'+distToHome+' distFromStamina:'+distFromStamina);
                
                let add = 0;
                if (ghostsOutOfVision.has(freeGhosts[j])) add = 800;
                arr[i][j] = distToGhost+distToHome+distFromStamina;
                
                output += ' '+arr[i][j];
            }
            if (debug) printErr(output);
        }
    
        for ( ; rows>0 && columns>0; ) {
            let min = getMinOfArray(arr, rows, columns);
            
            if (debug) printErr('row:'+min.row+' col:'+min.col+' val:'+min.val);
            
            // go catching
            let b = busters.get(freeBusters[min.row]);
            b.mode = ghostsOutOfVision.has(freeGhosts[min.col])?'searching':'catching';
            b.targetId = freeGhosts[min.col];
            b.targetType = 'Ghost';
            
            // reduce arrays
            freeBusters.splice(min.row, 1);
            freeGhosts.splice(min.col, 1);
            arr.splice(min.row, 1);
            rows--;
            for (let i = 0; i < rows; i++) {
                arr[i].splice(min.col, 1);
            }
            columns--;
        }
    }
}




function updateTargetsWhileNotAllGhostsFound(stamina = 3, range = 7000) {
    let freeBusters = Array.from(availableBusters);
    let freeGhosts = [];
    
    for (let id of availableGhosts) {
        let g = ghosts.get(id);
        if (g.state <= stamina && !g.isTrappedByOwnBusterNoHelpNeeded()) {
            freeGhosts.push(id);
        }
    }
    for (let id of ghostsOutOfVision) {
        let g = ghosts.get(id);
        if (!g.withinEnemyBaseRange(range) && g.state <= stamina) {
            freeGhosts.push(id);
        }
    }
    
    //printArray('freeBusters', freeBusters);
    //printArray('freeGhosts', freeGhosts);
    if (freeGhosts.length > 0) {
        let rows = freeBusters.length,
            columns = freeGhosts.length;
        let arr = new Array(rows);
        for (let i = 0; i < rows; i++) {
            arr[i] = new Array(columns);
            
            let output = freeBusters[i]+':';
            for (let j = 0; j < columns; j++) {
                let b = busters.get(freeBusters[i]);
                let g = ghosts.get(freeGhosts[j]);
                
                let distToGhost = distance(b.pos, g.posEst);
                let distToHome = distance(g.posEst, home);
                let distFromStamina = 0;//g.state * 800;
                
                //if (debug) printErr('g'+g.id+' distToGhost:'+distToGhost+' distToHome:'+distToHome+' distFromStamina:'+distFromStamina);
                
                let add = 0;
                if (ghostsOutOfVision.has(freeGhosts[j])) add = 800;
                arr[i][j] = distToGhost+distToHome+distFromStamina;
                
                output += ' '+arr[i][j];
            }
            if (debug) printErr(output);
        }
    
        for ( ; rows>0 && columns>0; ) {
            let min = getMinOfArray(arr, rows, columns);
            
            if (debug) printErr('row:'+min.row+' col:'+min.col+' val:'+min.val);
            
            // go catching
            let b = busters.get(freeBusters[min.row]);
            let g = ghosts.get(freeGhosts[min.col]);
            
            let dist = distance(b.pos, g.pos);
            
    		if (dist < ghostBustMin && dist > ghostBustMax) {
    		    b.mode = 'busting';   
    		} else {
    		    b.mode = ghostsOutOfVision.has(freeGhosts[min.col])?'searching':'catching';
    		}
            b.targetId = freeGhosts[min.col];
            b.targetType = 'Ghost';
            
            // reduce arrays
            freeBusters.splice(min.row, 1);
            arr.splice(min.row, 1);
            rows--;
            if (g.isSearched()) {
                freeGhosts.splice(min.col, 1);
                for (let i = 0; i < rows; i++) {
                    arr[i].splice(min.col, 1);
                }
                columns--;
            } else if (g.isTrappedByOwnBusterNoHelpNeeded() || g.isCatchedByOwnBuster()) {
                freeGhosts.splice(min.col, 1);
                for (let i = 0; i < rows; i++) {
                    arr[i].splice(min.col, 1);
                }
                columns--;
            }
        }
    }
}
function busting(stamina = 3) {
    for (let id of ownBusters) {
        let b = busters.get(id);
        let gInR = b.ghostsInBustRange();
        for (let gId of gInR) {
            let g = ghosts.get(gId.id);
            if (g.state <= stamina && !g.isTrappedByOwnBusterNoHelpNeeded()) {
                let state = g.state;
                let trapping = g.getBustersTrapping();
                
                b.mode = 'busting';
                b.targetId = g.id;
                b.targetType = 'Ghost';  
                
                if (availableBusters.has(id)) availableBusters.delete(id);
                
                break;
            }
        }
    }
}

function updateSearchingOrCatching() {
    for (let id of ownBusters) {
        let b = busters.get(id);
        
        let targetId = b.targetId;
        let targetType = b.targetType;
        
        if (targetType === 'Ghost') {
            let g = ghosts.get(targetId);
            let dist = distance(b.pos, g.pos);
            
    		if (dist < ghostBustMin && dist > ghostBustMax) {
    		    b.mode = 'busting';   
    		}
            
        } else if (targetType === 'Buster') {
            let e = busters.get(targetId);
            let dist = distance(b.pos, e.pos); 
            
            if (b.stunAvailable() && b.state !== 2) {
                if (dist < stunRange) {
                   b.mode = 'stunning'; 
               }
            }
        }
    }        
}

function updateTargets(stamina = 3, range = 7000) {
    let freeBusters = Array.from(availableBusters);
    let freeTargets = [];
    let targetSize = 0;
    
    for (let id of availableGhosts) { //  ghosts in Vision
        let g = ghosts.get(id);
        if (g.state <= stamina && !g.isTrappedByOwnBusterNoHelpNeeded()) {
            freeTargets.push({id:id, type: 'Ghost'});
            targetSize++;
        }
    }
    for (let id of ghostsOutOfVision) {
        let g = ghosts.get(id);
        if (!(g.withinEnemyBaseRange(range) && g.state <= 15) && g.state <= stamina) {
            freeTargets.push({id:id, type: 'Ghost'});
            targetSize++;
        }
    }
    
    for (let id of bustersInVision) {
        let b = busters.get(id);
        if (!ownBusters.has(b.id) && b.state === 1) {
            freeTargets.push({id:id, type: 'Buster'});
            targetSize++;
        }
    }     
    
    //if (debug) printArray('freeBusters', freeBusters);
    {
        let out = 'freeTargets { ';
        for (let element of freeTargets) out += element.id+' ';
        out += '}';
        //if (debug) printErr(out);
    }
    if (targetSize > 0) {
        let rows = freeBusters.length,
            columns = targetSize;
        let arr = new Array(rows);
        for (let i = 0; i < rows; i++) {
            arr[i] = new Array(columns);
            
            let output = freeBusters[i]+':';
            for (let j = 0; j < columns; j++) {
                let b = busters.get(freeBusters[i]);
                let id = freeTargets[j].id;
                let type = freeTargets[j].type;
                
                if (type === 'Ghost') {
                    let g = ghosts.get(id);
                    
                    let distToGhost = distance(b.pos, g.posEst);
                    let distToHome = distance(g.posEst, home);
                    let distFromStamina = g.state * 800;
                    
                    arr[i][j] = distToGhost+distToHome+distFromStamina;
                    
                } else if (type === 'Buster') {
                    let e = busters.get(id);
                    
                    let distToBuster = distance(b.pos, e.pos);
                    let distToHome = distance(e.pos, home);
                    let distFromStamina = 0;
                    
                    arr[i][j] = distToBuster+distToHome+distFromStamina;
                    
                }
                
                //if (debug) printErr('g'+g.id+' distToGhost:'+distToGhost+' distToHome:'+distToHome+' distFromStamina:'+distFromStamina);
                output += ' '+arr[i][j];
            }
            //if (debug) printErr(output);
        }
    
        for ( ; rows>0 && columns>0; ) {
            let min = getMinOfArray(arr, rows, columns);
            
            if (debug) printErr('row:'+min.row+' col:'+min.col+' val:'+min.val);
            
            // go catching
            let b = busters.get(freeBusters[min.row]);
            
            let id = freeTargets[min.col].id;
            let type = freeTargets[min.col].type;
                
            if (type === 'Ghost') {
                let g = ghosts.get(id);
                let dist = distance(b.pos, g.pos);
                
        		if (dist < ghostBustMin && dist > ghostBustMax) {
        		    b.mode = 'busting';   
        		} else {
        		    b.mode = ghostsOutOfVision.has(id)?'searching':'catching';
        		}
            } else if (type === 'Buster') {
                b.mode = 'catching';
            }
            
            b.targetId = id;
            b.targetType = type;
            
            // reduce arrays
            freeBusters.splice(min.row, 1);
            arr.splice(min.row, 1);
            rows--;
            if (type === 'Ghost') {
                let g = ghosts.get(id);
                if (g.isSearched()) {
                    freeTargets.splice(min.col, 1);
                    for (let i = 0; i < rows; i++) {
                        arr[i].splice(min.col, 1);
                    }
                    columns--;
                } else if (!allBustersToTarget) {  
                    if (turn<50 && (g.isTrappedByOwnBusterNoHelpNeeded() || g.isCatchedByOwnBuster())) {
                        freeTargets.splice(min.col, 1);
                        for (let i = 0; i < rows; i++) {
                            arr[i].splice(min.col, 1);
                        }
                        columns--;
                    }
                }
            }
        }
    }
}
var allBustersToTarget = false;
function ghostsOutOfVisionHalf40() {
    let all = 0, count = 0;
    for (let id of ghostsOutOfVision) {
        let g = ghosts.get(id);
        if (g.state === 40) {
            count++;
        }
        all++;
    }
    
    if (count/all > 0.5) {
        allBustersToTarget = true;
    }
}

function compute () {
    
    if (!allBustersToTarget) ghostsOutOfVisionHalf40();
    //printErr('allBustersToTarget '+allBustersToTarget)
    
    /* TODO mode auto one to the center
    
            let dist = distance(this.pos, {x:8000, y:4500});
            //printErr('dist:'+dist);
            if (dist < 2180) return 'RELEASE';
            // or move home
            dist = dist-releaseMin+2; // just for rounding issues
            let move = step(this.pos, home, dist<maxMove?dist:maxMove);
            //printErr('dist:'+dist);
    */

    checkStun();

    // check if any returning buster with a ghost is within home range
    busterWithinHomeRange();

    if (ghosts.size >= (ghostCount-1) || gameGrid2.getUnknownTerritoryPercentage() < 0.01) {
        updateTargets(40, 6000);
        
        updateSearchingOrCatching();

    } else {
        updateTargetsWhileNotAllGhostsFound(3, 7000);

        updateSearchingOrCatching();

    } 
        
    
        // check if any ghost is within bust range of a buster
        //busting(40);
        //busterWithinBustRangeNew();
        
        
        //if (debug) printSet('availableBusters', availableBusters);
        //if (availableBusters.size > 0) updateCatchingTargets();



/*
    if (ghosts.size < ghostCount-2 || gameGrid.stillUnknownTerritoryAroundHome(9200)) {
    
        // TODO better ??
        checkStun();
        
        // check if any returning buster with a ghost is within home range
        busterWithinHomeRange();
        
        // check if any ghost with stamina = 3 is within bust range of a buster
        busterWithinBustRangeNew(15);
        
        updateCatchingTargets(15);
    
    } else {
    
    //if (gameGrid.getUnknownTerritoryPercentage() < 0.40) {
    
        checkStun();
    
        // check if any returning buster with a ghost is within home range
        busterWithinHomeRange()
    
        // check if any ghost is within bust range of a buster
        busterWithinBustRangeNew();
    
        // if ghosts in vision, compute optimal pairs (ghost, buster) based on minimal distance
        //updateCatching();
    
    //
    //2. busters with ghostsInVision
    //           starting with buster with max ghosts, descending
    //            choose ghost with minimal distance
    
    //3. busters still available
    //            wenn in searching or catching mode
    //            überprüfen ob ein availableGhosts noch verfügbar ist
    //            wenn ja und die Distanz ist nur um 800 weiter als zum derzeitigen Target
    //            dann wechseln
    
    //4. busters still available d.h. nur mehr exploring
    //            min distance zu availableGhosts
    
    
                            
       
    
    
    //    5. busters still available d.h. nur mehr exploring
    //        min distance zu ghostsOutOfVision die noch keine targets sind
    //
    
    
    
    
        // compute optimal pairs (ghost, buster) based on minimal distance
        // penalty distance for ghostsOutOfVision
        // there is the risk the got captured by the opponent
        if (debug) printSet('availableBusters', availableBusters);
        if (availableBusters.size > 0) updateCatchingTargets();
    }
*/
    
// simple:
    // exploring
    // - known ghosts
    // - last known ghosts
    // - exploring
    // catchGhost (if in sight)
    // returning
    // bust
    // moveHome;
    // release
    
    
// advanced:
    // help buster
    // ...
}

// miscellaneous functions

function distance(p1, p2) {
    return Math.sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                     (p1.y - p2.y) * (p1.y - p2.y));
}
function step(pos, target, move) {
    let diffX = target.x - pos.x;
    let diffY = target.y - pos.y;
    
    let factor = move/Math.sqrt(diffX*diffX+diffY*diffY);
    
    return {'x': pos.x + factor * diffX, 'y': pos.y + factor * diffY};
}
function checkBounds(pos) {
    if (pos.x < 0) pos.x = 0;
    if (pos.x > (fieldWidth-1)) pos.x = (fieldWidth-1);
    if (pos.y < 0) pos.y = 0;
    if (pos.y > (fieldHeight-1)) pos.y = (fieldHeight-1);
    return pos;
}
function getBoundingGridPoints (pos) {
    let p1 = checkBounds({x: pos.x - rangeOfVision, y: pos.y - rangeOfVision});
    let p2 = checkBounds({x: pos.x + rangeOfVision, y: pos.y + rangeOfVision});
    
    p1 = gameGrid2.findNearestGridPoint(p1);
    p2 = gameGrid2.findNearestGridPoint(p2);
    
    return { p1: p1, p2: p2};
}
function isPointWithinBounds(pos) {
    if (pos.x < 0) return false;
    if (pos.x > (fieldWidth-1)) return false;
    if (pos.y < 0) return false;
    if (pos.y > (fieldHeight-1)) return false;
    

    if (myTeamId === 0) {    
        if (pos.x > 10000) return false;
    } else {
        if (pos.x < 6000) return false;
    }
    
    return true;
}
// exploring functions
// some code snippets and ideas from https://github.com/benfred/bens-blog-code/blob/master/circle-intersection/circle-intersection.js

function getExplorePositions(pos, moveDistance, intervals) {
    let diff = (2*PI/intervals);

    let start = 2*PI-diff, // 340 grad
        end = 2*PI; // 380 grad
    
		let intervalPoints = [];
		for (let i = 0; i <= intervals; i++) {
			let a = start - i * (2*PI-end+start) / intervals;
			let p = {x: pos.x + Math.cos(a)*moveDistance, y: pos.y - Math.sin(a)*moveDistance};
			
			//printErr('alpha:'+a/(2*PI)*360+' x:'+p.x+' y:'+p.y);
			
			if (isPointWithinBounds(p)) {
			    intervalPoints.push(p);
			}
		}
		
		return intervalPoints;
}

function unknownTerritoryEstimate (pos) {
    let unknown = 0;
    
    let center = gameGrid2.findNearestGridPoint(pos);
    
    let p = getBoundingGridPoints(pos);
    let p1 = p.p1,
        p2 = p.p2;
    
    for (let i = p1.x; i <= p2.x; i++) {
        for (let j = p1.y; j <= p2.y; j++) {
            if (gameGrid2.isWithinRange({x:i , y:j}, center, 220)) {
                unknown += gameGrid2.previewGrid[i][j];
            }
        }
    }
    
    //printErr('p:'+p1.x+','+p1.y+' '+p2.x+','+p2.y+' center:'+center.x+','+center.y+' '+unknown);
    
    return unknown;
}

// not used
function getBoundingRectangle (pos) {
    let p1 = checkBounds({x: pos.x - rangeOfVision, y: pos.y - rangeOfVision});
    let p2 = checkBounds({x: pos.x + rangeOfVision, y: pos.y + rangeOfVision});
    
    //printErr('x: '+p1.x+' y:'+p1.y+' height:'+(p2.y-p1.y)+' width:'+(p2.x-p1.x));
    return { x: p1.x, y: p1.y, height: p2.y - p1.y, width: p2.x - p1.x};
}
// not used
function monteCarloEstimate (pos, count = 50000) {
    let bustersPosition = getOwnBustersPosition();
    var contained = 0;
    var bound = getBoundingRectangle(pos);
    for (var i = 0; i < count; ++i) {
        var p = randomPoint(bound);
        if (withinRangeOfVision(p, [pos])) {
            if (!withinRangeOfVision(p, bustersPosition)) {
                contained += gameGrid.previewGridPoint(p); // new
            }
        }
    }
    return bound.width * bound.height * contained / count;
}
// not used
function getOwnBustersPosition() {
    let positions = [];
    for (let id of ownBusters) {
        positions.push(busters.get(id).pos);
    }
    return positions;
}
// not used
function randomPoint (rect) {
    return { x: rect.x + Math.random() * rect.width,
             y: rect.y + Math.random() * rect.height};
}
// not used
function withinRangeOfVision (point, bustersPosition) { // returns whether a point is in range of vision
    for (let i = 0; i < bustersPosition.length; i++) {
        if (distance(point, bustersPosition[i]) < rangeOfVision) {
            return true;
        }
    }
    return false;
}

// printing functions

function printSet (name, set) {
    let out = name+' { ';
    for (let item of set) out += item+' ';
    out += '}';
    printErr(out);
}
function printMap (name, map) {
    let out = name+' { ';
    for (let [key, value] of map) out += key+' ';
    out += '}';
    printErr(out);
}
function printArray (name, array) {
    let out = name+' { ';
    for (let element of array) out += element+' ';
    out += '}';
    printErr(out);
}

function getSymmetricPosition (pos) {
    let center = {x:8000, y:4500};
    let dist = distance(pos, center);
    
    let sym = step(pos, center, dist*2);
    
    return {x:Math.round(sym.x), y:Math.round(sym.y)};
}

// game loop
for (; ; turn++) {
    
    //  move all active ghosts from last turn to ghostsOutOfVision
    for (let ghost of ghostsInVision) {
        if (!ghostsOutOfVision.has(ghost)) ghostsOutOfVision.add(ghost);
        ghostsInVision.delete(ghost);
    } 
    
    // remove all busters in vision
    bustersInVision.clear();
    
    let entities = parseInt(readline()); // the number of busters and ghosts visible to you
    for (let i = 0; i < entities; i++) {
        let inputs = readline().split(' ');
        
        let entityId = parseInt(inputs[0]); // buster id or ghost id
        let x = parseInt(inputs[1]);
        let y = parseInt(inputs[2]); // position of this buster / ghost
        let entityType = parseInt(inputs[3]); // the team id if it is a buster, -1 if it is a ghost.
        let state = parseInt(inputs[4]); // For busters: 0=idle, 1=carrying a ghost.
        let value = parseInt(inputs[5]); // For busters: Ghost id being carried. For ghosts: number of busters attempting to trap this ghost.

        if (entityType === -1) { // ghosts
            if (ghostsOutOfVision.has(entityId)) ghostsOutOfVision.delete(entityId);
            ghostsInVision.add(entityId);
            
            let g = ghosts.get(entityId);
            if (typeof g === 'undefined') ghosts.set(entityId, new Ghost(entityId,x,y,state,value,turn));
            else {
                g.x = x; g.y = y; g.state = state; g.value = value; g.turn = turn;
                ghosts.set(entityId, g);
            }
            
            if (entityId !== 0) {
                let symEntityId;
                if (entityId%2 === 0) symEntityId = entityId-1;
                else symEntityId = entityId+1;
                
                if (!ghosts.has(symEntityId)) {
                    let symPos = getSymmetricPosition({x:x, y:y});
                    ghosts.set(symEntityId, new Ghost(symEntityId,symPos.x,symPos.y,state,value,-1));
                    ghostsOutOfVision.add(symEntityId);
                }
            }
        } else { // busters
            if (state === 1 && value !== -1) { // ghosts carried by a buster
                if (ghostsOutOfVision.has(value)) ghostsOutOfVision.delete(value);
                if (ghostsInVision.has(value)) ghostsInVision.delete(value);
            }
            
            let b = busters.get(entityId);
            if (typeof b === 'undefined') busters.set(entityId, new Buster(entityId,x,y,entityType,state,value,turn));
            else {
                if (b.team === myTeamId) {
                    b.stunTimer = Math.min(20, b.stunTimer+1);
                    b.updateHistory();
                }

                b.x = x; b.y = y; b.team = entityType; b.state = state; b.value = value; b.turn = turn;
                busters.set(entityId, b);
            }
            
            if (turn === 0 && entityType === myTeamId) {
                ownBusters.add(entityId);
            }
            
            // add all busters in vision
            bustersInVision.add(entityId);        
        }
        
        //if (debug) printSet('ghostsOutOfVision', ghostsOutOfVision);
        
        if (debug) printErr('entityId:'+entityId+' x:'+x+' y:'+y+' entityType:'+entityType+' state:'+state+' value:'+value+' turn:'+turn);
    }

    if (debug) printErr('ghostCount:'+ghostCount+' found:'+ghosts.size);
    if (debug) printSet('ghostsInVision', ghostsInVision);
    if (debug) printSet('ghostsOutOfVision', ghostsOutOfVision);
    if (debug) printSet('ghostsCaptured', ghostsCaptured);

    // update game grid
    //gameGrid.updateVisitedPoints();
    //if (debug) printErr('getUnknownTerritoryPercentage:'+gameGrid.getUnknownTerritoryPercentage()*100);
    gameGrid2.updateVisitedPoints();
    if (debug) printErr('getUnknownTerritoryPercentage2:'+gameGrid2.getUnknownTerritoryPercentage()*100);
        
    // estimate next position for all ghosts in vision based on all known busters
    for (let id of ghostsInVision) {
        ghosts.get(id).estimatedNextPosition(bustersInVision);
    }
    
    // debug output
    for (let id of ghostsInVision) {
        //if (debug) printErr(ghosts.get(id).toString());
    }
    for (let id of ghostsOutOfVision) {
        //if (debug) printErr(ghosts.get(id).toString());
    }
        
    // update all busters, check if some are idle (after busting, releasing, stunned)
    updateAllBusters();
    
    // update the relevant ghosts and busters for further actions
    availableGhosts.clear();
    for (let id of ghostsInVision) {
    	availableGhosts.add(id);
    }
    availableBusters.clear();
    for (let id of ownBusters) {
    	let b = busters.get(id);
    	if (b.mode === 'exploring' || b.mode === 'catching' || b.mode === 'searching') {
    		availableBusters.add(id);
    	}
    }

    // check if any ghost out of vision should be visible (posEst within buster range of vision)
    // if so, then this ghost maybe captured by the opponent
    checkGhostsOutOfVision ();
    
    // now action - busting, catching, ...
    compute();

    if (debug) printSet('ghostsOutOfVision', ghostsOutOfVision);

    // debug output
    for (let id of ghostsInVision) {
        if (debug) printErr(ghosts.get(id).toString());
    }

    for (let id of ownBusters) {
        if (debug) printErr(busters.get(id).toString());
        print(busters.get(id).nextMove());
        //print(busters.get(id).nextMove()+' f:'+(ghosts.size===ghostCount?'ALL FOUND':(ghostCount-ghosts.size))); // MOVE x y | BUST id | RELEASE
    }
}

/*
    200 is ok

    var start = new Date().getTime();
    for (let i = 0; i < 200; ++i) {
        unknownTerritoryEstimate({'x': 8000,'y': 4500});
    }
    var end = new Date().getTime();
    var time = end - start;
    printErr('Execution time: ' + time);
    
    printErr(monteCarloEstimate({'x': 8000,'y': 4500},100000));
    printErr(monteCarloEstimate({'x': 4820,'y': 2732}));
*/