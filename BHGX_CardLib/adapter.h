#pragma once

#ifndef _CARD_LIB_ADAPTER_H_
#define _CARD_LIB_ADAPTER_H_
#include "Card.h"

#ifdef  __cplusplus
extern "C" {
#endif

	int __stdcall apt_InitCoreDevice(const char *System);
	
	int __stdcall apt_CloseCoreDevice(void);
	int __stdcall apt_OpenDev(void);
	int __stdcall apt_CloseDev(void);

	int __stdcall apt_ScanCard(unsigned char *card_type);

	int __stdcall apt_IOCtl(int cmd, void *data);

	int __stdcall apt_InitGList(CardType eType);

	struct RWRequestS *  apt_CreateRWRequest(struct XmlSegmentS *listHead, int mode, CardType type);

	void apt_DestroyRWRequest(struct RWRequestS *list, int flag);


#ifdef  __cplusplus
};
#endif

#endif	// _CARD_LIB_ADAPTER_H_