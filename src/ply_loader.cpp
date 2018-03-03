#include "stdafx.h"
#include "ply_loader.h"

bool PLYLoader::LoadFromFile(const char *file)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		unsigned size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		auto data = std::make_unique<uint8_t[]>(size);
		if (data.get()) {
			fread(data.get(), 1, size, fp);
			fclose(fp);
		}
		else {
			fclose(fp);
			return false;
		}
	}

	return true;
}