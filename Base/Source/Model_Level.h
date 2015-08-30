#ifndef MODEL_LEVEL_H
#define MODEL_LEVEL_H
#include "Model.h"

//model for level class
class Model_Level : public Model
{
protected:
/************ All Game object vectors ************/ 

	/* render all general(not a specific object) game objects */
	vector<GameObject*> goList;	
	
	/* render all collision boxes (debug purpose only pls remove) */
	vector<Collision*> collisionList; 
	
	/* all item objects */
	vector<Item*> itemList;	
	
	/* all ui objects */
	vector<UI_Object*> UI_List;	
	
	/* Triggering Objects */
	vector<TriggerObject*> triggerObject;

	static bool init_Already;
public:
/************ current level map ************/ 
	vector<Map*>* level_map;

/************* Map manager (Only one for all levels) *****************/
	static MapManager mapManager;

	Vector3 mapSize;	

/************ Game Objects  ************/ 

	/* Universal game objects */
	//player
	static Player* player;
	static float hero_Health;

	// UI 
	static UI_Object cursor;
	static UI_Object start_Game;
	static UI_Object instruction;
	static UI_Object go_back;
	static UI_Object main_UI_bar;	//main UI in game

	// door/checkpoint 
	static TriggerObject* door;
	static TriggerObject* staircase;
	static bool stopGame;
	static bool doorUnlocked;
	static bool haveFire;
	static double Timer;
	static double mapTimer;

	/* Common game objects */
	//objects
	Ogre* E_Ogre;
	Item* item;

	//puzzle
	PuzzleManager *puzzleManager;
	bool puzzleOpen;
	
	static bool goNextLevel;
	static bool goPreviousLevel;

	/*********** constructor/destructor ***************/
	Model_Level();
	virtual ~Model_Level();

	/*********** core functions ***************/
	virtual void Init();
		void InitMaps(); //Initialize maps (RP)

	virtual void Update(double dt, bool* myKeys, Vector3 mousePos);

	virtual void Exit();

	/*********** getter/setter ***************/
	static bool NextLevel();	//whether can go to next level or not
	static bool PreviousLevel();
	static void setNextLevel(bool i);	//whether can go to next level or not
	static void setPreviousLevel(bool i);

	/********** Utilities *************/
	bool ReadFromFile(char* text);

	/*********** getter / setters ***************/
	vector<GameObject*>* getObject();
	vector<UI_Object*>* getUIList();
	vector<Item*>* getItemList();
	vector<Map*>* getLevelMap();
};

#endif