#pragma once

#include "model_loader.h"

class PLYLoader : public ModelLoader
{
public:
	bool LoadFromFile(const char *file);
};