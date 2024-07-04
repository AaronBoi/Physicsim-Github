var canvas = document.getElementById("myCanvas");
var c = canvas.getContext("2d");


function cX(pos) {
    return pos.x * window.cScale;
}

function cY(pos){
    return canvas.height - pos.y * window.cScale;
}

//scene

//vector math------------------------------

class Vector2 {
    constructor(x = 0.0, y = 0.0) {
        this.x = x; 
        this.y = y;
    }

    set(v) {
        this.x = v.x; this.y = v.y;
    }

    clone() {
        return new Vector2(this.x, this.y);
    }

    add(v, s = 1.0) {
        this.x += v.x * s;
        this.y += v.y * s;
        return this;
    }

    addVectors(a, b) {
        this.x = a.x + b.x;
        this.y = a.y + b.y;
        return this;
    }

    subtract(v, s = 1.0) {
        this.x -= v.x * s;
        this.y -= v.y * s;
        return this;
    }

    subtractVectors(a, b) {
        this.x = a.x - b.x;
        this.y = a.y - b.y;
        return this;			
    }

    length() {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }

    scale(s) {
        this.x *= s;
        this.y *= s;
        return this;
    }

    dot(v) {
        return this.x * v.x + this.y * v.y;
    }
    norm() {
        var l = this.length();
        if(l > 0) {
            this.x = this.x / l;
            this.y = this.y / l;
        }
        return this;
    }
}


class Ball {
    constructor(radius, pos, vel, mass, locked = false){
        this.locked = locked;
        this.radius = radius;
        this.mass = mass;
        this.pos = pos.clone();
        this.vel = vel.clone();
    }
    simulate(gravity, dt) {
        //this.vel.add(gravity, dt);
        if (!this.locked) {
            this.pos.add(this.vel, dt);
        }
    }
    
    addAcc(force, dt) {
        if(!this.locked) {
            this.vel.add(force, dt);
            //this.vel.scale(0.99);
        }
        
    }

}

class Spring {
    constructor(part1, part2, cSpring) {
        this.cSpring = cSpring;
        this.part1 = part1;
        this.part2 = part2;
        this.restLength = new Vector2().subtractVectors(this.part1.pos, this.part2.pos).length();
        this.lastd = this.restLength;
        this.damping = cSpring / 30;
    }
    calcForce(dt) {
        
        var dVect = new Vector2().subtractVectors(this.part1.pos, this.part2.pos);
        
        //Skalare Kräfte berechnen
        var d = dVect.length();
        var x = d - this.restLength;
        var localVel = (d - this.lastd) / dt;
        var springForce = x * this.cSpring;
        var dampForce = localVel * this.damping;
        
        //Aus den Kräften einen Kraftvektor bilden
        var forceVect = dVect.norm().scale(springForce + dampForce);

        //Den Kräfte auf die beiden Teile der Feder anhand deren Masse ausüben
        this.part1.addAcc(forceVect.clone().scale(-1 / this.part1.mass), dt);
        this.part2.addAcc(forceVect.clone().scale(1 / this.part2.mass), dt);

        this.lastd = d;
        
    }
    show() {
        drawLine(this.part1.pos, this.part2.pos, 1 , 'black');
    }
}

var physicsScene = 
{
    cSpring : 100,
    gravity : new Vector2(0.0, -9.81),
    dt : 1.0 / 75,
    //paused: false,
    particles: [],
    springs: [],
};

function setupScene() 
{
    resize();
    var radius = 1.0;
    var mass = 10;
    var pos = new Vector2(window.simWidth / 2, 10);
    var vel = new Vector2(0, 0);

    physicsScene.particles[0] = new Ball(radius, pos, vel, mass);
    physicsScene.particles[1] = new Ball(0.5, new Vector2(window.simWidth / 2, 19), vel, 100000, true);
    physicsScene.springs[0] = new Spring(physicsScene.particles[0], physicsScene.particles[1], physicsScene.cSpring);

}

function resize() {
    canvas.height = window.innerHeight - 100;
    canvas.width = window.innerWidth - 100;
    
    window.simMinWidth = 20.0;
    window.cScale = Math.min(canvas.width, canvas.height) / simMinWidth;
    
    window.simWidth = canvas.width / window.cScale;
    window.simHeight = canvas.height / window.cScale;
    console.log("canvas-Heigth: " + window.simHeight + ", canvas-Width: " + window.simWidth);
}

//drawing----------------------------------

function draw() 
{
    c.clearRect(0, 0, canvas.width, canvas.height);
    c.fillStyle = "#FF0000";

    for (var i = 0; i < physicsScene.springs.length; i++) {
        physicsScene.springs[i].show();
    }

    for (var i = 0; i < physicsScene.particles.length; i++) {
        drawDisc(physicsScene.particles[i]);
    }
    
}


//simulation

var numSteps = 1;
function simulate(){
    var dt = physicsScene.dt / numSteps;
    
    for (var i = 0; i < numSteps; i++) {
        for (var i = 0; i < physicsScene.springs.length; i++) {
            physicsScene.springs[i].calcForce(dt);
        }
        for (var i = 0; i < physicsScene.particles.length; i++) {
            physicsScene.particles[i].simulate(physicsScene.gravity, dt);
        }
        
    }

    for (var i = 0; i < physicsScene.particles.length; i++) {
        //physicsScene.particles[i].vel.scale(0.985);
    }
}

//update------------------------------------
function update(){
    
    simulate();
    draw();

    requestAnimationFrame(update);
}

setupScene();
update();


function drawLine(pos1, pos2, thickness = 2, color = 'black') {
    c.strokeStyle = color;
    c.lineWidth = thickness;
    c.beginPath();
    c.moveTo(cX(pos1), cY(pos1));
    c.lineTo(cX(pos2), cY(pos2));
    c.stroke();
}

function drawDisc(particle)
{
    c.beginPath();			
    c.arc(
        cX(particle.pos), cY(particle.pos), cScale * particle.radius, 0.0, 2.0 * Math.PI); 
    c.closePath();
    c.fill();
}


canvas.addEventListener("mousedown", onMouseDown, false);
canvas.addEventListener("mouseup", onMouseUp, false);
canvas.addEventListener("mousemove", onMouseMove, false);

function onMouseDown(event)
{
    var rect = canvas.getBoundingClientRect();
    var mousePos = new Vector2(
        (event.clientX - rect.left) / cScale,
        simHeight - (event.clientY - rect .top) / cScale);
    physicsScene.particles[0].pos = mousePos;
    physicsScene.particles[0].vel.scale(0);
    physicsScene.particles[0].locked = true;
}

function onMouseUp()
{
    physicsScene.particles[0].locked = false;
}

function onMouseMove(event)
{   
    if(physicsScene.particles[0].locked){
        var rect = canvas.getBoundingClientRect();
        var mousePos = new Vector2(
            (event.clientX - rect.left) / cScale,
            simHeight - (event.clientY - rect .top) / cScale);
        physicsScene.particles[0].vel.scale(0);
        physicsScene.particles[0].pos = mousePos;
    }
}
        

