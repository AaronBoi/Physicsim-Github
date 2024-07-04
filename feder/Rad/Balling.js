window.simMinWidth = 20;

var physicsScene = 
{
    gravity : new Vector2(0.0, -9,81),
    dt : 1.0 / 75,
    restitution : 0.7,
    //paused: false,
    particles: [],
    springs: [],
    wheels: [],
    rect : [],
};

function setupScene() 
{
    resize();
    
    pos = new Vector2(10, 10);

    
    //physicsScene.wheels[0] = new Wheel(pos, 4, 20);
    //physicsScene.wheels[0].init();
    createWheel(pos, 5, 20);
    physicsScene.rect[0] = new NonMovingRect(new Vector2(0, 0), new Vector2(window.simWidth, 0.5), "gray");
    
    
}

//drawing----------------------------------

function draw() 
{
    c.clearRect(0, 0, canvas.width, canvas.height);
    
    /*
    for (let i = 0; i < physicsScene.wheels.length; i++) {
        physicsScene.wheels[i].show();
    }*/
    
    for (let i = 0; i < physicsScene.particles.length; i++) {
        physicsScene.particles[i].show();
        
    }

    for (const spring of physicsScene.springs) {
        spring.show();
    }


    for (let i = 0; i < physicsScene.rect.length; i++) {
        physicsScene.rect[i].show();
        
    }
}




//simulation
var numSteps = 1;
function simulate(){
    var dt = physicsScene.dt / numSteps;
    
    for (const spring of physicsScene.springs) {
        spring.calcForce(dt);
    }
    for (const particle of physicsScene.particles) {
        if (physicsScene.gravity) {
            particle.addAcc(physicsScene.gravity, dt);
        }
        particle.simulate(dt);
        handleWallCollision(particle);
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



