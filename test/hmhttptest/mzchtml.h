#pragma once
#include <string>
#include "core/libhttp/httpinterface.h"

namespace mzchtml {
	StepResult ReturnMZCHtml(const std::string &pathName, 
		IConnection *conn, StepFireCond &cond, HttpStage &stage,
		IHttpRequest *request, IHttpResponse *resp);
}
