#define MODULE_NAME "INPUTCACHE"
#include "../Server/Log/Logger.h"
#include "InputCache.h"
#include "../API/Meteo/Meteo.h"
#include "../API/Spotpris/Spotpris.h"
#include "../Libs/Utils/utils.h"
#include "../Libs/Sockets.h"
#include "../Libs/Pipes.h"
#include "../Libs/Shm.h"
#include "../Libs/Homesystem.h"
#include <stdio.h>
#include <jansson.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define FIFO_METEO_READ "/tmp/fifo_meteo"
#define FIFO_SPOTPRIS_READ "/tmp/fifo_spotpris"

#define MAX_BACKLOG 5
#define MAX 5

const char *area_names[AREA_COUNT] = {"SE1", "SE2", "SE3", "SE4"};
static int notify_fd = -1;

int inputcache_Init(InputCacheContext_t *ctx, const char* file_path)
{
    if (!ctx || !file_path) {
        LOG_ERROR("Invalid parameters for InputCache_Init");
        return -1;
    }

    LOG_INFO("Loading homesystem file: %s", file_path);
    int loaded = homesystem_LoadAllCount(ctx->cache->home, file_path, MAX);
    if (loaded < 0)
    {
        LOG_ERROR("Failed to load homesystem file: %s", file_path);
        return -1;
    }
    ctx->cache->home_count = (size_t)loaded;
    LOG_INFO("Loaded %zu homesystem entries", ctx->cache->home_count);
    return 0;
}

int inputcache_CreateSocket(void)
{
    unlink(CACHE_SOCKET_PATH);
    
    int sock_fd = socket_CreateSocket();
    if (sock_fd < 0) {
        LOG_ERROR("Failed to create socket");
        return -1;
    }


    if (socket_Bind(sock_fd, CACHE_SOCKET_PATH) < 0) {
        LOG_ERROR("Failed to bind socket to %s", CACHE_SOCKET_PATH);
        close(sock_fd);
        return -1;
    }

    if (chmod(CACHE_SOCKET_PATH, 0660) < 0) {
        LOG_ERROR("Failed to set permissions on socket: %s", strerror(errno));
        close(sock_fd);
        unlink(CACHE_SOCKET_PATH);
        return -1;
    }

    if (socket_Listen(sock_fd, MAX_BACKLOG) < 0) {
        LOG_ERROR("Failed to listen on socket");
        close(sock_fd);
        unlink(CACHE_SOCKET_PATH);
        return -1;
    }

    LOG_INFO("Cache socket listening on %s (permissions: 0660)", CACHE_SOCKET_PATH);
    return sock_fd;
}

int inputcache_OpenFIFOs(int *meteo_fd, int *spotpris_fd)
{
    if (!meteo_fd || !spotpris_fd) {
        LOG_ERROR("Invalid fd pointers");
        return -1;
    }

    if (mkfifo(FIFO_METEO_READ, 0660) < 0 && errno != EEXIST) {
        LOG_WARNING("mkfifo %s: %s", FIFO_METEO_READ, strerror(errno));
    }
    if (mkfifo(FIFO_SPOTPRIS_READ, 0660) < 0 && errno != EEXIST) {
        LOG_WARNING("mkfifo %s: %s", FIFO_SPOTPRIS_READ, strerror(errno));
    }

    LOG_INFO("Opening FIFOs ...");

    *meteo_fd = open(FIFO_METEO_READ, O_RDWR);
    if (*meteo_fd < 0) {
        LOG_ERROR("Failed to open FIFO: %s - %s", FIFO_METEO_READ, strerror(errno));
        return -1;
    }

    *spotpris_fd = open(FIFO_SPOTPRIS_READ, O_RDWR);
    if (*spotpris_fd < 0) {
        LOG_ERROR("Failed to open FIFO: %s - %s", FIFO_SPOTPRIS_READ, strerror(errno));
        close(*meteo_fd);
        return -1;
    }

    LOG_INFO("FIFOs opened successfully");
    return 0;
}

int inputcache_InitShm(InputCacheContext_t *ctx)
{
    if (!ctx) {
        LOG_ERROR("Invalid cache pointer for shm init");
        return -1;
    }

    if (shm_Create() < 0) {
        LOG_ERROR("Failed to create shared memory");
        return -1;
    }

    ctx->shm = shm_Attach();
    if (!ctx->shm) {
        LOG_ERROR("Failed to attach to shared memory");
        shm_Destroy();
        return -1;
    }

    LOG_INFO("Shared memory initialized successfully");
    return 0;
}

