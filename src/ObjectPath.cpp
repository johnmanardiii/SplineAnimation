#include "ObjectPath.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include "Program.h"

mat4 linint_between_two_orientations(glm::vec3 ez_aka_lookto_1, glm::vec3 ey_aka_up_1, glm::vec3 ez_aka_lookto_2,
	glm::vec3 ey_aka_up_2, float t);

int realspeed = 0;

GLuint QuadVAOID, QuadIndexBuffer, InstanceBuffer, TimeBuffer, Texture3;

ObjectPath::ObjectPath(vec3 baseLineColor, vec3 splineColor)
{
	this->baseLineColor = baseLineColor;
	this->splineColor = splineColor;
	smoothrender.init();
	linerender.init();
}

void convertToPoints(const vector<PathPoint> &path, vector<vec3> &toBeFilled)
{
	for(int i = 0; i < path.size(); i++)
	{
		toBeFilled.push_back(path[i].pos);
	}
}

void ObjectPath::update()
{
	vector<vec3> toBeFilled;
	convertToPoints(linepoints, toBeFilled);
	linerender.re_init_line(toBeFilled);
	spline(splinePoints, toBeFilled, 120, 1.0);
	if(splinePoints.size() > 10)
	{
		splinePoints.erase(splinePoints.begin());
	}
	smoothrender.re_init_line(splinePoints);
}

void ObjectPath::addPoint(PathPoint p)
{
	linepoints.push_back(p);
	update();
}

ObjectPath::~ObjectPath()
{
	//for (PathPoint* p : linepoints)
	//{
	//	delete[] p;
	//}
}

void ObjectPath::popPoint()
{
	if(linepoints.empty())
	{
		return;
	}
	// PathPoint* p = linepoints[linepoints.size() - 1];
	linepoints.pop_back();
	update();
}


void ObjectPath::draw(mat4 P, mat4 V, shared_ptr<Program> prog,
	shared_ptr<Shape> arrows, bool drawSpheres)
{
	linerender.draw(P, V, baseLineColor);
	smoothrender.draw(P, V, splineColor);

	// draw orientation objects?
	prog->bind();
	for(int i = 0; i < linepoints.size(); i++)
	{
		PathPoint p = linepoints[i];
		mat4 R = lookAt(p.pos, p.dir, p.up);
		//static float rotateamount = 0;
		//rotateamount += .2;
		//mat4 R = rotate(mat4(1), radians(rotateamount), vec3(0, 0, 1));
		R[3][0] = R[3][1] = R[3][2] = 0;
		mat4 M = translate(mat4(1), p.pos);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		//TODO: get these arrows oriented correctly
		if (drawSpheres)
		{
			arrows->draw(prog);
		}
	}
	prog->unbind();
}

std::vector<std::string> split(const std::string str, char delim)
{
	std::vector<std::string> result;
	std::istringstream ss{ str };
	std::string token;
	while (std::getline(ss, token, delim)) {
		if (!token.empty()) {
			result.push_back(token);
		}
	}
	return result;
}

/*
 * Constructor that reads in a file and converts it to a path
 */
ObjectPath::ObjectPath(vec3 baseLineColor, vec3 splineColor, string fileName)
{
	this->baseLineColor = baseLineColor;
	this->splineColor = splineColor;
	smoothrender.init();
	linerender.init();

	string lineData;
	ifstream MyReadFile(fileName);

	// Use a while loop together with the getline() function to read the file line by line
	while (getline(MyReadFile, lineData)) {
		// Output the text from the file
		vec3 pos, dir, up;
		vector<string> values = split(lineData, ',');
		pos.x = stof(values[0]);
		pos.y = stof(values[1]);
		pos.z = stof(values[2]);
		getline(MyReadFile, lineData);
		values = split(lineData, ',');
		dir.x = stof(values[0]);
		dir.y = stof(values[1]);
		dir.z = stof(values[2]);
		getline(MyReadFile, lineData);
		values = split(lineData, ',');
		up.x = stof(values[0]);
		up.y = stof(values[1]);
		up.z = stof(values[2]);
		linepoints.push_back(PathPoint(pos, dir, up));
	}
	MyReadFile.close();

	update();
}



void ObjectPath::SavePathToFile(string fileName)
{
	ofstream file;
	file.open(fileName);
	if (!file.is_open())
	{
		cout << "warning! could not open pathinfo.txt file!" << endl;
	}

	for(int i = 0; i < linepoints.size(); i++)
	{
		PathPoint p = linepoints[i];
		file << p.pos.x << "," << p.pos.y << "," << p.pos.z << endl;
		file << p.dir.x << "," << p.dir.y << "," << p.dir.z << endl;
		file << p.up.x << "," << p.up.y << "," << p.up.z << endl;
		//file << p.roll << endl;
	}
	file.close();
	
}

PathPoint::PathPoint(vec3 _pos, vec3 _dir, vec3 _up)
{
	pos = _pos;
	//roll = _roll;
	dir = _dir;
	up = _up;
}

static float w = 0.0f;


/*
 * returns new position of plane
 */
