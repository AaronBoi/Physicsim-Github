//canvas setup-----------------------------

var canvas = document.getElementById("myCanvas");
var c = canvas.getContext("2d");

function cX(pos) {
    return pos.x * window.cScale;
}
function cY(pos){
    return canvas.height - pos.y * window.cScale;
}

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
        if(l != 0) {
            this.x = this.x / l;
            this.y = this.y / l;
        }
        return this;
    }
}

class Particle {
    constructor(radius, pos, vel, mass){
        this.locked = false;
        this.radius = radius;
        this.mass = mass;
        this.pos = pos.clone();
        this.vel = vel.clone();
    }
    simulate(dt) {
        this.vel.add(physicsScene.gravity, dt);
        if (!this.locked) {
            this.pos.add(this.vel, dt);
        }
        //this.vel.scale(0.99);
    }
    addAcc(acc, dt) {
        if(!this.locked) {
            this.vel.add(acc, dt);
        }
    }
    show() {
        drawDisc(this);
    }

}

class Spring {
    constructor(part1, part2, cSpring) {
        this.cSpring = cSpring;
        this.part1 = part1;
        this.part2 = part2;
        this.restLength = new Vector2().subtractVectors(this.part1.pos, this.part2.pos).length();
        this.lastd = this.restLength;
        this.damping = cSpring / 120;
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

class Wheel {
    constructor(pos, radius, numPtc) {
    
        this.radius = radius;
        this.numPtc = numPtc;

        this.ptcs = [];
        this.ptcRad = 0.01;
        this.ptcInnitVel = new Vector2(0.0, 0.0);
        this.ptcMass = 5;
        this.ptcsCenter = new Particle(this.ptcRad, pos, this.ptcInnitVel, this.ptcMass);
        
        this.springs = [];
        this.cSpring = 200;

    
    }
    init() {
        this.spawnParticles();
        this.spawnSprings();
    }

    draw(){
        this.showParticles();
        this.showSprings();
    }
    spawnParticles() {
        //wheel center
        //this.ptcs.push(new Particle(this.ptcRad, this.pos, this.ptcInnitVel, this.mass));

        //wheel circumference
        for (let i = 0; i < this.numPtc; i++) {
            var xOffset = Math.sin(i / this.numPtc * 2 * Math.PI) * this.radius;
            var yOffset = Math.cos(i / this.numPtc * 2 * Math.PI) * this.radius;
            var pos = new Vector2().addVectors(new Vector2(xOffset, yOffset), this.ptcsCenter.pos);
            this.ptcs.push(new Particle(this.ptcRad, pos, this.ptcInnitVel, this.ptcMass));
        }
    }

    spawnSprings() {

         
        //Spawn every spring connection
        for (let i = 0; i < this.ptcs.length; i++) {        
            for (let j = i; j < this.ptcs.length; j++) {
                
                this.springs.push(new Spring(this.ptcs[i], this.ptcs[j], this.cSpring)); 
            
            }
            this.springs.push(new Spring(this.ptcs[i], this.ptcsCenter, this.cSpring));
            
        }
        

        /*
        //custom stuff
        for (let i = 0; i < this.ptcs.length; i++) {  
            //Inner
            this.springs.push(new Spring(this.ptcs[i], this.ptcsCenter, this.cSpring));
            //Outer
            if (i < this.ptcs.length - 1) this.springs.push(new Spring(this.ptcs[i], this.ptcs[i + 1], this.cSpring));
            else this.springs.push(new Spring(this.ptcs[i], this.ptcs[0], this.cSpring));
            

            var rotate = i + Math.floor(this.ptcs.length / 3);
            if (rotate > this.ptcs.length - 1) {
                rotate -= this.ptcs.length;
            }
            this.springs.push(new Spring(this.ptcs[i], this.ptcs[rotate], this.cSpring));

            var rotate = i + Math.floor(this.ptcs.length * 2/3);
            if (rotate > this.ptcs.length - 1) {
                rotate -= this.ptcs.length;
            }
            this.springs.push(new Spring(this.ptcs[i], this.ptcs[rotate], this.cSpring));
            
        }*/
    }

    showParticles() {
        
        for (let i = 0; i < this.ptcs.length; i++) {
            this.ptcs[i].show();  
        }
        this.ptcsCenter.show();
    }
    showSprings() {
        for (let i = 0; i < this.springs.length; i++) {
            this.springs[i].show();
            
        }
    }

    simulate(dt) {
        
        for (let i = 0; i < this.springs.length; i++) {
            this.springs[i].calcForce(dt);
        }

        for (let i = 0; i < this.ptcs.length; i++) {
            this.ptcs[i].simulate(dt);
        }
        this.ptcsCenter.simulate(dt);

        this.borderCollision();

        //console.log(this.ptcs);
    }

    borderCollision(){

        for (let i = 0; i < this.ptcs.length; i++) {
            handleWallCollision(this.ptcs[i]);
        }
    }
}



var physicsScene = 
{
    //gravity : new Vector2(0.0, -9.81),
    gravity : new Vector2(0.0, -9,81),
    dt : 1.0 / 75,
    restitution : 0.7,
    //paused: false,
    particles: [],
    springs: [],
    wheels: []
};

function setupScene() 
{
    resize();
    



    pos = new Vector2(10, 10);

    physicsScene.wheels[0] = new Wheel(pos, 4, 20);
    physicsScene.wheels[0].init();
    
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
    
    for (let i = 0; i < physicsScene.wheels.length; i++) {
        physicsScene.wheels[i].draw();
        
    }
    
    
}

function handleWallCollision(ball) 
{
    if (ball.pos.x <= ball.radius) {
        ball.pos.x = ball.radius;
        ball.vel.x = -ball.vel.x * physicsScene.restitution;
    }
    if (ball.pos.x >= window.simWidth - ball.radius) {
        ball.pos.x = window.simWidth - ball.radius;
        ball.vel.x = -ball.vel.x * physicsScene.restitution;
    }
    if (ball.pos.y <= ball.radius) {
        ball.pos.y = ball.radius;
        ball.vel.y = -ball.vel.y * physicsScene.restitution;
    }

    if (ball.pos.y >= window.simHeight - ball.radius) {
        ball.pos.y = window.simHeight - ball.radius;
        ball.vel.y = -ball.vel.y * physicsScene.restitution;
    }
}


//simulation
var numSteps = 1;
function simulate(){
    var dt = physicsScene.dt / numSteps;

    physicsScene.wheels[0].simulate(dt);
}

//update------------------------------------

function update(){
    simulate();
    draw();

    requestAnimationFrame(update);
}

setupScene();
update();


//Useful Functions


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
    c.fillStyle = "#FF0000";
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
    physicsScene.wheels[0].ptcsCenter.pos = mousePos;
    physicsScene.wheels[0].ptcsCenter.locked = true;
}

function onMouseUp()
{
    physicsScene.wheels[0].ptcsCenter.locked = false;
}

function onMouseMove(event)
{   
    if(physicsScene.wheels[0].ptcsCenter.locked){
        var rect = canvas.getBoundingClientRect();
        var mousePos = new Vector2(
            (event.clientX - rect.left) / cScale,
            simHeight - (event.clientY - rect .top) / cScale);
        physicsScene.wheels[0].ptcsCenter.pos = mousePos;
    }
}

