/***********************************************************
Starter code for Assignment 3

This code was originally written by Jack Wang for
CSC418, SPRING 2005

Implementations of functions in raytracer.h, 
and the main function which specifies the 
scene to be rendered.	

***********************************************************/


#include "raytracer.h"
#include "bmp_io.h"
#include <cmath>
#include <iostream>
#include <cstdlib>

/***********************************************************
//Change mode to 1 for Part1 Phong model 
//Change mode to 2 for Part2 Advanced Ray Tracing
***********************************************************/

int mode =1;


/***********************************************************/
//rgb buffer, width and length of output
unsigned char* rbuffer; 
unsigned char* gbuffer; 
unsigned char* bbuffer; 
int width = 320; 
int height = 240; 

//width and length of earth map
int texture_width = 320;
int texture_height = 240;

// rendering variables defined ahead
int light_sample_num ;
int motion_num ; 
int anti_aliasing_num ;
int samples_num ;
/***********************************************************/



Raytracer::Raytracer() : _lightSource(NULL) {
	_root = new SceneDagNode();
}

Raytracer::~Raytracer() {
	delete _root;
}

SceneDagNode* Raytracer::addObject( SceneDagNode* parent, 
	SceneObject* obj, Material* mat ) {
	SceneDagNode* node = new SceneDagNode( obj, mat );
	node->parent = parent;
	node->next = NULL;
	node->child = NULL;

// Add the object to the parent's child list, this means
// whatever transformation applied to the parent will also
// be applied to the child.
	if (parent->child == NULL) {
		parent->child = node;
	}
	else {
		parent = parent->child;
		while (parent->next != NULL) {
			parent = parent->next;
		}
		parent->next = node;
	}

	return node;;
}

LightListNode* Raytracer::addLightSource( LightSource* light ) {
	LightListNode* tmp = _lightSource;
	_lightSource = new LightListNode( light, tmp );
	return _lightSource;
}

void Raytracer::rotate( SceneDagNode* node, char axis, double angle ) {
	Matrix4x4 rotation;
	double toRadian = 2*M_PI/360.0;
	int i;

	for (i = 0; i < 2; i++) {
		switch(axis) {
			case 'x':
			rotation[0][0] = 1;
			rotation[1][1] = cos(angle*toRadian);
			rotation[1][2] = -sin(angle*toRadian);
			rotation[2][1] = sin(angle*toRadian);
			rotation[2][2] = cos(angle*toRadian);
			rotation[3][3] = 1;
			break;
			case 'y':
			rotation[0][0] = cos(angle*toRadian);
			rotation[0][2] = sin(angle*toRadian);
			rotation[1][1] = 1;
			rotation[2][0] = -sin(angle*toRadian);
			rotation[2][2] = cos(angle*toRadian);
			rotation[3][3] = 1;
			break;
			case 'z':
			rotation[0][0] = cos(angle*toRadian);
			rotation[0][1] = -sin(angle*toRadian);
			rotation[1][0] = sin(angle*toRadian);
			rotation[1][1] = cos(angle*toRadian);
			rotation[2][2] = 1;
			rotation[3][3] = 1;
			break;
		}
		if (i == 0) {
			node->trans = node->trans*rotation; 	
			angle = -angle;
		} 
		else {
			node->invtrans = rotation*node->invtrans; 
		}	
	}
}

void Raytracer::translate( SceneDagNode* node, Vector3D trans ) {
	Matrix4x4 translation;

	translation[0][3] = trans[0];
	translation[1][3] = trans[1];
	translation[2][3] = trans[2];
	node->trans = node->trans*translation; 	
	translation[0][3] = -trans[0];
	translation[1][3] = -trans[1];
	translation[2][3] = -trans[2];
	node->invtrans = translation*node->invtrans; 
}

void Raytracer::scale( SceneDagNode* node, Point3D origin, double factor[3] ) {
	Matrix4x4 scale;

	scale[0][0] = factor[0];
	scale[0][3] = origin[0] - factor[0] * origin[0];
	scale[1][1] = factor[1];
	scale[1][3] = origin[1] - factor[1] * origin[1];
	scale[2][2] = factor[2];
	scale[2][3] = origin[2] - factor[2] * origin[2];
	node->trans = node->trans*scale; 	
	scale[0][0] = 1/factor[0];
	scale[0][3] = origin[0] - 1/factor[0] * origin[0];
	scale[1][1] = 1/factor[1];
	scale[1][3] = origin[1] - 1/factor[1] * origin[1];
	scale[2][2] = 1/factor[2];
	scale[2][3] = origin[2] - 1/factor[2] * origin[2];
	node->invtrans = scale*node->invtrans; 
}

