#ifndef BPTEXT_H
#define BPTEXT_H

#include <string>

class Text {
      public:
	Text();
	virtual ~Text();

	void SetText(std::string text);
	void SetFont(std::string font);
	void SetHeight(float height);
	void SetSpacing(float spacing);
	void SetPosition(double x, double y);
	void SetJustification(int type);
	void SetLayerID(int id);
	void SetHatched(bool hatched);
	void SetID(int id);

	std::string GetText();
	std::string GetFont();
	float GetHeight();
	float GetSpacing();
	double GetPositionX();
	double GetPositionY();
	int GetJustification();
	int GetLayerID();
	bool GetHatched();
	int GetID();

      private:
	std::string text;
	std::string font;
	int id;
	float height;
	float spacing;
	double x, y;
	int justifyType;
	int layerId;
	bool isHatched;
};
#endif /* end of include guard: BPTEXT_H */
