
#include <math.h>
#define _USE_MATH_DEFINES
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include "glut.h"
#elif __APPLE__
#include <GLUT/GLUT.h>
#endif

using namespace std;
int no = 0;
int animaciones = 0;
bool* animandose;

float PI = M_PI;

GLfloat ortleft = -6, ortright = 6, ortbottom = -6, orttop = 6, ortzNear = -10, ortzFar = 10;
GLfloat eyex = 0, eyey = 0, eyez = 0, centerx = 0, centery = 0, centerz = -1, upx = 0, upy = 1, upz = 0;

enum { //Tipos de animacion
	LINEAR = 0,
	EASE_IN,
	EASE_OUT,
	EASE_IN_OUT
};

enum { //Formas primitivas
	SPHERE = 0,
	CUBE,
	TEAPOT,
	CONE,
	WIRE_SPHERE,
	WIRE_CUBE,
	WIRE_TEAPOT,
	WIRE_CONE

};

struct vec {
	float x = 0, y = 0, z = 0;
};

struct obj {
	int id; //Id del objeto
	int forma; //Forma del objeto, primitivo
	float s, h; //Tamaño, altura
	int slices, stacks;
	float x = 0, y = 0, z = 0; //Posicion inicial
	float rx = 0, ry = 0, rz = 0; //Rotacion inicial
	float sx = 1, sy = 1, sz = 1; //Escalas
	float r=0, g=0, b=0; //Color
	int frame_aparicion = 0; //Frame en el que aparece el objeto
	int frame_adios = -1; //Usar -1 para evitar que desaparezca el objeto
	int padre = -1; //Padre del objeto, -1 si no tiene

	void reset() {
		id = -1;
		forma = SPHERE;
		s = h = x = y = z = rx = ry = rz = r = g = b = frame_aparicion = 0;
		sx = sy = sz = 1;
		frame_adios = padre = -1;
	}

	void print() {
		printf("id %d, forma %d, s %f, padre %d\n", id, forma, s, padre);
	}
};

struct anim { //struct que representa una animacion de objeto
	int obj_id; //Id del objeto a animar
	int type; //Tipo de animacion
	int frame; //Frame en el que empieza la animacion
	int frame_end; //Frame en el que termina la animacion
	float x = 0, y = 0, z = 0; //Traslacion
	float rx = 0, ry = 0, rz = 0; //Rotacion, angulo
	bool done = false;

	void reset() {
		obj_id = -1;
		type = LINEAR;
		frame = 0;
		frame_end = 0;
		x = y = z = rx = ry = rz = 0;
		done = false;

	}
};

struct anim* lista;
struct vec* posiciones;
struct vec* rotaciones;
struct obj* objetos;


struct anim crearAnim(int obj_id, int type, int frame, int frame_end, float x, float y, float z, float rx, float ry, float rz) {
	struct anim a;

	a.obj_id = obj_id;
	a.type = type;
	a.frame = frame;
	a.frame_end = frame_end;
	a.x = x;
	a.y = y;
	a.z = z;
	a.rx = rx;
	a.ry = ry;
	a.rz = rz;
	a.done = false;

	return a;
}

struct obj crearObj(int id, int forma, float s, float h, int slices, int stacks, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b, int frame_aparicion, int frame_adios, int padre) {
	struct obj o;

	o.id = id;
	o.forma = forma;
	o.s = s;
	o.h = h;
	o.slices = slices;
	o.stacks = stacks;
	o.x = x;
	o.y = y;
	o.z = z;
	o.rx = rx;
	o.ry = ry;
	o.rz = rz;
	o.sx = sx;
	o.sy = sy;
	o.sz = sz;
	o.r = r;
	o.g = g;
	o.b = b;
	o.frame_aparicion = frame_aparicion;
	o.frame_adios = frame_adios;
	o.padre = padre;

	return o;
}

float Linear(float inicial, float fin, int i, int t, int total) {
	float ti = ((t - i) / ((float)(total-i)));
	return (inicial * (1 - ti)) + (ti*fin);
}

float EaseIn(float inicial, float fin, int i, int t, int total) {
	float ti = ((t - i) / ((float)(total - i)));
	return inicial + (fin - inicial) + ((cosf(ti * PI/2))*-(fin - inicial));
}

float EaseOut(float inicial, float fin, int i, int t, int total) {
	float ti = ((t - i) / ((float)(total - i)));
	return inicial + (sinf(ti* PI/2))*(fin - inicial);
}

float EaseInOut(float inicial, float fin, int i, int t, int total) {
	float ti = ((t - i) / ((float)(total - i)));
	return inicial + ((sinf(ti*PI-PI/2)+1)/2)*(fin-inicial);
}

