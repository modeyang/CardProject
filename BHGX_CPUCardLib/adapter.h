#ifndef _CARDDATA_ADAPTER_H
#define _CARDDATA_ADAPTER_H
#pragma once


#ifdef  __cplusplus
extern "C" {
#endif

int __stdcall initCoreDevice(const char *System);
int __stdcall closeCoreDevice(void);

int __stdcall iCoreFindCard(void);

int __stdcall DestroyCPUData();

int __stdcall iReadCard(struct RWRequestS *);
int __stdcall iWriteCard(struct RWRequestS *list);


#ifdef  __cplusplus
};
#endif

#endif