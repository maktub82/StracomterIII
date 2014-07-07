#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <sstream>

#include "Polygon.h"
#include "MotorGrafico.h"
#include "TWindow.h"
#include "Object.h"
#include "Character.h"
#include "IOControl.h"
#include "Light.h"
#include "LightManager.h"
#include "Camera.h"
#include "Transformacion.h"
#include "TransformStack.h"
#include "Entity.h"
#include "EntityManager.h"
#include "GraphicTree.h"
#include "CoreNamespace.h"
#include "Shape.h"
#include "Polygon.h"
#include "World.h"
#include "Model.h"
#include "SceneManager.h"
#include "Text.h"
#include "Textura.h"

using namespace std;

//CONSTANTES
const int MODELO = 1;				//ID para la DisplayList.
const int ESFERA = 2;				//ID para la DisplayList.
const int QUAD = 3;					//ID para la DisplayList.
const int TEXTURA = 1;
const int WIDTH = 800;				//Ancho de la pantalla.
const int HEIGHT = 600;				//Alto de la pantalla.
const float ZOOM = 0.25;				//Variación del Zoom.
const float TRANS = 0.1;				//Variación de Translación.
const float ROT = 0.5;				//Variación de la Rotación.
const float COLOR = 0.01;			//Variación de Color.
const float INITZOOM = -1000;
const float INITTRANSX = 0;
const float INITTRANSY = 0;
const float INITTRANSZ = 0;
const float INITROTX = -40;
const float INITROTY = 145;
const float INITROTZ = 0;
const float MAX = 5;

//MENSAJES:
const string M_NAME = "Cargando el modelo: ";
const string TXT_TITLE = "Cargador de Texturas";
const string TXT_TRANS = "Translacion: W S A D";
const string TXT_ROT = "Rotacion: I K J L U O";
const string TXT_ZOOM = "Zoom: Up Down";
const string TXT_COLOR = "Color: R G B";
const string TXT_TEXTURA = "Textura: ";
const string TXT_MODELO = "Modelo: ";
const string TXT_RESET = "Valores por defecto: F8 ";
const string TXT_PUNTOS = "Número de vértices: ";

//Variables Globales [Entidades]
World *mundo;						//Mundo donde existen las entidades.
LightManager *manager;				//Gestor de luces.
ResourceManager *resource;			//Gestor de Recursos.
GraphicTree *tree;					//Árbol de renderizado.
int luz0, luz1, luz2;				//ID de las luces 0,1,2.
Element *light0, *light1, *light2;	//Puntero a las luces 0,1,2.
//Element *camera;					//Puntero a la cámara.
//Element *hud;						//Puntero a la cámara.
//Element *tescenario;				//Puntero a la transformación del escenario.

Element *modelo;					//Puntero a objeto Model que contiene el modelo.
Element *ejeX, *ejeY, *ejeZ;		//Puntero a objetos Shape de los tres ejes.
//Element *tmodelo;					//Transformación del modelo.
//Element *thud;						//Transformación del HUD.
SceneManager *scene;				//Escena.

Textura textura;

//Variables Globales [Escenario]
float aX = INITROTX;						//Giro en el eje X.
float aY = INITROTY;						//Giro en el eje Y.
float aZ = INITROTZ;						//Giro en el eje Z.
float tX = INITTRANSX;						//Translación en el eje X.
float tY= INITTRANSY;						//Translación en el eje Y.
float tZ = INITTRANSZ;						//Translación en el eje Z.
float zoom = INITZOOM;						//Zoom.

//Variables Globales [Auxiliares]
Color<float> color;					//Color del modelo.
string rutaM;						//Ruta hasta el modelo 3ds.
string rutaT;						//Ruta hasta la textura.
Text text;							//Texto
int intervalo;
int IDdisplay;
bool sube;


//Funciones
void Init(int, char*[]);			//Inicializa la escena y los recursos.
void inicializarOpenGL();			//Inicializa parámetros de OpenGL
void makeTree();					//Crea el árbol de renderizado.
void Update();						//Actualiza el estado del escenario.
void Render();						//Renderiza la escena, a través de árbol.
void Teclado(IOControl*, TWindow*);	//Controla los eventos de teclado (y ratón).
void Clear();						//Elimina todos los punteros.


int main(int argc, char *argv[])
{

	IOControl *eventosTeclado = NULL;
	TWindow window;
	window.openSFML(WIDTH,HEIGHT,false);
	eventosTeclado = new IOControl(&window);

	Init(argc, argv);

	while (window.isOpen()){

		Teclado(eventosTeclado, &window);
		Update();
		Render();
		window.display(false);
	}

	Clear();

	return 0;
}



