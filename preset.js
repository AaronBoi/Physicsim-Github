let canvas = document.getElementById("myCanvas");
let c = canvas.getContext("2d");
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

    static addVectors(a, b) {
        let x = a.x + b.x;
        let y = a.y + b.y;
        return new Vector2(x, y);
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

    static dot(a, b) {
        let x = a.x * b.x;
        let y = a.y * b.y;
        return x + y;
        //return this.x * v.x + this.y * v.y;
    }
    norm() {
        let l = this.dist();
        if(l != 0) {
            this.x = this.x / l;
            this.y = this.y / l;
        }
        return this;
    }
    perpL() {
        return new Vector2(-this.y, this.x);
    }
    perpR() {
        return new Vector2(-this.y, this.x);
    }
    static cross(a, b) {
        return a.x * b.y - a.y * b.x;
    }
}

function closestPointOnLine(pointA, pointB, target) {

    let a = Vector2.subtractVectors(pointB, pointA);
    let b = Vector2.subtractVectors(target, pointA);

    let ba = a.clone();
    ba.scale(Vector2.dot(ba.norm(), b));
    //make it so the the dist has a direction
    let sign = Math.sign(Vector2.dot(ba, a));
    let dist = ba.dist() * sign;
    //clamp the closest point to the line
    dist = Math.min(Math.max(dist, 0), a.dist());


    return Vector2.addVectors(pointA, a.norm().scale(dist));
}

class NonMovingRect {
    constructor(pos, angle, length, width, color = "black", fill = true, lineWidth = 6) {
        
        this.width = width;
        this.length = length;
        this.angleVect = new Vector2(Math.cos(angle), Math.sin(angle)).norm();
        this.positions = [];
        this.positions.push(pos);
        this.positions.push(Vector2.addVectors(pos, this.angleVect.clone().scale(length)));
        this.positions.push(Vector2.addVectors(this.positions[1], this.angleVect.clone().perpL().scale(width)));
        this.positions.push(Vector2.addVectors(pos, this.angleVect.clone().perpL().scale(width)));
    
        this.color = color;
        this.fill = fill;
        this.lineWidth = lineWidth;
        this.locked = true;
    }
    show() {
        drawPolygon(this.positions, this.color, this.fill, this.lineWidth);
    }

}

