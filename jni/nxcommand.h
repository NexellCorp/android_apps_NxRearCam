//
// Created by jmbahn on 19. 8. 21.
//

#ifndef NXBACKGEARSERVICE_NXCOMMAND_H
#define NXBACKGEARSERVICE_NXCOMMAND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum{
	STOP = 1,
	RUN,
	MAX_COMMAND
};

void* NX_GetCommandHandle();
int NX_CreateCmdFile(void*, char*);
int NX_WriteCmd(void*, char*, char*);
int NX_StartCommandService(void*, char*);
void NX_StopCommandService(void*, char*);
int NX_CheckReceivedStopCmd(void*);
#ifdef __cplusplus
}
#endif



#endif //NXBACKGEARSERVICE_NXCOMMAND_H




