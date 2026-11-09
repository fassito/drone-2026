#pragma once

// ============================================================
// Matemáticas de navegación (puras, sin hardware).
// ============================================================

// Distancia en metros entre dos coordenadas (fórmula de haversine).
float haversine_distance_m(float lat1, float lon1, float lat2, float lon2);

// Rumbo inicial en radianes [0, 2π) desde (lat1,lon1) hacia (lat2,lon2).
float bearing_to_rad(float lat1, float lon1, float lat2, float lon2);

// Normaliza un ángulo en radianes al rango [-π, π].
float wrap_pi(float rad);

// Normaliza un ángulo en radianes al rango [0, 2π).
float wrap_2pi(float rad);
