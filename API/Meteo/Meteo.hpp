#ifndef METEO_HPP
#define METEO_HPP

#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <array>
#include <optional>
#include <cstddef>

namespace meteocpp {

constexpr size_t KVARTAR_TOTALT = 128;
constexpr size_t PROPERTIES_MAX = 5;

constexpr std::string_view METEO_LINK =
    "https://api.open-meteo.com/v1/forecast?"
    "latitude={:.2f}&longitude={:.2f}"
    "&minutely_15=temperature_2m,shortwave_radiation,direct_normal_irradiance,"
    "diffuse_radiation,cloud_cover,is_day"
    "&forecast_days=3&forecast_minutely_15=128&timezone=Europe/Stockholm";

struct Sample {
    std::string time_start;
    float temp              = 0.0f;
    float ghi               = 0.0f;
    float dni               = 0.0f;
    float diffuse_radiation = 0.0f;
    float cloud_cover       = 0.0f;
    bool is_day             = false;
    bool valid              = false;
};

struct PropertyInfo {
    int id = -1;
    std::string property_name;
    double lat = 0.0;
    double lon = 0.0;
    std::array<Sample, KVARTAR_TOTALT> samples{};
    std::string raw_json_data;
};

[[nodiscard]] std::optional<std::vector<PropertyInfo>> loadGlennergy(std::string_view path);
[[nodiscard]] std::optional<std::vector<Sample>>       parseSamples(std::string_view jsonRaw);
[[nodiscard]] std::optional<std::string>               fetchJson(double lat, double lon);

class meteo {
public:
    meteo() = default;

    bool load(std::string_view configPath = "/etc/Glennergy-Fastigheter.json");
    bool fetchAll();

    const std::vector<PropertyInfo> &properties() const { return m_properties; }
    size_t propertyCount() const                        { return m_properties.size(); }

private:
    std::vector<PropertyInfo> m_properties;
};

} // namespace meteocpp

#endif // METEO_HPP