bool hayAnimacion(int id, int frame) {
	for (int i = 0;i < animaciones; i++) {
		if (lista[i].obj_id == id) {
			if (lista[i].frame <= frame && lista[i].frame_end >= frame) {
				animandose[id] = true;
				return true;
			}
		}
	}
	animandose[id] = false;
	return false;
}

void animar(struct anim &a, int id, int frame_actual) {

	if (frame_actual >= a.frame && frame_actual <= a.frame_end && !a.done) {
		float x= posiciones[id].x, y= posiciones[id].y, z= posiciones[id].z, rx= rotaciones[id].x, ry= rotaciones[id].y, rz= rotaciones[id].z;
		switch (a.type) {
		case LINEAR:
			x = Linear(0, a.x, a.frame, frame_actual, a.frame_end);
			y = Linear(0, a.y, a.frame, frame_actual, a.frame_end);
			z = Linear(0, a.z, a.frame, frame_actual, a.frame_end);
						
			rx = Linear(0, a.rx, a.frame, frame_actual, a.frame_end);
			ry = Linear(0, a.ry, a.frame, frame_actual, a.frame_end);
			rz = Linear(0, a.rz, a.frame, frame_actual, a.frame_end);
			glTranslatef(x, y, z);
			glRotatef(rx, 1, 0, 0);
			glRotatef(ry, 0, 1, 0);
			glRotatef(rz, 0, 0, 1);

			break;
		case EASE_IN:
			x = EaseIn(0, a.x, a.frame, frame_actual, a.frame_end);
			y = EaseIn(0, a.y, a.frame, frame_actual, a.frame_end);
			z = EaseIn(0, a.z, a.frame, frame_actual, a.frame_end);

			rx = EaseIn(0, a.rx, a.frame, frame_actual, a.frame_end);
			ry = EaseIn(0, a.ry, a.frame, frame_actual, a.frame_end);
			rz = EaseIn(0, a.rz, a.frame, frame_actual, a.frame_end);
			glTranslatef(x, y, z);
			glRotatef(rx, 1, 0, 0);
			glRotatef(ry, 0, 1, 0);
			glRotatef(rz, 0, 0, 1);

			break;
		case EASE_OUT:
			x = EaseOut(0, a.x, a.frame, frame_actual, a.frame_end);
			y = EaseOut(0, a.y, a.frame, frame_actual, a.frame_end);
			z = EaseOut(0, a.z, a.frame, frame_actual, a.frame_end);

			rx = EaseOut(0, a.rx, a.frame, frame_actual, a.frame_end);
			ry = EaseOut(0, a.ry, a.frame, frame_actual, a.frame_end);
			rz = EaseOut(0, a.rz, a.frame, frame_actual, a.frame_end);
			glTranslatef(x, y, z);
			glRotatef(rx, 1, 0, 0);
			glRotatef(ry, 0, 1, 0);
			glRotatef(rz, 0, 0, 1);
			break;
		case EASE_IN_OUT:
			x = EaseInOut(0, a.x, a.frame, frame_actual, a.frame_end);
			y = EaseInOut(0, a.y, a.frame, frame_actual, a.frame_end);
			z = EaseInOut(0, a.z, a.frame, frame_actual, a.frame_end);

			rx = EaseInOut(0, a.rx, a.frame, frame_actual, a.frame_end);
			ry = EaseInOut(0, a.ry, a.frame, frame_actual, a.frame_end);
			rz = EaseInOut(0, a.rz, a.frame, frame_actual, a.frame_end);
			glTranslatef(x, y, z);
			glRotatef(rx, 1, 0, 0);
			glRotatef(ry, 0, 1, 0);
			glRotatef(rz, 0, 0, 1);
			break;
		}
	}
	
	if (frame_actual >= a.frame_end && !a.done) {
		if (a.done == false) {
			a.done = true;

			posiciones[id].x += a.x;
			posiciones[id].y += a.y;
			posiciones[id].z += a.z;

			rotaciones[id].x += a.rx;
			rotaciones[id].y += a.ry;
			rotaciones[id].z += a.rz;

			a.obj_id = -1;
			
		}
	}
}

void BuscarAnimacion(int id, int frame) {
	for (int i = 0;i < animaciones;i++) {
		struct anim &a = lista[i];
		
			if (a.obj_id == id) {
				animar(a, id, frame);
			}

	}
}

void Init();
void Display();
void Reshape();

void Init()
{
	glClearColor(1, 1, 1, 1);
	glEnable(GL_DEPTH_TEST);

}

void update(int id, int frame) {
	struct obj o = objetos[id];
	glTranslatef(o.x, o.y, o.z);
	glRotatef(o.rx, 1, 0, 0);
	glRotatef(o.ry, 0, 1, 0);
	glRotatef(o.rz, 0, 0, 1);
	glTranslatef(posiciones[id].x, posiciones[id].y, posiciones[id].z);
	glRotatef(rotaciones[id].x, 1, 0, 0);
	glRotatef(rotaciones[id].y, 0, 1, 0);
	glRotatef(rotaciones[id].z, 0, 0, 1);
}

