#pragma once
#include "config.h"

namespace notstd {
	class NOTSTD_API NetEnviroment {
	public:
		static bool GetExtFuncPointer();

	public:
		static bool Init();
		static void Term();
	};
}
