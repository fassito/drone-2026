#pragma once

// ============================================================
// Calibración de magnetómetro (hard iron + escala). Funciones
// puras, testeables sin hardware.
// ============================================================

// Offset hard-iron a partir de los extremos medidos en cada eje:
// offset[i] = (min[i] + max[i]) / 2.
void mag_hard_iron_offsets(const float mn[3], const float mx[3], float offsets[3]);

// Aplica calibración: resta offset y multiplica por escala.
void mag_apply_cal(const float raw[3], const float offsets[3],
                   const float scale[3], float out[3]);

// Escala para normalizar la elipse de cada eje a un radio común.
void mag_compute_scale(const float mn[3], const float mx[3], float scale[3]);
