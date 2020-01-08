//
// Created by jmbahn on 19. 8. 21.
//

#include "nxcommand.h"

#include <stdio.h>
#include <poll.h>
#include <stdint.h>	//	intXX_t
#include <fcntl.h>	//	O_RDWR
#include <string.h>	//	memset
#include <unistd.h>
#include <stddef.h>	//	struct sockaddr_un
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#define NX_DTAG "[rearcam_nxcommand]"
#include <NX_DbgMsg.h>


//#include "CNX_BaseClass.h"
#include "nxcommand.h"

#define POLL_TIMEOUT_MS		(2000)
//#define POLL_TIMEOUT_MS		(100)

//class NX_CCommand : protected CNX_Thread
class NX_CCommand
{
public:
	void RegEventCallback( void (*cbFunc)( int32_t) );
	int StartService(char *);
	void StopService();
	int IsReceivedStopCmd();
	int CreateCmdFile(char* m_pCmdFileName);
	int WriteCmd(char* command, char* m_pCmdFileName);

	NX_CCommand();
	~NX_CCommand();

	//  Implementation Pure Virtual Function
	//virtual void  ThreadProc();
private:
    static void*	ThreadStub( void *pObj );
    void			ThreadProc( void );

private:
	//
	// Hardware Depend Parameter
	//

	static NX_CCommand* m_pCommandServer;

	pthread_t		m_hThread;

	int				m_bExitLoop;
	int 			fd_gpio;
	int				fd_cmd;

	struct pollfd poll_fds;
	int poll_ret;
	unsigned char rx_buf[256];

	int receive_stop_cmd;

	char *m_pStopCmdFileName;
	char *m_pStatusCmdFileName;

	//	for Callback Function
	void				(*m_EventCallBack)(int32_t);




private:
	NX_CCommand (const NX_CCommand &Ref);
	NX_CCommand &operator=(const NX_CCommand &Ref);
};

//------------------------------------------------------------------------------
NX_CCommand::NX_CCommand()
	:m_bExitLoop ( false )
    , receive_stop_cmd(0)
{

}

//------------------------------------------------------------------------------
NX_CCommand::~NX_CCommand()
{
}


//------------------------------------------------------------------------------
int NX_CCommand::CreateCmdFile(char* m_pCmdFileName)
{
    int ret = -1;

    if( 0 == access(m_pCmdFileName, F_OK))
    {
    	remove(m_pCmdFileName);
    }

    ret = mknod(m_pCmdFileName, S_IFIFO | 0666, 0 );

    return ret;
}

//----------------------------------------------------------------------------
int NX_CCommand::WriteCmd(char* command, char* m_pCmdFileName)
{
    int cmd_fd = 0;

    cmd_fd = open(m_pCmdFileName, O_RDWR);
    if(cmd_fd > 0)
    {
        write(cmd_fd, "stop", sizeof("stop"));
        close( cmd_fd );
        return 1;
    }

    return -1;
}


int NX_CCommand::StartService(char *m_pCmdFileName)
{
    NxDbgMsg( NX_DBG_INFO, "%s()++++\n", __FUNCTION__);
	m_bExitLoop = false;

	m_pStopCmdFileName = m_pCmdFileName;

    //Start();

    if(0>pthread_create(&this->m_hThread, NULL, this->ThreadStub, this)){
        return -1;
    }
    NxDbgMsg( NX_DBG_INFO, "%s()----\n", __FUNCTION__);

    return 1;

}

void NX_CCommand::StopService()
{
    NxDbgMsg( NX_DBG_INFO, "%s()++++\n", __FUNCTION__);
	m_bExitLoop = true;

    close( fd_cmd );

    if(m_hThread )
    {
        pthread_join(m_hThread, NULL);
        //m_hThread = NULL;
		m_hThread = 0;
    }

	//Stop();

	NxDbgMsg( NX_DBG_INFO, "%s()----\n", __FUNCTION__);
}

int NX_CCommand::IsReceivedStopCmd()
{
    return receive_stop_cmd;
}

void* NX_CCommand::ThreadStub(void *pObj)
{
    if(NULL != pObj) {
        ((NX_CCommand*)pObj)->ThreadProc();
    }

    return (void*)0xDEADDEAD;
}

void  NX_CCommand::ThreadProc()
{
	usleep(100000);

	receive_stop_cmd = 0;

    CreateCmdFile(m_pStopCmdFileName);

	fd_cmd  = open( m_pStopCmdFileName, O_RDWR );
    NxDbgMsg( NX_DBG_INFO, "%s()++++  command file open : %s -  fd : %d\n", __FUNCTION__, m_pStopCmdFileName, fd_cmd);


	do{
        if(fd_cmd >= 0)
        {
        	memset( &poll_fds, 0, sizeof(poll_fds) );
            poll_fds.fd = fd_cmd;
            poll_fds.events = POLLIN;

            poll_ret = poll( &poll_fds, 1, POLL_TIMEOUT_MS );
            if( poll_ret == 0 )
            {
                printf("Poll Timeout\n");
                continue;
            }
            else if( 0 > poll_ret )
            {
                printf("Poll Error !!!\n");
            }
            else
            {
                int read_size;

                //if( poll_fds.revents & POLLIN )
                {
                    memset( rx_buf, 0, sizeof(rx_buf) );
                    read_size = read( fd_cmd, rx_buf, sizeof(rx_buf));
                    NxDbgMsg( NX_DBG_INFO, "CMD : Size = %d, %s\n", read_size, rx_buf);
                    if(read_size == -1)
                        continue;

                    if(read_size > 0)
                    {
                        if(!strncmp((const char*)rx_buf, "stop", sizeof("stop")))
                        {
                            NxDbgMsg( NX_DBG_INFO, "----------------Received Stop Command\n");
                            receive_stop_cmd = 1;
                            m_bExitLoop = true;
                        }else
                        {
                            receive_stop_cmd = 0;
                        }
                    }

                }
            }
        }
        //else
        //{
        //    NxDbgMsg( NX_DBG_INFO, "----------------command file open\n");
        //    fd_cmd  = open( m_pStopCmdFileName, O_RDWR );
        //}
        usleep(100000);

	}while( !m_bExitLoop );

}

//
//		External Interface
//
void* NX_GetCommandHandle()
{
	NX_CCommand *pInst = new NX_CCommand();

	return (void*)pInst;
}

int NX_CreateCmdFile(void *pObj, char *m_pCtrlFileName)
{
    int ret = 0;
    NX_CCommand *pInst = (NX_CCommand *)pObj;

    ret = pInst->CreateCmdFile(m_pCtrlFileName);

    return ret;
}

int NX_WriteCmd(void *pObj, char *command, char *m_pCtrlFileName)
{
    int ret = 0;
    NX_CCommand *pInst = (NX_CCommand *)pObj;

    ret = pInst->WriteCmd(command, m_pCtrlFileName);

    return ret;
}

int NX_StartCommandService(void *pObj, char *m_pCtrlFileName)
{
    int ret = 0;
	NX_CCommand *pInst = (NX_CCommand *)pObj;

	ret = pInst->StartService(m_pCtrlFileName);
	return ret;
}

void NX_StopCommandService(void *pObj, char *m_pCtrlFileName)
{

	NX_CCommand *pInst = (NX_CCommand*)pObj;

	pInst->StopService();

	delete pInst;
}

int NX_CheckReceivedStopCmd(void *pObj)
{
    NX_CCommand *pInst = (NX_CCommand*)pObj;

    return pInst->IsReceivedStopCmd();
}
