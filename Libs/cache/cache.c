#define _POSIX_C_SOURCE 200809L
#define MODULE_NAME "CACHE"
#include "../../Server/Log/Logger.h"
#include "cache.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CACHE_SIZE (10 * 1024 * 1024)  // 10MB limit
#define MAX_FILENAME 256


int cache_Init(Cache *cache, const char *cache_dir, time_t set_ttl)
{
    if (!cache || !cache_dir)
        return -1;
    
    struct stat st;
    if (stat(cache_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        LOG_ERROR("Cache directory doesn't exist: %s", cache_dir);
        LOG_ERROR("Run 'make install' or create manually");
        return -1;
    }
    
    cache->cache_dir = strdup(cache_dir);
    if (!cache->cache_dir)
    return -1;
    
    cache->ttl = set_ttl;
    mkdir(cache->cache_dir, 0755);
    
    if (pthread_mutex_init(&cache->mutex, NULL) != 0)
    {
        free(cache->cache_dir);
        return -1;
    }
    
    LOG_INFO("Initialized: %s (TTL: %ld seconds)", cache->cache_dir, cache->ttl);
    return 0;
}

bool cache_IsValid(Cache *cache, const char *key)
{
    if (!cache || !key)
        return false;

    char filename[MAX_FILENAME];
    snprintf(filename, sizeof(filename), "%s/%s.json", cache->cache_dir, key);

    struct stat fileinfo;
    if (stat(filename, &fileinfo) != 0) // File doesn't exist
        return false;  

    time_t file_age = time(NULL) - fileinfo.st_mtime;

    if (cache->ttl > 0 && file_age > cache->ttl)
    {
        LOG_INFO("expired: %s (age: %ld sec, TTL: %ld sec)", key, file_age, cache->ttl);
        return false;
    }

    return true;
}

bool cache_Get(Cache *cache, const char *key, char **buffer, size_t *size)
{
    if (!cache || !key || !buffer || !size)
        return false;
        
    bool result = false;
    char filename[MAX_FILENAME];
    FILE *fptr = NULL;
    char *json_data = NULL;
    struct stat fileinfo;

    //pthread_mutex_lock(&cache->mutex);
    snprintf(filename, sizeof(filename), "%s/%s.json", cache->cache_dir, key);

    fptr = fopen(filename, "rb");
    if (!fptr) {
        //pthread_mutex_unlock(&cache->mutex);
        LOG_ERROR("fopen() failed for %s", filename);
        return false;  // Quick exit - no cleanup needed
    }
    
    if (fstat(fileno(fptr), &fileinfo) != 0) {
        LOG_ERROR("fstat() failed for %s", filename);
        goto cleanup;
    }

    time_t file_age = time(NULL) - fileinfo.st_mtime;
    if (cache->ttl > 0 && file_age > cache->ttl) {
        LOG_INFO("Expired: %s", key);
        goto cleanup;
    }

    size_t filesize = fileinfo.st_size;
    if (filesize > MAX_CACHE_SIZE) {
        LOG_ERROR("file to large: %s (size: %zu bytes)", filename, filesize);
        goto cleanup;
    }

    json_data = malloc((filesize + 1) * sizeof(char));
    if (!json_data) {
        LOG_ERROR("malloc() failed for %s", filename);
        goto cleanup;
    }


    size_t bytes_read = fread(json_data, sizeof(char), filesize, fptr);
    if (bytes_read != filesize) {
        LOG_ERROR("fread() failed for %s", filename);
        free(json_data);
        goto cleanup;
    }
    json_data[bytes_read] = '\0';

    *buffer = json_data;  // Caller must free()
    if (size) *size = bytes_read;

    result = true;

cleanup:
    fclose(fptr);
    //pthread_mutex_unlock(&cache->mutex);
    if(result)
        LOG_INFO("Loaded: %s (size: %zu bytes, age: %ld sec)", key, bytes_read, file_age);
    return result;
}

int cache_Set(Cache *cache, const char *key, const char *data, size_t size)
{
    if (!cache || !key || !data)
        return -1;

    char filename[MAX_FILENAME];
    char temp_filename[MAX_FILENAME + 8];
    //pthread_mutex_lock(&cache->mutex);
    snprintf(filename, sizeof(filename), "%s/%s.json", cache->cache_dir, key);
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE *fptr = fopen(temp_filename, "w");
    if (!fptr)
    {
        LOG_ERROR("fopen() failed for %s", temp_filename);
        //pthread_mutex_unlock(&cache->mutex);
        return -1;
    }

    
    size_t written = fwrite(data, sizeof(char), size, fptr);
    fclose(fptr);
    
    if (written != size) {
        LOG_ERROR("fwrite() incomplete: %zu/%zu bytes written to %s", written, size, temp_filename);
        unlink(temp_filename);
        return -1;
    }

    if (rename(temp_filename, filename) != 0) {
        LOG_ERROR("rename() failed from %s to %s", temp_filename, filename);
        unlink(temp_filename);
        return -1;
    }
    //pthread_mutex_unlock(&cache->mutex);
    LOG_INFO("Saved: %s (size:%zu)", filename, size);
    return 0;
}

void cache_Dispose(Cache *cache)
{
    if (!cache)
        return;

    LOG_INFO("Dispose() cache: %s", cache->cache_dir ? cache->cache_dir : "NULL");
    free(cache->cache_dir);
    cache->cache_dir = NULL;
    pthread_mutex_destroy(&cache->mutex);
    
}