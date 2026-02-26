#define MODULE_NAME "METEO"

extern "C" {
#include "../../Server/Log/Logger.h"
#include "../../Libs/Fetcher.h"
}

#include "Meteo.hpp"
#include "../../Libs/json.hpp"
#include <fstream>
#include <algorithm>
#include <format>
#include <numeric>

namespace meteocpp {

using json = nlohmann::json;

static auto parseProperty(const json &obj) -> std::optional<PropertyInfo>
{
    if (!obj.is_object() ||
        !obj.contains("id") || !obj.contains("city") ||
        !obj.contains("lat") || !obj.contains("lon"))
        return std::nullopt;

    return PropertyInfo{
        .id            = obj["id"].get<int>(),
        .property_name = obj["city"].get<std::string>(),
        .lat           = obj["lat"].get<double>(),
        .lon           = obj["lon"].get<double>(),
    };
}

static auto parseSample(const json &time, const json &temp,
                        const json &ghi, const json &dni,
                        const json &diffuse, const json &cloud,
                        const json &day) -> Sample
{
    bool isDay = day.get<int>() != 0;
    return Sample{
        .time_start        = time.get<std::string>(),
        .temp              = temp.get<float>(),
        .ghi               = ghi.get<float>(),
        .dni               = dni.get<float>(),
        .diffuse_radiation = diffuse.get<float>(),
        .cloud_cover       = cloud.get<float>(),
        .is_day            = isDay,
        .valid             = isDay,
    };
}

static auto buildUrl(double lat, double lon) -> std::string
{
    return std::vformat(METEO_LINK, std::make_format_args(lat, lon));
}

std::optional<std::string> fetchJson(double lat, double lon)
{
    CurlResponse response;
    if (Curl_Initialize(&response) < 0)
    {
        LOG_WARNING("Curl initialization failed\n");
        return std::nullopt;
    }

    auto url = buildUrl(lat, lon);
    LOG_INFO("URL: %s\n", url.c_str());

    int result = Curl_HTTPGet(&response, url.data());
    if (result != 0 || response.data == nullptr)
    {
        LOG_WARNING("HTTP fetch failed with code %d\n", result);
        Curl_Dispose(&response);
        return std::nullopt;
    }

    std::string data(response.data);
    Curl_Dispose(&response);
    return data;
}

std::optional<std::vector<PropertyInfo>> loadGlennergy(std::string_view path)
{
    std::ifstream file{std::string{path}};
    if (!file.is_open())
    {
        LOG_ERROR("failed to load file: %s\n", std::string(path).c_str());
        return std::nullopt;
    }

    json root;
    try {
        root = json::parse(file);
    } catch (const json::parse_error &e) {
        LOG_ERROR("JSON parse failed: %s\n", e.what());
        return std::nullopt;
    }

    if (!root.contains("systems") || !root["systems"].is_array())
    {
        LOG_ERROR("'systems' is not an array!\n");
        return std::nullopt;
    }

    const auto &systems = root["systems"];
    std::vector<PropertyInfo> properties;

    for (const auto &obj : systems)
    {
        if (properties.size() >= PROPERTIES_MAX)
            break;

        auto prop = parseProperty(obj);
        if (prop)
        {
            LOG_INFO("Loaded property: %s (ID: %d, Lat: %.2f, Lon: %.2f)\n",
                     prop->property_name.c_str(), prop->id, prop->lat, prop->lon);
            properties.push_back(std::move(*prop));
        }
    }

    return properties;
}

std::optional<std::vector<Sample>> parseSamples(std::string_view jsonRaw)
{
    json root;
    try {
        root = json::parse(jsonRaw);
    } catch (const json::parse_error &e) {
        LOG_ERROR("JSON parse failed: %s\n", e.what());
        return std::nullopt;
    }

    if (!root.contains("minutely_15") || !root["minutely_15"].is_object())
    {
        LOG_ERROR("JSON missing 'minutely_15' object\n");
        return std::nullopt;
    }

    const auto &h = root["minutely_15"];
    const auto &times   = h["time"];
    const auto &temps   = h["temperature_2m"];
    const auto &ghi     = h["shortwave_radiation"];
    const auto &dni     = h["direct_normal_irradiance"];
    const auto &diffuse = h["diffuse_radiation"];
    const auto &cloud   = h["cloud_cover"];
    const auto &day     = h["is_day"];

    size_t count = std::min(temps.size(), KVARTAR_TOTALT);
    std::vector<Sample> samples;
    samples.reserve(count);

    for (size_t j = 0; j < count; j++)
        samples.push_back(parseSample(times[j], temps[j], ghi[j], dni[j],
                                      diffuse[j], cloud[j], day[j]));

    return samples;
}


bool meteo::load(std::string_view configPath)
{
    auto result = loadGlennergy(configPath);
    if (!result)
        return false;

    m_properties = std::move(*result);
    return true;
}

bool meteo::fetchAll()
{
    return std::all_of(m_properties.begin(), m_properties.end(),
        [](PropertyInfo &prop) -> bool
        {
            auto rawJson = fetchJson(prop.lat, prop.lon);
            if (!rawJson)
                return false;

            auto samples = parseSamples(*rawJson);
            if (!samples)
                return false;

            size_t count = std::min(samples->size(), KVARTAR_TOTALT);
            std::copy_n(samples->begin(), count, prop.samples.begin());

            prop.raw_json_data = std::move(*rawJson);
            return true;
        });
}

} // namespace meteocpp