window.simMinWidth = 20;

var physicsScene = 
{
    gravity : new Vector2(0.0, -9,81),
    dt : 1.0 / 75,
    restitution : 0.9,
    //paused: false,
    particles: [],
    springs: [],
    angularSprings: [],
    wheels: [],
    rect : [],
    constraints : [],
};

function setupScene() 
{
    resize();
    cSpring = 1000;
    pos1 = new Vector2(13, 5);
    pos2 = new Vector2(15, 10);
    pos3 = new Vector2(17,5);
    physicsScene.particles.push(new Particle(0.5, pos1));
    physicsScene.particles.push(new Particle(.5, pos2));
    physicsScene.particles.push(new Particle(.5, pos3));
    physicsScene.springs.push(new Spring(physicsScene.particles[0], physicsScene.particles[1], cSpring)); 
    physicsScene.springs.push(new Spring(physicsScene.particles[1], physicsScene.particles[2], cSpring)); 
    physicsScene.angularSprings.push(new AngularSpring(physicsScene.particles[0], physicsScene.particles[1], physicsScene.particles[2], cSpring/10));

    physicsScene.rect.push(new NonMovingRect(new Vector2(0, 0), toRadiant(0), window.simWidth, 1, "gray"));

    
    
    //physicsScene.constraints.push(new LengthConstraint(physicsScene.particles[0], physicsScene.particles[21]));
    //createWheel2(pos, 1, 20, 100);

    console.log(physicsScene.particles)
}

//drawing----------------------------------

function draw() 
{


    for (let i = 0; i < physicsScene.rect.length; i++) {
        physicsScene.rect[i].show();
        
    }
    for (let i = 0; i < physicsScene.particles.length; i++) {
        physicsScene.particles[i].show();
        
    }

    for (const spring of physicsScene.springs) {
        spring.show();
    }

}


//simulation
var numSteps = 10;
function simulate(){

    c.clearRect(0, 0, canvas.width, canvas.height);
    var dt = physicsScene.dt / numSteps;
    for (let i = 0; i < numSteps; i++) {
        
        for (const constraint of physicsScene.constraints) {
            //constraint.applyConstraint();
        }
        
        for (const spring of physicsScene.springs) {
            spring.calcForce(dt);
        }
        for (const particle of physicsScene.particles) {
            if (physicsScene.gravity) {
                particle.addAcc(physicsScene.gravity, dt);
            }
            particle.simulate(dt);

            for (let j = 0; j < physicsScene.rect.length; j++) {
                particle.collideWithRect(physicsScene.rect[j], physicsScene.restitution);
            }
            for (let j = 0; j < physicsScene.particles.length; j++) {
                //particle.collideWithParticle(physicsScene.particles[j]);
                
            }
        }
        for (const aSpring of physicsScene.angularSprings) {
            aSpring.calcForce(dt);
        }
        
    }
    

}


//update------------------------------------
var timeArr = [];
var frameCount = 0;
var startTime = 0;
var endTime = 0;
function update(){
    frameCount++;
    startTime = performance.now();
    simulate();
    draw();

    endTime = performance.now();
    timeArr.unshift(endTime - startTime);
    
    if (timeArr.length > 100) {
        timeArr.pop();
    }
    if (frameCount % 100 == 0) {
        document.getElementById("Performance").innerHTML = getAvarage(timeArr);
    }
    
      //console.log(getAngle(physicsScene.particles[0].pos,physicsScene.particles[1].pos,physicsScene.particles[2].pos));

    requestAnimationFrame(update);
}

setupScene();
update();


//Useful Functions