class Particle {
    constructor(radius, pos, vel = new Vector2(0, 0), mass = 10, color = "black"){
        this.locked = false;
        this.radius = radius;
        this.mass = mass;
        this.color = color;
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
    collideWithRect(rect, restitution){
        let d = new Vector2();
        let closest = new Vector2();
        let normal = new Vector2();
        let minDist = Number.MAX_VALUE;
        
        for (let i = 0; i < rect.positions.length; i++) {
            let a = rect.positions[i];
            let b =  i + 1 < rect.positions.length ? rect.positions[i + 1] : rect.positions[0];
            let c = closestPointOnLine(a, b, this.pos);
            d = Vector2.subtractVectors(this.pos, c);
            let dist = d.dist();
            
            if (dist < minDist) {
                minDist = dist;
                closest = c;
                normal = Vector2.subtractVectors(a, b).perpL();
            }
        }
        //  -----------push ball out---------------
        //drawDisc(closest, 0.5, "red");
        d = Vector2.subtractVectors(this.pos, closest);
        let dist = d.dist();
        if (dist == 0) { 
            //  if particle is directly on the collision point, point directly outwards
            d = normal.clone();
            dist = normal.dist();
        }
        d.norm();
        if (Vector2.dot(d, normal) >= 0) {
            //  particle is outside of rectangle
            if (dist > this.radius) return;
            this.pos.add(d, this.radius - dist);
        }
        
        else {
            //  particle is inside of rectangle
            this.pos.add(d, -(dist + this.radius));
        }

        //  -----------update velocity---------------
        //d.norm();
        let r = Vector2.subtractVectors(this.vel, d.scale(2 * Vector2.dot(this.vel, d)));
        this.vel = r.scale(restitution);

        return;

    }

    collideWithParticle(particle, restitution)
    {

        let dir = Vector2.subtractVectors(particle.pos, this.pos)
        var d = dir.dist();
        if (d == 0.0 || d >= this.radius + particle.radius)
            return;

        dir.scale(1.0 / d);

        let corr = (this.radius + particle.radius - d) / 2.0;
        this.pos.add(dir, -corr);
        particle.pos.add(dir, corr);

        let v1 = Vector2.dot(this.vel, dir);
        let v2 = Vector2.dot(particle.vel, dir);
        
        var m1 = this.mass;
        var m2 = particle.mass;

        

        var newV1 = (m1 * v1 + m2 * v2 - m2 * (v1 - v2) * restitution) / (m1 + m2);
        var newV2 = (m1 * v1 + m2 * v2 - m1 * (v2 - v1) * restitution) / (m1 + m2);

        this.vel.add(dir, newV1 - v1);
        particle.vel.add(dir, newV2 - v2);
    }

    show() {
        
        drawDisc(this.pos, this.radius, this.color);
    }

}

class LengthConstraint {
    constructor(part1, part2) {
        this.part1 = part1;
        this.part2 = part2;
        this.dist = part2.pos.dist(part1.pos);
    }
    applyConstraint(){
        let dir = Vector2.subtractVectors(this.part2.pos, this.part1.pos);
        let d = dir.dist() - this.dist;
        dir.norm();
        this.part1.pos.add(dir, 0.99 * d);
        this.part2.pos.add(dir, 0.99 *-d);
        //this.part1.pos.add(corr.scale(-1));
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
        
        let dVect = Vector2.subtractVectors(this.part1.pos, this.part2.pos);
        
        //Skalare Kräfte berechnen
        let d = dVect.dist();
        
        let x = d - this.restLength;
        let localVel = (d - this.lastd) / dt;
        let springForce = x * this.cSpring;
        let dampForce = localVel * this.damping;
        
        //Aus den Kräften einen Kraftvektor bilden
        let forceVect = dVect.norm().scale(springForce + dampForce);

        //Den Kräfte auf die beiden Teile der Feder anhand deren Masse ausüben
        this.part1.addAcc(forceVect.clone().scale(-1 / this.part1.mass), dt);
        this.part2.addAcc(forceVect.clone().scale(1 / this.part2.mass), dt);

        this.lastd = d;
        
    }
    show() {
        drawLine(this.part1.pos, this.part2.pos, 1 , 'black');
    }
}

class AngularSpring {
    constructor(part1, part2, part3, cSpring) {
        this.cSpring = cSpring /50;
        this.part1 = part1;
        this.part2 = part2;
        this.part3 = part3;
        this.restAngle = getAngle(this.part1.pos, this.part2.pos, this.part3.pos);
        this.lastA = this.restAngle;
        
    }
    calcForce(dt)
    {
        let angle = getAngle(this.part1.pos, this.part2.pos, this.part3.pos);
        let force = Math.pow((this.restAngle - angle),1) * this.cSpring;
        let v1 = Vector2.subtractVectors(this.part1.pos, this.part2.pos);
        let v3 = Vector2.subtractVectors(this.part3.pos, this.part2.pos);
        let p1 = v1.clone().perpL().norm();
        let p3 = v3.clone().perpL().norm();
        let f1 = p1.clone().scale(-force * v1.dist());
        let f3 = p3.clone().scale(force * v3.dist());
        let fRes = Vector2.addVectors(f1, f3).scale(-1);
        
        this.part1.addAcc(f1, dt);
        this.part3.addAcc(f3, dt);
        this.part2.addAcc(fRes, dt);
        //drawDisc(Vector2.addVectors(this.part1.pos, f1), .1);
        //drawDisc(Vector2.addVectors(this.part3.pos, f3), .1);
    }
}

function createWheel(pos, radius, numPtc, stiffness) {

    let ptcRad = 0.001;
    let ptcInnitVel = new Vector2(0.0, 0.0);
    let ptcMass = 5;
    let cSpring = stiffness;
    let ptcs = [];

    ptcs.push(new Particle(ptcRad, pos, ptcInnitVel, ptcMass));

    for (let i = 0; i < numPtc; i++) {
        let xOffset = Math.sin(i / numPtc * 2 * Math.PI) * radius;
        let yOffset = Math.cos(i / numPtc * 2 * Math.PI) * radius;
        let circlePos = new Vector2(xOffset, yOffset).add(pos);
        ptcs.push(new Particle(ptcRad, circlePos, ptcInnitVel, ptcMass));
    }
    
    //Add Springs
    for (let i = 0; i < ptcs.length; i++) {        
        for (let j = i; j < ptcs.length; j++) {
            
            physicsScene.springs.push(new Spring(ptcs[i], ptcs[j], cSpring)); 
            
        }
        
    }

    physicsScene.particles.push(...ptcs);
}

function createLine(startPos, endPos, numPtc, stiffness) {

    let ptcRad = 0.05;
    let ptcInnitVel = new Vector2(0.0, 0.0);
    let ptcMass = 5;
    let ptcs = [];

    let dir = Vector2.subtractVectors(endPos, startPos);
    let dist = dir.dist() / numPtc;
    dir = dir.norm();
    
    for (let i = 0; i < numPtc; i++) {
        let ptcPos = startPos.clone().add(dir, dist * i);
        ptcs.push(new Particle(ptcRad, ptcPos, ptcInnitVel, ptcMass));
    }
    
    for (let i = 0; i < ptcs.length; i++) {        
        for (let j = i; j < ptcs.length; j++) {
            
            physicsScene.springs.push(new Spring(ptcs[i], ptcs[j], stiffness)); 
            
        }
        
    }

    physicsScene.particles.push(...ptcs);
}

function createWheel2(pos, radius, numPtc, stiffness) {

    let ptcRad = 0.1;
    let ptcInnitVel = new Vector2(0.0, 0.0);
    let ptcMass = 5;
    let cSpring = stiffness;
    let ptcs = [];
    let inner = [];
    let outer = [];

    for (let i = 0; i < numPtc; i++) {
        let xOffset = Math.sin(i / numPtc * 2 * Math.PI) * radius;
        let yOffset = Math.cos(i / numPtc * 2 * Math.PI) * radius;
        let circlePos = new Vector2(xOffset, yOffset).add(pos);
        outer.push(new Particle(ptcRad, circlePos, ptcInnitVel, ptcMass));
        circlePos = new Vector2(0.7 * xOffset, 0.7 * yOffset).add(pos);
        inner.push(new Particle(ptcRad, circlePos, ptcInnitVel, ptcMass));
    }
    ptcs.push(new Particle(ptcRad, pos, ptcInnitVel, ptcMass));

    for (let i = 0; i < inner.length; i++) {
        physicsScene.constraints.push(new LengthConstraint(inner[i], ptcs[0]));
        physicsScene.springs.push(new Spring(inner[i], outer[i], cSpring));
        if (i < inner.length -1 ) {
            physicsScene.constraints.push(new LengthConstraint(inner[i], inner[i+1]));
            physicsScene.springs.push(new Spring(outer[i], outer[i+1], cSpring));  
        }
        else {
            physicsScene.constraints.push(new LengthConstraint(inner[i], inner[0]));
            physicsScene.springs.push(new Spring(outer[i], outer[0], cSpring));   
        }
    }

    physicsScene.particles.push(...ptcs);
    physicsScene.particles.push(...inner);
    physicsScene.particles.push(...outer);


}

//approach with angular springs
function createWheel3(pos, radius, numPtc, stiffness) {

    let ptcRad = 0.07;
    let ptcInnitVel = new Vector2(0.0, 0.0);
    let ptcMass = 5;
    let cSpring = stiffness;
    let ptcs = [];

    //ptcs.push(new Particle(ptcRad, pos, ptcInnitVel, ptcMass));

    for (let i = 0; i < numPtc; i++) {
        let xOffset = Math.sin(i / numPtc * 2 * Math.PI) * radius;
        let yOffset = Math.cos(i / numPtc * 2 * Math.PI) * radius;
        let circlePos = new Vector2(xOffset, yOffset).add(pos);
        ptcs.push(new Particle(ptcRad, circlePos, ptcInnitVel, ptcMass));
    }
    
    //Add Springs
    for (let i = 0; i < ptcs.length - 1; i++) {     
        
        physicsScene.springs.push(new Spring(ptcs[i], ptcs[i + 1], cSpring)); 
        if(i == ptcs.length - 2) {
            physicsScene.springs.push(new Spring(ptcs[i + 1], ptcs[0], cSpring)); 
            continue;
        }
        
    }

    //Add AngularSprings
    for (let i = 0; i < ptcs.length - 2; i++) {     
              
        physicsScene.angularSprings.push(new AngularSpring(ptcs[i], ptcs[i + 1], ptcs[i + 2], cSpring));

        if(i == ptcs.length - 3) {
            physicsScene.angularSprings.push(new AngularSpring(ptcs[i], ptcs[i + 1], ptcs[0], cSpring));
       
        }

        if(i == ptcs.length - 2) {
            physicsScene.angularSprings.push(new AngularSpring(ptcs[i], ptcs[0], ptcs[1], cSpring));
           
        }
        
    }

    physicsScene.particles.push(...ptcs);
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

function resize() {
    canvas.height = window.innerHeight - 100;
    canvas.width = window.innerWidth - 100;

    window.cScale = Math.min(canvas.width, canvas.height) / simMinWidth;
    
    window.simWidth = canvas.width / window.cScale;
    window.simHeight = canvas.height / window.cScale;
    console.log("canvas-Heigth: " + window.simHeight + ", canvas-Width: " + window.simWidth);
}

function getAvarage(arr){
    let sum = 0;
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

function drawDisc(pos, radius, color = "black")
{
    c.fillStyle = color;
    c.beginPath();			
    c.arc(
        cX(pos), cY(pos), cScale * radius, 0.0, 2.0 * Math.PI); 
    c.closePath();
    c.fill();
}

function drawPolygon(positions, color = 'black', fill = true, thickness = 2) 
{
    c.strokeStyle = color;
    c.lineWidth = thickness;
    c.beginPath();
    //c.moveTo(cX(positions[0]), cY(positions[0]));

    for (const pos of positions) {
        c.lineTo(cX(pos), cY(pos));
    }

    c.closePath();
    if (fill) {
        c.fillStyle = color;
        c.fill();
    } 
    else {
        c.strokeStyle = color;
        c.lineWidth = thickness;
        c.stroke();
    }
}


function getAngle(p1, p2, p3)
{
    let v1 = Vector2.subtractVectors(p2, p1);
    let v2 = Vector2.subtractVectors(p2, p3);
    let deg = toDegree(Math.atan2(v2.y*v1.x-v2.x*v1.y,Vector2.dot(v1,v2)))
    if (Math.sign(deg) == -1) {
        deg = deg + 360;
    }
    return deg;
    let sign = Math.sign(Math.asin(Vector2.cross(v1, v2)/(v1.dist()*v2.dist())));
    if (!sign) sign = 1;
    return toDegree(Math.acos(Vector2.dot(v1, v2)/(v1.dist()*v2.dist()))) //* sign;
    
}



let toDegree = radiant => radiant * 180 / Math.PI;
let toRadiant = degree => degree * Math.PI / 180;

canvas.addEventListener("mousedown", onMouseDown, false);
canvas.addEventListener("mouseup", onMouseUp, false);
canvas.addEventListener("mousemove", onMouseMove, false);

let heldParticle = null;

function onMouseDown(event)
{   
    
    let rect = canvas.getBoundingClientRect();
    let mousePos = new Vector2(
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

    if (heldParticle) {
        heldParticle.locked = false;
        heldParticle.color = "black";
    }
    
    heldParticle = null;
    
}

function onMouseMove(event)
{   
    if (heldParticle){
        let rect = canvas.getBoundingClientRect();
        let mousePos = new Vector2(
            (event.clientX - rect.left) / cScale,
            simHeight - (event.clientY - rect .top) / cScale);
        heldParticle.pos = mousePos;
        heldParticle.vel.scale(0);
        heldParticle.color = "red";
        
    }
}

//scuffed
function followMouse(mousePos) {
    if (heldParticle && mousePos) {
        let dir = Vector2.subtractVectors(mousePos, heldParticle.pos);
        heldParticle.addAcc(dir, physicsScene.dt * 10);  
        requestAnimationFrame(followMouse(mousePos));
        heldParticle.color = "red";
    }
}
        