void Render() {

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	scene->DrawScene();
	glEnable(GL_DEPTH_TEST);
	glCallList(QUAD);
}

void Teclado(IOControl *teclado, TWindow *window){

	sf::Event event;

	if(teclado->IsKeyDown(Core::Key::Escape))
		window->close();

	//EJE X
	if (teclado->IsKeyDown(Core::Key::K)) {
		aX -= ROT;
	}
	if (teclado->IsKeyDown(Core::Key::I)) {
		aX += ROT;
	}

	//EJE Y
	if (teclado->IsKeyDown(Core::Key::J)) {
		aY += ROT;
	}
	if (teclado->IsKeyDown(Core::Key::L)) {
		aY -= ROT;
	}

	//EJE Z
	if (teclado->IsKeyDown(Core::Key::O)) {
		aZ -= ROT;
	}
	if (teclado->IsKeyDown(Core::Key::U)) {
		aZ += ROT;
	}

	//TRANSLACIÓN X
	if (teclado->IsKeyDown(Core::Key::D)) {
		tX -= TRANS;
	}
	if (teclado->IsKeyDown(Core::Key::A)) {
		tX += TRANS;
	}

	//TRANSLACIÓN Y
	if (teclado->IsKeyDown(Core::Key::W)) {
		tY -= TRANS;
	}
	if (teclado->IsKeyDown(Core::Key::S)) {
		tY += TRANS;
	}

	//ZOOM
	if (teclado->IsKeyDown(Core::Key::Up)) {
		zoom += ZOOM;
	}
	if (teclado->IsKeyDown(Core::Key::Down)) {
		zoom -= ZOOM;
	}

	//COLOR R
	if (teclado->IsKeyDown(Core::Key::R)) {
		float aux = color.getR() + COLOR;
		if(aux > 1)
			aux -= 1;
		color.setR(aux);
	}

	//COLOR G
	if (teclado->IsKeyDown(Core::Key::G)) {
		float aux = color.getG() + COLOR;
		if(aux > 1)
			aux -= 1;
		color.setG(aux);
	}

	//COLOR B
	if (teclado->IsKeyDown(Core::Key::B)) {
		float aux = color.getB() + COLOR;
		if(aux > 1)
			aux -= 1;
		color.setB(aux);
	}

	while(window->getEvent(event))
	{
		//CAMBIO LA CÁMARA [ORTHO/PERSPECTIVE]
		if (event.Type == sf::Event::KeyReleased &&
				event.Key.Code == sf::Keyboard::P) {

			Element* camera = scene->getNode(Graphics::DirectNodes::c_escena)->getItem();
			switch(((Camera*)camera)->getProjection())
			{
			case Graphics::Projection::c_ortho:
				((Camera*)camera)->setPerspective();
				break;
			case Graphics::Projection::c_perspective:
				((Camera*)camera)->setOrtho();
				break;
			case Graphics::Projection::c_none:
				((Camera*)camera)->setPerspective();
				break;
			}

		}else if(event.Type == sf::Event::KeyReleased &&
				event.Key.Code == sf::Keyboard::F8) {
				aX = INITROTX;
				aY = INITROTY;
				aZ = INITROTZ;
				tX = INITTRANSX;
				tY= INITTRANSY;
				tZ = INITTRANSZ;
				zoom = INITZOOM;
		}
		//ACTIVAR/DESACTIVAR LUZ 0
		else if (event.Type == sf::Event::KeyReleased &&
				event.Key.Code == sf::Keyboard::Num1) {

			if(manager->isActive(0))
			{
				manager->switchOff(0);
			}
			else
			{
				manager->switchOn(0);
			}

		}
		//ACTIVAR/DESACTIVAR LUZ 1
		else if (event.Type == sf::Event::KeyReleased &&
				event.Key.Code == sf::Keyboard::Num2) {

			if(manager->isActive(1))
			{
				manager->switchOff(1);
			}
			else
			{
				manager->switchOn(1);
			}

		}
		//ACTIVAR/DESACTIVAR LUZ 2
		else if (event.Type == sf::Event::KeyReleased &&
				event.Key.Code == sf::Keyboard::Num3) {

			if(manager->isActive(2))
			{
				manager->switchOff(2);
			}
			else
			{
				manager->switchOn(2);
			}

		}
		//CERRAR LA PANTALLA
		else if (event.Type == sf::Event::Closed) {
			window->close();
		} else if (event.Type == sf::Event::KeyReleased
				&& event.Key.Code == sf::Keyboard::Escape) {
			window->close();
		}
	}

}

