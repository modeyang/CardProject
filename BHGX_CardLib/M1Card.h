#ifndef M1CARD_H
#define M1CARD_H

#include "Card.h"

#ifdef  __cplusplus
extern "C" {
#endif

	int __stdcall InitPwd(unsigned char *newKeyB);

	int __stdcall aFormatCard(unsigned char cFlag);

	CardOps * __stdcall InitM1CardOps();

	void __stdcall M1clear();

#ifdef  __cplusplus
};
#endif

#endif