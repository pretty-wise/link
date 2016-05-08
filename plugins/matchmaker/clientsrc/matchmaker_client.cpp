/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin/matchmaker/matchmaker_client.h"
#include "base/core/macro.h"
#include "common/protobuf_stream.h"
//#include "protocol/gate.pb.h"

namespace Link {
namespace Matchmaker {

static const Base::LogChannel kMatchLog("match_client");

struct context_t {
	Gate::context_t* server;
};


struct context_t* Create(Gate::context_t* ctx, void* udata) {
	if(ctx == nullptr) {
		return nullptr;
	}

	context_t* context = new context_t();
	context->server = ctx;

	BASE_INFO(kMatchLog, "created %p", context);	
	return context;
}

void Destory(struct context_t* ctx) {
	delete ctx;
	BASE_INFO(kMatchLog, "destroyed %p", ctx);
}

} // namespace Matchmaker 
} // namespace Link
