/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>


#define MAX_DEBUG_LOG_BUFF_SIZE 1024

void logprintf(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char gDebugPrintBuffer[MAX_DEBUG_LOG_BUFF_SIZE];           
	vsnprintf(gDebugPrintBuffer, MAX_DEBUG_LOG_BUFF_SIZE, format, args);   
	gDebugPrintBuffer[(MAX_DEBUG_LOG_BUFF_SIZE-1)] = 0; 

	va_end(args);          
	struct timeval t;      
	gettimeofday(&t, NULL);
	printf("%ld:%03ld : %s\n", (long int)t.tv_sec, (long int)t.tv_usec / 1000, gDebugPrintBuffer);
}