Matrix4x4 Raytracer::initInvViewMatrix( Point3D eye, Vector3D view, 
	Vector3D up ) {
	Matrix4x4 mat; 
	Vector3D w;
	view.normalize();
	up = up - up.dot(view)*view;
	up.normalize();
	w = view.cross(up);

	mat[0][0] = w[0];
	mat[1][0] = w[1];
	mat[2][0] = w[2];
	mat[0][1] = up[0];
	mat[1][1] = up[1];
	mat[2][1] = up[2];
	mat[0][2] = -view[0];
	mat[1][2] = -view[1];
	mat[2][2] = -view[2];
	mat[0][3] = eye[0];
	mat[1][3] = eye[1];
	mat[2][3] = eye[2];

	return mat; 
}

void Raytracer::traverseScene( SceneDagNode* node, Ray3D& ray ) {
	SceneDagNode *childPtr;

// Applies transformation of the current node to the global
// transformation matrices.
	_modelToWorld = _modelToWorld*node->trans;
	_worldToModel = node->invtrans*_worldToModel; 
	if (node->obj) {
// Perform intersection.
		if (node->obj->intersect(ray, _worldToModel, _modelToWorld)) {
			ray.intersection.mat = node->mat;
		}
	}
// Traverse the children.
	childPtr = node->child;
	while (childPtr != NULL) {
		traverseScene(childPtr, ray);
		childPtr = childPtr->next;
	}

// Removes transformation of the current node from the global
// transformation matrices.
	_worldToModel = node->trans*_worldToModel;
	_modelToWorld = _modelToWorld*node->invtrans;
}

void Raytracer::computeShading( Ray3D& ray ) {
	LightListNode* curLight = _lightSource;
	Colour result;
	for (;;) {
		if (curLight == NULL) break;
		std::vector<Point3D> samples = curLight->light->sampling();
		std::vector<Ray3D> shadow_rays;

        // Call sampling function of the lightsource,
        // if it is a point light, then will only return one single point light
        // if it is area light, then will sample a n*n grid, with each square of grid
        // containing a pointlight, thus to approximate an area light effect.
		for(int i = 0; i < samples.size(); i++) {
			Ray3D shadow_ray;
			shadow_ray.origin = samples[i];
			shadow_ray.dir = ray.intersection.point - samples[i];
			traverseScene(_root, shadow_ray);
			shadow_rays.push_back(shadow_ray);
		}
		curLight->light->shade(ray, shadow_rays);

		result = result + ray.col;
		curLight = curLight->next;
	}
	ray.col = result;
	ray.col.clamp();
}

void Raytracer::initPixelBuffer() {
	int numbytes = _scrWidth * _scrHeight * sizeof(unsigned char);
	_rbuffer = new unsigned char[numbytes];
	_gbuffer = new unsigned char[numbytes];
	_bbuffer = new unsigned char[numbytes];
	for (int i = 0; i < _scrHeight; i++) {
		for (int j = 0; j < _scrWidth; j++) {
			_rbuffer[i*_scrWidth+j] = 0;
			_gbuffer[i*_scrWidth+j] = 0;
			_bbuffer[i*_scrWidth+j] = 0;
		}
	}
}

void Raytracer::flushPixelBuffer( char *file_name ) {
	bmp_write( file_name, _scrWidth, _scrHeight, _rbuffer, _gbuffer, _bbuffer );
	delete _rbuffer;
	delete _gbuffer;
	delete _bbuffer;
}


