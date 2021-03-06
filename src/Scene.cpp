#include <iostream>

#include "Scene.h"
#include "Particle.h"
#include "Cloth.h"
#include "Shape.h"
#include "Program.h"

using namespace std;
using namespace Eigen;

Scene::Scene() :
	t(0.0),
	h(1e-2),
	grav(0.0, 0.0, 0.0)
{
}

Scene::~Scene()
{
}

void Scene::load(const string &RESOURCE_DIR)
{
	// Units: meters, kilograms, seconds
	h = 2e-3;
	
	grav << 0.0, -9.8, 0.0;
	
	int rows = 10;
	int cols = 10;
	double mass = 0.1;
	double stiffness = 1e1;
	Vector2d damping(1.0, 1.0);
	Vector3d x00(-0.5, 0.6, 0.0);
	Vector3d x01(0.0, 0.6, 0.0);
	Vector3d x10(-0.5, 0.6, -0.5);
	Vector3d x11(0.0, 0.6, -0.5);
	cloth = make_shared<Cloth>(rows, cols, x00, x01, x10, x11, mass, stiffness, damping);
    
    Vector3d xx00(0.0, 0.6, 0.0);
    Vector3d xx01(0.5, 0.6, 0.0);
    Vector3d xx10(0.0, 0.6, -0.5);
    Vector3d xx11(0.5, 0.6, -0.5);
    cloth2 = make_shared<Cloth>(rows, cols, xx00, xx01, xx10, xx11, mass, stiffness, damping);
	
	sphereShape = make_shared<Shape>();
	sphereShape->loadMesh(RESOURCE_DIR + "sphere2.obj");
	
	auto sphere = make_shared<Particle>(sphereShape);
	spheres.push_back(sphere);
	sphere->r = 0.1;
	sphere->x = Vector3d(0.0, 0.2, 0.0);
}

void Scene::init()
{
	sphereShape->init();
	cloth->init();
    cloth2->init();
}

void Scene::tare()
{
	for(int i = 0; i < (int)spheres.size(); ++i) {
		spheres[i]->tare();
	}
	cloth->tare();
    cloth2->tare();
}

void Scene::reset()
{
	t = 0.0;
	for(int i = 0; i < (int)spheres.size(); ++i) {
		spheres[i]->reset();
	}
	cloth->reset();
    cloth2->reset();
}

void Scene::addSpheres(std::vector<float> points)
{
//    spheres.erase(spheres.begin()+1, spheres.end());

//    spheres.resize(1);
//    spheres.shrink_to_fit();
    spheres.clear();
    
    for (int i = 0; i < (float)points.size()/3.0; i++)
    {
        if (true || i % 3 == 0)
        {
            auto s = make_shared<Particle>(sphereShape);
            s->r = 0.03;
            s->x = Eigen::Vector3d((double)points[3*i] * 0.4, (double)points[3*i+1] * 0.4, (double)points[3*i+2] * 0.4);
            spheres.push_back(s);
        }
    }
}

void Scene::step()
{
	t += h;
	
	// Move the sphere
	if(!spheres.empty()) {
		auto s = spheres.front();
		Vector3d x0 = s->x;
		double radius = 0.5;
		double a = 2.0*t;
		s->x(2) = radius * sin(a);
		Vector3d dx = s->x - x0;
		s->v = dx/h;
	}
	
	// Simulate the cloth
	cloth->step(h, grav, spheres);
    cloth2->step(h, grav, spheres);
}

void Scene::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog) const
{
	glUniform3fv(prog->getUniform("kdFront"), 1, Vector3f(1.0, 1.0, 1.0).data());
//	for(int i = 0; i < (int)spheres.size(); ++i) {
//		spheres[i]->draw(MV, prog);
//	}
	cloth->draw(MV, prog);
    cloth2->draw(MV, prog);
}
