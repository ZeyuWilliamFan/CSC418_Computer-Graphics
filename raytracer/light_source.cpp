/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

		extern int texture_width; 
		extern int texture_height; 
		extern	unsigned char* rbuffer; 
		extern	unsigned char* gbuffer; 
		extern	unsigned char* bbuffer; 
		extern 	int light_sample_num;

		double dmax(double a, double b) {
			return a < b ? b : a;
		}

		std::vector<Point3D> PointLight::sampling() {
			std::vector<Point3D> samples;
			samples.push_back(_pos);
			return samples;
		}

		std::vector<Point3D> AreaLight::sampling() {
    int n =light_sample_num; //Parameter, n determines how many samples for a point light to span.
    std::vector<Point3D> samples;
    for (int p=0; p<n; p++) {
    	for (int q=0; q<n; q++) {
    		float r1 = static_cast<float>(rand())/RAND_MAX;
    		float r2 = static_cast<float>(rand())/RAND_MAX;
    		Point3D point = _pos+(p+r1)/n*_u+(q+r2)/n*_v;
    		samples.push_back(point);
    	}
    }
    return samples;
}



void PointLight::shade( Ray3D& ray, std::vector<Ray3D> incident_rays ) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.  



	Colour col;


// Get all corresponding vectors: normal, light, reflect and view vector.
// Normalize them all.
	Vector3D normal_vector = ray.intersection.normal;

	Vector3D light_vector = _pos - ray.intersection.point;

	Vector3D reflect_vector = 2 * light_vector.dot(normal_vector) * normal_vector - light_vector;

	Vector3D view_vector = -ray.dir;


	normal_vector.normalize();

	light_vector.normalize();

	reflect_vector.normalize();

	view_vector.normalize();

	Ray3D shadow_ray = incident_rays[0];




