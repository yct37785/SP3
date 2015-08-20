#include "model_2D.h"
#include "GL\glew.h"
#include "shader.hpp"
#include "MeshBuilder.h"
#include "Controller.h"
#include "Utility.h"
#include "LoadTGA.h"
#include "LoadHmap.h"
#include "SpriteAnimation.h"
#include <sstream>

#include "StaticObject.h"
//(tip) If create bullet, bullet class has a static TRS so that TRS update with current bullet pos

#include "irrKlang.h"
using namespace irrklang;

extern ISoundEngine* sfxengine;
extern ISound *sfx_ambience;

/*********** constructor/destructor ***************/
Model_2D::Model_2D()
{
	minimap = NULL;
}

Model_2D::~Model_2D()
{
}

/*********** core functions ***************/
void Model_2D::Init()
{
	Model::Init();
	
	InitMaps();	//create map for all levels
	mapManager->SetMap(0);	//set to map 0 first

	/* Coord of screen */
	m_view_width = 1024.f;
	m_view_height = 800.f;

	/* World boundaries */
	worldDimension.Set(800, 800, 100);

	/* timer and delay time */
	delayTime = 0.3;
	keyPressedTimer = delayTime;

	//object
	InitObject();

	//Init sound
	sfxengine = createIrrKlangDevice();
//	if (!sfxengine) exit(0);	//error starting up engine // Is it possible to continue running regardless?
}

void Model_2D::InitObject()
{	
	/** Set up player **/
	player = new Player(Geometry::meshList[Geometry::GEO_CUBE], Vector3(1, 1, 0), Vector3(50, 50, 1), 0, 10, true);
	elementObject.push_back(player);

	// Player start pos
	player->translate(500,400,0);

	/** Set up object */
	float x = 100;
	for(int i = 0; i < 10; ++i)
	{
		x = i * 111 + 100;
		obj_arr[i] = new StaticObject(Geometry::meshList[Geometry::GEO_CUBE], Vector3(x, 500, 0), 
			Vector3(60, 60, 1), 10, 0, false, GameObject::GO_FURNITURE);
		elementObject.push_back(obj_arr[i]);
	}

	/** init **/
	for(std::vector<Object*>::iterator it = elementObject.begin(); it != elementObject.end(); ++it)
	{
		Object *go = (Object *)*it;
		go->Init();
	}
}

void Model_2D::InitMaps()
{
	mapManager->CreateMap(32, 25, 32, "Image//Map//MapDesign.csv");
	mapManager->CreateMap(32, 25, 32, "Image//Map//MapDesign_lvl1.csv");
	mapManager->CreateMap(32, 25, 32, "Image//Map//MapDesign_lvl2.csv");
}

void Model_2D::Update(double dt, bool* myKeys)
{
	/* parent class update */
	Model::Update(dt, myKeys);

	if(keyPressedTimer < delayTime)
		keyPressedTimer += dt;
	
	/* Update based on states */
	switch (stateManager->GetState())
	{
		case stateManager->MAIN_MENU:
			UpdateMainMenu(dt, myKeys);
			break;
		case stateManager->GAME:
			UpdateGame(dt, myKeys);
			break;
		case stateManager->INSTRUCTION:
			UpdateInstructions(dt, myKeys);
			break;
	}

	/* If in transition */
	if (stateManager->isTransition())
	{
		stateManager->UpdateTransitionTime(dt);
	}
}

void Model_2D::UpdateGame(double dt, bool* myKeys)
{
	// Sound - ambience
	if (!sfxengine->isCurrentlyPlaying("musfiles/Verdant_Forest.ogg"))
	{
		sfx_ambience = sfxengine->play2D("musfiles/Verdant_Forest.ogg");
	}

	/* Update player */
	player->Update(dt, myKeys);

	getCamera()->position.Set(player->getPosition().x-500, player->getPosition().y-400, 1);
	getCamera()->target.Set(player->getPosition().x-500, player->getPosition().y-400, 0);

	/* check collision with object */
	//start: Set up collision bound before checking with the others
	player->StartCollisionCheck();

	for(int i = 0; i < 10; ++i)
	{
		//check
		player->CollisionCheck(obj_arr[i]);
	}

	/* check collision with map */
	mapManager->GetCurrentMap()->CheckCollisionWith(player);

	/* Collision response */
	player->CollisionResponse();	//translate to new pos if collides

	if(myKeys[KEY_K])
	{
		camera.position.x += 1;
	}

	/* Update target */
	camera.target = camera.position;
	camera.target.z -= 10;

	/* Press space to go back main menu */
	if(myKeys[KEY_SPACE] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->MAIN_MENU);
	}

	/* Load/change map */
	//Key B to move to next map (RP)
	static bool ButtonBState = false;
	if (!ButtonBState && myKeys[KEY_B])
	{
		ButtonBState = true;
		std::cout << "BBUTTON DOWN" << std::endl;
		//stateManager->ChangeState(StateManager::MAIN_MENU);
		mapManager->ChangeNextMap();
	}
	else if (ButtonBState && !(myKeys[KEY_B]))
	{
		ButtonBState = false;
		std::cout << "BBUTTON UP" << std::endl;
	}
}

void Model_2D::UpdateInstructions(double dt, bool* myKeys)
{
	if(myKeys[KEY_I] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->MAIN_MENU);
	}
}

void Model_2D::UpdateMainMenu(double dt, bool* myKeys)
{
	if(myKeys[KEY_SPACE] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->GAME);
	}

	if(myKeys[KEY_I] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->INSTRUCTION);
	}
}

void Model_2D::UpdateLight(double dt, bool* myKeys, Light* light)
{
}

void Model_2D::Exit()
{
	Model::Exit();
}