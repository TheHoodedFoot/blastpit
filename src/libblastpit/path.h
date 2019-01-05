#ifndef BPPATH_H
#define BPPATH_H

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <list>
#include <string>

class Layer;
class Element;

class Path {
      public:
	Path();
	virtual ~Path();

	int Circle(double x, double y, double radius);
	int Rectangle(double x1, double y1, double x2, double y2);
	int Line(double x1, double y1, double x2, double y2);

	void Rotate(double x1, double x2, double angle);

	int GetElementCount();
	Element *GetElement(int index);
	void AddElement(Element *newElement);
	void DeleteElement(int index);

	int GetID();
	std::string GetName();
	uint32_t GetColour();
	uint8_t GetLinetype();
	int GetLayerID();

	void SetID(int);
	void SetName(std::string);
	void SetColour(uint32_t);
	void SetLinetype(uint8_t);
	void SetLayer(std::string);
	int SetLayer(Layer *);
	void SetLayerID(int);
	int SetHatchType(std::string);
	int SetMarkingType(std::string);

      protected:
	int id;
	std::string name;
	uint32_t colour;
	uint8_t linetype;
	int layer_ID;
	int hatch_ID;
	int marking_type_ID;
	std::list<Element *> elements;
};
#endif
