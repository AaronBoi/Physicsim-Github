window.simMinWidth = 20;

var physicsScene = 
{
    gravity : new Vector2(0.0, -9,81),
    dt : 1.0 / 75,
    restitution : 0.1,
    //paused: false,
    particles: [],
    springs: [],
    wheels: [],
    rect : [],
    constraints : [],
};

function setupScene() 
{
    resize();
    
    pos = new Vector2(10, 10);

    //physicsScene.particles.push(new Particle(2, new Vector2(5, 10)));
    //physicsScene.particles.push(new Particle(2, new Vector2(15, 10)));
    //physicsScene.constraints.push(new LengthConstraint(physicsScene.particles[0], physicsScene.particles[1]));



    createWheel(pos, 2, 20, 2000);
    pos.x  +=5;
    createWheel(pos, 2, 20, 2000);
    //physicsScene.constraints.push(new LengthConstraint(physicsScene.particles[0], physicsScene.particles[21]));
    //createWheel2(pos, 1, 20, 100);

    physicsScene.rect.push(new NonMovingRect(new Vector2(0, 0), toRadiant(0), window.simWidth, 1, "gray"));
    physicsScene.rect.push(new NonMovingRect(new Vector2(0, 0), toRadiant(0), 1, window.simHeight, "gray"));
    physicsScene.rect.push(new NonMovingRect(new Vector2(0, window.simHeight - 1), toRadiant(0), window.simWidth, 1, "gray"));
    physicsScene.rect.push(new NonMovingRect(new Vector2(window.simWidth - 1, 0), toRadiant(0), 1, window.simHeight, "gray"));
    physicsScene.rect.push(new NonMovingRect(new Vector2(0, 0), toRadiant(5), window.simWidth, 1, "gray"));
    physicsScene.rect.push(new NonMovingRect(new Vector2(0, 0), toRadiant(60), 10, 1, "gray"));
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
            constraint.applyConstraint();
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
            //for (let j = 0; j < physicsScene.particles.length; j++) {
            //    particle.collideWithParticle(physicsScene.particles[j]);
                
            //}
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


    requestAnimationFrame(update);
}

setupScene();
update();


//Useful Functions



