#pragma once

namespace level_sensor
{

    void init();

    bool water_ok_raw();
    bool water_low_raw();

    bool water_ok_confirmed();
    bool water_low_confirmed();

} // namespace level_sensor