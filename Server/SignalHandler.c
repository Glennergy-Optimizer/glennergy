#include <signal.h>
#include <sys/types.h>
#include "SignalHandler.h"

static sig_atomic_t stop = 0;

void SignalHandler_Handle(int sig)
{
    (void)sig;
    stop = 1;
}

void SignalHandler_Initialize()
{
    signal(SIGINT, SignalHandler_Handle);
    signal(SIGTERM, SignalHandler_Handle);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
}

int SignalHandler_Stop()
{
    return stop;
}