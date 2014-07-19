
//General functions
typedef INT (CALLBACK *ZBRGetHandle)(LPHANDLE hPrinter, LPSTR pName, INT* printerType, LPINT err);
typedef INT (CALLBACK *ZBRCloseHandle)(HANDLE hPrinter, LPINT err);

typedef void (CALLBACK *ZBRPRNGetSDKVer)(LPINT major, LPINT minor, LPINT engLevel);
typedef INT (CALLBACK *ZBRPRNSendCmd)(HANDLE hPrinter, INT printerType, LPSTR cmd, LPINT err);
typedef INT (CALLBACK *ZBRPRNSendCmdEx)(HANDLE hPrinter, INT printerType, LPSTR cmd, LPSTR response, LPINT respBytesNeeded, LPINT err);

typedef INT (CALLBACK *ZBRPRNEjectCard)(HANDLE hPrinter, INT printerType, LPINT err);
typedef INT (CALLBACK *ZBRPRNGetPrinterStatus)(LPINT errorStatus);

// Mag
typedef INT (CALLBACK *ZBRPRNSetEncodingDir)(HANDLE hPrinter, INT printerType, INT dir, LPINT err);
typedef INT (CALLBACK *ZBRPRNSetTrkDensity)(HANDLE hPrinter, INT printerType, INT trkNumb, INT density, LPINT err);
typedef INT (CALLBACK *ZBRPRNResetMagEncoder)(HANDLE hPrinter, INT printerType, LPINT err);
typedef INT (CALLBACK *ZBRPRNSetEncoderCoercivity)(HANDLE hPrinter, INT printerType, INT coercivity, LPINT err);
typedef INT (CALLBACK *ZBRPRNSetMagEncodingStd)(HANDLE hPrinter, INT printerType, INT std, LPINT err);
typedef INT (CALLBACK *ZBRPRNReadMag)(HANDLE hPrinter, INT printerType, INT trksToRead, LPSTR trk1Buf, LPINT trk1BytesNeeded, LPSTR trk2Buf, LPINT trk2BytesNeeded, LPSTR trk3Buf, LPINT trk3BytesNeeded, LPINT err);
typedef INT (CALLBACK *ZBRPRNReadMagByTrk)(HANDLE hPrinter, INT printerType, INT trkNumb, LPSTR trkBuf, LPINT trkBytesNeeded, LPINT err);
typedef INT (CALLBACK *ZBRPRNWriteMag)(HANDLE hPrinter, INT printerType, INT trksToWrite, LPSTR trk1Data, LPSTR trk2Data, LPSTR trk3Data, LPINT err);
typedef INT (CALLBACK *ZBRPRNWriteMagPassThru)(HDC hDC, INT printerType, INT trksToWrite, LPSTR trk1Data, LPSTR trk2Data, LPSTR trk3Data, LPINT err);
typedef INT (CALLBACK *ZBRPRNWriteMagByTrk)(HANDLE hPrinter, INT printerType, INT trkNumb, LPSTR trkData, LPINT err);


// For printer types
#define ZBR_100 100
#define ZBR_110 110
#define ZBR_120 120
#define ZBR_330 330
#define ZBR_360 360
#define ZBR_420 420
#define ZBR_430 430
#define ZBR_640 640

// For magnetic encoding
#define TRK1 1
#define TRK2 2
#define TRK3 4

// Errors returned by the driver
#define ZBR_ERROR_PRINTER_MECHANICAL_ERROR              -1
#define ZBR_ERROR_BROKEN_RIBBON                          1
#define ZBR_ERROR_TEMPERATURE                            2
#define ZBR_ERROR_MECHANICAL_ERROR                       3
#define ZBR_ERROR_OUT_OF_CARD                            4
#define ZBR_ERROR_CARD_IN_ENCODER                        5
#define ZBR_ERROR_CARD_NOT_IN_ENCODER                    6
#define ZBR_ERROR_PRINT_HEAD_OPEN                        7
#define ZBR_ERROR_OUT_OF_RIBBON                          8
#define ZBR_ERROR_REMOVE_RIBBON                          9
#define ZBR_ERROR_PARAMETERS_ERROR                      10
#define ZBR_ERROR_INVALID_COORDINATES                   11
#define ZBR_ERROR_UNKNOWN_BARCODE                       12
#define ZBR_ERROR_UNKNOWN_TEXT                          13
#define ZBR_ERROR_COMMAND_ERROR                         14
#define ZBR_ERROR_BARCODE_DATA_SYNTAX                   20
#define ZBR_ERROR_TEXT_DATA_SYNTAX                      21
#define ZBR_ERROR_GRAPHIC_DATA_SYNTAX                   22
#define ZBR_ERROR_GRAPHIC_IMAGE_INITIALIZATION          30
#define ZBR_ERROR_GRAPHIC_IMAGE_MAXIMUM_WIDTH_EXCEEDED  31
#define ZBR_ERROR_GRAPHIC_IMAGE_MAXIMUM_HEIGHT_EXCEEDED 32
#define ZBR_ERROR_GRAPHIC_IMAGE_DATA_CHECKSUM_ERROR     33
#define ZBR_ERROR_DATA_TRANSFER_TIME_OUT                34
#define ZBR_ERROR_CHECK_RIBBON                          35
#define ZBR_ERROR_INVALID_MAGNETIC_DATA                 40
#define ZBR_ERROR_MAG_ENCODER_WRITE                     41
#define ZBR_ERROR_READING_ERROR                         42
#define ZBR_ERROR_MAG_ENCODER_MECHANICAL                43
#define ZBR_ERROR_MAG_ENCODER_NOT_RESPONDING            44
#define ZBR_ERROR_MAG_ENCODER_MISSING_OR_CARD_JAM       45
#define ZBR_ERROR_ROTATION_ERROR                        47
#define ZBR_ERROR_COVER_OPEN							48
#define ZBR_ERROR_ENCODING_ERROR                        49
#define ZBR_ERROR_MAGNETIC_ERROR                        50
#define ZBR_ERROR_BLANK_TRACK                           51
#define ZBR_ERROR_FLASH_ERROR                           52
#define ZBR_ERROR_NO_ACCESS                             53
#define ZBR_ERROR_SEQUENCE_ERROR                        54
#define ZBR_ERROR_PROX_ERROR							55
#define ZBR_ERROR_CONTACT_DATA_ERROR					56
#define ZBR_ERROR_PROX_DATA_ERROR						57

// Errors returned by the SDK
#define ZBR_ERROR_PRINTER_NOT_SUPPORTED					60
#define ZBR_ERROR_CANNOT_GET_PRINTER_HANDLE				61
#define ZBR_ERROR_CANNOT_GET_PRINTER_DRIVER				62
#define ZBR_ERROR_GETPRINTERDATA_ERROR					63
#define ZBR_ERROR_INVALID_MAG_TRK_NUMB					64
