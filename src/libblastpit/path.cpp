#include "path.h"
#include "element.h"
#include "layer.h"

#include <vector>

Path::Path() {}
Path::~Path() {}
int
Path::GetElementCount()
{
	return static_cast<int>(elements.size());
}
/*BpElement *BpPath::GetElement(int index)
   {
	return &elements[index];
   }*/
void
Path::AddElement(Element *element)
{
	elements.push_back(element);
}
/*void BpPath::DeleteElement(int index)
   {
	elements.erase(elements.begin() + index);
   }*/
int
Path::GetID()
{
	return id;
}
void
Path::SetName(std::string newName)
{
	name = newName;
}
void
Path::SetID(int newId)
{
	id = newId;
}
void
Path::SetColour(uint32_t newColour)
{
	colour = newColour;
}
void
Path::SetLayerID(int new_ID)
{
	layer_ID = new_ID;
}
int
Path::GetLayerID()
{
	return layer_ID;
}
void
Path::SetLinetype(uint8_t linetype)
{
	this->linetype = linetype;
}
uint8_t
Path::GetLinetype()
{
	return linetype;
}
uint32_t
Path::GetColour()
{
	return colour;
}
std::string
Path::GetName()
{
	return name;
}
/// Creates a circle. This replaces the existing path.
/// \param [in] x,y Coordinates of the centre of the circle
/// \param [in] radius The circle radius
/// \return 0 if successful, otherwise path error code
int
Path::Circle(double x, double y, double radius)
{
	(void)x;
	(void)y;
	(void)radius;

	return false;
}
/// Creates a rectangle. This replaces the existing path.
/// \param [in] x1,y1,x2,y2 Coordinates of the opposite corners of the
/// rectangle \return 0 if successful, otherwise path error code
int
Path::Rectangle(double x1, double y1, double x2, double y2)
{
	(void)x1;
	(void)x2;
	(void)y1;
	(void)y2;

	return false;
}

/// Creates a line. This replaces the existing path.
/// \param [in] x1,y1,x2,y2 Coordinates of the start and end of the line
/// \return 0 if successful, otherwise path error code
int
Path::Line(double x1, double y1, double x2, double y2)
{
	(void)x1;
	(void)y1;
	(void)x2;
	(void)y2;

	return false;
}
/// Rotates the path about a point
/// \param [in] x,y Coordinates of the rotation point
/// \param [in] angle The rotation angle in radians
/// \return 0 if successful, otherwise path error code
void
Path::Rotate(double x, double y, double angle)
{
	(void)x;
	(void)y;
	(void)angle;

	return;
}
/// Sets the path's layer.
/// \param [in] layer Pointer to a Layer object
/// \return 0 if successful, otherwise path error code
int
Path::SetLayer(Layer *layer)
{
	(void)layer;

	return false;
}
/// Sets or removes the path's hatch type.
/// Setting the type to an empty string removes any existing hatch.
/// \param hatch Name of hatch, which must already exist
/// \return 0 if successful, otherwise path error code
int
Path::SetHatchType(std::string hatch)
{
	(void)hatch;

	return false;
}
/// Sets the path's marking type.
/// Setting the type to an empty string sets the default marking type.
/// \param marking_type Name of marking type
/// \return 0 if successful, otherwise path error code
int
Path::SetMarkingType(std::string marking_type)
{
	(void)marking_type;

	return false;
}
