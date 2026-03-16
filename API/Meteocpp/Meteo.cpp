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

static auto buildUrl(const std::string& lats, const std::string& lons) -> std::string
{
    return std::vformat(METEO_LINK, std::make_format_args(lats, lons));
}

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

static bool parseSamplesDirect(PropertyInfo& prop, const json& root)
{
    if (!root.contains("minutely_15") || !root["minutely_15"].is_object())
    {
        return false;
    }

    const auto &h = root["minutely_15"];
    const auto &times   = h["time"];
    const auto &temps   = h["temperature_2m"];
    const auto &ghi     = h["shortwave_radiation"];
    const auto &dni     = h["direct_normal_irradiance"];
    const auto &diffuse = h["diffuse_radiation"];
    const auto &cloud   = h["cloud_cover"];
    const auto &day     = h["is_day"];

    size_t count = std::min(temps.size(), static_cast<size_t>(KVARTAR_TOTALT));

    for (size_t j = 0; j < count; j++)
    {
        std::strncpy(prop.sample[j].time_start, times[j].get<std::string>().c_str(), sizeof(prop.sample[j].time_start) - 1);
        prop.sample[j].time_start[sizeof(prop.sample[j].time_start) - 1] = '\0';

        prop.sample[j].temp = temps[j].get<float>();
        prop.sample[j].ghi = ghi[j].get<float>();
        prop.sample[j].dni = dni[j].get<float>();
        prop.sample[j].diffuse_radiation = diffuse[j].get<float>();
        prop.sample[j].cloud_cover = cloud[j].get<float>();
        prop.sample[j].is_day = day[j].get<int>() != 0;
        prop.sample[j].valid = prop.sample[j].is_day;
    }

    std::string dumped = root.dump();
    std::strncpy(prop.raw_json_data, dumped.c_str(), RAW_DATA_MAX - 1);
    prop.raw_json_data[RAW_DATA_MAX - 1] = '\0';

    return true;
}

meteo::meteo()
{
    std::memset(&m_data, 0, sizeof(MeteoData));
}

bool meteo::load(std::string_view configPath)
{
    return loadGlennergy(m_data, configPath);
}

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