void cargar(int id, int frame) {

	if (objetos[id].padre != -1) {
		cargar(objetos[id].padre, frame);
	}
	hayAnimacion(id, frame);
	update(id, frame);
	BuscarAnimacion(id, frame);
}

void displayObj(int id, int frame, struct obj &o) {
	glColor3f(o.r, o.g, o.b);

	glLoadIdentity();

	cargar(id, frame);

	glScalef(o.sx, o.sy, o.sz);

	switch (o.forma) {
		case SPHERE:
			glutSolidSphere(o.s, o.slices, o.stacks);
			break;
		case CUBE:
			glutSolidCube(o.s);
			break;
		case TEAPOT:
			glutSolidTeapot(o.s);
			break;
		case CONE:
			glutSolidCone(o.s, o.h, o.slices, o.stacks);
			break;
		case WIRE_SPHERE:
			glutWireSphere(o.s, o.slices, o.stacks);
			break;
		case WIRE_CUBE:
			glutWireCube(o.s);
			break;
		case WIRE_TEAPOT:
			glutWireTeapot(o.s);
			break;
		case WIRE_CONE:
			glutWireCone(o.s, o.h, o.slices, o.stacks);
			break;
	}

	
}

void ponerObj(int id, int frame) {
	struct obj o = objetos[id];
	if (o.frame_adios < 0) {
		if (o.frame_aparicion <= frame) {
			displayObj(id, frame, o);
		}
	}
	else {
		if (o.frame_aparicion <= frame && frame <= o.frame_adios) {
			displayObj(id, frame, o);
		}
	}
}



void Display()
{
	int msec = glutGet(GLUT_ELAPSED_TIME);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);


	for (int i = 0;i < no;i++) {
		ponerObj(i, msec);
	}

	glutSwapBuffers();
	glutPostRedisplay();

}



void Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(ortleft, ortright, ortbottom, orttop, ortzNear, ortzFar);
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
}

char lowercase(char c) {
	if (c <= 'Z' && c >= 'A')
		return c - ('Z' - 'z');
	return c;
}


