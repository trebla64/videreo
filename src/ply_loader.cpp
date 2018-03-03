#include "stdafx.h"
#include "ply_loader.h"
#include <sstream>

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

			fseek(fp, 0, SEEK_SET);
			char line[128];
			fgets(line, sizeof(line), fp);
			if (strncmp(line, "ply", 3)) {
				fclose(fp);
				return false;
			}

			while (strncmp(line, "end_header", 10)) {
				fgets(line, sizeof(line), fp);

				if (!strncmp(line, "element vertex ", 15)) {
					// Parse vertex count
					//int sscanf(const char *str, const char *format, ...)
				}
				else if (!strncmp(line, "element face ", 13)) {
					// Parse face count
				}
			}

			fclose(fp);
		}
		else {
			fclose(fp);
			return false;
		}
	}

	return true;
}