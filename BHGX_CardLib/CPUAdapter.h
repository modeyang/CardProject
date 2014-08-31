#pragma once
#include "Card.h"

#ifdef  __cplusplus
extern "C" {
#endif

	CardOps * __stdcall InitCpuCardOps();

	void __stdcall CPUClear();

#ifdef  __cplusplus
};
#endif
