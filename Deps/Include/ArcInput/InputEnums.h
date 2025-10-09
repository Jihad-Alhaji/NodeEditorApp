#pragma once
#include"Macros.h"
namespace AppComponents {
namespace InputManagement
{
	/*
	* any key that is clickable
	* TODO: provide support for other platforms[now tested on windows]
	*/
	enum class ARCINPUT_API EKeys: unsigned short
	{
		//standard mouse buttons
		MOUSE_LBUTTON  = 0x01,
		MOUSE_RBUTTON		 ,
		MOUSE_CANCEL		 ,
		MOUSE_MBUTTON		 ,
		MOUSE_X1			 ,//special btn 1
		MOUSE_X2			 , //special btn 2

		//keyboard keys
		BACK     = 0x08,
		TAB			   ,

		CLEAR    = 0x0C,
		RETURN         ,

		SHIFT    = 0x10,
		CONTROL        ,
		MENU           ,
		PAUSE          ,
		CAPITAL        ,

		ESCAPE	 = 0x1B,
		CONVERT        ,
		NONCONVERT     ,
		ACCEPT         ,
		MODECHANGE     ,
		SPACE          ,
		PRIOR          ,
		NEXT           ,
		END            ,
		HOME           ,
		LEFT           ,
		UP             ,
		RIGHT          ,
		DOWN           ,
		SELECT         ,
		PRINT          ,
		EXECUTE        ,
		SNAPSHOT       ,
		INSERT         ,
		DELETE         ,
		HELP           ,

		NUM_0  = '0',
		NUM_1		,
		NUM_2		,
		NUM_3		,
		NUM_4		,
		NUM_5		,
		NUM_6		,
		NUM_7		,
		NUM_8		,
		NUM_9		,
		
		A      = 'A',
		B			,
		C			,
		D			,
		E			,
		F			,
		G			,
		H			,
		I			,
		J			,
		K			,
		L			,
		M			,
		N			,
		O			,
		P			,
		Q			,
		R			,
		S			,
		T			,
		U			,
		V			,
		W			,
		X			,
		Y			,
		Z			,
		LWIN		,
		RWIN		,
		APPS		,

		SLEEP     =0x5F,
		NUMPAD0        ,
		NUMPAD1        ,
		NUMPAD2        ,
		NUMPAD3        ,
		NUMPAD4        ,
		NUMPAD5        ,
		NUMPAD6        ,
		NUMPAD7        ,
		NUMPAD8        ,
		NUMPAD9        ,
		MULTIPLY       ,
		ADD            ,
		SEPARATOR      ,
		SUBTRACT       ,
		DECIMAL        ,
		DIVIDE         ,
		F1             ,
		F2             ,
		F3             ,
		F4             ,
		F5             ,
		F6             ,
		F7             ,
		F8             ,
		F9             ,
		F10            ,
		F11            ,
		F12            ,
		F13            ,
		F14            ,
		F15            ,
		F16            ,
		F17            ,
		F18            ,
		F19            ,
		F20            ,
		F21            ,
		F22            ,
		F23            ,
		F24            ,

		NUMLOCK   =0x90,
		SCROLL         ,
		NUMPAD_EQUAL   ,

		LSHIFT    =0xA0,
		RSHIFT         ,
		LCONTROL       ,
		RCONTROL       ,
		LMENU          ,
		RMENU          ,

		VOLUME_MUTE    =0xAD,
		VOLUME_DOWN         ,
		VOLUME_UP           ,
		MEDIA_NEXT_TRACK    ,
		MEDIA_PREV_TRACK    ,
		MEDIA_STOP          ,
		MEDIA_PLAY_PAUSE    ,
		LAUNCH_MAIL         ,
		LAUNCH_MEDIA_SELECT ,
		LAUNCH_APP1         ,
		LAUNCH_APP2         ,

		OEM_1     =0xBA,   // ';:' for US
		PLUS       ,   // '+' any country
		COMMA      ,   // ',' any country
		MINUS      ,   // '-' any country
		PERIOD     ,   // '.' any country
		OEM_2          ,   // '/?' for US
		OEM_3          ,   // '`~' for US

		GAMEPAD_A                    =0xC3, 
		GAMEPAD_B                         , 
		GAMEPAD_X                         , 
		GAMEPAD_Y                         , 
		GAMEPAD_RIGHT_SHOULDER            , 
		GAMEPAD_LEFT_SHOULDER             , 
		GAMEPAD_LEFT_TRIGGER              , 
		GAMEPAD_RIGHT_TRIGGER             , 
		GAMEPAD_DPAD_UP                   , 
		GAMEPAD_DPAD_DOWN                 , 
		GAMEPAD_DPAD_LEFT                 , 
		GAMEPAD_DPAD_RIGHT                , 
		GAMEPAD_MENU                      , 
		GAMEPAD_VIEW                      , 
		GAMEPAD_LEFT_THUMBSTICK_BUTTON    , 
		GAMEPAD_RIGHT_THUMBSTICK_BUTTON   ,

		OEM_4     =0xDB,  //  '[{' for US
		OEM_5          ,  //  '\|' for US
		OEM_6          ,  //  ']}' for US
		OEM_7          ,  //  ''"' for US
	};
	/*
	*keyboard keys action
	*for now it is supporting window , not sure if its cross platform
	*/
	enum class ARCINPUT_API EKeyAcions : unsigned short
	{
		PRESSED = 0xff00,
		RELEASED = 0x0000
	};
	/*
	* input tyes that are represented by an axis : mouse position, joystick
	* TODO: #include<dinput.h> use this file to provide support for joysticks
	*/
	enum class ARCINPUT_API EAxis
	{
		MOUSE_X		,
		MOUSE_Y		,	
		GAMEPAD_LEFT_THUMBSTICK_UP   =0xD3, 
		GAMEPAD_LEFT_THUMBSTICK_DOWN      , 
		GAMEPAD_LEFT_THUMBSTICK_RIGHT     , 
		GAMEPAD_LEFT_THUMBSTICK_LEFT      , 
		GAMEPAD_RIGHT_THUMBSTICK_UP       , 
		GAMEPAD_RIGHT_THUMBSTICK_DOWN     , 
		GAMEPAD_RIGHT_THUMBSTICK_RIGHT    , 
		GAMEPAD_RIGHT_THUMBSTICK_LEFT     , 
		//these are ARCEngine defined types
		GAMEPAD_LEFT_THUMBSTICK_X			,
		GAMEPAD_LEFT_THUMBSTICK_Y			,
		GAMEPAD_RIGHT_THUMBSTICK_X			,
		GAMEPAD_RIGHT_THUMBSTICK_Y
	};
}
}