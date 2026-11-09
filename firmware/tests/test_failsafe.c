#include "test_util.h"
#include "safety/failsafe.h"

void test_failsafe(void) {
    test_section("Failsafe (componente)");

    // Estado inicial desarmado.
    failsafe_init();
    CHECK(failsafe_get_state().mode == FS_DISARMED, "inicio desarmado");

    // Armado rechazado sin condiciones.
    CHECK(!failsafe_arm_request(), "armado rechazado sin enlaces ni throttle mínimo");

    // Con enlaces OK + throttle mínimo → arma.
    failsafe_set_rx_link(true);
    failsafe_set_imu_link(true);
    failsafe_set_throttle_min(true);
    CHECK(failsafe_arm_request(), "armado con enlaces OK + throttle mínimo");
    CHECK(failsafe_get_state().mode == FS_ARMED, "estado armado");

    // Pérdida de enlace de radio en vuelo → failsafe.
    failsafe_set_rx_link(false);
    failsafe_tick();
    CHECK(failsafe_get_state().mode == FS_FAILSAFE, "pérdida de radio → FS_FAILSAFE");

    // Reinicio y pérdida de IMU en vuelo → failsafe.
    failsafe_init();
    failsafe_set_rx_link(true);
    failsafe_set_imu_link(true);
    failsafe_set_throttle_min(true);
    failsafe_arm_request();
    failsafe_set_imu_link(false);
    failsafe_tick();
    CHECK(failsafe_get_state().mode == FS_FAILSAFE, "pérdida de IMU → FS_FAILSAFE");

    // Desarmado manual.
    failsafe_init();
    failsafe_set_rx_link(true);
    failsafe_set_imu_link(true);
    failsafe_set_throttle_min(true);
    failsafe_arm_request();
    failsafe_disarm();
    CHECK(failsafe_get_state().mode == FS_DISARMED, "desarmado manual");

    // Armado bloqueado con throttle fuera de mínimo.
    failsafe_init();
    failsafe_set_rx_link(true);
    failsafe_set_imu_link(true);
    failsafe_set_throttle_min(false);
    CHECK(!failsafe_arm_request(), "armado bloqueado con throttle ≠ mínimo");
}
