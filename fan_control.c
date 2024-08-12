#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>
#include <getopt.h>

#define FAN_PIN 18 // GPIO-Pin für den Lüfter

// Funktion zum Auslesen der CPU-Temperatur
float get_cpu_temperature() {
    FILE *fp;
    float temp;

    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL) {
        fprintf(stderr, "Konnte CPU-Temperatur nicht lesen\n");
        exit(1);
    }
    fscanf(fp, "%f", &temp);
    fclose(fp);

    return temp / 1000.0; // Temperatur in °C umwandeln
}

// Funktion zur Steuerung des Lüfters basierend auf der Temperatur
void control_fan(int pin, float temperature, int temp_on, int temp_off, int full_speed_temp) {
    static int fan_running = 0; // Zustand des Lüfters (0 = aus, 1 = an)
    int duty_cycle = 0;

    if (temperature >= full_speed_temp) {
        duty_cycle = 255; // 100% PWM (Lüfter läuft auf voller Geschwindigkeit)
        fan_running = 1;
    } else if (temperature >= temp_on) {
        duty_cycle = (int)((temperature - temp_on) / (full_speed_temp - temp_on) * 255);
        fan_running = 1;
    } else if (temperature <= temp_off) {
        duty_cycle = 0; // Lüfter ausschalten
        fan_running = 0;
    }

    // Lüfter nur anpassen, wenn er läuft oder ausgeschaltet wird
    if (fan_running || duty_cycle == 0) {
        gpioPWM(pin, duty_cycle);
    }
}

int main(int argc, char *argv[]) {
    // Standardwerte für die Parameter
    int temp_on = 55;
    int temp_off = 50;
    int full_speed_temp = 70;
    int check_interval = 10;

    // Optionen definieren
    static struct option long_options[] = {
        {"temp-on", required_argument, 0, 'o'},
        {"temp-off", required_argument, 0, 'f'},
        {"full-speed", required_argument, 0, 's'},
        {"interval", required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:f:s:i:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                temp_on = atoi(optarg);
                break;
            case 'f':
                temp_off = atoi(optarg);
                break;
            case 's':
                full_speed_temp = atoi(optarg);
                break;
            case 'i':
                check_interval = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [--temp-on temp_on] [--temp-off temp_off] [--full-speed full_speed_temp] [--interval check_interval]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // pigpio initialisieren
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;
    }

    // PWM an GPIO18 (Pin 12) einstellen
    gpioSetPWMfrequency(FAN_PIN, 25000);

    while (1) {
        // CPU-Temperatur auslesen
        float temperature = get_cpu_temperature();

        // Lüfter steuern
        control_fan(FAN_PIN, temperature, temp_on, temp_off, full_speed_temp);

        // Wartezeit
        sleep(check_interval);
    }

    // pigpio sauber beenden (wird nie erreicht, da Endlosschleife)
    gpioTerminate();

    return 0;
}
