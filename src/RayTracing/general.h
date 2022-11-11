#pragma once

#include <cmath>

#include "RayTracing/vec.h"
#include "RayTracing/color.h"
#include "RayTracing/Ray.h"
#include "RayTracing/Objects/hittable_list.h"

#include "RayTracing/Texture/fTexture.h"


// https://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere
// z output unused
vec3 sampleSphericalMap(vec3 v) { // Equirectangular map
	v = normalize(v);

    vec3 uv; // z output unused
	uv.x() = .5 + atan2(v.z(), v.x()) / (2 * 3.1415926535);
	uv.y() = .5 + asin(v.y()) / 3.1415926535;

    return uv;
}

color ray_color(const Ray& r, const hittable& world, const fTexture& skybox, const int depth) {
	if (depth <= 0) // max bounces between objects
		return color(0, 0, 0);

	const double INF = 1. / 0.;

	hit_record rec;
	if (world.hit(r, 0.00001, INF, rec)) {
		Ray scattered;
		color attenuation;
		if (rec.material->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, skybox, depth-1);
		return color(0, 0, 0); // ?? un-reachable ??
	}

	// const vec3 unit_direction = unit_vector(r.dir);
	// const float t = 0.5 * (unit_direction.y() + 1.0);
	// return lerp(color(1.0, 1.0, 1.0), color(0.5, 0.7, 1.0), t);

	vec3 skyboxIndex = sampleSphericalMap(r.dir);
	skyboxIndex.x() = std::min<float>(std::max<float>(skyboxIndex.x(), 0), 1);
	skyboxIndex.y() = std::min<float>(std::max<float>(skyboxIndex.y(), 0), 1);

	const uint32_t col = skybox.pixels[
		std::min<size_t>((size_t)(skyboxIndex.y() * skybox.height), skybox.height - 1)
			* skybox.width
			+ std::min<size_t>((size_t)(skyboxIndex.x() * skybox.width), skybox.width - 1)
	];

	return vec3(
		(col >> 16) & 0xFF, // R
		(col >> 8) & 0xFF, // G
		(col >> 0) & 0xFF // B
	 ) / 255.f;
}