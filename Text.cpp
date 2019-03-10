#include "pch.h"
#include "Text.h"

std::string Text::getText() 
{
	return _text;
}

Text::Text():_text("")
{
}

Text::Text(std::string str):_text(str)
{
}

Text::~Text()
{
}