// Bounce is the number of bounces we allow each ray
// We get bounce-1 reflections, as we recursively called shadeRay bounce-1 times.
Colour Raytracer::shadeRay( Ray3D& ray, int bounce ) {
	Colour col(0.0, 0.0, 0.0);
	if (bounce > 0) {
//std::cout<< bounce <<std::endl;
		traverseScene(_root, ray);
// Don't bother shading if the ray didn't hit
// anything.
		if (!ray.intersection.none) {
			computeShading(ray);

			col = ray.col;
// Refraction doesnt seem to work correctly.
// Glossy Reflection works properly.
			if(ray.intersection.mat->glossy != 1)
			{
				if (!(ray.intersection.mat->specular == Colour(0.0, 0.0, 0.0))) { 
					Vector3D r, u, v;

                    // function to find basis, so that we can achive a fuzzy reflection effect.
                    // The multiple reflected ray will shape like a square pyramid.
					r = ray.dir-2*(ray.dir.dot(ray.intersection.normal))*ray.intersection.normal;
					r.normalize();
					u = Vector3D(r);
					int min = u[0];
					int minIndex = 0;
					for (int j = 1; j < 3; j++) {
						if (u[j] < min) {
							min = u[j];
							minIndex = j;
						}
					}
					u[minIndex] = 1;
					u.normalize();
					v = r.cross(u);
					v.normalize();


					int num_samples = samples_num; 
					double gi = ray.intersection.mat->glossy; 
					Colour reflection_color;
					for (int i = 0; i < num_samples; i++) {
						Vector3D reflection_vector;
						if(gi != 0)
						{
                            //Generate some offsets to the perfect reflected ray,
                            // so that we can achieve a fuzzy reflection effect
							double r1 = static_cast<double>(rand())/RAND_MAX;
							double r2 = static_cast<double>(rand())/RAND_MAX;
							double random_u = -gi/2.0+r1*gi;
							double random_v = -gi/2.0+r2*gi;
							reflection_vector = r+random_u*u+random_v*v;
						}
						else 
						{
                            //If gi == 0, then we want the perfect mirror-like reflection.
							Vector3D reflection_vector = ray.dir-2*(ray.dir.dot(ray.intersection.normal))*ray.intersection.normal;
						}

						reflection_vector.normalize();
						Ray3D reflection_ray(ray.intersection.point+0.001*reflection_vector, reflection_vector);
                        // Recursively call shadeRay function.
						Colour c = shadeRay(reflection_ray, bounce-1);
						reflection_color = reflection_color + c;
					}
                    // Averaging all the reflected ray colors.
					reflection_color = 1.0/num_samples*reflection_color;


					col = col + ray.intersection.mat->specular*reflection_color;	
				}
			}
		}
	}
	col.clamp();
	return col;
}


void Raytracer::render( int width, int height, Point3D eye, Vector3D view, 
	Vector3D up, double fov, char* fileName , SceneDagNode* node) {
	Matrix4x4 viewToWorld;
	_scrWidth = width;
	_scrHeight = height;
	double factor = (double(height)/2)/tan(fov*M_PI/360.0);

	initPixelBuffer();
	viewToWorld = initInvViewMatrix(eye, view, up);




	for (int m = motion_num; m > 0; m--){
		// std::cout <<"m: "<< m <<std::endl;
        // Construct a ray for each pixel.
		for (int i = 0; i < _scrHeight; i++) {
			for (int j = 0; j < _scrWidth; j++) {
				//testing purpose
				// std::cout <<"m: "<< m << " , "<<"i: "<< i << " , "<<"j: "<< j <<std::endl;

				Colour col;
				int n = anti_aliasing_num;
				for (int p = 0; p < n; p++) {
					for(int q = 0; q < n; q++) {
						double r1 = static_cast<double>(rand())/RAND_MAX;
						double r2 = static_cast<double>(rand())/RAND_MAX;
                        // Sets up ray origin and direction in view space,
                        // image plane is at z = -1.
						Point3D origin(0, 0, 0);
						Point3D imagePlane;
						imagePlane[0] = (-double(width)/2+j+(p+r1)/n)/factor;
						imagePlane[1] = (-double(height)/2+i+(q+r2)/n)/factor;
						imagePlane[2] = -1;
                        // Convert ray to world space and call
                        // shadeRay(ray) to generate pixel colour.
						Vector3D dir = viewToWorld*(imagePlane-origin);
						dir.normalize();

						Ray3D ray(viewToWorld*origin, dir);
						col = col + shadeRay(ray, 3);
					}
				}
                // take average of color
				col = (1.0/pow(n,2))* col;

				if (mode == 1){
					_rbuffer[i*width+j] += int(col[0]*255);
					_gbuffer[i*width+j] += int(col[1]*255);
					_bbuffer[i*width+j] += int(col[2]*255);
				}

                // If mode == 2 ,means we enabling the motion blur, thus need to modify the RGB buffer.
				else if (mode == 2) {

					_rbuffer[i*width+j] += int(col[0]*255)/(double)pow(2,m);
					_gbuffer[i*width+j] += int(col[1]*255)/(double)pow(2,m);
					_bbuffer[i*width+j] += int(col[2]*255)/(double)pow(2,m);


				}

			}	
		}

        // The motion blurring will occur as a moving up motion.
		if (mode == 2)this->translate(node, Vector3D(0,0.3,0));

	}

	if (mode == 2)this->translate(node, Vector3D(0, -0.3*motion_num,0));

	flushPixelBuffer(fileName);
}


