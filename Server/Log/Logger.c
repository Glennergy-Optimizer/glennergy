#include "Logger.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>

// add threadid, LOG_FILE_SIZE, name:YYYY_MM_DD new file everyday? blocking for dev, fix message before shutdown

#define LOG_MSG_MAX 256

typedef struct {
    time_t timestamp;
    pid_t pid;
    char level[16];
    char module[32];
    char message[LOG_MSG_MAX];
} LogMessage;

static int write_fd = -1;
static pid_t logger_pid = -1;
static LogLevel log_level = LOG_LEVEL_INFO;

static FILE* log_file = NULL;
static char log_path[128] = ""; //will be set in log_Init

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void log_ProcessLoop(int read_fd);
static void log_ToFile(const LogMessage* log_msg);
const char* log_GetLevelString(LogLevel level);

int log_Init(const char* filename)
{
    const char* base_dir = "/var/log/glennergy";
    const char* default_filename = "glennergy.log";
    const char* log_filename;
    
    if (filename == NULL) {
        log_filename = default_filename;
    } else {
        log_filename = filename;
    }

    snprintf(log_path, sizeof(log_path), "%s/%s", base_dir, log_filename);

    if (access(base_dir, W_OK) != 0) {
        fprintf(stderr, "FATAL: Log directory not writable: %s\n", base_dir);
        fprintf(stderr, "Run 'make dirs' to create directories.\n");
        return -1;
    }

    int pipe_fds[2];

    if (pipe(pipe_fds) < 0)
    {
        perror("pipe");
        return -1;
    }
    logger_pid = fork();

    if (logger_pid < 0)
    {
        perror("fork");
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return -1;
    }

    if (logger_pid == 0)
    {
        //child process
        close(pipe_fds[1]); 
        log_ProcessLoop(pipe_fds[0]); //loop only reads

        close(pipe_fds[0]);
        exit(0);
    }
    else
    {
        //parent process
        close(pipe_fds[0]); 
        write_fd = pipe_fds[1]; //only writes, server child processes will inherit

        int flags = fcntl(write_fd, F_GETFL, 0);
        if (flags != -1) {
            fcntl(write_fd, F_SETFL, flags | O_NONBLOCK);
        }

        printf("Logger initialized (PID: %d)\n", logger_pid);    
    }
    return 0;
}

void log_Message(LogLevel level, const char* module, const char* msg)
{
    if(write_fd < 0)
    {
        fprintf(stderr, "Logger not initialized!\n");
        return;
    }

    if(level < log_level)
        return;
    
    pthread_mutex_lock(&log_mutex);
    
    LogMessage log_msg = {0};
    log_msg.timestamp = time(NULL);
    log_msg.pid = getpid();
    strncpy(log_msg.level, log_GetLevelString(level), sizeof(log_msg.level) - 1);
    strncpy(log_msg.module, module, sizeof(log_msg.module) - 1);
    strncpy(log_msg.message, msg, sizeof(log_msg.message) - 1);
    
    ssize_t written = write(write_fd, &log_msg, sizeof(LogMessage));

    if (written == sizeof(LogMessage))
    {
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    
    // Handle write errors
    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        static unsigned long dropped = 0;
        dropped++;
        
        if (dropped % 1000 == 0)
            fprintf(stderr, "WARNING: Log buffer full, %lu messages dropped\n", dropped);
        
        if (level >= LOG_LEVEL_ERROR)
            fprintf(stderr, "[DROPPED ERROR] [%s] %s\n", module, msg);
    }
    else if (written < 0)
    {
        perror("log_Message() write");
    }
    else
    {
        fprintf(stderr, "WARNING: Partial log write (%zd/%zu bytes)\n", 
                written, sizeof(LogMessage));
    }
    
    pthread_mutex_unlock(&log_mutex);
}

void log_MessageFmt(LogLevel level, const char* module, const char* fmt, ...)
{
    char buffer[LOG_MSG_MAX];  // Larger buffer for formatting
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Call existing log_Message with formatted string
    log_Message(level, module, buffer);
}

static void log_ProcessLoop(int read_fd)
{
    LogMessage log_msg;
    ssize_t bytes_read;

    log_file = fopen(log_path, "a");
    if (!log_file)
    {
        perror("log_ProcessLoop fopen");
        return;
    }

    //line buffering
    setvbuf(log_file, NULL, _IOLBF, 8192);

    while (1) 
    {
        bytes_read = read(read_fd, &log_msg, sizeof(LogMessage));

        if (bytes_read == 0)
        {
            printf("Logger: pipe closed\n");
            break;
        }

        if (bytes_read < 0)
        {
            if (errno == EINTR)
                continue;  // Interrupted by signal, retry
            
            perror("Logger: read error");
            break;  // Fatal error, exit loop
        }

        if (bytes_read != sizeof(LogMessage))
        {
            fprintf(stderr, "Logger: incomplete read (%zd/%zu bytes)\n", bytes_read, sizeof(LogMessage));
            continue;
        }

        log_ToFile(&log_msg);    
    }
    if(log_file)
    {
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }
}

static void log_ToFile(const LogMessage* log_msg)
{
    if (!log_file)
        return;

    char time_buf[64];
    struct tm* tm_info = localtime(&log_msg->timestamp);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] [PID:%d] [%s] [%s]: %s\n", time_buf, log_msg->pid, log_msg->level, log_msg->module, log_msg->message);

    #ifdef DEBUG
    fprintf(stderr, "[%s] [%s] [%s]: %s\n", 
            time_buf, log_msg->level, log_msg->module, log_msg->message);
    #endif

}

const char* log_GetLevelString(LogLevel level)
{
    switch(level)
    {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARN:    return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default: return "INFO";
    }
}

void log_SetLevel(LogLevel level)
{
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_ERROR)
        return;
    log_level = level;
}

void log_CloseWrite(void)
{
    if (write_fd >= 0)
    {
        close(write_fd);
        write_fd = -1;
    }
}

void log_Cleanup(void)
{
    if (write_fd >= 0)
    {
        close(write_fd);
        write_fd = -1;
    }
    if (logger_pid > 0)
    {
        printf("Waiting for logger to exit...\n");
        waitpid(logger_pid, NULL, 0);
        printf("Logger exited.\n");
        logger_pid = -1;
    }
    
    pthread_mutex_destroy(&log_mutex);
}