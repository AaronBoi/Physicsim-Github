var canvas = document.getElementById("myCanvas");
var c = canvas.getContext("2d");
window.simMinWidth = 20.0;

function cX(pos) {
    return pos.x * window.cScale;
}

function cY(pos){
    return canvas.height - pos.y * window.cScale;
}

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

    static subtractVectors(a, b) {
        
        let x = a.x - b.x;
        let y = a.y - b.y;
        return new Vector2(x, y);			
    }

    dist(v = new Vector2(0, 0)) {
        let xdiff = this.x - v.x;
        let ydiff = this.y - v.y;
        return Math.sqrt(xdiff * xdiff + ydiff * ydiff);
        
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
        var l = this.dist();
        if(l != 0) {
            this.x = this.x / l;
            this.y = this.y / l;
        }
        return this;
    }
}



class StaticParticle {

}

class DynamicParticle {

}
class NonMovingDisk {

}

class NonMovingRect {
    constructor(pos1, pos2, color = "black", fill = true, lineWidth = 6) {
        this.pos1 = pos1;
        this.pos2 = pos2;
        this.color = color;
        this.fill = fill;
        this.lineWidth = lineWidth;
    }
    show() {
        c.beginPath();
        c.rect(cX(this.pos1), cY(this.pos1), cX(this.pos2) - cX(this.pos1), cY(this.pos2) - cY(this.pos1));  

        if (this.fill) {
            c.fillStyle = this.color;
            c.fill();
        } 
        else {
            c.strokeStyle = this.color;
            c.lineWidth = this.lineWidth;
            c.stroke();
        }
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
        if (!this.locked) {
            this.pos.add(this.vel, dt);
        }
    }
    addAcc(acc, dt) {
        if(!this.locked) {
            this.vel.add(acc, dt);
        }
    }
    isInside(pos) {
        
        return this.dist(pos) < this.radius;
            
        
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
        this.restLength = this.part1.pos.dist(this.part2.pos);
        this.lastd = this.restLength;
        this.damping = cSpring / 120;
    }
    calcForce(dt) {
        
        var dVect = Vector2.subtractVectors(this.part1.pos, this.part2.pos);
        
        //Skalare Kräfte berechnen
        var d = dVect.dist();
        
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

function createWheel(pos, radius, numPtc) {

    let ptcRad = 0.01;
    let ptcInnitVel = new Vector2(0.0, 0.0);
    let ptcMass = 5;
    let cSpring = 1500;
    let ptcs = [];

    for (let i = 0; i < numPtc; i++) {
        var xOffset = Math.sin(i / numPtc * 2 * Math.PI) * radius;
        var yOffset = Math.cos(i / numPtc * 2 * Math.PI) * radius;
        var circlePos = new Vector2(xOffset, yOffset).add(pos);
        ptcs.push(new Particle(ptcRad, circlePos, ptcInnitVel, ptcMass));
    }
    ptcs.push(new Particle(ptcRad, pos, ptcInnitVel, ptcMass));


    for (let i = 0; i < ptcs.length; i++) {        
        for (let j = i; j < ptcs.length; j++) {
            
            physicsScene.springs.push(new Spring(ptcs[i], ptcs[j], cSpring)); 
        
        }
        
    }

    physicsScene.particles.push(...ptcs);


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

function resize() {
    canvas.height = window.innerHeight - 100;
    canvas.width = window.innerWidth - 100;

    window.cScale = Math.min(canvas.width, canvas.height) / simMinWidth;
    
    window.simWidth = canvas.width / window.cScale;
    window.simHeight = canvas.height / window.cScale;
    console.log("canvas-Heigth: " + window.simHeight + ", canvas-Width: " + window.simWidth);
}

function getAvarage(arr){
    var sum = 0;
    for (let i = 0; i < arr.length; i++) {
        sum += arr[i];
        
    }
    return sum / arr.length;
}

//drawing----------------------------------



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

let heldParticle = null;

function onMouseDown(event)
{   
    
    var rect = canvas.getBoundingClientRect();
    var mousePos = new Vector2(
        (event.clientX - rect.left) / cScale,
        simHeight - (event.clientY - rect .top) / cScale);
    let closest = 2;
    for (const particle of physicsScene.particles) {
        let dist = particle.pos.dist(mousePos);
        if (dist < closest) {
            
            closest = dist;
            heldParticle = particle;
        }
        
    }
    if (heldParticle) heldParticle.locked = true;
}

function onMouseUp()
{

    if (heldParticle) heldParticle.locked = false;
    heldParticle = null;
    
}

function onMouseMove(event)
{   
    if(heldParticle){
        var rect = canvas.getBoundingClientRect();
        var mousePos = new Vector2(
            (event.clientX - rect.left) / cScale,
            simHeight - (event.clientY - rect .top) / cScale);
        heldParticle.pos = mousePos;
        heldParticle.vel.scale(0);
    }
}
        

