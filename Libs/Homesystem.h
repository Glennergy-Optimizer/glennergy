#ifndef HOMESYSTEM_H
#define HOMESYSTEM_H


#define NAME_MAX 128
#define AREA_MAX 4

#include <stddef.h>

typedef struct {
    int id;
    char city[NAME_MAX];
    double panel_capacitykwh;           // Total installed solar capacity (kWh)
    double panel_tiltdegrees;           // Panel tilt angle (0-90 degrees)
    double panel_azimuthdegrees;        // Panel orientation (0=North, 90=East, 180=South, 270=West)
    
    double lat;
    double lon;
    char electricity_area[AREA_MAX];            // Swedish price area: "SE1", "SE2", "SE3", "SE4"

} Homesystem_t;

Homesystem_t* homesystem_Create(Homesystem_t *hs);
void homesystem_Destroy(Homesystem_t **hs);

int homesystem_LoadAll(Homesystem_t **systems, const char* file_path);

int homesystem_LoadAllCount(Homesystem_t *systems, const char* file_path, size_t max_count);

int homesystem_Append(const char *json_data, const char* config_path);
void homesystem_update(const char *json_data, int home_id, const char* config_path);



#endif // HOMESYSTEM_H
