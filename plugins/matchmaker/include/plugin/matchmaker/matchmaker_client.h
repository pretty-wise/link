/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/network/url.h"
#include "plugin/matchmaker/matchmaker_defs.h"

namespace Link {

namespace Gate {
	struct context_t;
} // namespace Gate

namespace Matchmaker {

struct context_t* Create(Gate::context_t* ctx, void* udata);

void Destroy(struct context_t* ctx);

} // namespace Matchmaker
} // namespace Link
