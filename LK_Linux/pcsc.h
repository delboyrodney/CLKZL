/*
Copyright (c) 2011, Gerhard H. Schalk (www.smartcard-magic.net)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/
#include <winscard.h>
#include <conio.h>
#include <stdio.h>

#ifndef PCSC_H_INCLUDE
#define PCSC_H_INCLUDE

	#define PCSC_STATUS(lRetValue, msg)						  \
		if(lRetValue == SCARD_S_SUCCESS)					  \
		{													  \
			printf("\n   " msg  ": %s",						  \
					SCardGetErrorString(lRetValue));		  \
		}													  \
		else												  \
		{													  \
			printf("\n   " msg  ": Error 0x%04X %s",		  \
				   lRetValue,SCardGetErrorString(lRetValue)); \
			return lRetValue;								  \
	    }								

	#define PCSC_ERROR(lRetValue, msg)				          \
		if(lRetValue != SCARD_S_SUCCESS)					  \
		{													  \
			printf("\n   " msg  ": Error 0x%04X %s",		  \
				   lRetValue,SCardGetErrorString(lRetValue)); \
			return lRetValue;								  \
		}
	
    #define PCSC_EXIT_ON_ERROR(lRetValue)   	              \
		if(lRetValue != SCARD_S_SUCCESS)					  \
		{													  \
            while(!_kbhit());                                 \
			return 0;								          \
		}

	LONG PCSC_Connect(LPTSTR sReader );
	LONG PCSC_ActivateCard(void);

	LONG PCSC_Exchange(LPCBYTE pbSendBuffer ,DWORD  cbSendLength ,
					   LPBYTE  pbRecvBuffer ,LPDWORD pcbRecvLength );
	LONG PCSC_Disconnect(void);
	
    LONG PCSC_WaitForCardPresent(void);
    LONG PCSC_WaitForCardRemoval(void);
	
    LONG PCSC_GetAtrString(LPBYTE atr, LPINT atrLen);
	LONG PCSC_GetVentorName();

	CHAR*  SCardGetErrorString(LONG lRetValue);	
	
#endif 
