#define uc unsigned char
#ifdef  __cplusplus
extern "C" {
#endif
//1.
HANDLE  __stdcall  dc_init(__int16 port,long baud);
//2.
__int16  __stdcall dc_exit(HANDLE icdev);
//3.
__int16  __stdcall dc_config(HANDLE icdev,unsigned char _Mode,unsigned char _Baud);
//4.
__int16  __stdcall dc_request(HANDLE icdev,unsigned char _Mode,unsigned __int16  *TagType);
//5.
__int16  __stdcall  dc_anticoll(HANDLE icdev,unsigned char _Bcnt,unsigned long *_Snr);
//6.
__int16  __stdcall dc_select(HANDLE icdev,unsigned long _Snr,unsigned char *_Size);
//7.
__int16  __stdcall dc_authentication(HANDLE icdev,unsigned char _Mode,unsigned char _SecNr);
//8.
__int16  __stdcall dc_halt(HANDLE icdev);
//9
__int16  __stdcall dc_read(HANDLE icdev,unsigned char _Adr,unsigned char *_Data);
//10.
__int16  __stdcall dc_read_hex(HANDLE icdev,unsigned char _Adr,char *_Data);
//11.
__int16  __stdcall dc_write(HANDLE icdev,unsigned char _Adr,unsigned char *_Data);
//12.
__int16  __stdcall dc_write_hex(HANDLE icdev,unsigned char _Adr,char *_Data);
//13.
__int16  __stdcall dc_load_key(HANDLE icdev,unsigned char _Mode,unsigned char _SecNr,
							   unsigned char *_NKey);
//14.
__int16  __stdcall dc_load_key_hex(HANDLE icdev,unsigned char _Mode,unsigned char _SecNr,
								   char *_NKey);
//15.
__int16  __stdcall dc_card(HANDLE icdev,unsigned char _Mode,unsigned long *_Snr);
//16
__int16 __stdcall dc_card_hex(HANDLE icdev,unsigned char _Mode,unsigned char *snrstr);
//17.
__int16  __stdcall dc_changeb3(HANDLE icdev,unsigned char _SecNr,unsigned char *_KeyA,
							   unsigned char _B0,unsigned char _B1,unsigned char _B2,
							   unsigned char _B3,unsigned char _Bk,unsigned char *_KeyB);
//18.
__int16  __stdcall dc_restore(HANDLE icdev,unsigned char _Adr);
//19
__int16  __stdcall dc_transfer(HANDLE icdev,unsigned char _Adr);
//20
__int16  __stdcall dc_increment(HANDLE icdev,unsigned char _Adr,unsigned long _Value);
//21.
__int16  __stdcall dc_decrement(HANDLE icdev,unsigned char _Adr,unsigned long _Value);
//22.
__int16  __stdcall dc_initval(HANDLE icdev,unsigned char _Adr,unsigned long _Value);
//23.
__int16  __stdcall dc_readval(HANDLE icdev,unsigned char _Adr,unsigned long *_Value);
//24
__int16  __stdcall dc_initval_ml(HANDLE icdev,unsigned __int16   _Value);
//25
__int16  __stdcall dc_readval_ml(HANDLE icdev,unsigned __int16   *_Value);//17
//26
__int16  __stdcall dc_decrement_ml(HANDLE icdev,unsigned __int16   _Value);
//27
__int16  __stdcall dc_authentication_2(HANDLE icdev,unsigned char _Mode,unsigned char KeyNr,
									   unsigned char Adr);
//28
__int16  __stdcall  dc_anticoll2(HANDLE icdev,unsigned char _Bcnt,unsigned long *_Snr);
//29
__int16  __stdcall dc_select2(HANDLE icdev,unsigned long _Snr,unsigned char *_Size);
//30.
__int16  __stdcall dc_HL_write(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
							   unsigned long *_Snr,unsigned char *_Data);
//31
__int16  __stdcall dc_HL_writehex(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
								  unsigned long *_Snr,unsigned char *_Data);

//32.
__int16  __stdcall dc_HL_read(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
							  unsigned long _Snr,unsigned char *_Data,unsigned long *_NSnr);
//33
__int16  __stdcall dc_HL_readhex(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
								 unsigned long _Snr,unsigned char *_Data,unsigned long *_NSnr);

//34.
__int16  __stdcall dc_HL_authentication(HANDLE icdev,unsigned char reqmode,unsigned long snr,
										unsigned char authmode,unsigned char secnr);
//35.
__int16  __stdcall dc_check_write(HANDLE icdev,unsigned long Snr,unsigned char authmode,
								  unsigned char Adr,unsigned char * _data);
//36
__int16  __stdcall dc_check_writehex(HANDLE icdev,unsigned long Snr,unsigned char authmode,
									 unsigned char Adr,unsigned char * _data);

//37.
__int16 __stdcall dc_getver(HANDLE icdev,unsigned char *sver);
//38
__int16 __stdcall dc_update(HANDLE icdev);
//39
__int16  __stdcall dc_clr_control_bit(HANDLE icdev,unsigned char _b);
//40.
__int16  __stdcall dc_set_control_bit(HANDLE icdev,unsigned char _b);
//41.
__int16  __stdcall dc_reset(HANDLE icdev,unsigned __int16   _Msec);
//42
__int16  __stdcall dc_beep(HANDLE icdev,unsigned short _Msec);

//44
__int16  __stdcall dc_srd_eeprom(HANDLE icdev,__int16   offset,__int16 lenth,
								 unsigned char *rec_buffer);
//45
__int16  __stdcall dc_swr_eeprom(HANDLE icdev,__int16   offset,__int16 lenth,
								 unsigned char* send_buffer);
//46
__int16 __stdcall swr_alleeprom(HANDLE icdev,__int16 offset,__int16 lenth,
								unsigned char* snd_buffer);
//47
__int16 __stdcall srd_alleeprom(HANDLE icdev,__int16 offset,__int16 lenth,
								unsigned char *receive_buffer);
//48
__int16  __stdcall dc_srd_eepromhex(HANDLE icdev,__int16   offset,__int16   lenth,
									unsigned char *rec_buffer);

//49
__int16  __stdcall dc_swr_eepromhex(HANDLE icdev,__int16   offset,__int16   lenth,
									unsigned char* send_buffer);


//57
__int16  __stdcall dcdeshex(unsigned char *key,unsigned char *sour,unsigned char *dest,
							__int16 m);

//58
__int16 __stdcall dcdes(unsigned char *key,unsigned char *sour,unsigned char *dest,__int16 m);
//59
__int16 __stdcall dc_light(HANDLE icdev,unsigned short _OnOff);
//85
__int16 __stdcall hex_a(unsigned char *hex,unsigned char *a,__int16 length);
//86
__int16 __stdcall a_hex(unsigned char *a,unsigned char *hex,__int16 len);
//87
__int16 __stdcall dc_config_card(HANDLE icdev,unsigned char cardtype);
//89
__int16 __stdcall dc_slotmarker(HANDLE icdev,unsigned char N, unsigned char *ATQB);
//90
__int16 __stdcall dc_attrib(HANDLE icdev,unsigned char *PUPI, unsigned char CID);
//91
__int16 __stdcall dc_open_door(HANDLE icdev,unsigned char cflag);
//92
__int16 __stdcall dc_open_timedoor(HANDLE icdev,unsigned __int16 utime);
//93
__int16 __stdcall dc_read_random(HANDLE icdev, unsigned char *data);
//94
__int16 __stdcall dc_write_random(HANDLE icdev,__int16 len, unsigned char *data);
//95
__int16 __stdcall dc_read_random_hex(HANDLE icdev, unsigned char *data);
//96
__int16 __stdcall dc_write_random_hex(HANDLE icdev,__int16 len, unsigned char *data);
//97
__int16 __stdcall dc_erase_random(HANDLE icdev,__int16 len);

//99
__int16 __stdcall dc_authentication_pass(HANDLE icdev,unsigned char _Mode,
										 unsigned char _Addr,unsigned char *passbuff);


__int16 __stdcall dc_card_double(HANDLE icdev,unsigned char _Mode,unsigned char *_Snr);
__int16 __stdcall dc_card_double_hex(HANDLE icdev,unsigned char _Mode,unsigned char *_Snr);


__int16 __stdcall dc_authentication_pass_hex(HANDLE icdev,unsigned char _Mode,
										 unsigned char _Addr,unsigned char *passbuff);




//------------------------------新增的为了整齐DLL而做的代码------------------------------
__int16  __stdcall dc_HL_write_hex(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
								   unsigned long *_Snr,unsigned char *_Data);
__int16  __stdcall dc_HL_read_hex(HANDLE icdev,unsigned char _Mode,unsigned char _Adr,
								  unsigned long _Snr,unsigned char *_Data,unsigned long *_NSnr);
__int16  __stdcall dc_check_write_hex(HANDLE icdev,unsigned long Snr,unsigned char authmode,
									  unsigned char Adr,unsigned char * _data);

__int16  __stdcall dc_srd_eeprom_hex(HANDLE icdev,__int16   offset,__int16   lenth,
									 unsigned char *rec_buffer);
__int16  __stdcall dc_swr_eeprom_hex(HANDLE icdev,__int16   offset,__int16   lenth,
									 unsigned char* send_buffer);


__int16  __stdcall dc_des_hex(unsigned char *key,unsigned char *sour,unsigned char *dest,
							  __int16 m);
__int16 __stdcall dc_des(unsigned char *key,unsigned char *sour,unsigned char *dest,__int16 m);



__int16 __stdcall dc_authentication_passaddr(HANDLE icdev,unsigned char _Mode,
										 unsigned char _Addr,unsigned char *passbuff);
__int16 __stdcall dc_authentication_passaddr_hex(HANDLE icdev,unsigned char _Mode,
											 unsigned char _Addr,unsigned char *passbuff);


__int16 __stdcall dc_setusbtimeout(unsigned char ntimes);


__int16 __stdcall dc_tripledes(unsigned char *key,unsigned char *src,unsigned char *dest,__int16 m);

__int16 __stdcall dc_tripledes_hex(unsigned char *key,unsigned char *src,unsigned char *dest,__int16 m);

#ifdef  __cplusplus
}
#endif