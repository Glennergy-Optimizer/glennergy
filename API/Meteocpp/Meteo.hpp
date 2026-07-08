/**
 * @file Meteo.hpp
 * @brief High-level Meteo C++ interface.
 *
 * @defgroup MeteoCppModule MeteoCpp Module
 */

#ifndef METEO_HPP
#define METEO_HPP

#include <string_view>
#include <optional>
#include "meteo_types.hpp"

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
 * @note Memory ownership:
 * - Owns MeteoData internally
 * - No dynamic allocation exposed
 */
class meteo {
public:
    /**
     * @brief Constructor.
     *
     * @post Internal data zero-initialized
     */
    meteo();

    /**
     * @brief Load configuration file.
     *
     * @param[in] configPath Path to JSON config
     * @return true on success, false on failure
     *
     * @pre File must exist and be valid JSON
     * @post Internal data populated
     */
    bool load(std::string_view configPath = "Glennergy-Fastigheter.json");

    /**
     * @brief Fetch weather data for all properties.
     *
     * @return true on success, false on failure
     *
     * @pre load() must have been called
     * @post sample arrays populated
     *
     * @warning Network-dependent operation
     */
    bool fetchAll();

    /**
     * @brief Access internal data.
     *
     * @return Const reference to MeteoData
     *
     * @note No ownership transfer
     */
    const MeteoData& data() const { return m_data; }

    /**
     * @brief Get number of properties.
     *
     * @return Number of loaded properties
     */
    size_t propertyCount() const  { return m_data.pCount; }

private:
    MeteoData m_data; /**< Internal data storage */
};

} // namespace meteocpp

#endif // METEO_HPP