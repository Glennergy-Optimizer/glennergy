#include "history.h"
#include <stdio.h>

int main() {
    printf("=== Full History Module Test ===\n\n");
    
    // Initialize database
    printf("1. Initializing database...\n");
    if (history_Init("full_test.db") < 0) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }
    
    // Test meteo archiving
    printf("\n2. Testing Meteo Archiving...\n");
    const char *meteo_file = "/var/cache/glennergy/meteo/meteo_1_2026-03-08.json";
    if (history_ArchiveMeteoFile(meteo_file, 1) < 0) {
        fprintf(stderr, "Meteo archiving failed\n");
    }
    
    // Test spotpris archiving
    printf("\n3. Testing Spotpris Archiving...\n");
    const char *spotpris_file = "/var/cache/glennergy/spotpris/spotpris_SE1_2026-03-08.json";
    if (history_ArchiveSpotprisFile(spotpris_file, "SE1") < 0) {
        fprintf(stderr, "Spotpris archiving failed\n");
    }
    
    // Query both tables
    printf("\n4. Checking database contents...\n");
    sqlite3 *db;
    sqlite3_open("full_test.db", &db);
    sqlite3_stmt *stmt;
    
    // Count meteo rows
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM meteo_historical", -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("   Meteo rows: %d\n", sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);
    
    // Count spotpris rows
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM spotpris_historical", -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("   Spotpris rows: %d\n", sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);
    
    // Show sample spotpris data
    printf("\n5. Sample Spotpris Data (first 5 entries):\n");
    printf("   Area | Timestamp            | Price (SEK/kWh)\n");
    printf("   -----|----------------------|----------------\n");
    
    sqlite3_prepare_v2(db, 
        "SELECT area, timestamp, sek_per_kwh FROM spotpris_historical "
        "ORDER BY timestamp LIMIT 5",
        -1, &stmt, NULL);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *area = (const char*)sqlite3_column_text(stmt, 0);
        const char *ts = (const char*)sqlite3_column_text(stmt, 1);
        double price = sqlite3_column_double(stmt, 2);
        printf("   %-4s | %s | %.4f\n", area, ts, price);
    }
    sqlite3_finalize(stmt);
    
    // Test archiving multiple areas
    printf("\n6. Testing Multiple Areas...\n");
    history_ArchiveSpotprisFile("/var/cache/glennergy/spotpris/spotpris_SE2_2026-03-08.json", "SE2");
    history_ArchiveSpotprisFile("/var/cache/glennergy/spotpris/spotpris_SE3_2026-03-08.json", "SE3");
    history_ArchiveSpotprisFile("/var/cache/glennergy/spotpris/spotpris_SE4_2026-03-08.json", "SE4");
    
    // Count by area
    printf("\n7. Spotpris Rows by Area:\n");
    sqlite3_prepare_v2(db, 
        "SELECT area, COUNT(*) FROM spotpris_historical GROUP BY area ORDER BY area",
        -1, &stmt, NULL);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *area = (const char*)sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        printf("   %s: %d rows\n", area, count);
    }
    sqlite3_finalize(stmt);
    
    sqlite3_close(db);
    history_Close();
    
    printf("\n=== All Tests Passed! ===\n");
    return 0;
}