// If no texture is being mapped.
	if(ray.intersection.mat->flag == 0)
	{
		// if the t_val is between 0.01 and 0.99, it is in shadow then.
		//if(!(incident_rays[0].intersection.t_value > 0.01 && incident_rays[0].intersection.t_value < 0.99))
		
			col = ray.intersection.mat->ambient * _col_ambient + 
			dmax(0, normal_vector.dot(light_vector)) * ray.intersection.mat->diffuse * _col_diffuse +
			dmax(0, pow(view_vector.dot(reflect_vector),ray.intersection.mat->specular_exp)) * ray.intersection.mat->specular * _col_specular;
		
		ray.col = col;
		ray.col.clamp();
	}else if(ray.intersection.mat->flag == 2)
	{
		Point3D IntersectPoint = ray.intersection.point; 

		Point3D centerOfPlane = ray.intersection.CenterPoint; 

		double difference_x = ray.intersection.point[0] - ray.intersection.CenterPoint[0];
		double difference_y = ray.intersection.point[1] - ray.intersection.CenterPoint[1];
		double difference_z = ray.intersection.point[2] - ray.intersection.CenterPoint[2];



		if(cos(difference_x* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else ray.col = ray.col +Colour(0,0,0) ;


		if(sin(difference_y* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else
			ray.col = ray.col +Colour(0,0,0) ;



		if(sin(difference_z* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else 
			ray.col = ray.col +Colour(0,0,0) ;



		if(ray.col[0]+ray.col[1]+ray.col[2]!=3){
			ray.col = Colour(0,0,0);
		}else {
			ray.col = Colour(0.5,0.5,0.5);

		}


		ray.col = ray.col + ray.intersection.mat->ambient * _col_ambient +
		dmax(0, normal_vector.dot(light_vector)) * ray.intersection.mat->diffuse * _col_diffuse +
		dmax(0, pow(view_vector.dot(reflect_vector),ray.intersection.mat->specular_exp)) * ray.intersection.mat->specular * _col_specular;

		ray.col.clamp();

	}


	else // Means texture mapping is triggered on.
	{

		//reference from https://www.siggraph.org/education/materials/HyperGraph/mapping/spheretx.htm
		Point3D IntersectPoint = ray.intersection.point; 
		Point3D centerOfSphere = ray.intersection.CenterPoint; 
		
		//theta from 0 to pi
		double theta = acos(std::abs(IntersectPoint[2]-centerOfSphere[2])/2); 

		//phi from -pi tp pi
		double phi = atan2(IntersectPoint[1]-centerOfSphere[1],IntersectPoint[0]-centerOfSphere[0]); 
		
		double u = phi/(2.0*M_PI);
		double v = (M_PI- theta)/M_PI; 
		
		u *= texture_width; 
		v *= texture_height; 

		int temp = floor(v)*texture_width+floor(u); 

		ray.col[0] += (rbuffer[temp])/(double)255 ;
		ray.col[1] += (gbuffer[temp])/(double)255 ;
		ray.col[2] += (bbuffer[temp])/(double)255 ;


		// if the t_val is between 0.01 and 0.99, it is in shadow then.
		if(!(incident_rays[0].intersection.t_value > 0.01 && incident_rays[0].intersection.t_value < 0.99))
		//if(1)
		{
			col = ray.intersection.mat->ambient * _col_ambient + 
			dmax(0, normal_vector.dot(light_vector)) * ray.intersection.mat->diffuse * _col_diffuse +
			dmax(0, pow(view_vector.dot(reflect_vector),ray.intersection.mat->specular_exp)) * ray.intersection.mat->specular * _col_specular;
		}
		else {
			col = ray.intersection.mat->ambient * _col_ambient;
		}
		ray.col = ray.col + col;
		ray.col.clamp();



	}


  

}





void AreaLight::shade( Ray3D& ray, std::vector<Ray3D> incident_rays ) {
    //Get properties of intersection surface
  //  std::cout<< "111111" <<std::endl;

	Point3D p = ray.intersection.point;

    //Apply function to colour and add to ambient + diffuse


	Colour r_a = ray.intersection.mat->ambient; 
	Colour r_d = ray.intersection.mat->diffuse;
	Colour r_s = ray.intersection.mat->specular;

    //Get properties of light
	Colour I_a = _col_ambient;
	Colour I_d = _col_diffuse;
	Colour I_s = _col_specular;

	Vector3D normal_vector = ray.intersection.normal;

	Vector3D light_vector = _pos - ray.intersection.point;

	Vector3D reflect_vector = 2 * light_vector.dot(normal_vector) * normal_vector - light_vector;

	Vector3D view_vector = -ray.dir;


	normal_vector.normalize();

	light_vector.normalize();

	reflect_vector.normalize();

	view_vector.normalize();

	if(ray.intersection.mat->flag == 0){


	    //Apply Phong
		Colour result_a = r_a * I_a;
		Colour result_d;
		Colour result_s;
		Colour colour;

		for (int i = 0; i < incident_rays.size(); i++) {
			Ray3D shadow_ray = incident_rays[i];

			Vector3D light_vector =  shadow_ray.origin - ray.intersection.point;

			Vector3D reflect_vector =  2 * (normal_vector.dot(light_vector))*normal_vector - light_vector;
			light_vector.normalize();
			reflect_vector.normalize();

	        // if the t_val is between 0.01 and 0.99, it is in shadow then.
			if (!(incident_rays[0].intersection.t_value > 0.01 && incident_rays[0].intersection.t_value < 0.99)) {
				result_d = fmax(0, normal_vector.dot(light_vector)) * (r_d * I_d);
				result_s = pow(fmax(0, reflect_vector.dot(view_vector)), ray.intersection.mat->specular_exp) * r_s * I_s;
			}
			colour = colour + result_a + result_d + result_s;
		}
		ray.col = 1.0 / incident_rays.size() * colour;
		ray.col.clamp();
	}
	else if(ray.intersection.mat->flag == 1){
		

		//reference from https://www.siggraph.org/education/materials/HyperGraph/mapping/spheretx.htm
		Point3D IntersectPoint = ray.intersection.point; 
		Point3D centerOfSphere = ray.intersection.CenterPoint; 
		
		//theta from 0 to pi
		double theta = acos(std::abs(IntersectPoint[2]-centerOfSphere[2])/2); 

		//phi from -pi tp pi
		double phi = atan2(IntersectPoint[1]-centerOfSphere[1],IntersectPoint[0]-centerOfSphere[0]); 
		
		double u = phi/(2.0*M_PI);
		double v = (M_PI - theta)/M_PI; 
		
		u *= texture_width; 
		v *= texture_height; 

		int temp = floor(v)*texture_width+floor(u); 


		ray.col[0] += (rbuffer[temp])/(double)255 ;


		ray.col[1] += (gbuffer[temp])/(double)255 ;
		ray.col[2] += (bbuffer[temp])/(double)255 ;

		//Apply Phong
		Colour result_a = r_a * I_a;
		Colour result_d;
		Colour result_s;
		Colour colour;

		for (int i = 0; i < incident_rays.size(); i++) {
			Ray3D shadow_ray = incident_rays[i];

			Vector3D light_vector =  shadow_ray.origin - ray.intersection.point;

			Vector3D reflect_vector =  2 * (normal_vector.dot(light_vector))*normal_vector - light_vector;
			light_vector.normalize();
			reflect_vector.normalize();

	  		// if the t_val is between 0.01 and 0.99, it is in shadow then.
			if (!(incident_rays[0].intersection.t_value > 0.01 && incident_rays[0].intersection.t_value < 0.99)) {
				result_d = fmax(0, normal_vector.dot(light_vector)) * (r_d * I_d);
				result_s = pow(fmax(0, reflect_vector.dot(view_vector)), ray.intersection.mat->specular_exp) * r_s * I_s;
			}
			colour = colour + result_a + result_d + result_s;
		}
		ray.col = ray.col + 1.0 / incident_rays.size() * colour;
		ray.col.clamp();

	}else if(ray.intersection.mat->flag == 2){
		Point3D IntersectPoint = ray.intersection.point; 

		Point3D centerOfPlane = ray.intersection.CenterPoint; 

		double difference_x = ray.intersection.point[0] - ray.intersection.CenterPoint[0];
		double difference_y = ray.intersection.point[1] - ray.intersection.CenterPoint[1];
		double difference_z = ray.intersection.point[2] - ray.intersection.CenterPoint[2];



		if(sin(difference_x* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else ray.col = ray.col +Colour(0,0,0) ;


		if(cos(difference_y* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else
			ray.col = ray.col +Colour(0,0,0) ;

		if(cos(difference_z* M_PI)>=0)
			ray.col = ray.col +Colour(0.5,0.5,0.5) ;

		else 
			ray.col = ray.col +Colour(0,0,0) ;



		if(ray.col[0]+ray.col[1]+ray.col[2]!=3)

			ray.col = Colour(0,0,0);
		else 
			ray.col = Colour(1,1,1);


		if(ray.col[0]+ray.col[1]+ray.col[2]!=3){
			ray.col = Colour(0,0,0);
		}else {
			ray.col = Colour(0.5,0.5,0.5);

		}

//Apply Phong
		Colour result_a = r_a * I_a;
		Colour result_d;
		Colour result_s;
		Colour colour;

		for (int i = 0; i < incident_rays.size(); i++) {
			Ray3D shadow_ray = incident_rays[i];

			Vector3D light_vector =  shadow_ray.origin - ray.intersection.point;

			Vector3D reflect_vector =  2 * (normal_vector.dot(light_vector))*normal_vector - light_vector;
			light_vector.normalize();
			reflect_vector.normalize();

	  		// if the t_val is between 0.01 and 0.99, it is in shadow then.
			if (!(incident_rays[0].intersection.t_value > 0.01 && incident_rays[0].intersection.t_value < 0.99)) {
				result_d = fmax(0, normal_vector.dot(light_vector)) * (r_d * I_d);
				result_s = pow(fmax(0, reflect_vector.dot(view_vector)), ray.intersection.mat->specular_exp) * r_s * I_s;
			}
			colour = colour + result_a + result_d + result_s;
		}
		ray.col = ray.col + 1.0 / incident_rays.size() * colour;
		ray.col.clamp();

	}
}





