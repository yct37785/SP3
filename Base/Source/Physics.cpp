#include "Physics.h"


Physics::Physics(void)
	: Pos(0,0,0)
	, Dir(0,0,0)
	, Speed(0)
	, Force(0)
{

}
void Physics::setPos(Vector2 Pos)
{
	this->Pos.x = Pos.x;
	this->Pos.y = Pos.y;
}

void Physics::setDir(Vector2 Dir)
{
	this->Dir.x = Dir.x;
	this->Dir.y = Dir.y;
}

void Physics::setForce(int Force)
{
	this->Force = Force;
}

void Physics::setSpeed(int Speed)
{
	this->Speed = Speed;
}

Vector3 Physics::getPos(void)
{
	return Pos;
}

Vector3 Physics::getDir(void)
{
	return Dir;
}

int Physics::getSpeed(void)
{
	return Speed;
}

Vector3 Physics::getForce(void)
{
	return Force;
}

Physics::~Physics(void)
{
}
