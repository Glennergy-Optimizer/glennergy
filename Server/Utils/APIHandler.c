// #define MODULE_NAME "APIHANDLER"
// #include "../Log/Logger.h"
// #include "APIHandler.h"
// #include <jansson.h>
// #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>

// int APIHandler_Init(APIHandler_t **ctx)
// {
//     APIHandler_t *api = malloc(sizeof(APIHandler_t));
//     if (api == NULL) {
//         LOG_ERROR("Failed to allocate memory for APIHandler");
//         return -1;
//     }
//     // Retry attaching to shared memory (InputCache creates it)
//     LOG_INFO("Waiting for shared memory to be created by InputCache...");
//     for (int retry = 0; retry < 10; retry++) {
//         api->shm = shm_Attach();
//         if (api->shm != NULL) {
//             *ctx = api;
//             LOG_INFO("APIHandler initialized, attached to shared memory");
//             return 0;
//         }
        
//         if (retry < 9) {
//             LOG_DEBUG("Shared memory not ready, retry %d/10 in 500ms...", retry + 1);
//             usleep(500000); // Wait 500ms
//         }
//     }
//     LOG_ERROR("Failed to attach to shared memory after 10 retries (5 seconds)");
//     free(api);
//     return -1;
// }

// static char* APIHandler_ErrorResponse(const char *message)
// {
//     json_t *root = json_object();
//     json_object_set_new(root, "error", json_string(message));
    
//     char *result = json_dumps(root, JSON_COMPACT);
//     json_decref(root);
//     return result;
// }

// static const char* strategy_to_string(EnergyStrategy_t strategy)
// {
//     switch (strategy) {
//         case STRATEGY_USE_SOLAR: return "USE_SOLAR";
//         case STRATEGY_USE_GRID_CHEAP: return "USE_GRID_CHEAP";
//         case STRATEGY_AVOID_GRID: return "AVOID_GRID";
//         case STRATEGY_EXCESS_SOLAR: return "EXCESS_SOLAR";
//         default: return "UNKNOWN";
//     }
// }

// static char* APIHandler_GetResult(APIHandler_t *ctx, int home_id)
// {
//     if (!ctx->shm) {
//         LOG_ERROR("Shared memory not available");
//         return APIHandler_ErrorResponse("Shared memory not available");
//     }

//     AlgorithmResults_t *res;
//     AlgorithmResults_t result_copy;

//     shm_Lock_Read(ctx->shm);

//     res = shm_GetResult(ctx->shm, home_id);
//     if (!res || !res->valid) {
//         shm_Unlock_Read(ctx->shm);
//         LOG_WARNING("No valid results found for home_id %d", home_id);
//         return APIHandler_ErrorResponse("No valid results found for the specified home_id");
//     }
//     memcpy(&result_copy, res, sizeof(AlgorithmResults_t));

//     shm_Unlock_Read(ctx->shm);

//     json_t *result = json_object();
//     json_object_set_new(result, "home_id", json_integer(result_copy.home_id));
//     json_object_set_new(result, "last_calculated", json_integer(result_copy.last_calculated));
//     json_object_set_new(result, "total_solar_kwh", json_real(result_copy.total_solar_kwh));
//     json_object_set_new(result, "avg_grid_price", json_real(result_copy.avg_grid_price));
//     json_object_set_new(result, "peak_solar_slot", json_integer(result_copy.peak_solar_slot));
//     json_object_set_new(result, "cheapest_grid_slot", json_integer(result_copy.cheapest_grid_slot));
//     json_object_set_new(result, "most_expensive_slot", json_integer(result_copy.most_expensive_slot));

//     json_t *slots = json_array();
//     for (int i = 0; i < 96; i++) {
//         json_t *slot = json_object();
//         json_object_set_new(slot, "timestamp", json_string(result_copy.slots[i].timestamp));
//         json_object_set_new(slot, "solar_kwh", json_real(result_copy.slots[i].solar_kwh));
//         json_object_set_new(slot, "grid_price", json_real(result_copy.slots[i].grid_price));
//         json_object_set_new(slot, "strategy", json_string(strategy_to_string(result_copy.slots[i].strategy)));
//         json_array_append_new(slots, slot);
//     }
//     json_object_set_new(result, "slots", slots);

//     char *response = json_dumps(result, JSON_INDENT(2) | JSON_REAL_PRECISION(4));
//     json_decref(result);
//     return response;
// }

// int APIHandler_HandleRequest(APIHandler_t *ctx, HTTPRequest_t *req, HTTPResponse_t *resp)
// {
//     if (ctx->shm == NULL) {
//         LOG_ERROR("Shared memory not attached");
//         resp->status_code = 500;
//         resp->body = APIHandler_ErrorResponse("Service unavailable");
//         return -1;
//     }
//     LOG_INFO("API Request: %s %s", req->method == HTTP_METHOD_GET ? "GET" : req->method == HTTP_METHOD_POST ? "POST" : "OPTIONS", req->path);

//     int home_id;
//     char trailing;

//     if (sscanf(req->path, "/api/results/%d%c", &home_id, &trailing) == 1) {
//         if (home_id > 0) {
//             resp->status_code = 200;
//             resp->body = APIHandler_GetResult(ctx, home_id);
//             if (resp->body == NULL) {
//                 resp->status_code = 500;
//                 resp->body = APIHandler_ErrorResponse("Failed to retrieve results");
//                 return 0;
//             } else {
//                 LOG_INFO("Successfully retrieved results for home_id %d", home_id);
//                 return 0;    
//             }
//         }
//     }
//     resp->status_code = 404;
//     resp->body = APIHandler_ErrorResponse("Not found");
//     return 0;
// }
// void APIHandler_Dispose(APIHandler_t **ctx)
// {
//     if (ctx == NULL || *ctx == NULL) {
//         return;
//     }
//     APIHandler_t *api = *ctx;

//     if (api->shm != NULL) {
//         shm_Detach(api->shm);
//         api->shm = NULL;
//         LOG_INFO("APIHandler detached from shared memory");
//     }

//     free(api);
//     *ctx = NULL;
// }