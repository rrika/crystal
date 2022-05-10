#pragma once
#include <cstdint>

namespace cdc {

class IRenderDrawable {
protected:
	friend class DrawableList;
	uint16_t word4 = 0;
	uint16_t word6 = 0;
	float float8 = 0.0f;
public:
	virtual void renderDrawable0() = 0;
	virtual uint32_t renderDrawable4() = 0;
	uint32_t getVtable() { return *(uint32_t*)this; }
};

}
