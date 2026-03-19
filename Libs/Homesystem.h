/**
 * @file Homesystem.h
 * @brief Data structures and functions for managing homesystem configurations.
 * 
 * Defines the Homesystem_t structure and functions to create, destroy,
 * load, append, and update homesystem configurations stored in JSON files.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#ifndef HOMESYSTEM_H
#define HOMESYSTEM_H

#include <stddef.h>

#define NAME_MAX 128
#define AREA_MAX 4

/**
 * @brief Represents a homesystem configuration.
 */
typedef struct {
    int id;                                /**< Unique identifier for the home system */
    char city[NAME_MAX];                   /**< City name */
    double panel_capacitykwh;              /**< Total installed solar capacity (kWh) */
    double panel_tiltdegrees;              /**< Panel tilt angle (0-90 degrees) */
    double panel_azimuthdegrees;           /**< Panel orientation (0=North, 90=East, 180=South, 270=West) */
    double lat;                            /**< Latitude */
    double lon;                            /**< Longitude */
    char electricity_area[AREA_MAX];       /**< Swedish electricity price area ("SE1"-"SE4") */
} Homesystem_t;

/**
 * @brief Allocate and initialize a Homesystem_t structure.
 *
 * @param hs Pointer to pre-allocated Homesystem_t (optional, can be NULL)
 * @return Pointer to initialized Homesystem_t
 */
Homesystem_t* homesystem_Create(Homesystem_t *hs);

/**
 * @brief Free a Homesystem_t structure.
 *
 * @param hs Pointer to pointer to Homesystem_t to destroy
 */
void homesystem_Destroy(Homesystem_t **hs);

/**
 * @brief Load all homesystems from a JSON config file.
 *
 * @param systems Pointer to store allocated array of Homesystem_t
 * @param file_path Path to JSON config file
 * @return Number of homesystems loaded, or -1 on error
 */
int homesystem_LoadAll(Homesystem_t **systems, const char* file_path);

/**
 * @brief Load homesystems from a JSON file up to a maximum count.
 *
 * @param systems Pre-allocated array to load homesystems into
 * @param file_path Path to JSON config file
 * @param max_count Maximum number of homesystems to load
 * @return Number of homesystems loaded, or -1 on error
 */
int homesystem_LoadAllCount(Homesystem_t *systems, const char* file_path, size_t max_count);

/**
 * @brief Append a new homesystem to the JSON config file.
 *
 * Currently commented out; placeholder for future use.
 *
 * @param json_data JSON string representing the new homesystem
 * @param config_path Path to the config file
 * @return 0 on success, -1 on error
 */
// int homesystem_Append(const char *json_data, const char* config_path);

/**
 * @brief Update an existing homesystem in the JSON config file.
 *
 * Currently commented out; placeholder for future use.
 *
 * @param json_data JSON string with updated homesystem data
 * @param home_id ID of the homesystem to update
 * @param config_path Path to the config file
 */
// void homesystem_update(const char *json_data, int home_id, const char* config_path);

#endif // HOMESYSTEM_H