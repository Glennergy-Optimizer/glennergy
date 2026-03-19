/**
 * @file Homesystem.c
 * @brief Implementation of homesystem management functions.
 *
 * Provides APIs for loading homesystem configurations from JSON,
 * creating and destroying Homesystem_t structures.
 * Placeholder functions for append/update are included as commented code.
 * 
 * Uses Jansson library for JSON parsing.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#include "Homesystem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

/**
 * @brief Load all homesystems from a JSON config file.
 *
 * Allocates an array of Homesystem_t and fills it from the JSON array
 * stored under key "systems".
 *
 * @param systems Pointer to store allocated array
 * @param file_path Path to JSON config
 * @return Number of homesystems loaded, -1 on error
 */
int homesystem_LoadAll(Homesystem_t **systems, const char* file_path)
{
    if (systems == NULL || file_path == NULL)
        return -1;

    json_error_t err;
    json_t *root = json_load_file(file_path, 0, &err);
    if (root == NULL)
    {
        fprintf(stderr, "Failed to load homesystem config from %s: %s\n", file_path, err.text);
        return -1;
    }

    json_t *systems_array = json_object_get(root, "systems");
    if (!json_is_array(systems_array))
    {
        fprintf(stderr, "Invalid config format: 'systems' should be an array\n");
        json_decref(root);
        return -1;
    }

    size_t count = json_array_size(systems_array);
    *systems = malloc(count * sizeof(Homesystem_t));
    if (*systems == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        json_decref(root);
        return -1;
    }

    for (size_t i = 0; i < count; i++)
    {
        json_t *item = json_array_get(systems_array, i);
        const char *city, *electricity_area;

        int result = json_unpack(item, "{s:i, s:s, s:F, s:F, s:F, s:F, s:F, s:s}",
                                "id",                   &(*systems)[i].id,
                                "city",                 &city,
                                "lat",                  &(*systems)[i].lat,
                                "lon",                  &(*systems)[i].lon,
                                "panel_capacitykwh",     &(*systems)[i].panel_capacitykwh,
                                "panel_tiltdegrees",    &(*systems)[i].panel_tiltdegrees,
                                "panel_azimuthdegrees", &(*systems)[i].panel_azimuthdegrees,
                                "electricity_area",     &electricity_area);

        if (result == 0)
        {
            strncpy((*systems)[i].city, city, NAME_MAX - 1);
            (*systems)[i].city[NAME_MAX - 1] = '\0';

            strncpy((*systems)[i].electricity_area, electricity_area, AREA_MAX - 1);
            (*systems)[i].electricity_area[AREA_MAX - 1] = '\0';
        }
        else
        {
            fprintf(stderr, "Failed to parse system %zu\n", i);
        }
    }

    printf("Loaded %zu homesystems\n", count);
    json_decref(root);
    return (int)count;
}

/**
 * @brief Load homesystems from JSON config into a pre-allocated array up to max_count.
 *
 * @param systems Pre-allocated array
 * @param config_path Path to JSON config
 * @param max_count Maximum number of systems to load
 * @return Number of homesystems loaded, -1 on error
 */
int homesystem_LoadAllCount(Homesystem_t *systems, const char* config_path, size_t max_count)
{
    if (!systems || !config_path)
        return -1;

    json_error_t err;
    json_t *root = json_load_file(config_path, 0, &err);
    if (!root)
    {
        fprintf(stderr, "Failed to load homesystem config: %s\n", err.text);
        return -1;
    }

    json_t *systems_array = json_object_get(root, "systems");
    if (!json_is_array(systems_array))
    {
        fprintf(stderr, "Invalid config format\n");
        json_decref(root);
        return -1;
    }

    size_t count = json_array_size(systems_array);
    if (count > max_count)
        count = max_count;

    for (size_t i = 0; i < count; i++)
    {
        json_t *item = json_array_get(systems_array, i);
        const char *city, *electricity_area;

        int result = json_unpack(item, "{s:i, s:s, s:F, s:F, s:F, s:F, s:F, s:s}",
                                "id", &systems[i].id,
                                "city", &city,
                                "lat", &systems[i].lat,
                                "lon", &systems[i].lon,
                                "panel_capacitykwh", &systems[i].panel_capacitykwh,
                                "panel_tiltdegrees", &systems[i].panel_tiltdegrees,
                                "panel_azimuthdegrees", &systems[i].panel_azimuthdegrees,
                                "electricity_area", &electricity_area);

        if (result == 0)
        {
            strncpy(systems[i].city, city, NAME_MAX - 1);
            systems[i].city[NAME_MAX - 1] = '\0';

            strncpy(systems[i].electricity_area, electricity_area, AREA_MAX - 1);
            systems[i].electricity_area[AREA_MAX - 1] = '\0';
        }
        else
        {
            fprintf(stderr, "Failed to parse system %zu\n", i);
        }
    }

    json_decref(root);
    printf("Loaded %zu homesystems into array\n", count);
    return (int)count;
}

/*
 * @brief Append a new homesystem to the JSON config.
 *
 * Placeholder / commented out for future implementation.
 *
 * @param json_data JSON string representing the new homesystem
 * @param file_path Path to the JSON config file
 * @return 0 on success, -1 on error
 */
// int homesystem_Append(const char *json_data, const char* file_path) { ... }

/*
 * @brief Update an existing homesystem in the JSON config.
 *
 * Placeholder / commented out for future implementation.
 *
 * @param json_data JSON string with updated data
 * @param home_id ID of the homesystem to update
 * @param config_path Path to the JSON config file
 */
// void homesystem_update(const char *json_data, int home_id, const char* config_path) { ... }