vec3 update_plane(shared_ptr<Program> pplane, shared_ptr<Shape> plane,
	float frametime, mat4 P, mat4 V, ObjectPath *path, vec3 campos)
{
	mat4 M;
	vec3 positionOnLine;
	static mat4 lastNose = mat4(1);
	int currentIndex = 1;
	//static float w = 0.0f;
	w += frametime;
	float plane_time = w * 40.0f;
	int maxIndex = path->splinePoints.size() - 1;
	currentIndex = (int)plane_time % maxIndex;
	if((int)plane_time > maxIndex)
	{
		w = 0.0f;
		plane_time -= maxIndex;
	}
	float withDecimal = plane_time - (maxIndex * ((int)plane_time / maxIndex));
	float t = withDecimal - currentIndex;
  	positionOnLine = path->splinePoints[currentIndex] * (1.0f - t) + path->splinePoints[currentIndex + 1] * t;
	float sangle = -3.1415926 / 2.;
	glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(1, 0, 0));
	mat4 planeScale = scale(mat4(1), vec3(4.9));

	float rotationTime = plane_time / 118.0f;
	maxIndex = path->linepoints.size();
	currentIndex = (int)rotationTime % maxIndex;
	withDecimal = rotationTime - (maxIndex * ((int)rotationTime / maxIndex));
	t = withDecimal - currentIndex;
	PathPoint currentP = path->linepoints[currentIndex];
	if (currentIndex + 1 == maxIndex)
	{
		return path->linepoints[0].pos;
	}
	PathPoint nextP = path->linepoints[currentIndex + 1];
	mat4 R = linint_between_two_orientations(currentP.dir, currentP.up, nextP.dir, nextP.up, t);
	M = glm::translate(glm::mat4(1.0f), positionOnLine) * R *
		translate(mat4(1), vec3(0, .1, 0)) * RotateXPlane * planeScale;
	pplane->bind();
	glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	glUniform3fv(pplane->getUniform("campos"), 1, &campos[0]);
	plane->draw(pplane);			//render!!!!!!!
	pplane->unbind();

	return positionOnLine;
}



/*
 * CameraModes
 *	0 = midpoint follow
 *	1 = p1 follow
 *	2 = p2 follow
 */
mat4 generate_path_view(ObjectPath camPath, float frametime, ObjectPath p1,
	ObjectPath p2, camera mycam, vec3* pos_to_change, int cameraMode)
{
	vec3 positionOnLine;
	int currentIndex = 1;
	//static float w = 0.0f;
	float plane_time = w * 40.0f;
	int maxIndex = camPath.splinePoints.size() - 1;
	currentIndex = (int)plane_time % maxIndex;
	if ((int)plane_time > maxIndex)
	{
		w = 0.0f;
		plane_time -= maxIndex;
	}
	float withDecimal = plane_time - (maxIndex * ((int)plane_time / maxIndex));
	float t = withDecimal - currentIndex;
	positionOnLine = camPath.splinePoints[currentIndex] * (1.0f - t) + camPath.splinePoints[currentIndex + 1] * t;

	vec3 pointToLookAt;
	if(cameraMode == 0)
	{
		pointToLookAt = (p1.lastPos + p2.lastPos) / 2.0f;
	}

	pointToLookAt = normalize(pointToLookAt - positionOnLine);
	//pointToLookAt = normalize(pointToLookAt);
	//positionOnLine = vec3(0);
	mat4 m1 = lookAt(positionOnLine, positionOnLine + pointToLookAt, vec3(0, 1, 0));

	static float z_rot = 0.0f;
	int roll_input = -mycam.y + mycam.t;
	const float roll_speed = .03f;
	z_rot += roll_input * roll_speed;
	mat4 roll = rotate(mat4(1), z_rot, vec3(0, 0, 1));
	*pos_to_change = -positionOnLine;
	
	return roll * m1;// *translate(mat4(1), -positionOnLine);
}

float cosinterpolation(float t, float pctLinear)
{
	float ct = (1.f - (cos(t * 3.1415926535f) + 1.f) / 2.0f);
	return ct * (1.0 - pctLinear) + t * pctLinear;
}

mat4 linint_between_two_orientations(glm::vec3 ez_aka_lookto_1, glm::vec3 ey_aka_up_1, glm::vec3 ez_aka_lookto_2,
	glm::vec3 ey_aka_up_2, float t)
{
	t = cosinterpolation(t, .5f);
	glm::mat4 m1, m2;
	glm::quat q1, q2;
	glm::vec3 ex, ey, ez;
	ey = ey_aka_up_1;
	ez = ez_aka_lookto_1;
	ex = cross(ey, ez);
	m1[0][0] = ex.x;		m1[0][1] = ex.y;		m1[0][2] = ex.z;		m1[0][3] = 0;
	m1[1][0] = ey.x;		m1[1][1] = ey.y;		m1[1][2] = ey.z;		m1[1][3] = 0;
	m1[2][0] = ez.x;		m1[2][1] = ez.y;		m1[2][2] = ez.z;		m1[2][3] = 0;
	m1[3][0] = 0;			m1[3][1] = 0;			m1[3][2] = 0;			m1[3][3] = 1.0f;
	ey = ey_aka_up_2;
	ez = ez_aka_lookto_2;
	ex = cross(ey, ez);
	m2[0][0] = ex.x;		m2[0][1] = ex.y;		m2[0][2] = ex.z;		m2[0][3] = 0;
	m2[1][0] = ey.x;		m2[1][1] = ey.y;		m2[1][2] = ey.z;		m2[1][3] = 0;
	m2[2][0] = ez.x;		m2[2][1] = ez.y;		m2[2][2] = ez.z;		m2[2][3] = 0;
	m2[3][0] = 0;			m2[3][1] = 0;			m2[3][2] = 0;			m2[3][3] = 1.0f;
	q1 = glm::quat(m1);
	q2 = glm::quat(m2);
	glm::quat qt = glm::slerp(q1, q2, t); //<---
	qt = glm::normalize(qt);
	glm::mat4 mt = glm::mat4(qt);
	//mt = transpose(mt);		 //<---
	return mt;
}
