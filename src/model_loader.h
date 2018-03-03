#pragma once

class ModelLoader
{
public:
	virtual bool LoadFromFile(const char *file) = 0;
};