#pragma once

#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>

#include "line.h"
#include "Shape.h"


struct TrailParticle
{
	float spawnTime;
	// maybe speed?
	//int cloudFrame;		// section of the texture
	//float size;
	vec4 pos;

	TrailParticle(vec4 _pos, float _spawnTime)
	{
		pos = _pos;
		spawnTime = _spawnTime;
	}
};

struct PathPoint;
/*
 * This class will provide the "paths" that are used by plane1, plane2, and the camera
 *
 * Each path will have 2 linerenderers (one for pure line points, and the other for the generated spline)
 */
class ObjectPath
{
public:
	void draw(mat4, mat4, shared_ptr<Program>,
		shared_ptr<Shape>, bool);
	~ObjectPath();
	ObjectPath(vec3 baseLineColor, vec3 splineColor);
	ObjectPath(vec3 baseLineColor, vec3 splineColor, string fileName);
	void update();
	void addPoint(PathPoint);
	void popPoint();
	void SavePathToFile(string fileName);
	vector<vec3> splinePoints;
	vector<PathPoint> linepoints;
	vec3 lastPos = vec3(0);
private:
	Line linerender, smoothrender;
	vec3 baseLineColor, splineColor;
};

vec3 update_plane(shared_ptr<Program> pplane, shared_ptr<Shape> plane,
	float w, mat4 P, mat4 V, ObjectPath *path, vec3 campos);

extern int realspeed;

extern GLuint QuadVAOID, QuadIndexBuffer, InstanceBuffer, TimeBuffer, Texture3;

class camera
{
public:
	glm::vec3 pos, rot;
	float pitch, yaw;
	bool firstMouse;
	int w, a, s, d, q, e, z, c, t, y;

	const vec3 cameraUp = vec3(0, 1, 0);

	camera()
	{
		w = a = s = d = q = e = z = c = t = y = 0;
		pos = rot = glm::vec3(0, 0, 0);
		//pos = vec3(186, -72, 38);
		//rot = vec3(.266f, .768f, 0.0f);
		yaw = 0.0f;
		pitch = 0.0f;
		firstMouse = true;
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;

		float fwdspeed = 30;
		if (realspeed)
			fwdspeed = 120;

		if (w == 1)
		{
			speed = fwdspeed * ftime;
		}
		else if (s == 1)
		{
			speed = -fwdspeed * ftime;
		}

		float yangle = 0;
		/*if (a == 1)
			yangle = -3*ftime;
		else if(d==1)
			yangle = 3*ftime;*/
		static float lastYaw;
		static float lastPitch;
		yangle = (yaw - lastYaw) * .008f;

		rot.y += yangle;
		float zangle = 0;
		if (q == 1)
			zangle = -3 * ftime;
		else if (e == 1)
			zangle = 3 * ftime;
		rot.z += zangle;
		float xangle = 0;
		/*if (z == 1)
			xangle = -0.3 * ftime;
		else if (c == 1)
			xangle = 0.3 * ftime;*/
		xangle += (lastPitch - pitch) * .008f;
		rot.x += xangle;

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		R = Rz * Rx * R;
		dir = dir * R;
		vec3 camForwards = vec4(0, 0, 1, 1) * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);

		// lateral movement:
		vec3 side = normalize(cross(vec3(normalize(camForwards)), cameraUp));
		float sideSpeed = 10 * ftime;
		if (a == 1)
		{
			pos -= side * sideSpeed;
		}
		if (d == 1)
		{
			pos += side * sideSpeed;
		}
		if (z == 1)
		{
			pos += cameraUp * sideSpeed;
		}
		if (c == 1)
		{
			pos -= cameraUp * sideSpeed;
		}
		glm::mat4 T = glm::translate(glm::mat4(1), pos);

		lastYaw = yaw;
		lastPitch = pitch;



		return R * T;
	}
	void get_dirpos(vec3& up, vec3& dir, vec3& position)
	{
		position = -pos;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir4 = glm::vec4(0, 0, 1, 0);
		R = Rz * Rx * R;
		dir4 = dir4 * R;
		dir = vec3(dir4);
		glm::vec4 up4 = glm::vec4(0, 1, 0, 0);
		up4 = R * vec4(0, 1, 0, 0);
		up4 = vec4(0, 1, 0, 0) * R;
		up = vec3(up4);
	}
	/*void get_dirpos(vec3& position, float& roll)
	{
		position = -pos;
		roll = rot.z;
	}*/

};

/*
 * Values to interpolate between as the plane travels along its desired
 * path.
 * pos: position along path
 * zbase: direction the plane is facing
 * ybase: up direction to base rotation off of.
 */
struct PathPoint
{
	vec3 pos, dir, up;
	//float roll;
	PathPoint(vec3 _pos, vec3 _dir, vec3 _up);
};

mat4 generate_path_view(ObjectPath camPath, float frametime, ObjectPath p1,
	ObjectPath p2, camera mycam, vec3* pos_to_change, int cameraMode);


mat4 linint_between_two_orientations(glm::vec3 ez_aka_lookto_1, glm::vec3 ey_aka_up_1, glm::vec3 ez_aka_lookto_2,
	glm::vec3 ey_aka_up_2, float t);