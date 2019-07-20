#include "layer.h"

#include <string>

#include "path.h"

Layer::Layer(std::string layer_name, float layer_height,
	     uint32_t layer_colour, uint8_t layer_linetype, bool is_laserable)
{
	// TODO: Prevent similar names (both here and SetName())
	name = layer_name;
	height = layer_height;
	colour = layer_colour;
	linetype = layer_linetype;
	laserable = is_laserable;
}
int
Layer::GetID()
{
	return ID;
}
std::string
Layer::GetName()
{
	return name;
}
/// Sets the layer name.
/// This will raise an error if the name is already in use.
void
Layer::SetName(std::string name)
{
	this->name = name;
}
void
Layer::SetColour(uint32_t colour)
{
	this->colour = colour;
}
void
Layer::SetLinetype(uint8_t linetype)
{
	this->linetype = linetype;
}
void
Layer::SetHeight(float height)
{
	this->height = height;
}
void
Layer::SetIsLaserable(bool is_laserable)
{
	this->laserable = is_laserable;
}
uint8_t
Layer::GetLinetype()
{
	return linetype;
}
uint32_t
Layer::GetColour()
{
	return colour;
}
float
Layer::GetHeight()
{
	return height;
}
bool
Layer::GetIsLaserable()
{
	return laserable;
}
int
Layer::AddPath(Path *path)
{
	(void)path;

	return false;
}