int inputcache_InitNotifyPipe(void)
{
    if (access(NOTIFY_FIFO_PATH, F_OK) != 0) {
        mkfifo(NOTIFY_FIFO_PATH, 0660);
    }
    
    notify_fd = open(NOTIFY_FIFO_PATH, O_WRONLY | O_NONBLOCK);
    if (notify_fd < 0 && errno == ENXIO) {
        LOG_INFO("Notification pipe ready, waiting for Algorithm...");
        notify_fd = -1;
        return 0;
    }
    
    if (notify_fd >= 0) {
        LOG_INFO("Notification pipe connected");
    }
    return 0;
}

int inputcache_InitAll(InputCacheContext_t *ctx, const char* config_path)
{
    ctx->meteo_fd = -1;
    ctx->spotpris_fd = -1;
    ctx->socket_fd = -1;
    ctx->cache = NULL;
    ctx->updated_meteo = false;
    ctx->updated_spotpris = false;
    ctx->shm = NULL;
    
    ctx->cache = malloc(sizeof(CacheData_t));
    if (!ctx->cache) {
        LOG_ERROR("malloc() failed");
        return -1;
    }
    memset(ctx->cache, 0, sizeof(CacheData_t));
    
    if (inputcache_Init(ctx, config_path) != 0) {
        free(ctx->cache);
        return -1;
    }
    
    if (inputcache_OpenFIFOs(&ctx->meteo_fd, &ctx->spotpris_fd) != 0) {
        free(ctx->cache);
        return -1;
    }
    
    ctx->socket_fd = inputcache_CreateSocket();
    if (ctx->socket_fd < 0) {
        close(ctx->meteo_fd);
        close(ctx->spotpris_fd);
        free(ctx->cache);
        return -1;
    }
    
    if (inputcache_InitShm(ctx) != 0) {
        close(ctx->socket_fd);
        close(ctx->meteo_fd);
        close(ctx->spotpris_fd);
        free(ctx->cache);
        return -1;
    }
    
    if (inputcache_InitNotifyPipe() != 0) {
        LOG_WARNING("Notification pipe init failed, will retry");
    }
    
    LOG_INFO("InputCache fully initialized");
    return 0;
}

void inputcache_HandleRequest(InputCacheContext_t *ctx, int client_fd)
{
    CacheRequest req;
    CacheResponse resp;

    ssize_t bytesread = recv(client_fd, &req, sizeof(req), 0);
    if (bytesread != sizeof(req)) 
    {
        LOG_ERROR("Invalid request size: %zd", bytesread);
        close(client_fd);
        return;
    }

    if (req.command < CMD_GET_ALL || (req.command > CMD_SET_RESULT && req.command != CMD_PING))
    {
        LOG_ERROR("Invalid command: %d from client", req.command);
        resp.status = 1;  // Error
        resp.data_size = 0;
        
        send(client_fd, &resp, sizeof(resp), 0);
        close(client_fd);
        return;
    }
    LOG_INFO("Received request with command: %d", req.command);

    switch(req.command)
    {
        case CMD_GET_ALL:
            LOG_INFO("Handling CMD_GET_ALL request");
            resp.status = 0;
            resp.data_size = sizeof(CacheData_t);

            send(client_fd, &resp, sizeof(resp), 0);
            send(client_fd, ctx->cache, sizeof(CacheData_t), 0);
            LOG_INFO("Sent complete CacheData to algorithm");
            break;
        
        case CMD_GET_METEO:
            LOG_INFO("Handling CMD_GET_METEO request");
            resp.status = 0;
            resp.data_size = sizeof(Meteo_t) * ctx->cache->meteo_count;

            send(client_fd, &resp, sizeof(resp), 0);
            send(client_fd, ctx->cache->meteo, sizeof(Meteo_t) * ctx->cache->meteo_count, 0);
            LOG_INFO("Sent %zu Meteo entries to algorithm", ctx->cache->meteo_count);
            break;

        case CMD_GET_SPOTPRIS:
            LOG_INFO("Handling CMD_GET_SPOTPRIS request");
            resp.status = 0;
            resp.data_size = sizeof(Spot_t);
            
            send(client_fd, &resp, sizeof(resp), 0);
            send(client_fd, &ctx->cache->spotpris, sizeof(Spot_t), 0);
            LOG_INFO("Sent Spotpris data to client");
            break;

        case CMD_SET_RESULT:
        {
            LOG_INFO("Handling CMD_SET_RESULT request");
            ResultRequest set_req;
            ssize_t result_bytes = recv(client_fd, &set_req, sizeof(set_req), 0);

            if (result_bytes != sizeof(set_req)) {
                LOG_ERROR("Failed to receive complete results data (got %zd bytes)", result_bytes);
                resp.status = 1;
                resp.data_size = 0;
                send(client_fd, &resp, sizeof(resp), 0);
                break;
            }

            if (!ctx->shm) {
                LOG_ERROR("Shared memory not initialized");
                resp.status = 1;
                resp.data_size = 0;
                send(client_fd, &resp, sizeof(resp), 0);
                break;
            }

            if (shm_Lock_Write(ctx->shm) == 0) {
                int updated = 0;
                for (size_t i = 0; i < set_req.count; i++) {
                    if (shm_UpdateResults(ctx->shm, &set_req.results[i]) == 0) {
                        updated++;
                    } else {
                        LOG_WARNING("Failed to update results for home_id %d", set_req.results[i].home_id);
                    }
                }
                shm_Unlock_Write(ctx->shm);

                LOG_INFO("Updated %d/%zu results in shared memory", updated, set_req.count);
                resp.status = 0;
            } else {
                LOG_ERROR("Failed to lock shared memory for writing");
                resp.status = 1;
            }
            resp.data_size = 0;
            send(client_fd, &resp, sizeof(resp), 0);
        }
        break;
            
        case CMD_PING:
            LOG_INFO("Handling CMD_PING request");
            resp.status = 0;
            resp.data_size = 0;
            send(client_fd, &resp, sizeof(resp), 0);
            LOG_INFO("Sent PING response to client");
            break;

        default:
            LOG_WARNING("Received unknown command: %d", req.command);
            resp.status = 1;
            resp.data_size = 0;
            send(client_fd, &resp, sizeof(resp), 0);
            break;
    }

    close(client_fd);
}

