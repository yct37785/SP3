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
	InitSprites();

	//Init sound
	sfxengine = createIrrKlangDevice();
//	if (!sfxengine) exit(0);	//error starting up engine // Is it possible to continue running regardless?
	
	

	//For inventory
	inventory = new Inventory;
	inventory->addItem(item);
}

void Model_2D::InitObject()
{	
	/** Set up player **/
	//player = new Player(Geometry::meshList[Geometry::GEO_CUBE], Vector3(1, 1, 0), Vector3(70, 50, 1), 0, 10, true);
	//goList.push_back(player);
	ReadFromFile("Save_Load_File.txt");
	// Player start pos
	player->translate(300,300,0);

	enemy = new Enemy(Geometry::meshList[Geometry::GEO_CUBE], Vector3(700, 500, 0), Vector3(50, 50, 1), 0, 10, true);
	enemy->setState(Enemy::ES_IDLE);

	goList.push_back(enemy);

	/*///** Set up object */
	/*float x = 100;
	for(int i = 0; i < 10; ++i)
	{
		x = i * 111 + 100;
		obj_arr[i] = new StaticObject(Geometry::meshList[Geometry::GEO_CUBE], Vector3(x, 500, 0), 
			Vector3(60, 60, 1), 10, 0, false, GameObject::GO_FURNITURE);
		goList.push_back(obj_arr[i]);
	}*/

	/** init **/
	for(std::vector<GameObject*>::iterator it = goList.begin(); it != goList.end(); ++it)
	{
		Object *go = (Object *)*it;
		go->Init();
	}
}

void Model_2D::InitSprites()
{
	//Player sprites
	player->storeSpriteAnimation("black guard", 21, 13, "Image//Sprites//guard.tga");
	player->processSpriteAnimation(Player::UP, 0.5f, 0, 8, 8, 8, 1);
	player->processSpriteAnimation(Player::DOWN, 0.5f, 0, 10, 8, 10, 1);
	player->processSpriteAnimation(Player::LEFT, 0.5f, 0, 9, 8, 9, 1);
	player->processSpriteAnimation(Player::RIGHT, 0.5f, 0, 11, 8, 11, 1);
	player->processSpriteAnimation(Player::ATTACKUP, 0.5f, 0, 4, 7, 4, 1);
	player->processSpriteAnimation(Player::ATTACKDOWN, 0.5f, 0, 6, 7, 6, 1);
	player->processSpriteAnimation(Player::ATTACKLEFT, 0.5f, 0, 5, 7, 5, 1);
	player->processSpriteAnimation(Player::ATTACKRIGHT, 0.5f, 0, 7, 7, 7, 1);
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
			UpdateMainMenu(dt, myKeys, Controller::mouse_current_x, Controller::mouse_current_y);
			break;
		case stateManager->GAME:
			UpdateGame(dt, myKeys);
			break;
		case stateManager->INSTRUCTION:
			UpdateInstructions(dt, myKeys, Controller::mouse_current_x, Controller::mouse_current_y);
			break;
	}

	/* If in transition */
	if (stateManager->isTransition())
	{
		stateManager->UpdateTransitionTime(dt);
	}

	if(myKeys[KEY_D])
	{
		inventory->deleteItem(1);
	}
}

void Model_2D::UpdateGame(double dt, bool* myKeys)
{
	// Sound - ambience
	if (!sfxengine->isCurrentlyPlaying("musfiles/Verdant_Forest.ogg"))
	{
		sfx_ambience = sfxengine->play2D("musfiles/Verdant_Forest.ogg");
	}
	UpdateEnemy(dt);
	/* Update player */
	player->Update(dt, myKeys);

	if(myKeys[KEY_K])
	{
		player->Translate(Vector3(659, 389, 0));
	}

	//getCamera()->position.Set(player->getPosition().x-500, player->getPosition().y-400, 1);
	//getCamera()->target.Set(player->getPosition().x-500, player->getPosition().y-400, 0);

	//cout << player->getPosition() << endl;

	mapManager->GetCurrentMap()->getWalkable(player->getPosition().x,player->getPosition().y);
	
	
	/* check collision with object */
	//start: Set up collision bound before checking with the others
	player->StartCollisionCheck();


	///* check collision with map */
	mapManager->GetCurrentMap()->CheckCollisionWith(player);

	/* reset */
	player->getCollideBound()->Reset();

	/* Collision response */
	player->CollisionResponse();	//translate to new pos if collides


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

void Model_2D::UpdateEnemy(double dt)
{
	enemy->Update(dt,mapManager);

	/* start set up */
	enemy->StartCollisionCheck();

	/* check with wall */
	mapManager->GetCurrentMap()->CheckCollisionWith(player);

	/* check with all other objects */

	/* reset */
	enemy->getCollideBound()->Reset();

	//response
	enemy->CollisionResponse();
}

void Model_2D::UpdateInstructions(double dt, bool* myKeys, double mouse_x, double mouse_y)
{
	if(mouse_x < 280 && mouse_x > 60 && mouse_y < 540 && mouse_y > 520 && myKeys[KEY_LMOUSE] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->MAIN_MENU);
	}
}

void Model_2D::UpdateMainMenu(double dt, bool* myKeys, double mouse_x, double mouse_y)
{
	if(mouse_x < 618 && mouse_x > 243 && mouse_y < 631 && mouse_y > 600 && myKeys[KEY_LMOUSE] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->GAME);
	}

	if(mouse_x < 581 && mouse_x > 336 && mouse_y < 656 && mouse_y > 636 && myKeys[KEY_LMOUSE] && keyPressedTimer >= delayTime)
	{
		keyPressedTimer = 0.0;
		stateManager->ChangeState(stateManager->INSTRUCTION);
	}
}