int main(int argc, char* argv[])
{	
// Build your scene and setup your camera here, by calling 
// functions from Raytracer.  The code here sets up an example
// scene and renders it from two different view points, DO NOT
// change this if you're just implementing part one of the 
// assignment.  
	Raytracer raytracer;


	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}

	if (argc == 4){

		mode = atoi(argv[3]);
	}

		std::cout<<"How to choose part1 or part2 ? : "<<std::endl;
		std::cout<<"type for part1 : ./raytracer 320 240 1 "<<std::endl;
		std::cout<<"type for part2 : ./raytracer 320 240 2 "<<std::endl;
		std::cout<<"Or simply type for part1 : ./raytracer"<<std::endl;
		std::cout<<"Running..."<<std::endl;



if (mode ==1 ){ // part1
//constants controlling the rays and rendering
	light_sample_num = 1;
	motion_num = 1; 
	anti_aliasing_num =2;
	samples_num = 1;
}
else if (mode ==2){ // part2

	// light_sample_num = 3;
	// motion_num = 10; 
	// anti_aliasing_num =2;
	// samples_num = 2;

	light_sample_num = 1;
	motion_num = 1; 
	anti_aliasing_num =1;
	samples_num = 1;
}




if(mode == 1){


	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	// Defines a material for shading.
	Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648), 
		Colour(0.628281, 0.555802, 0.366065), 
		51.2 ,1, -1.0,0 );
	

	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63), 
		Colour(0.316228, 0.316228, 0.316228), 
		12.8, 1, -1.0,0 );

	// Defines a point light source.
	raytracer.addLightSource( new PointLight(Point3D(0, 0, 5), 
		Colour(0.9, 0.9, 0.9) ) );

	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* plane = raytracer.addObject( new UnitSquare(), &jade );
	
	// Apply some transformations to the unit square.
	double factor1[3] = { 1.0, 2.0, 1.0 };
	double factor2[3] = { 6.0, 6.0, 6.0 };
	raytracer.translate(sphere, Vector3D(0, 0, -5));	
	raytracer.rotate(sphere, 'x', -45); 
	raytracer.rotate(sphere, 'z', 45); 
	raytracer.scale(sphere, Point3D(0, 0, 0), factor1);

	raytracer.translate(plane, Vector3D(0, 0, -7));	
	raytracer.rotate(plane, 'z', 45); 
	raytracer.scale(plane, Point3D(0, 0, 0), factor2);

	// Render the scene, feel free to make the image smaller for
	// testing purposes.	
	raytracer.render(width, height, eye, view, up, fov, "view1.bmp", sphere);
	
	// Render it from a different point of view.
	Point3D eye2(4, 2, 1);
	Vector3D view2(-4, -2, -6);
	raytracer.render(width, height, eye2, view2, up, fov, "view2.bmp",sphere );

}else if(mode ==2 ) {


	int numbytes = width * height * sizeof(unsigned char);
	rbuffer = new unsigned char[numbytes];
	gbuffer = new unsigned char[numbytes];
	bbuffer = new unsigned char[numbytes];


	long unsigned int* bmp_width = new long unsigned int(texture_width);
	long int* bmp_height = new long int(texture_height); 

	std::cout<<"Loading Texture..."<<std::endl;
	std::cout<<"Still Running..."<<std::endl;

	bmp_read("earth.bmp", bmp_width, bmp_height, &rbuffer, &gbuffer, &bbuffer); 

//reference from http://www.it.hiof.no/~borres/j3d/explain/light/p-materials.html
// Defines a material for shading.
	Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648), 
		Colour(0.628281, 0.555802, 0.366065), 
		51.2, 0.1, -1.0,0);


	Material board( Colour(0.1, 0.1, 0.1), Colour(0.1, 0.1, 0.1), 
		Colour(0.628281, 0.555802, 0.366065), 
		51.2, 0.05, -1.0,2);

	Material earth_m( Colour(0.3, 0.3, 0.3), Colour(0.3, 0.3, 0.3), 
		Colour(0.3, 0.3, 0.3), 
		4, 1, -1.0,1);

	Material jade( Colour(0.135, 0.2225, 0.1575), Colour(0.54, 0.89, 0.63), 
		Colour(0.316228, 0.316228, 0.316228), 
		12.8, 0.5, -1.0,0 );

	Material Chrome( Colour(0.25, 0.25, 0.25), Colour(0.4, 0.4, 0.4),
		Colour(0.674597, 0.674597, 0.674597),
		66.8, 0.2, -1.0,0);

	Material glass( Colour(0.0, 0.0, 0.0), Colour(0.588235, 0.670588, 0.729412),
		Colour(0.9, 0.9, 0.9),
		96.0, 0.01, 1.5 ,0);

	Material red( Colour(0.1745,0.01175,0.01175), Colour(0.6,0.04,0.04), 
		Colour(0.7,0.6,0.6), 
		51.2, 1, -1.0,0);


	Material green( Colour(0.0215, 0.1745, 0.02), Colour(0.075164, 0.60648, 0.072648), 
		Colour(0.628281, 0.755802, 0.66065), 
		51.2, 1, -1.0,0);

	Material blue( Colour(0.1, 0.18, 0.745), Colour(0.35164, 0.70648, 0.62648), 
		Colour(0.298281, 0.305802, 0.306065), 
		51.2, 1, -1.0,0);

	Material yellow( Colour(0.32, 0.22, 0.027), Colour(0.75164, 0.560648, 0.112648), 
		Colour(0.6281, 0.6802, 0.6065),
		51.2, 1, -1.0,0);

	raytracer.addLightSource( new AreaLight(Point3D(3.8, 8.8, -13.3),
		Vector3D(1, 0, 0), Vector3D(0, 1, 0),
		Colour(0.9, 0.9, 0.9) ) );


	SceneDagNode* earth = raytracer.addObject( new UnitSphere(), &earth_m );

	SceneDagNode* sphere1 = raytracer.addObject( new UnitSphere(), &Chrome );
	SceneDagNode* sphere2 = raytracer.addObject( new UnitSphere(), &Chrome );


	SceneDagNode* plane1 = raytracer.addObject( new UnitSquare(), &board);
	SceneDagNode* plane2 = raytracer.addObject( new UnitSquare(), &red);
	SceneDagNode* plane3 = raytracer.addObject( new UnitSquare(), &green);
	SceneDagNode* plane4 = raytracer.addObject( new UnitSquare(), &blue);
	SceneDagNode* plane5 = raytracer.addObject( new UnitSquare(), &yellow);

	SceneDagNode* cylinder = raytracer.addObject( new UnitCylinder(), &Chrome );


	double earth_f[3] = { 2.0, 2.0, 2.0 };
	double ground_f[3] = { 10, 10, 10 };
	double cylinder_f[3] = { 1.6, 4.5, 1.6 };

	double sphere1_f[3] = { 1.0, 1.0, 1.0 };
	double sphere2_f[3] = { 1.0, 1.0, 1.0 };


	raytracer.translate(plane1, Vector3D(0, 0, -10));
	raytracer.rotate(plane1, 'x', -90);
	raytracer.scale(plane1, Point3D(0, 0, 0), ground_f);

	raytracer.translate(plane2, Vector3D(0, 10, -10));
	raytracer.rotate(plane2, 'x', 90);
	raytracer.scale(plane2, Point3D(0, 0, 0), ground_f);

	raytracer.translate(plane3, Vector3D(-5, 5, -10));
	raytracer.rotate(plane3, 'y', 90);
	raytracer.scale(plane3, Point3D(0, 0, 0), ground_f);

	raytracer.translate(plane4, Vector3D(5, 5, -10));
	raytracer.rotate(plane4, 'y', -90);
	raytracer.scale(plane4, Point3D(0, 0, 0), ground_f);

	raytracer.translate(plane5, Vector3D(0, 5, -15));
	raytracer.scale(plane5, Point3D(0, 0, 0), ground_f);




	raytracer.translate(sphere1, Vector3D(-3, sphere1_f[1]/2, -10.5));
	raytracer.scale(sphere1, Point3D(0, 0, 0), sphere1_f);

	raytracer.translate(sphere2, Vector3D(3, sphere1_f[1]/2, -10.5));
	raytracer.scale(sphere2, Point3D(0, 0, 0), sphere2_f);


	raytracer.translate(cylinder, Vector3D(0, cylinder_f[1]/2, -13));
	raytracer.scale(cylinder, Point3D(0, 0, 0), cylinder_f);

	raytracer.translate(earth, Vector3D(0, earth_f[1] + 0.8, -13));	
	raytracer.scale(earth, Point3D(0, 0, 0), earth_f);


	Point3D eye(0, 6, 0);
	Vector3D view(0, -0.1, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;


	raytracer.render(width, height, eye, view, up, fov, "part2 only used for test.bmp", earth);

}

return 0;
}

