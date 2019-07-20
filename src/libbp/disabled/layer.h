#ifndef BPLAYER_H
#define BPLAYER_H

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <string>

class Path;

class Layer {
      public:
	Layer(std::string layer_name, float layer_height = 0,
	      uint32_t layer_colour = 0, uint8_t layer_linetype = 0,
	      bool is_laserable = 1);

      public:
	std::string GetName();
	int GetID();
	uint32_t GetColour();
	uint8_t GetLinetype();
	float GetHeight();
	bool GetIsLaserable();

	void SetName(std::string);
	void SetColour(uint32_t);
	void SetLinetype(uint8_t);
	void SetHeight(float);
	void SetIsLaserable(bool);

	int AddPath(Path*);

      protected:
	int ID;  ///< An automatically generated identifier unique within each
		 /// Blastpit
	/// instance
	std::string name;
	uint32_t colour;
	uint8_t linetype;
	float height;
	bool laserable;  ///< Indicates whether laser should be activated for
			 /// this layer
};
#endif
