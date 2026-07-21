/**
 * @file Meteo.hpp
 * @brief Public C++ interface for the Meteo module.
 *
 * @defgroup MeteoCppModule MeteoCpp Module
 * @brief C++ weather-fetching interface used by Glennergy.
 *
 * Provides configuration loading and weather-data fetch operations for the
 * Open-Meteo backend.
 */

#ifndef METEO_HPP
#define METEO_HPP

#include <string_view>
#include <optional>
#include "meteo_types.hpp"
#include "../../Libs/GlennergyPaths.h"

namespace meteocpp {

/**
 * @brief Open-Meteo API URL template.
 */
constexpr std::string_view METEO_LINK =
    "https://api.open-meteo.com/v1/forecast?"
    "latitude={}&longitude={}"
    "&minutely_15=temperature_2m,shortwave_radiation,direct_normal_irradiance,"
    "diffuse_radiation,cloud_cover,is_day,weather_code,uv_index"
    "&forecast_days=3&forecast_minutely_15=128&timezone=Europe/Stockholm";

/**
 * @brief Meteo service class.
 *
 * Owns the loaded configuration and fetched weather data.
 */
class meteo {
public:
    /**
     * @brief Constructor.
     */
    meteo();

    /**
     * @brief Loads the configuration file.
     *
     * @param[in] configPath Path to the JSON configuration file.
     *
     * @return true on success, false on failure.
     */
    bool load(std::string_view configPath = GLENNERGY_CONFIG_PATH);

    /**
     * @brief Fetches weather data for all configured properties.
     *
     * @return true on success, false on failure.
     */
    bool fetchAll();

    /**
     * @brief Returns the internal data object.
     *
     * @return Const reference to the stored MeteoData.
     */
    const MeteoData& data() const { return m_data; }

    /**
     * @brief Returns the number of loaded properties.
     *
     * @return Number of loaded properties.
     */
    size_t propertyCount() const  { return m_data.pCount; }

private:
    MeteoData m_data; /**< Internal data storage. */
};

} // namespace meteocpp

#endif // METEO_HPP
