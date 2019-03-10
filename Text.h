#pragma once
#include <string>
class Text
{
protected:
	const std::string _text;
public:
	Text();
	Text(std::string);

	std::string getText();
	virtual ~Text();
};

