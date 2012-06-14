#ifndef _PRINT_DLL_H
#define _PRINT_DLL_H

#pragma once

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT int __stdcall iProbePrinter();
	DLL_EXPORT int __stdcall iFreePrinter();
	DLL_EXPORT int __stdcall iFeedCardToM1(void);
	DLL_EXPORT int __stdcall iBackCardFromM1(void);
	DLL_EXPORT int __stdcall iInitGraphics (char *pszPrinterName );
	DLL_EXPORT int __stdcall iPrintText(long nPosx, long nPosy, char *szContent,
										char *szFont, long nFontSize, 
										long nFontStyle, long nFontColoer);

	DLL_EXPORT int __stdcall iFlushGraphics ( void );

	DLL_EXPORT int __stdcall iCloseGraphics ( void );

	DLL_EXPORT int __stdcall iOutCard(void);

	DLL_EXPORT int __stdcall iCheckPrinterStatus();


#ifdef __cplusplus
}
#endif

#endif