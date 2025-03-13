//canvas setup-----------------------------

var canvas = document.getElementById("myCanvas");
var c = canvas.getContext("2d");

let data
const ctx = document.getElementById('Chart1');

let chart = new Chart(ctx, {
  type: 'line',
  data: {
    datasets: [{
      label: 'Velocities',
      data: data,
      borderWidth: 0.5
    }]
  },
  options: {
    scales: {
      y: {
        beginAtZero: true
      }
    }
  }
});

function resize() {
    canvas.height = window.innerHeight/2;
    canvas.width = window.innerWidth/3;
    
    window.simMinWidth = 5.0;
    window.cScale = Math.min(canvas.width, canvas.height) / simMinWidth;
    
    window.simWidth = canvas.width / window.cScale;
    window.simHeight = canvas.height / window.cScale;
    console.log("canvas-Heigth: " + window.simHeight + ", canvas-Width: " + window.simWidth);
}
resize();
function cX(pos) {
    return pos * window.cScale;
}

function cY(pos){
    return canvas.height - pos * window.cScale;
}

//vector math------------------------------

class Hash {
    constructor(spacing, maxNumObjects)
    {
        this.spacing = spacing;
        //this.tableSize = 2 * maxNumObjects;
        this.tableSize = 100;
        this.cellStart = new Int32Array(this.tableSize + 1);
        this.cellEntries = new Int32Array(maxNumObjects);
        this.queryIds = new Int32Array(maxNumObjects);
        this.querySize = 0;
    }

    hashCoords(xi, yi) {
        var h = (xi * 92837111) ^ (yi * 689287499);
        return Math.abs(h) % this.tableSize;
    }

    intCoord(coord) {
        return Math.floor(coord / this.spacing);
    }

    hashPos(pos, nr) {
        return this.hashCoords(
            this.intCoord(pos[2 * nr]),
            this.intCoord(pos[2 * nr + 1])
        );
    }

    create(pos) {
        var numObjects = Math.min(pos.length / 2, this.cellEntries.length);
        
        //determine cell sizes

        this.cellStart.fill(0);
        this.cellEntries.fill(0);

        for (var i = 0; i < numObjects; i++) {
            var h = this.hashPos(pos, i);
            this.cellStart[h]++;
        }

        //determine cell starts

        var start = 0;
        for (var i = 0; i < this.tableSize; i++) {
            start += this.cellStart[i];
            this.cellStart[i] = start;
        }
        
        this.cellStart[this.tableSize] = start; //guard

        //fill in objects ids

        for (var i = 0; i < numObjects; i++) {
            var h = this.hashPos(pos, i);
            this.cellStart[h]--;
            this.cellEntries[this.cellStart[h]] = i;
        }
        
    }

    query(pos, nr, maxDist) {
        
        var x0 = this.intCoord(pos[2 * nr] - maxDist);
        var y0 = this.intCoord(pos[2 * nr + 1] - maxDist);

        var x1 = this.intCoord(pos[2 * nr] + maxDist);
        var y1 = this.intCoord(pos[2 * nr + 1] + maxDist);

        this.querySize = 0;

        for (var xi = x0; xi < x1; xi++) {
            for (var yi = y0; yi < y1; yi++) {
                var h = this.hashCoords(xi, yi);
                var start = this.cellStart[h];
                var end = this.cellStart[h + 1];

                for (var i = start; i < end; i++) {
                    this.queryIds[this.querySize] = this.cellEntries[i];
                    this.querySize++;
                }
            }
            
        }
    }

}

function vecScale(a,anr, scale) {
    anr *= 2;
    a[anr++] *= scale;
    a[anr]   *= scale;
}

function vecCopy(a,anr, b, bnr) {
    anr *= 2; bnr *= 2;
    a[anr++] = b[bnr++];
    a[anr]   = b[bnr];
}

function vecAdd(a,anr, b, bnr, scale = 1.0) {
    anr *= 2; bnr *= 2;
    a[anr++] += b[bnr++] * scale;
    a[anr]   += b[bnr] * scale;
}
function vecSetDiff(dst,dnr, a,anr, b,bnr, scale = 1.0) {
    dnr *= 2; anr *= 2; bnr *= 2;
    dst[dnr++] = (a[anr++] - b[bnr++]) * scale;
    dst[dnr]   = (a[anr] - b[bnr]) * scale;
}
function vecLengthSquared(a,anr) {
    anr *= 2; 
    let a0 = a[anr], a1 = a[anr + 1];
    return a0 * a0 + a1 * a1;
}
function vecDot(a,anr, b, bnr) {
    anr *= 2; bnr *= 2;
    return a[anr] * b[bnr] + a[anr + 1] * b[bnr + 1];
}

//physics scene----------------------------

var physicsScene = 
{
    gravity : [0.0, 0.0],
    dt : 1.0 / 60,
    worldSize : [window.simWidth, window.simHeight],
    balls: null,
    restitution : 1.0,
};