static int inputcache_SaveMeteo(const MeteoData *_Data)
{
    if (!_Data)
        return -1;

    int result = 0;

    const char *meteo_folder = "/var/cache/glennergy/meteo";
    dir_result_t dir_res = create_folder(meteo_folder);
    if (dir_res == DIR_ERROR)
    {
        LOG_ERROR("Error creating cache folder: %s", meteo_folder);
        return -2;
    }

    char filename[64];
    char date_str[16];
    GetTodayDateFile(date_str, sizeof(date_str));

    for (size_t i = 0; i < _Data->pCount; i++)
    {
        snprintf(filename, sizeof(filename), "%s/meteo_%d_%s.json", meteo_folder, _Data->pInfo[i].id, date_str);
        json_error_t error;
        json_t *root = json_loads(_Data->pInfo[i].raw_json_data, 0, &error);

        if (root == NULL)
        {
            LOG_ERROR("Failed to load raw json [METEO]");
            return -2;
        }
        result = json_dump_file(root, filename, JSON_INDENT(4));

        if (result < 0)
        {
            LOG_WARNING("Failed to dump save file for meteo, property id: %d", _Data->pInfo[i].id);
        }
        json_decref(root);
    }

    LOG_INFO("Saving %zu meteo entries to file\n", _Data->pCount);
    return 0;
}

void inputcache_HandleMeteoData(InputCacheContext_t *ctx, int meteo_fd)
{            
        MeteoData meteo_test;
        ssize_t bytesReadMeteo = Pipes_ReadBinary(meteo_fd, &meteo_test, sizeof(MeteoData));

        if (bytesReadMeteo == sizeof(MeteoData))
        {
            LOG_INFO("Got new data meteo %zd, count %zu", bytesReadMeteo, meteo_test.pCount);

            ctx->cache->meteo_count = meteo_test.pCount;
            for (size_t i = 0; i < ctx->cache->meteo_count; i++)
            {
                ctx->cache->meteo[i].id = meteo_test.pInfo[i].id;
                strncpy(ctx->cache->meteo[i].city, meteo_test.pInfo[i].property_name, NAME_MAX - 1);
                ctx->cache->meteo[i].lat = meteo_test.pInfo[i].lat;
                ctx->cache->meteo[i].lon = meteo_test.pInfo[i].lon;
            
                memcpy(ctx->cache->meteo[i].sample, meteo_test.pInfo[i].sample, sizeof(MeteoEntry_t) * KVARTAR_TOTALT);
                
            }
            LOG_INFO("meteo after copy: %zu meteodata entries", ctx->cache->meteo_count);
            inputcache_SaveMeteo(&meteo_test);

            ctx->updated_meteo = true;
        } else {
            LOG_ERROR("failed to read meteo data, got %zd bytes", bytesReadMeteo); //fixed size so should never trigger can still be wrong
        }
}