void inicializarOpenGL() {

	GLfloat luzAmbiente[]   = {0.1, 0.1, 0.1, 1.0};
	GLfloat luzDifusa[]     = {0.7, 0.7, 0.7, 1.0};
	GLfloat luzEspecular[]  = {0.9, 0.9, 0.9, 1.0};
	GLfloat luzExponente[]  = {64.0, 64.0, 64.0, 1.0};

	Color<float> amb(0.1, 0.1, 0.1, 1.0);
	Color<float> dif(0.7, 0.7, 0.7, 1.0);
	Color<float> esp(0.9, 0.9, 0.9, 1.0);
	Point pos0(0, 0, 10);
	Point pos1(0, 10, 0);
	Point pos2(10, 0, 0);

	glClearIndex((GLfloat) 0);
	glClearDepth(1000.0);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_NORMALIZE);

	// Habilita los materiales
	glEnable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT, GL_AMBIENT, luzAmbiente);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, luzDifusa);
	glMaterialfv(GL_FRONT, GL_SPECULAR, luzEspecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, luzExponente);

	//Crear las luces
	manager = new LightManager();
	luz0 = manager->addLight(amb,dif,esp,pos0);
	luz1 = manager->addLight(amb,dif,esp,pos1);
	luz2 = manager->addLight(amb,dif,esp,pos2);

	light0 = manager->getLight(luz0);
	light1 = manager->getLight(luz1);
	light2 = manager->getLight(luz2);

	//manager->exportOpenGL();


	//Suavizado
	glShadeModel(GL_SMOOTH);

	// Luz Ambiente
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

	glEnable(GL_NORMALIZE);

}

void makeTree() {


	Text *txt_title;
	Text *txt_trans;
	Text *txt_rot;
	Text *txt_zoom;
	Text *txt_color;
	Text *txt_textura;
	Text *txt_reset;
	Text *txt_modelo;
	Text *txt_puntos;


	scene->makeStracomterTree();

	ejeX = new Shape(Polygon(Point(0,0,0), Point(100,0,0)), Color<float>(1, 0, 0));
	ejeY = new Shape(Polygon(Point(0,0,0), Point(0,100,0)), Color<float>(0, 1, 0));
	ejeZ = new Shape(Polygon(Point(0,0,0), Point(0,0,100)), Color<float>(0, 0, 1));

	txt_title = new Text(Graphics::Font::Monospace, Point(-395,-280,0), Color<float>(0.8,0.8,0.8,1), 20, TXT_TITLE);
	txt_rot = new Text(Graphics::Font::Monospace, Point(-395,-260,0), Color<float>(0.8,0.8,0.8,1), 16, TXT_ROT);
	txt_zoom = new Text(Graphics::Font::Monospace, Point(-395,-240,0), Color<float>(0.8,0.8,0.8,1), 16, TXT_ZOOM);
	txt_trans = new Text(Graphics::Font::Monospace, Point(-395,-220,0), Color<float>(0.8,0.8,0.8,1), 16, TXT_TRANS);
	txt_color = new Text(Graphics::Font::Monospace, Point(-395,-200,0), Color<float>(0.8,0.8,0.8,1), 16, TXT_COLOR);
	txt_reset = new Text(Graphics::Font::Monospace, Point(-395,-180,0), Color<float>(0.8,0.8,0.8,1), 16, TXT_RESET);
	txt_textura = new Text(Graphics::Font::Monospace, Point(-395,260,0), Color<float>(0.8,0.8,0.8,1), 14, TXT_TEXTURA + rutaT);
	txt_modelo = new Text(Graphics::Font::Monospace, Point(-395,280,0), Color<float>(0.8,0.8,0.8,1), 14, TXT_MODELO + rutaM);



	//cout << "rutaM " << rutaM << endl;
	//cout << "rutaT: " << rutaT << endl;

	rutaT = "Graphics/Resources/texturas/sniper_flat.tga";
	textura = Textura(rutaT, TEXTURA);

	modelo = new Model("Graphics/Resources/modelos/sn1.3ds",5001,textura);
	((Model*)modelo)->createDisplayList();

	modelo = new Model("Graphics/Resources/modelos/sn2.3ds",5002,textura);
	((Model*)modelo)->createDisplayList();


	modelo = new Model("Graphics/Resources/modelos/sn3.3ds",5003,textura);
	((Model*)modelo)->createDisplayList();

	modelo = new Model("Graphics/Resources/modelos/sn4.3ds",5004,textura);
	((Model*)modelo)->createDisplayList();

	modelo = new Model("Graphics/Resources/modelos/sn5.3ds",5005,textura);
	((Model*)modelo)->createDisplayList();


	//((Model*)modelo)->setCall(3);

	stringstream ss;
	ss << TXT_PUNTOS << ((Model *)modelo)->getVertices();
	txt_puntos = new Text(Graphics::Font::Monospace, Point(-395,240,0), Color<float>(0.8,0.8,0.8,1), 14, ss.str());


	for(unsigned int i = 0; i < scene->getDirectNodes().size();i++)
		if(scene->getDirectNodes()[i] != NULL)
			scene->getDirectNodes()[i]->setVisible(false);

	scene->getNode(Graphics::DirectNodes::t_escena)->setVisible(true);
	scene->getNode(Graphics::DirectNodes::c_escena)->setVisible(true);
	scene->getNode(Graphics::DirectNodes::c_hud)->setVisible(true);
	scene->getNode(Graphics::DirectNodes::t_screen)->setVisible(true);

	if(scene->getNode(Graphics::DirectNodes::t_light0) != NULL)
	{
		scene->getNode(Graphics::DirectNodes::t_light0)->setVisible(true);
		((Light*)scene->getNode(Graphics::DirectNodes::t_light0)->getItem())->setPosicion(Point(0,0,-10));
	}
	if(scene->getNode(Graphics::DirectNodes::t_light1) != NULL)
	{
		scene->getNode(Graphics::DirectNodes::t_light1)->setVisible(true);
		((Light*)scene->getNode(Graphics::DirectNodes::t_light1)->getItem())->setPosicion(Point(0,-10,0));
	}
	if(scene->getNode(Graphics::DirectNodes::t_light2) != NULL)
	{
		scene->getNode(Graphics::DirectNodes::t_light2)->setVisible(true);
		((Light*)scene->getNode(Graphics::DirectNodes::t_light2)->getItem())->setPosicion(Point(-10,0,0));
	}

	GraphicNode *nodo = scene->getNode(Graphics::DirectNodes::t_escena);
	scene->addElement(ejeX,nodo);
	scene->addElement(ejeY,nodo);
	scene->addElement(ejeZ,nodo);
	scene->addElement(modelo,nodo);

	nodo = scene->getNode(Graphics::DirectNodes::t_screen);
	((Transform*)nodo->getItem())->addScale(1,-1,1);
	scene->addElement(txt_title, nodo);
	scene->addElement(txt_rot, nodo);
	scene->addElement(txt_zoom, nodo);
	scene->addElement(txt_trans, nodo);
	scene->addElement(txt_color, nodo);
	scene->addElement(txt_reset, nodo);
	scene->addElement(txt_textura, nodo);
	scene->addElement(txt_modelo, nodo);
	scene->addElement(txt_puntos, nodo);

}

