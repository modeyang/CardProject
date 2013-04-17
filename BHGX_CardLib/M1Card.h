#ifndef M1CARD_H
#define M1CARD_H

#ifdef  __cplusplus
extern "C" {
#endif

int __stdcall InitPwd(unsigned char *newKeyB);

int __stdcall aFormatCard(unsigned char cFlag);

int __stdcall InitM1CardOps();

#ifdef  __cplusplus
};
#endif

#endif