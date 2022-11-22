#pragma once

#include <memory>


#include "RayTracing/vec.h"
#include "RayTracing/Ray.h"
#include "RayTracing/AABB.h"
#include "RayTracing/hit_record.h"

#include "hittable.h"

#include "RayTracing/Materials/Material.h"
#include "RayTracing/Materials/Lambertian.h"
#include "RayTracing/Materials/Metal.h"
#include "RayTracing/Materials/Dielectric.h"


class VoxelVolume : public hittable {
	std::vector<std::shared_ptr<Material>> materials;

private:
	size_t width = 8, height = 8, depth = 8;
	// size_t width = 32, height = 32, depth = 32;
	size_t *voxels; // 0 = air
	vec3 scale;
	AABB aabb;

	size_t index(const size_t x, const size_t y, const size_t z) const {
		return z * (width * height) + y * width + x;
	}

public:
	VoxelVolume():
			materials{} {
		voxels = new size_t[width * height * depth]{};
		scale = vec3(1.f);
		// scale = vec3(.1f);

		aabb = AABB(vec3(0, 0, 0), vec3(width * scale.x(), height * scale.y(), depth * scale.z()));
		// aabb.max *= .75f;
		
		// materials.push_back(std::make_unique<Dielectric>(1.5f)); // glass
		// materials.push_back(std::make_unique<Dielectric>(1.f)); // glass
		materials.push_back(std::make_unique<Metal>(vec3(1.f, .2f, .2f), .001f)); // metal

		// materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, .5f))); // purple
		materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, 0.f))); // red
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 1.f, 0.f))); // green
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 0.f, 1.f))); // blue
		materials.push_back(std::make_unique<Metal>(vec3(1.f, .2f, .2f), .001f)); // metal

		// materials.push_back(std::make_unique<Dielectric>(1.5f)); // glass
		// materials.push_back(std::make_unique<Dielectric>(1.f)); // glass


		// for(size_t x = 0; x < width; x++) {
		// 	for(size_t y = 0; y < height; y++) {
		// 		voxels[index(x, y, 0)] = 6;
		// 		voxels[index(x, y, depth - 1)] = 6;
		// 	}
		// }

		// for(size_t x = 2; x < width - 2; x++) {
		// 	for(size_t y = 2; y < height - 2; y++) {
		// 		voxels[index(x, y, 2)] = 5;
		// 		voxels[index(x, y, depth - 1 - 2)] = 5;
		// 	}
		// }

		// for(size_t x = 0; x < width; x++)
		// 	for(size_t y = 0; y < height; y++)
		// 		for(size_t z = 0; z < depth; z++)
		// 			// if(vec3((int)x - width / 2, (int)y - height / 2, (int)z - depth / 2).length<float>() < width / 2)
		// 			if(rand() %  5 == 0)
		// 				voxels[index(x, y, z)] = rand() % (materials.size() + 1);

		// voxels[index(3, 3, 3)] = 1;
		// voxels[index(3, 3, 4)] = 1;
		// voxels[index(3, 4, 3)] = 1;
		// voxels[index(3, 4, 4)] = 1;
		// voxels[index(4, 3, 3)] = 1;
		// voxels[index(4, 3, 4)] = 1;
		// voxels[index(4, 4, 3)] = 1;
		// voxels[index(4, 4, 4)] = 1;

		// voxels[index(2, 3, 3)] = 2;
		// voxels[index(2, 3, 4)] = 2;
		// voxels[index(2, 4, 3)] = 2;
		// voxels[index(2, 4, 4)] = 2;

		// voxels[index(3, 2, 3)] = 3;
		// voxels[index(3, 2, 4)] = 3;
		// voxels[index(4, 2, 3)] = 3;
		// voxels[index(4, 2, 4)] = 3;

		// voxels[index(3, 3, 2)] = 4;
		// voxels[index(3, 4, 2)] = 4;
		// voxels[index(4, 3, 2)] = 4;
		// voxels[index(4, 4, 2)] = 4;

		voxels[index(0, 0, 0)] = 1;
		voxels[index(0, 0, 1)] = 1;
		voxels[index(0, 1, 0)] = 1;
		voxels[index(0, 1, 1)] = 1;

		voxels[index(2, 0, 0)] = 2;
		voxels[index(2, 0, 1)] = 2;
		voxels[index(2, 1, 0)] = 2;
		voxels[index(2, 1, 1)] = 2;

		voxels[index(3, 0, 0)] = 3;
		voxels[index(3, 0, 1)] = 3;
		voxels[index(3, 1, 0)] = 3;
		voxels[index(3, 1, 1)] = 3;
	}

	virtual bool hit(const Ray& r, const double t_min, const double t_max, hit_record& rec) const override {
		float tHitBounds;
		ivec3 hitBoundsNormal;

		if(!aabb.intersects(r, hitBoundsNormal, tHitBounds))
			return false;

		// const vec3 viewPos = r.origin() / scale;
		// const vec3 viewPos = r.at(tHitBounds > 0 ? tHitBounds : 0) / scale;
		const vec3 viewPos = r.at(tHitBounds > 0 ? tHitBounds : 0) / scale + normalize(r.direction()) * .01f;
		const vec3 viewDir = r.direction();

		ivec3 currentBlock = floor(viewPos);
		const vec3 deltaT = abs(1.f / viewDir);


		//calculate step and initial sideDist:
		ivec3 step;
		vec3 sideDist;
		for(uint8_t dim = 0; dim < 3; dim++) {
			step[dim] = viewDir[dim] < 0 ? -1 : 1;
			sideDist[dim] = (viewDir[dim] < 0)
				?  (viewPos[dim] - currentBlock[dim]) * deltaT[dim]
				: -(viewPos[dim] - currentBlock[dim] - 1.) * deltaT[dim];
		}



		int side = 0;
		// for(int i = 0; i < 500; i++) {
		for(;;) {

			//jump to next cube
			const float minDim = std::min<float>(std::min<float>(sideDist.x(), sideDist.y()), sideDist.z());
			for(uint8_t dim = 0; dim < 3; dim++) {
				if (sideDist[dim] == minDim) {
					sideDist[dim] += deltaT[dim];
					currentBlock[dim] += step[dim];
					side = dim;
					break;
				}
			}

			const auto inside =
				[=](const ivec3& pos) -> bool {
					if(pos.x() < 0 || pos.x() >= width) return false;
					if(pos.y() < 0 || pos.y() >= height) return false;
					if(pos.z() < 0 || pos.z() >= depth) return false;
					return true;
				};

			//Check if ray hit
			if(!inside(currentBlock))
				return false;

			ivec3 normal(0);
			normal[side] = viewDir[side] > 0 ? -1 : 1;

			const ivec3 prev = currentBlock + normal;

			const size_t mat = voxels[index(currentBlock.x(), currentBlock.y(), currentBlock.z())];
			const size_t prevMat = inside(prev) ? voxels[index(prev.x(), prev.y(), prev.z())] : 0;

			if(mat != prevMat) {
			// if(mat) {
				// rec.front_face = true;
				// rec.material = materials[mat - 1];

				rec.front_face = mat != 0;
				rec.material = (mat != 0) ? materials[mat - 1] : materials[prevMat - 1];

				rec.p = vec3(currentBlock.x(), currentBlock.y(), currentBlock.z());
				// rec.p -= sideDist * normalize(viewDir);
				rec.p *= scale;
				rec.normal = vec3(normal.x(), normal.y(), normal.z());

				return true;
			}
		}
	}
};
