#include "mlx90164_manager.h"

IRTherm therm; // Create an IRTherm object to interact with throughout
TwoWireArtemis WireArtemis;

void turn_mlx_on(void){
    pinMode(PIN_QWIIC_PWR, OUTPUT);
    digitalWrite(PIN_QWIIC_PWR, HIGH);
    delay(1000);
    wdt.restart();
}

void turn_mlx_off(void){
    pinMode(PIN_QWIIC_PWR, OUTPUT);
    digitalWrite(PIN_QWIIC_PWR, LOW);
}

void MLX90164_Manager::push_1_measurement(void){
    delay(1000);
    if (therm.read()) // On success, read() will return 1, on fail 0.
    {
        // Use the object() and ambient() functions to grab the object and ambient
        // temperatures.
        // They'll be floats, calculated out to the unit you set with setUnit().
        SERIAL_USB->print("Object: " + String(therm.object(), 2));
        SERIAL_USB->println(F("C"));
        SERIAL_USB->print("Ambient: " + String(therm.ambient(), 2));
        SERIAL_USB->println(F("C"));

        MLX_Information mlx_information{
            board_time_manager.get_posix_timestamp(),
            therm.object(),
            therm.ambient()
        };

        crrt_accumulator_MLX.push_back(mlx_information);
    }
    wdt.restart();
}

bool MLX90164_Manager::acquire_n_readings(size_t nbr_readings){
    turn_mlx_on();
    wdt.restart();

    // ------------------------------------------------------
    // start the sensor

    // give time for the sensor to wake up
    delay(500);
    WireArtemis.begin();
    wdt.restart();

    for (int i=0; i<5; i++){
        if (therm.begin() == false){ // Initialize the MLX90614
            SERIAL_USB->println(F("Qwiic IR thermometer did not start; aborting"));
            delay(1000);

            if (i==4){
                turn_mlx_off();
                return false;
            }
        }
        else {
            break;
        }
    }
    SERIAL_USB->println(F("Qwiic IR thermometer started"));
    wdt.restart();

    if (therm.readID()){
        SERIAL_USB->println("ID: 0x" + 
                    String(therm.getIDH(), HEX) +
                    String(therm.getIDL(), HEX));
    }
    SERIAL_USB->println(String(therm.readEmissivity()));
    wdt.restart();

    therm.setUnit(TEMP_C);
    crrt_accumulator_MLX.clear();
    wdt.restart();

    // buoy started
    // ------------------------------------------------------

    // TODO: do the measurements
    for (int i=0; i<nbr_readings; i++){
        push_1_measurement();
    }

    turn_mlx_off();

    return true;
}

MLX90164_Manager mlx90164_manager;