void Update() {

	((Model*)modelo)->setColor(color);
	Element *tescenario = scene->getNode(Graphics::DirectNodes::t_escena)->getItem();
	((Transform*)tescenario)->setIdentity();
	((Transform*)tescenario)->addTranslation(tX,tY,zoom);
	((Transform*)tescenario)->addRotationY(180);
	((Transform*)tescenario)->addRotationX(aX);
	((Transform*)tescenario)->addRotationY(aY);
	((Transform*)tescenario)->addRotationZ(aZ);

	intervalo++;
	if(intervalo >= 50)
	{

		IDdisplay++;
		if(IDdisplay >= MAX+1)
			IDdisplay = 1;

		((Model*)modelo)->setCall(5000 + IDdisplay);
		intervalo = 0;
	}

	cout << IDdisplay << endl;

}

void Init(int argc, char *argv[]) {



	color = Color<float>(1,1,1);
	inicializarOpenGL();
	scene = SceneManager::getInstance();
	makeTree();

	scene->setSmooth(true);
	scene->setZbuffer(true);
	scene->setLuzAmbiente(Color<float>(0.1, 0.1, 0.1, 1.0));

	text.setFont(Graphics::Font::SansSerif);
	text.setColor(Color<float>(1,1,1,0.9));
	intervalo = 0;
	IDdisplay = 1;
	sube  = true;

}

void Clear() {

	if(mundo != NULL)
	{
		delete mundo;
		mundo = NULL;
	}

	if(manager != NULL)
	{
		delete manager;
		manager = NULL;
	}

	if(resource != NULL)
	{
		delete resource;
		resource = NULL;
	}


	if(ejeX != NULL)
	{
		delete ejeX;
		ejeX = NULL;
	}

	if(ejeY != NULL)
	{
		delete ejeY;
		ejeY = NULL;
	}

	if(ejeZ != NULL)
	{
		delete ejeZ;
		ejeZ = NULL;
	}

	if(modelo != NULL)
	{
		delete modelo;
		modelo = NULL;
	}

	if(scene != NULL)
	{
		delete scene;
		scene = NULL;
	}

}

