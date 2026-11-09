#pragma once

// ============================================================
// Mezcla de motores (cuadricóptero en X).
// Responsable: flight-control-engineer.
// ============================================================

// STUB — pendiente de implementar.

// Convierte los comandos de vuelo en la señal de los 4 motores.
// roll/pitch/yaw en [-1, +1], throttle en [0, +1].
// out[]: valores de motor en [0, +1] (orden: 1=delante-derecha,
// 2=detrás-izquierda, 3=delante-izquierda, 4=detrás-derecha).
void mixer_apply(float roll, float pitch, float yaw, float throttle, float out[4]);
