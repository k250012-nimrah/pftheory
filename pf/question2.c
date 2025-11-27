#include <stdio.h>

int calculateFuel(int fuel, int consumption, int recharge,
                  int solarBonus, int planet, int totalPlanets) {

    // Base case: fuel exhausted
    if (fuel <= 0) {
        printf("Planet %d: Fuel depleted! Mission failed.\n", planet);
        return 0; // failure
    }

    // Base case: all planets visited
    if (planet > totalPlanets) {
        printf("Mission completed successfully!\n");
        return 1; // success
    }

    // Fuel consumption at each planetary stop
    fuel -= consumption;

    // Gravitational assist recharge
    fuel += recharge;

    // Solar bonus every 4th planet
    if (planet % 4 == 0) {
        fuel += solarBonus;
    }

    // Print status
    printf("Planet %d: Fuel Remaining = %d\n", planet, fuel);

    // Recursive call for next planet
    return calculateFuel(fuel, consumption, recharge,
                         solarBonus, planet + 1, totalPlanets);
}


int main() {
    int fuel = 500;
    int consumption = 60;
    int recharge = 20;
    int solarBonus = 50;
    int totalPlanets = 10;

    printf("Spacecraft Fuel Tracking Simulation:\n\n");

    int result = calculateFuel(fuel, consumption, recharge,
                               solarBonus, 1, totalPlanets);

    if (result)
        printf("\nThe spacecraft successfully completed its mission!\n");
    else
        printf("\nThe spacecraft could NOT complete its mission.\n");

    return 0;
}
