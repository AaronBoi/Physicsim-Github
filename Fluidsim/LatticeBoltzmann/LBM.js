var physicsScene =
{
	
	gravity: new THREE.Vector3(0.0, -10, 0.0),
	dt: 1.0 / 30,
	worldSize : { x: 1.5, z : 2.5 },
	paused: true,
	balls : [],
};

let dt = 1.0 / 60;
let width = 20;
let heigth = 10;
let gridsize = 0.06;

let c = gridsize / dt;
let w = 1; //
/*
let n0, nE, nN, nW, nS, nN, nN, nS, nSE;
function init(width, heigth)
{
	
	n0 = zeros([width, heigth]);
	nE = zeros([width, heigth]);
	nN = zeros([width, heigth]);
	nW = zeros([width, heigth]);
	nS = zeros([width, heigth]);
	nNE = zeros([width, heigth]);
	nNW = zeros([width, heigth]);
	nSW = zeros([width, heigth]);
	nSE = zeros([width, heigth]);
}
*/


let n_arr;	//2D array in which cells are densities of the 9 vectors to neighbor cells.
let w_arr;	//Boltzmann Distribution weights for the neighbor vectors.

let e_arr = [[0, 0], [1, 0], [0, 1], [-1, 0], [0, -1], [1, 1], [-1, 1], [-1, -1], [1, -1]];

function init(width, heigth)
{
	
	n_arr = [];
	dims = 9;  //n0, nE, nN, nW, nS, nN, nN, nS, nSE
	w_arr = [4.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/36, 1.0/36, 1.0/36, 1.0/36];
	n_empty = [];
	for (let i = 0; i < dims; i++) {
		n_empty.push(0.0);
	}
	for (let x = 0; x < width; x++) {
		n_arr.push([]);
		for (let y = 0; y < heigth; y++) {
			n_arr[x][y] = n_empty;
		}
	}
	n_arr[0][0] = [0.5,0,0,0,0,0,0,0,0];
}

function collision()
{	
	
	for (let x = 0; x < width; x++) {
		for (let y = 0; y < heigth; y++) {
			let n_vec = n_arr[x][y];
			let rho = 0;
			let u_x_sq = 0;
			let u_y_sq = 0;
			for (let i = 0; i < dims; i++) {
				rho += n_vec[i];
				u_x_sq += Math.pow(e_arr[i][0] * c, 2) * w_arr[i];
				u_y_sq += Math.pow(e_arr[i][1] * c, 2) * w_arr[i];
			}
			let u = [Math.sqrt(u_x_sq), Math.sqrt(u_y_sq)];

			let n_new = [];
			for (let i = 0; i < dims; i++) {
				
				let dot = dot_product(e_arr[i], u);
				let temp = 1 + 3 * dot + 9.0 / 2 * Math.pow(dot, 2) - 3.0 / 2 * (u_x_sq + u_y_sq);
				let n_eq = rho * w_arr[i] * temp;
				//n_eq.push(rho * w_arr[i] * temp2);
				n_new.push(n_vec[i] + w * (n_eq - n_vec[i]));

			}
			n_arr[x][y] = n_new.slice();
			
		}
		
	}
}
 

init(width, heigth);
collision();
console.log(n_arr[0][0][0]);


function zeros(dimensions) {
    var array = [];

    for (var i = 0; i < dimensions[0]; ++i) {
        array.push(dimensions.length == 1 ? 0 : zeros(dimensions.slice(1)));
    }

    return array;
}

function dot_product(arr1, arr2)
{
	if (arr1.length != arr2.length)
	{
		throw new Error("Arrays are not of same length!");
	}
	let sum = 0;
	for (let i = 0; i < arr1.length; i++) {
		sum += arr1[i] * arr2[i];
	}
	return sum;
}