int inputcache_SaveSpotpris(const AllaSpotpriser *spotpris)
{
    if (!spotpris)
        return -1;

    int result = 0;
    // Finns cache-foldern?

    const char *cache_folder = "/var/cache/glennergy/spotpris";
    dir_result_t dir_res = create_folder(cache_folder);
    if (dir_res == DIR_ERROR)
    {
        LOG_ERROR("Error creating cache folder: %s", cache_folder);
        return -2;
    }

    char filename[64];
    char date_str[16];
    GetTodayDateFile(date_str, sizeof(date_str));

    for (int i = 0; i < 4; i++)
    {
        snprintf(filename, sizeof(filename), "%s/spotpris_%s_%s.json", cache_folder, spotpris->areas[i].areaname, date_str);

        json_t *root = json_array();

        for (size_t j = 0; j < spotpris->areas[i].count; j++)
        {
            json_t *obj = json_object();
            json_object_set_new(obj, "time_start", json_string(spotpris->areas[i].kvartar[j].time_start));
            //json_object_set_new(obj, "time_end", json_string(spotpris->areas[i].kvartar[j].time_end));
            json_object_set_new(obj, "SEK_per_kWh", json_real(spotpris->areas[i].kvartar[j].sek_per_kwh));
            //json_object_set_new(obj, "EUR_per_kWh", json_real(spotpris->areas[i].kvartar[j].eur_per_kwh));
            //json_object_set_new(obj, "EXR", json_real(spotpris->areas[i].kvartar[j].exchange_rate));
            json_array_append_new(root, obj);
        }

        result = json_dump_file(root, filename, JSON_INDENT(4));

        if (result < 0)
        {
            LOG_WARNING("Failed to dump save file for spotpris area: %s", spotpris->areas[i].areaname);
        }

        json_decref(root);
    }

     LOG_INFO("Saving spotpris data for %zu areas to file\n", AREA_COUNT);
    return 0;
}

void inputcache_HandleSpotprisData(InputCacheContext_t *ctx, int spotpris_fd)
{
        AllaSpotpriser spotpris_test;
        
        ssize_t bytesReadSpotpris = Pipes_ReadBinary(spotpris_fd, &spotpris_test, sizeof(AllaSpotpriser));

        
        if (bytesReadSpotpris == sizeof(AllaSpotpriser))
        {
            LOG_INFO("Got new data spotpris %zd", bytesReadSpotpris);

            for (int area = 0; area < AREA_COUNT; area++)
            {
                ctx->cache->spotpris.count[area] = spotpris_test.areas[area].count;
                
                for (size_t entry = 0; entry < spotpris_test.areas[area].count; entry++)
                {
                    strncpy(ctx->cache->spotpris.data[area][entry].time_start, spotpris_test.areas[area].kvartar[entry].time_start, sizeof(ctx->cache->spotpris.data[area][entry].time_start) - 1);
                    ctx->cache->spotpris.data[area][entry].time_start[sizeof(ctx->cache->spotpris.data[area][entry].time_start) - 1] = '\0';

                    ctx->cache->spotpris.data[area][entry].sek_per_kwh = spotpris_test.areas[area].kvartar[entry].sek_per_kwh;
                }
                
                LOG_INFO("Area %s: %zu price entries copied", area_names[area], ctx->cache->spotpris.count[area]);
            }
            
            inputcache_SaveSpotpris(&spotpris_test);
            LOG_INFO("Spotpris data updated and saved");

            ctx->updated_spotpris = true;
        } else {
            LOG_ERROR("Failed to read spotpris data, got %zd bytes", bytesReadSpotpris); //fixed size so can still be wrong
        }
}

void inputcache_SendNotification(NotifyMessageType type, uint16_t count)
{
    // Try to connect if needed
    if (notify_fd < 0) {
        notify_fd = open(NOTIFY_FIFO_PATH, O_WRONLY | O_NONBLOCK);
        if (notify_fd < 0) return;  // Algorithm not ready yet
    }
    
    NotifyMessage msg = {
        .type = type,
        .priority = 0,
        .data_count = count,
        .timestamp = (uint32_t)time(NULL)
    };
    
    if (write(notify_fd, &msg, sizeof(msg)) != sizeof(msg)) {
        // Pipe closed or error - will reconnect next time
        close(notify_fd);
        notify_fd = -1;
    }
}

void inputcache_CleanupShm(InputCacheContext_t *ctx)
{
    if (ctx && ctx->cache && ctx->shm) {
        shm_Detach(ctx->shm);
        ctx->shm = NULL;
    }
    shm_Destroy();
    LOG_INFO("Shared memory cleaned up");
}

void inputcache_CleanupAll(InputCacheContext_t *ctx)
{
    if (!ctx) return;
    
    if (notify_fd >= 0) {
        //inputcache_SendNotification(NOTIFY_SHUTDOWN, 0);
        close(notify_fd);
        notify_fd = -1;
    }
    
    if (ctx->meteo_fd >= 0) {
        close(ctx->meteo_fd);
    }
    if (ctx->spotpris_fd >= 0) {
        close(ctx->spotpris_fd);
    }
    
    if (ctx->socket_fd >= 0) {
        close(ctx->socket_fd);
        unlink(CACHE_SOCKET_PATH);
    }
    
    if (ctx->cache) {
        inputcache_CleanupShm(ctx);
        free(ctx->cache);
        ctx->cache = NULL;
    }
    
    LOG_INFO("InputCache cleanup complete");
}