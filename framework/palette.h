
#pragma once

#include "framework/includes.h"
#include "framework/logger.h"
#include "renderer.h"

namespace OpenApoc
{

class Palette
{
  public:
	std::vector<Colour> colours;
	sp<RendererImageData> rendererPrivateData;

	Palette(unsigned int size = 256, Colour initialColour = {0, 0, 0, 0});
	~Palette();

	const Colour &getColour(unsigned int idx) const
	{
		LogAssert(idx < colours.size());
		return colours[idx];
	}
	void setColour(unsigned int idx, Colour c)
	{
		LogAssert(idx < colours.size());
		colours[idx] = std::move(c);
	}

	// Copy constructor copies everything /except/ the renderer private data
	Palette(const Palette &);
};

}; // namespace OpenApoc
