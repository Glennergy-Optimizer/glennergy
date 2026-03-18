#include "Homesystem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>



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


// int homesystem_Append(const char *json_data, const char* file_path)
// {
//     if (!json_data || !file_path) return -1;
    
//     json_error_t err;
//     json_t *new_obj = json_loads(json_data, 0, &err);
//     if (!new_obj) {
//         fprintf(stderr, "Invalid JSON: %s\n", err.text);
//         return -1;
//     }

//     json_t *root = json_load_file(file_path, 0, &err);
//     if (!root || !json_is_array(root)) {
//         root = json_array();  // Create new if doesn't exist
//     }
    

//     json_array_append_new(root, new_obj);
//     int ret = json_dump_file(root, file_path, JSON_INDENT(2));
//     json_decref(root);
    
//     printf("Appended system to config\n");
//     return ret;
// }

// void homesystem_update(const char *json_data, int home_id, const char* file_path)
// {
//     if (json_data == NULL || file_path == NULL)
//         return;

//     json_t *root = json_object();

//     json_object_set_new(root, "home_id", json_string(hs->home_id));
//     json_object_set_new(root, "solar_panel_capacity_kw", json_real(hs->panel_capacitykw));
//     json_object_set_new(root, "panel_tilt_degrees", json_real(hs->panel_tiltdegrees));
//     json_object_set_new(root, "panel_azimuth_degrees", json_real(hs->panel_azimuthdegrees));
//     json_object_set_new(root, "lat", json_real(hs->lat));
//     json_object_set_new(root, "lon", json_real(hs->lon));
//     json_object_set_new(root, "electricity_area", json_string(hs->electricity_area));

//     if (json_dump_file(root, file_path, JSON_INDENT(2)) != 0)
//     {
//         fprintf(stderr, "Failed to save homesystem config to %s\n", file_path);
//     }

//     json_decref(root);
// }