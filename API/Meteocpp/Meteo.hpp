#ifndef METEO_HPP
#define METEO_HPP

#include <string_view>
#include <optional>
#include "meteo_types.hpp"

namespace meteocpp {

constexpr std::string_view METEO_LINK =
    "https://api.open-meteo.com/v1/forecast?"
    "latitude={}&longitude={}"
    "&minutely_15=temperature_2m,shortwave_radiation,direct_normal_irradiance,"
    "diffuse_radiation,cloud_cover,is_day"
    "&forecast_days=3&forecast_minutely_15=128&timezone=Europe/Stockholm";

class meteo {
public:
    meteo();

    bool load(std::string_view configPath = "Glennergy-Fastigheter.json");
    bool fetchAll();

    const MeteoData& data() const { return m_data; }
    size_t propertyCount() const  { return m_data.pCount; }

private:
    MeteoData m_data;
};

} // namespace meteocpp

#endif // METEO_HPP
