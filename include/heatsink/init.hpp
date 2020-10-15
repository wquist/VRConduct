#pragma once

#include <cstdlib>

namespace hs
{
	struct settings
	{
	public:
		settings()
			: settings(3, 3) {}
		settings(size_t ma, size_t mi)
			: version_major{ma}, version_minor{mi}, samples{1}, forward_only{false} {}

	public:
		size_t version_major;
		size_t version_minor;

		// Number of multisamples requested
		size_t samples;
		// Forward-compatibility only; required by OSX
		bool forward_only;
	};
}
