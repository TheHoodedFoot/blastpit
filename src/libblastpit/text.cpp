#include "text.h"

Text::Text() {}
Text::~Text() {}
int
Text::GetID()
{
	return id;
}
void
Text::SetFont(std::string font)
{
	this->font = font;
}
void
Text::SetHeight(float height)
{
	this->height = height;
}
void
Text::SetSpacing(float spacing)
{
	this->spacing = spacing;
}
void
Text::SetPosition(double x, double y)
{
	this->x = x;
	this->y = y;
}
void
Text::SetJustification(int type)
{
	this->justifyType = type;
}
std::string
Text::GetFont()
{
	return font;
}
std::string
Text::GetText()
{
	return text;
}
float
Text::GetHeight()
{
	return height;
}
void
Text::SetID(int id)
{
	this->id = id;
}
float
Text::GetSpacing()
{
	return spacing;
}
double
Text::GetPositionX()
{
	return x;
}
double
Text::GetPositionY()
{
	return y;
}
int
Text::GetJustification()
{
	return justifyType;
}
void
Text::SetText(std::string text)
{
	this->text = text;
}
bool
Text::GetHatched()
{
	return isHatched;
}
int
Text::GetLayerID()
{
	return layerId;
}
