/***********************************************************
Starter code for Assignment 3

This code was originally written by Jack Wang for
    CSC418, SPRING 2005

implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

#include "scene_object.h"





bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
    const Matrix4x4& modelToWorld ) {
// TODO: implement intersection code for UnitSquare, which is
// defined on the xy-plane, with vertices (0.5, 0.5, 0), 
// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
// (0, 0, 1).
//
// Your goal here is to fill ray.intersection with correct values
// should an intersection occur.  This includes intersection.point, 
// intersection.normal, intersection.none, intersection.t_value.   
//
// HINT: Remember to first transform the ray into object space  
// to simplify the intersection test.
    
    bool intersection_occured = false;
    
    Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
    
    double t_value = - (rayModelSpace.origin[2] / rayModelSpace.dir[2]);
    double x_val = rayModelSpace.origin[0] + t_value * rayModelSpace.dir[0];
    double y_val = rayModelSpace.origin[1] + t_value * rayModelSpace.dir[1];
    
    if (t_value <= 0 || x_val < -0.5 || x_val > 0.5 || y_val < -0.5 || y_val > 0.5)
        return false;
    
    if(ray.intersection.none || t_value < ray.intersection.t_value){
        
        intersection_occured = true;
        
        Intersection intersection;
        
        intersection.normal = worldToModel.transpose() * Vector3D(0, 0, 1);
        intersection.normal.normalize();

        intersection.point = modelToWorld * Point3D(x_val, y_val, 0);
        intersection.t_value = t_value;
        intersection.none = false;
        

        intersection.CenterPoint = modelToWorld * Point3D(0,0,0); 

        ray.intersection = intersection;
    }
    
    return intersection_occured;
}

bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
    const Matrix4x4& modelToWorld ) {
// TODO: implement intersection code for UnitSphere, which is centred 
// on the origin.  
//
// Your goal here is to fill ray.intersection with correct values
// should an intersection occur.  This includes intersection.point, 
// intersection.normal, intersection.none, intersection.t_value.   
//
// HINT: Remember to first transform the ray into object space  
// to simplify the intersection test.
    
    bool intersection_occured = false;
    
    Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
    
    Vector3D rayOriginVector = rayModelSpace.origin - Point3D(0, 0, 0);
    
    double a = rayModelSpace.dir.dot(rayModelSpace.dir);
    double b = 2 * rayModelSpace.dir.dot(rayOriginVector);
    double c = rayOriginVector.dot(rayOriginVector) - 1;
    double t_value;
    
    double discriminant = b * b - 4 * a * c;
    
    if(discriminant >= 0){
        double root1 = (-b + sqrt(discriminant))/ (2*a);
        double root2 = (-b - sqrt(discriminant))/ (2*a);
        


        if ( root1 < 0 && root2 < 0)
            return false;
        else if ( root1 < 0 )
            t_value = root2;
        else if ( root2 < 0 )
            t_value = root1;

        else
            t_value = fmin(root1, root2);


    }else{
        return false;
    }
    
    if(ray.intersection.none || t_value < ray.intersection.t_value){
        
        intersection_occured = true;

        Intersection intersection;
        
        Point3D intersectionPoint = rayModelSpace.origin + t_value *rayModelSpace.dir;
        
        intersection.normal = worldToModel.transpose()* Vector3D(intersectionPoint[0], intersectionPoint[1], intersectionPoint[2]);
        intersection.normal.normalize();
        intersection.point = modelToWorld * intersectionPoint;
        
        intersection.t_value = t_value;
        intersection.none = false;
        
        

        intersection.CenterPoint = modelToWorld * Point3D(0,0,0); 



        ray.intersection = intersection;
    }
    
    
    return intersection_occured;
}


bool isSame(double a, double b)
{
    return std::fabs(a - b) <= std::numeric_limits<double>::epsilon();
}

bool UnitCylinder::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
 const Matrix4x4& modelToWorld ) {
    
    
    bool intersection_occured = false;
    
    Ray3D rayModelSpace = Ray3D(worldToModel * ray.origin, worldToModel * ray.dir);
    

    std::vector<double> t_values;
    double t_value;

    double dir_x = rayModelSpace.dir[0];
    double dir_y = rayModelSpace.dir[1];
    double dir_z = rayModelSpace.dir[2];
    
    double origin_x = rayModelSpace.origin[0];
    double origin_y = rayModelSpace.origin[1];
    double origin_z = rayModelSpace.origin[2];
    
    double a = dir_x * dir_x + dir_z * dir_z;
    double b = 2 *(origin_x* dir_x +  origin_z* dir_z);
    double c = origin_x * origin_x + origin_z * origin_z-1.0;
    
    double discriminant = b * b - 4 * a * c;
    
    if(discriminant >= 0){
        double root1 = (-b + sqrt(discriminant))/ (2*a);
        double root2 = (-b - sqrt(discriminant))/ (2*a);
        
        if (root1>0){
            Point3D point = rayModelSpace.origin + root1 * rayModelSpace.dir;
            
            if (point[1]>=-0.5 && point[1]<= 0.5)
                
                t_values.push_back(root1);
        }
        
        if (root2 >0){
            
            Point3D point = rayModelSpace.origin + root2 * rayModelSpace.dir;
            
            if (point[1]>=-0.5 && point[1]<= 0.5)
                t_values.push_back(root2);
        }
    }else{
        return false;
    }
    
    
    
    double t_top = (0.5 - origin_y) / dir_y;
    double t_bottom = (-0.5 - origin_y) / dir_y;
    
    Point3D t_top_point = rayModelSpace.origin + t_top * rayModelSpace.dir ;
    Point3D t_bottom_point = rayModelSpace.origin + t_bottom * rayModelSpace.dir ;

    
    
    if (t_top >0 && t_top_point[0] * t_top_point[0] + t_top_point[2]*t_top_point[2] <=1 ) {
        t_values.push_back(t_top);
    }
    
    if (t_bottom > 0 && t_bottom_point[0] * t_bottom_point[0] + t_bottom_point[2]*t_bottom_point[2] <=1) {
        t_values.push_back(t_bottom);
    }
    

    if(t_values.size() > 0){
        t_value = *std::min_element(t_values.begin(), t_values.end());
    }else{
        return false;
    }


    if (t_values.size() > 0 && (ray.intersection.none || t_value < ray.intersection.t_value)) {
        intersection_occured = true;
        Intersection intersection;
        Point3D intersectionPointModelSpace = rayModelSpace.origin+ t_value * rayModelSpace.dir;
        intersection.point = modelToWorld * intersectionPointModelSpace;
        Vector3D normal;
        if (isSame(t_value, t_top)) {
            normal = Vector3D(0, 1.0, 0);
        } else if (isSame(t_value, t_bottom)) {
            normal = Vector3D(0, -1.0, 0);
        } else {
            normal = Vector3D(intersectionPointModelSpace[0], 0, intersectionPointModelSpace[2]);
        }
        intersection.normal = worldToModel.transpose() * normal;
        intersection.normal.normalize();

        intersection.t_value = t_value;
        intersection.none = false;
        ray.intersection = intersection;
    }
    
    
    return intersection_occured;
}

