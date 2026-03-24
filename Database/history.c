#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

static sqlite3 *db = NULL;

// Database schema - creates tables if they don't exist
static const char *schema_sql = 
    "CREATE TABLE IF NOT EXISTS meteo_historical ("
    "  property_id INTEGER NOT NULL,"
    "  timestamp TEXT NOT NULL,"
    "  temperature REAL,"
    "  ghi REAL,"
    "  dni REAL,"
    "  diffuse_radiation REAL,"
    "  cloud_cover REAL,"
    "  PRIMARY KEY (property_id, timestamp)"
    ") WITHOUT ROWID;"
    
    "CREATE INDEX IF NOT EXISTS idx_meteo_time ON meteo_historical(timestamp);"
    
    "CREATE TABLE IF NOT EXISTS spotpris_historical ("
    "  area TEXT NOT NULL,"
    "  timestamp TEXT NOT NULL,"
    "  sek_per_kwh REAL NOT NULL,"
    "  PRIMARY KEY (area, timestamp)"
    ") WITHOUT ROWID;"
    
    "CREATE INDEX IF NOT EXISTS idx_spotpris_time ON spotpris_historical(timestamp);";

int history_Init(const char *db_path)
{
    if (db != NULL) {
        return 0;
    }

    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);

    char *err_msg = NULL;
    sqlite3_exec(db, schema_sql, NULL, NULL, &err_msg); // sqlite3_exec "PRAGMA jorunal_mode=WAL"?  first NULL is callback, second is arg to callback, third is error message pointer
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create tables: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;
        return -1;
    }

    printf("Database initialized successfully at %s\n", db_path);
    return 0;
}

void history_Close(void)
{
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int history_ArchiveMeteoFile(const char *file_Path, int id)
{
    if (!db || !file_Path) {
        return -1;
    }
    json_error_t error;
    json_t *root = json_load_file(file_Path, 0, &error);
    if (!root) {
        fprintf(stderr, "Failed to load JSON file: %s\n", error.text);
        return -1;
    }

    json_t *minutely_15 = json_object_get(root, "minutely_15");
    if (!minutely_15) {
        fprintf(stderr, "JSON missing 'minutely_15' object\n");
        json_decref(root);
        return -1;
    }
    json_t *times = json_object_get(minutely_15, "time");
    json_t *temps = json_object_get(minutely_15, "temperature_2m");
    json_t *ghi = json_object_get(minutely_15, "shortwave_radiation");
    json_t *dni = json_object_get(minutely_15, "direct_normal_irradiance");
    json_t *diffuse = json_object_get(minutely_15, "diffuse_radiation");
    json_t *cloud_cover = json_object_get(minutely_15, "cloud_cover");
    //json_t *is_day = json_object_get(minutely_15, "is_day");

    sqlite3_stmt *statement;
    const char *insert_sql = "INSERT OR REPLACE INTO meteo_historical "
                            "(property_id, timestamp, temperature, ghi, dni, diffuse_radiation, cloud_cover) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?)";

    int resultcode = sqlite3_prepare_v2(db, insert_sql, -1, &statement, NULL);
    if (resultcode != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        json_decref(root);
        return -1;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    size_t count = json_array_size(times);
    int inserted = 0;

    for (size_t i = 0; i < count; i++)          // json timestamp from meteo "2026-03-02T03:45" add :00 or remove from
    {
        const char *timestamp = json_string_value(json_array_get(times, i));
        if (!timestamp) {
            fprintf(stderr, "Invalid timestamp at index %zu\n", i);
            continue;
        }
        double temperature = json_number_value(json_array_get(temps, i));
        double ghi_value = json_number_value(json_array_get(ghi, i));
        double dni_value = json_number_value(json_array_get(dni, i));
        double diffuse_value = json_number_value(json_array_get(diffuse, i));
        double cloud_cover_value = json_number_value(json_array_get(cloud_cover, i));

        sqlite3_bind_int(statement, 1, id);
        sqlite3_bind_text(statement, 2, timestamp, -1, SQLITE_STATIC);
        sqlite3_bind_double(statement, 3, temperature);
        sqlite3_bind_double(statement, 4, ghi_value);
        sqlite3_bind_double(statement, 5, dni_value);
        sqlite3_bind_double(statement, 6, diffuse_value);
        sqlite3_bind_double(statement, 7, cloud_cover_value);

        resultcode = sqlite3_step(statement);
        if (resultcode == SQLITE_DONE) {
            inserted++;
        } else {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_reset(statement);
    }
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    
    sqlite3_finalize(statement);
    json_decref(root);

    printf("Archived %d meteo entries for property ID %d\n", inserted, id);
    return 0;

}

int history_ArchiveSpotprisFile(const char *file_path, const char *area_name)
{
    if (!db) {
        return -1;
    }
    json_error_t error;
    json_t *root = json_load_file(file_path, 0, &error);
    if (!root) {
        fprintf(stderr, "Failed to load JSON file: %s\n", error.text);
        return -1;
    }
    if (!json_is_array(root)) {
        fprintf(stderr, "Expected JSON array in spotpris file\n");
        json_decref(root);
        return -1;
    }
    

    sqlite3_stmt *statement;
    const char *insert_sql = "INSERT OR REPLACE INTO spotpris_historical (area, timestamp, sek_per_kwh) VALUES (?, ?, ?)";
    
    int resultcode = sqlite3_prepare_v2(db, insert_sql, -1, &statement, NULL);
    if (resultcode != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        json_decref(root);
        return -1;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    
    size_t count = json_array_size(root);
    int inserted = 0;

    for (size_t i = 0; i < count; i++)
    {
        json_t *obj = json_array_get(root, i);
        const char *timestamp = json_string_value(json_object_get(obj, "time_start"));
        double sek_per_kwh = json_number_value(json_object_get(obj, "SEK_per_kWh"));

        if (!timestamp) {
            fprintf(stderr, "Invalid timestamp at index %zu\n", i);
            continue;
        }

        sqlite3_bind_text(statement, 1, area_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(statement, 2, timestamp, -1, SQLITE_STATIC);
        sqlite3_bind_double(statement, 3, sek_per_kwh);

        resultcode = sqlite3_step(statement);
        if (resultcode == SQLITE_DONE) {
            inserted++;
        } else {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_reset(statement);
    }
    
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

    sqlite3_finalize(statement);
    json_decref(root);

    printf("Archived %d spotpris entries for area %s\n", inserted, area_name);
    return 0;
}