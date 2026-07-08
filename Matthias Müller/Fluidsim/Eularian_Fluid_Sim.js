resize();

let cnt = 0;

class Fluid{
	constructor(density, numX, numY, h) {
		this.density = density;
		this.numX = numX + 2;
		this.numY = numY + 2;
		this.numCells = this.numX * this.numY;
		this.h = h;
		

		//erstelle m x n Matrix aus Geschwindigkeitsvektoren
		let u = new Array(this.numY).fill(new Vector2(), 0);
		this.v = new Array(this.numX).fill(u,0);
		
		//Skalarmatrix: s=0 -> obstacle, s=1 -> fluidcell
		u = new Array(this.numY).fill(0, 0);
		this.s = new Array(this.numX).fill(u,0);
		
		//Skalarmatrix
		u = new Array(this.numY).fill(1, 0);
		this.m = new Array(this.numX).fill(u,0);

		let num = numX * numY;
	}

	integrate(dt, gravity){
		let n = this.numY;
		for (let i = 0; i < this.numX; i++) {
			for (let j = 0; j < this.numY-1; j++) {
				if (this.s[i][j] != 0 && this.s[i][j-1] != 0) {
					this.v[i][j].y += gravity * dt;
				}
				
			}
			
		}
	}
}

new Fluid(1,1,1,1);