void Model_2D::UpdateLight(double dt, bool* myKeys, Light* light)
{
}

bool getWordFromString(string& sentence, string& word, char min, char max, int& index)
{
	bool returnMe = false;
	word = "";
	for(; index < sentence.length(); ++index)	//loop through
	{
		/* get word based on min and max har*/
		
		if(sentence[index] >= min && sentence[index] <= max)	
		{
			word += sentence[index];	//get the whole word
			returnMe = true;
		}
		else if(returnMe)	//if reach end of min max char
			return true;
		
		if(index == sentence.length() - 1 && returnMe)	//last char and is still min max range
		{
			++index;
		
			return true;
		}
	}
	return false;
}

float stringTofloat(string& number)
{
	/* interpret any ascii at front not 48-57 as minus sign */
	float returnVal = 0;
	int stopIndex = 0;
	float multiplier = 1;

	if(number[0] == '-')
		stopIndex = 1;

	/* check if got floating point */
	int floatPoint_total = -1;	//get index
	for(int i = number.length() - 1; i >= stopIndex; --i)
	{
		if(number[i] == '.')	//floating point
		{
			floatPoint_total = (number.length() - 1) - i;
			break;
		}
	}

	/* set the multiplier to starting value */
	if(floatPoint_total != -1)
	{
		for(int i = 0; i < floatPoint_total; ++i)
			multiplier /= 10.f;
	}


	/* calculate num */
	for(int i = number.length() - 1; i >= stopIndex; --i)
	{
		if(number[i] >= 48 && number[i] <= 57)
		{
			returnVal += (number[i] - 48) * multiplier;
			multiplier *= 10;
		}
	}

	if(stopIndex > 0)
	{
		return returnVal * -1;
	}
	else
	{
		return returnVal;

	}
}

bool Model_2D::ReadFromFile(char* text)
{
	string sentence, object_word, case_word;

	int index;
	bool tmp_active;
	Vector3 tmp_pos, tmp_scale;
	float tmp_angle;
	float tmp_floatArr[3];	//to store tmp float variable for vector3
	string tmp_string;		//to store tmp string values
	int tmp_playerID;	//for goalposts

	string active_txt = "active";
	string pos_txt = "pos";
	string scale_txt = "scale";

	ifstream myFile(text);

	if(myFile.fail())
	{
		cout << "Failed to find the text file, the text file is empty" << endl;
		return false;
	}

	while(!myFile.eof())
	{
		std::getline(myFile, sentence, '\n');
		cout << sentence << endl;
		index = 0;
		
		/******************** if is comment/non-caps for first char, skip ********************/
		if(sentence.length() > 0 && !(sentence[index] >= 65 && sentence[index] <= 90) )	
		{
			continue;
		}
		/******************** Get KEYWORD so we know what object to create ********************/
		if( !getWordFromString(sentence, object_word, 65, 90, index) )
		{
			cout << "Line undefined: going to next line" << endl;
			continue;
		}
		
		/************************** get variable words **************************/
		while(index < sentence.length())
		{
			case_word = "";	//the keyword

			/* got word */
			if(getWordFromString(sentence, case_word, 97, 122, index) )
			{
				bool start = false;	//start assigning
				string variables  = "";	//(x, y, z)

				/* get the words inside the bracket */
				for(;index < sentence.length(); ++index)
				{
					if(sentence[index] == ')')
					{
						break;
					}

					if(start)
					{
						variables += sentence[index];
					}

					if(sentence[index] == '(')
					{
						start = true;
					}
				}

				/************************** get number/s from brackets, eg. (x, y, z) **************************/
				string value = "";
				bool correct = false;
				int numIndex = 0;	//index
				unsigned counter = 0; //if is Vector3, count 3 times(take in value) and break, if is float, just count one time and break
				while(numIndex < variables.length())
				{
					if(case_word == active_txt)	//active: T/F
					{
						correct = getWordFromString(variables, value, 60, 90, numIndex);
					}
					else	//the rest: minus sign and 0-9
					{
						correct = getWordFromString(variables, value, 45, 57, numIndex);
					}

					if(!correct)
					{
						cout << "Unidentified variable: moving on to next variable" << endl;
						break;
					}

					/* get values from txt file */
					if(case_word == active_txt)	//keyword is active: special case immediately assign here
					{
						tmp_string = value;
					}
					else						//keyword is float values
					{
						tmp_floatArr[counter] = stringTofloat(value);
						++counter;
					}
				}

				/** Assign tmp values here **/
				if(case_word == pos_txt)
				{
					tmp_pos.Set(tmp_floatArr[0], tmp_floatArr[1], tmp_floatArr[2]);
				}
				else if(case_word == scale_txt)
				{
					tmp_scale.Set(tmp_floatArr[0], tmp_floatArr[1], tmp_floatArr[2]);
				}
				else if(case_word == active_txt)
				{
					if(tmp_string == "T")	//true
					{
						tmp_active = true;
					}
					else
					{
						tmp_active = false;
				
					}
				}
			}
		}

		/************************** Create Relevant object **************************/
		if(object_word == "PLAYER")
		{
			player = new Player(Geometry::meshList[Geometry::GEO_CUBE], Vector3(tmp_pos.x, tmp_pos.y, 0), Vector3(tmp_scale.x, tmp_scale.y, 1), 0, 10, true);
			goList.push_back(player);
		}
	}
	myFile.close();
	return true;
}

void Model_2D::Exit()
{
	Model::Exit();
}