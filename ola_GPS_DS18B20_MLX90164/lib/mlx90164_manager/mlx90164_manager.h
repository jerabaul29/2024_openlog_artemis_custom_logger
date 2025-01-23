#ifndef MLX_MANAGER
#define MLX_MANAGER

#include "etl/vector.h"

#include "time_manager.h"
#include "watchdog_manager.h"

#include <defWireArtemis.h>
#include <SparkFunMLX90614.h>//Click here to get the library: http://librarymanager/All#Qwiic_IR_Thermometer by SparkFun
#include <SoftWireArtemis/MLX_SoftWireArtemis.h>

// NOTE: the default I2C pins, that may change from board to board, are set at:
// defWireArtemis.h

struct MLX_Information{
    unsigned long posix_timestamp;
    float ir_temperature;
    float sensor_temperature;
};

class MLX90164_Manager{
    public:
        bool acquire_n_readings(size_t nbr_readings=20);

        static constexpr size_t size_buffer {30};
        etl::vector<MLX_Information, size_buffer> crrt_accumulator_MLX;

    private:
        void push_1_measurement(void);
};

extern MLX90164_Manager mlx90164_manager;

#endif