void leer() {

	animaciones = 0;
	no = 0;

	ifstream file;
	file.open("formato.txt");
	
	if (file.fail()) {
		exit(1);
	}

	int o = 0, a = 0;
	bool anim = false, obj = false;
	
	string line;
	struct anim animacion;
	struct obj objeto;

	while (getline(file, line))
	{
		replace(line.begin(), line.end(), ':', ' ');

		istringstream p(line);

		string param, value;

		if (p >> param >> value) {

			//cout << string("param ") << param << "\n";
			//cout << string("value ") << value << "\n";

			transform(param.begin(), param.end(), param.begin(), lowercase);
			transform(value.begin(), value.end(), value.begin(), lowercase);

			if (param == "objects") {
				no = stoi(value);
				objetos = (struct obj *)malloc(sizeof(struct obj) * no);
			}
			else if (param == "animations") {
				animaciones = stoi(value);
				lista = (struct anim *)malloc(sizeof(struct anim) * animaciones);
			}
			else if (param == "object") {
				if (obj) {
					if (value == "sphere") {
						objeto.forma = SPHERE;
					}
					else if (value == "cube") {
						objeto.forma = CUBE;
					}
					else if (value == "teapot") {
						objeto.forma = TEAPOT;
					}
					else if (value == "cone") {
						objeto.forma = CONE;
					}
					else if (value == "wiresphere") {
						objeto.forma = WIRE_SPHERE;
					}
					else if (value == "wirecube") {
						objeto.forma = WIRE_CUBE;
					}
					else if (value == "wireteapot") {
						objeto.forma = WIRE_TEAPOT;
					}
					else if (value == "wirecone") {
						objeto.forma = WIRE_CONE;
					}

				}
			}
			else if (param == "size") {
				if (obj) {
					objeto.s = stof(value);
				}
			}
			else if (param == "height") {
				if (obj) {
					objeto.h = stof(value);
				}
			}
			else if (param == "slices") {
				if (obj) {
					objeto.slices = stoi(value);
				}
			}
			else if (param == "stacks") {
				if (obj) {
					objeto.stacks = stoi(value);
				}
			}
			else if (param == "x") {
				if (obj) {
					objeto.x = stof(value);
				}
				else if (anim) {
					animacion.x = stof(value);
				}
			}
			else if (param == "y") {
				if (obj) {
					objeto.y = stof(value);
				}
				else if (anim) {
					animacion.y = stof(value);
				}
			}
			else if (param == "z") {
				if (obj) {
					objeto.z = stof(value);
				}
				else if (anim) {
					animacion.z = stof(value);
				}
			}
			else if (param == "rx") {
				if (obj) {
					objeto.rx = stof(value);
				}
				else if (anim) {
					animacion.rx = stof(value);
				}
			}
			else if (param == "ry") {
				if (obj) {
					objeto.ry = stof(value);
				}
				else if (anim) {
					animacion.ry = stof(value);
				}
			}
			else if (param == "rz") {
				if (obj) {
					objeto.rz = stof(value);
				}
				else if (anim) {
					animacion.rz = stof(value);
				}
			}
			else if (param == "sx") {
				if (obj) {
					objeto.sx = stof(value);
				}
			}
			else if (param == "sy") {
				if (obj) {
					objeto.sy = stof(value);
				}
			}
			else if (param == "sz") {
				if (obj) {
					objeto.sz = stof(value);
				}
			}
			else if (param == "r") {
				if (obj) {
					objeto.r = stof(value);
				}
			}
			else if (param == "g") {
				if (obj) {
					objeto.g = stof(value);
				}
			}
			else if (param == "b") {
				if (obj) {
					objeto.b = stof(value);
				}
			}
			else if (param == "start") {
				if (obj) {
					objeto.frame_aparicion = (int)(stof(value) * 1000);
				}
				else if (anim) {
					animacion.frame = (int)(stof(value) * 1000);
				}
			}
			else if (param == "end") {
				if (obj) {
					objeto.frame_adios = (int)(stof(value) * 1000);
				}
				else if (anim) {
					animacion.frame_end = (int)(stof(value) * 1000);
				}
			}
			else if (param == "parent") {
				if (obj) {
					objeto.padre = stoi(value);
				}
			}
			else if (param == "objectid") {
				if (anim) {
					animacion.obj_id = stoi(value);
				}
			}
			else if (param == "type") {
				if (anim) {
					if (value == "linear") {
						animacion.type = LINEAR;
					}
					else if (value == "easein") {
						animacion.type = EASE_IN;
					}
					else if (value == "easeout") {
						animacion.type = EASE_OUT;
					}
					else if (value == "easeinout") {
						animacion.type = EASE_IN_OUT;
					}
				}
			}
			else if (param == "left") {
				ortleft = stof(value);
			}
			else if (param == "right") {
				ortright = stof(value);
			}
			else if (param == "bottom") {
				ortbottom = stof(value);
			}
			else if (param == "top") {
				orttop = stof(value);
			}
			else if (param == "znear") {
				ortzNear = stof(value);
			}
			else if (param == "zfar") {
				ortzFar = stof(value);
			}
			else if (param == "eyex") {
				eyex = stof(value);
			}
			else if (param == "eyey") {
				eyey = stof(value);
			}
			else if (param == "eyez") {
				eyez = stof(value);
			}
			else if (param == "centerx") {
				centerx = stof(value);
			}
			else if (param == "centery") {
				centery = stof(value);
			}
			else if (param == "centerz") {
				centerz = stof(value);
			}
			else if (param == "upx") {
				upx = stof(value);
			}
			else if (param == "upy") {
				upy = stof(value);
			}
			else if (param == "upz") {
				upz = stof(value);
			}

		}
		else {
			p = istringstream(line);
		}
		if (p >> param) {
			//cout << string("param ") << param << "\n";
			transform(param.begin(), param.end(), param.begin(), lowercase);
			if (param == "object") {
				anim = false;
				obj = true;
			}
			else if (param == "animation") {
				anim = true;
				obj = false;
			}
			else if (param == "-") {
				//Terminamos de procesar el objeto/animacion
				if (obj) {
					if (o < no) {
						objeto.id = o;
						//objeto.print();
						objetos[o] = objeto;
						o++;
						objeto.reset();
						//printf("Insertando objeto %d\n", o);
						
					}

					obj = false;
				}
				else if (anim) {
					if (a < animaciones) {
						lista[a] = animacion;
						a++;
						animacion.reset();
						//printf("Insertando animacion %d\n", a);
					}
					anim = false;
				}
			}
		}
		else {

		}

	}
	file.close();
}




int main(int artcp, char **argv)
{
	leer();

	posiciones = (struct vec *)malloc(sizeof(struct vec) * no);
	for (int i = 0;i < no;i++) {
		posiciones[i].x = 0;
		posiciones[i].y = 0;
		posiciones[i].z = 0;
	}
	rotaciones = (struct vec *)malloc(sizeof(struct vec) * no);
	for (int i = 0;i < no;i++) {
		rotaciones[i].x = 0;
		rotaciones[i].y = 0;
		rotaciones[i].z = 0;
	}
	animandose = (bool *)malloc(sizeof(bool) * no);
	
	glutInit(&artcp, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutCreateWindow("Animacion");
	
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutTimerFunc(16, Timer, 0);
	Init();
	glutMainLoop();
	return 0;

}