class Balls {
    constructor(radius, pos, vel) {
    this.radius = radius;
    this.pos = pos;
    this.vel = vel;
    this.numBalls = Math.floor(pos.length / 2);
    this.hash = new Hash(2.0 * radius, this.numBalls);

    this.normal = new Float32Array(2);
    }
    addBall(posx,posy,velx,vely){
        this.pos.push(posx,posy);
        this.vel.push(velx,vely);    
    }
    simulate(dt, gravity) {
        
        var worldSize = physicsScene.worldSize;
        var minDist = 2.0 * this.radius;

        //integrate
        for (var i = 0; i < this.numBalls; i++) {
            vecAdd(this.vel, i, gravity, 0, dt);
            vecAdd(this.pos, i, this.vel, i, dt);
        }

        this.hash.create(this.pos);
        
        //handle collision

        
        for (var i = 0; i < this.numBalls; i++) {

            //world collision
            for (var dim = 0; dim < 2; dim++) {
                var nr = 2 * i + dim;
                if (this.pos[nr] < 0 + this.radius) {
                    this.pos[nr] = 0 + this.radius;
                    this.vel[nr] = -this.vel[nr] * physicsScene.restitution;
                }
                else if (this.pos[nr] > worldSize[dim] - this.radius) {
                    this.pos[nr] = worldSize[dim] - this.radius;
                    this.vel[nr] = -this.vel[nr] * physicsScene.restitution;
                }
                
            } 
        
        
            //interball collision

            this.hash.query(this.pos, i , 2.0 * this.radius);    
            
            for (var nr = 0; nr < this.hash.querySize; nr++) {
                var j = this.hash.queryIds[nr];
                
                vecSetDiff(this.normal, 0, this.pos, i, this.pos, j);
                var d2 = vecLengthSquared(this.normal, 0);
                
                //are the balls overlapping?

                if (d2 > 0.0 && d2 < minDist * minDist) {
                    
                    var d = Math.sqrt(d2);
                    vecScale(this.normal, 0, 1.0 / d);
                    
                    //separate the balls
                    
                    var corr = (minDist - d) * 0.5;

                    vecAdd(this.pos, i, this.normal, 0, corr);
                    vecAdd(this.pos, j, this.normal, 0, -corr);

                    var vi = vecDot(this.vel, i, this.normal, 0);
                    var vj = vecDot(this.vel, j, this.normal, 0);

                    vecAdd(this.vel, i, this.normal, 0, (vj - vi) * physicsScene.restitution);
                    vecAdd(this.vel, j, this.normal, 0, (vi - vj) * physicsScene.restitution);
                    
                }
            }
        }
}
}



function setupScene() 
{
    resize();
    var radius = 0.02;
    
    var pos = [];
    var vel = [];
    let velsies = 1;
    let ballsies = 1000;
    physicsScene.balls = new Balls(radius, pos, vel)
    for (let i = 0; i < ballsies ; i++) {
        physicsScene.balls.addBall(
            Math.random() * physicsScene.worldSize[0],
            Math.random() * physicsScene.worldSize[1],
            velsies * Math.pow(-1,Math.round(Math.random())),
            velsies * Math.pow(-1,Math.round(Math.random()))
            
        )
        
        
    }
    
    physicsScene.balls.numBalls = Math.floor(physicsScene.balls.pos.length / 2);
    physicsScene.balls.hash = new Hash(2.0 * physicsScene.balls.radius, physicsScene.balls.numBalls);

}
//drawing----------------------------------

function draw() 
{

    c.fillStyle = "#000000";
    c.clearRect(0, 0, canvas.width, canvas.height);
    for (i = 0; i < physicsScene.balls.pos.length / 2; i++) {
        var balls = physicsScene.balls;
        c.beginPath();			
        c.arc(
            cX(balls.pos[2 * i]), cY(balls.pos[2 * i + 1]), window.cScale * balls.radius, 0.0, 2.0 * Math.PI); 
        c.closePath();
        c.fill();
        
    }
   
}

//update------------------------------------
var buckets = 15;
var timeSum = 0;
var timeFrames = 0;

var count = 0;

function update(){
    count++;
    var numSteps = 1;
    var sdt = physicsScene.dt / numSteps;
    
    var startTime = performance.now();

    for (var i = 0; i < numSteps ; i++) {
        physicsScene.balls.simulate(sdt, physicsScene.gravity);
    }

    draw();

    var endTime = performance.now();
    timeSum += endTime - startTime; 
    timeFrames++;

    if (timeFrames > 10) {
        timeSum /= timeFrames;
        document.getElementById("ms").innerHTML = timeSum.toFixed(3);		
        timeFrames = 0;
        timeSum = 0;
    }

    requestAnimationFrame(update);

    document.getElementById("test").innerHTML = physicsScene.balls.numBalls;

    if (count % 10 == 0) {
        let data = [];
        let labels = []
        let squarevelocities = [];
        for (let i = 0; i < physicsScene.balls.numBalls; i++) {
            squarevelocities.push(vecLengthSquared(physicsScene.balls.vel,i));
        }
        for (let i = 0; i < buckets; i++) {
            data.push(0);
            labels.push(i);
        }
        let bucketdist = Math.max(...squarevelocities)/buckets;
        for (let i = 0; i < squarevelocities.length; i++) {
            let ind = Math.floor(squarevelocities[i] / bucketdist);
            data[ind]+=1;
        }
        chart.data.labels = labels;
        chart.data.datasets[0].data = data;
        chart.update('none');
   
        
    }
    
}

setupScene();
update();

document.getElementById("restitutionSlider").oninput = function() {
physicsScene.restitution = this.value / 10.0;
} 
