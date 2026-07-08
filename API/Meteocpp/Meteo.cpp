/**
 * @file Meteo.cpp
 * @brief Implementation of Meteo C++ module.
 *
 * @ingroup MeteoCppModule
 */

// #define MODULE_NAME "METEO"

// #include "Logger.hpp"
#include "fetcher.hpp"
#include "Meteo.hpp"
#include "json.hpp"
#include <fstream>
#include <algorithm>
#include <format>
#include <cstring>
#include <iostream>

namespace meteocpp {

using json = nlohmann::json;

/**
 * @brief Build Open-Meteo API URL.
 *
 * @param[in] lats Comma-separated latitudes
 * @param[in] lons Comma-separated longitudes
 * @return Formatted URL string
 */
static auto buildUrl(const std::string& lats, const std::string& lons) -> std::string
{
    return std::vformat(METEO_LINK, std::make_format_args(lats, lons));
}

/**
 * @brief Fetch JSON data from API.
 *
 * @param[in] lats Latitude string
 * @param[in] lons Longitude string
 * @return Optional JSON string
 *
 * @note Internal helper
 */
std::optional<std::string> fetchJson(const std::string& lats, const std::string& lons)
{
    auto url = buildUrl(lats, lons);
    // LOG_INFO("URL: %s\n", url.c_str());

    glennergy::fetcher http_client;
    auto result = http_client.get(url);
    if (result.empty())
    {
        // LOG_WARNING("HTTP fetch failed\n");
        return std::nullopt;
    }

    return result;
}

/**
 * @brief Load configuration from file.
 */
static bool loadGlennergy(MeteoData& data, std::string_view path)
{
    std::ifstream file{std::string{path}};
    if (!file.is_open())
    {
        return false;
    }

    json root;
    try {
        root = json::parse(file);
    } catch (const json::parse_error &e) {
        return false;
    }

    if (!root.contains("systems") || !root["systems"].is_array())
    {
        return false;
    }

    const auto &systems = root["systems"];

    data.pCount = 0;
    for (const auto &obj : systems)
    {
        if (data.pCount >= PROPERTIES_MAX)
        {
            break;
        }

        if (!obj.is_object() ||
            !obj.contains("id") || !obj["id"].is_number_integer() ||
            !obj.contains("city") || !obj["city"].is_string() ||
            !obj.contains("lat") || !obj["lat"].is_number() ||
            !obj.contains("lon") || !obj["lon"].is_number() ||
            !obj.contains("electricity_area") || !obj["electricity_area"].is_string())
            continue;

        auto& prop = data.pInfo[data.pCount];
        prop.id = obj["id"].get<int>();
        std::strncpy(prop.property_name, obj["city"].get<std::string>().c_str(), METEO_NAME_MAX - 1);
        prop.property_name[METEO_NAME_MAX - 1] = '\0';
        prop.lat = obj["lat"].get<double>();
        prop.lon = obj["lon"].get<double>();
        std::strncpy(prop.electricity_area, obj["electricity_area"].get<std::string>().c_str(), 4);
        prop.electricity_area[4] = '\0';

        data.pCount++;
    }

    return true;
}

/**
 * @brief Parse weather samples from JSON.
 */
static bool parseSamplesDirect(PropertyInfo& prop, const json& root)
{
    if (!root.contains("minutely_15") || !root["minutely_15"].is_object())
    {
        return false;
    }

    const auto &h = root["minutely_15"];
    const auto &times   = h["time"];
    const auto &temps   = h["temperature_2m"];
    const auto &diffuse = h["diffuse_radiation"];
    const auto &cloud   = h["cloud_cover"];
    const auto &day     = h["is_day"];
    const auto &weather = h["weather_code"];
    const auto &uv_idx  = h["uv_index"];

    size_t count = std::min(temps.size(), static_cast<size_t>(KVARTAR_TOTALT));

    std::cout << "sizes: "
          << times.size() << " "
          << temps.size() << " "
          << weather.size() << " "
          << uv_idx.size() << "\n";

    // Suggestion: Validate all arrays have equal size before accessing

    for (size_t j = 0; j < count; j++)
    {
        std::strncpy(prop.sample[j].time_start, times[j].get<std::string>().c_str(), sizeof(prop.sample[j].time_start) - 1);
        prop.sample[j].time_start[sizeof(prop.sample[j].time_start) - 1] = '\0';

        prop.sample[j].temp = temps[j].get<float>();
        prop.sample[j].diffuse_radiation = diffuse[j].get<float>();
        prop.sample[j].cloud_cover = cloud[j].get<float>();
        prop.sample[j].is_day = day[j].get<int>() != 0;
        prop.sample[j].valid = prop.sample[j].is_day;
        prop.sample[j].weather_code = weather[j].get<int>();
        prop.sample[j].uv_index = uv_idx[j].get<int>();
    }

    std::string dumped = root.dump();
    std::strncpy(prop.raw_json_data, dumped.c_str(), RAW_DATA_MAX - 1);
    prop.raw_json_data[RAW_DATA_MAX - 1] = '\0';

    return true;
}

/**
 * @brief Constructor.
 */
meteo::meteo()
{
    std::memset(&m_data, 0, sizeof(MeteoData));
}

/**
 * @brief Load configuration.
 */
bool meteo::load(std::string_view configPath)
{
    return loadGlennergy(m_data, configPath);
}

/**
 * @brief Fetch all weather data.
 */
bool meteo::fetchAll()
{
    if (m_data.pCount == 0) return true;

    std::string lats = "";
    std::string lons = "";

    for (size_t i = 0; i < m_data.pCount; i++)
    {
        if (i > 0) {
            lats += ",";
            lons += ",";
        }
        lats += std::format("{:.2f}", m_data.pInfo[i].lat);
        lons += std::format("{:.2f}", m_data.pInfo[i].lon);
    }

    auto rawJson = fetchJson(lats, lons);
    if (!rawJson)
        return false;

    json rootArray;
    try {
        rootArray = json::parse(*rawJson);
    } catch (const json::parse_error &e) {
        return false;
    }

    if (rootArray.is_array()) {
        size_t count = std::min(static_cast<size_t>(rootArray.size()), m_data.pCount);
        for (size_t i = 0; i < count; i++) {
            parseSamplesDirect(m_data.pInfo[i], rootArray[i]);
        }
    } else if (rootArray.is_object() && m_data.pCount == 1) {
        parseSamplesDirect(m_data.pInfo[0], rootArray);
    } else {
        return false;
    }

    return true;
}

} // namespace meteocpp