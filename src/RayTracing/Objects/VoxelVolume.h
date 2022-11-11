#pragma once

#include <memory>

#include "RayTracing/vec.h"
#include "RayTracing/Ray.h"
#include "RayTracing/hit_record.h"

#include "hittable.h"

#include "RayTracing/Materials/Material.h"
#include "RayTracing/Materials/Lambertian.h"
#include "RayTracing/Materials/Metal.h"

class VoxelVolume : public hittable {
	std::vector<std::shared_ptr<Material>> materials;

private:
	// size_t width = 8, height = 8, depth = 8;
	size_t width = 32, height = 32, depth = 32;
	size_t *voxels; // 0 = air
	vec3 scale;

	size_t index(const size_t x, const size_t y, const size_t z) const {
		return z * (width * height) + y * width + x;
	}

public:
	const vec3 p0, p1, p2;

public:
	VoxelVolume():
			materials{} {
		voxels = new size_t[width * height * depth]{};
		// scale = vec3(1.f);
		scale = vec3(.1f);
		
		materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, .5f))); // purple
		materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, 0.f))); // red
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 1.f, 0.f))); // green
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 0.f, 1.f))); // blue
		materials.push_back(std::make_unique<Metal>(vec3(.8f, .8f, 1.f), .5f)); // blue

		for(size_t x = 0; x < width; x++)
			for(size_t y = 0; y < height; y++)
				for(size_t z = 0; z < depth; z++)
					// if(vec3((int)x - width / 2, (int)y - height / 2, (int)z - depth / 2).length<float>() < width / 2)
					if(rand() %  5 == 0)
						voxels[index(x, y, z)] = rand() % (materials.size() + 1);

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
	}
	// VoxelVolume(const point3& p0, const point3& p1, const point3& p2, std::shared_ptr<Material> m)
	// 	: p0(p0), p1(p1), p2(p2), material(m) {};

	virtual bool hit(const Ray& r, const double t_min, const double t_max, hit_record& rec) const override {

		const vec3 viewPos = r.origin() / scale;
		const vec3 viewDir = r.direction();

		ivec3 currentBlock = floor(viewPos);
		const vec3 deltaT = abs(1.f / viewDir);
		bool hit = false;
		int side = 0;

		ivec3 step {0, 0, 0};
		vec3 sideDist;

		//calculate step and initial sideDist
		if (viewDir.x() < 0) {
			step.x() = -1;
			sideDist.x() = (viewPos.x() - currentBlock.x()) * deltaT.x();
		} else {
			step.x() = 1;
			sideDist.x() = (currentBlock.x() + 1.0 - viewPos.x()) * deltaT.x();
		}

		if (viewDir.y() < 0) {
			step.y() = -1;
			sideDist.y() = (viewPos.y() - currentBlock.y()) * deltaT.y();
		} else {
			step.y() = 1;
			sideDist.y() = (currentBlock.y() + 1.0 - viewPos.y()) * deltaT.y();
		}

		if (viewDir.z() < 0) {
			step.z() = -1;
			sideDist.z() = (viewPos.z() - currentBlock.z()) * deltaT.z();
		} else {
			step.z() = 1;
			sideDist.z() = (currentBlock.z() + 1.0 - viewPos.z()) * deltaT.z();
		}

		for(int i = 0; i < 500 && !hit; i++) {
			// float t = min(sideDistX, sideDistY);

			//jump to next cube
			if (sideDist.x() < sideDist.y() && sideDist.x() < sideDist.z()) {
				sideDist.x() += deltaT.x();
				currentBlock.x() += step.x();
				side = 0;
			} else if (sideDist.y() < sideDist.z()) {
				sideDist.y() += deltaT.y();
				currentBlock.y() += step.y();
				side = 1;
			} else {
				sideDist.z() += deltaT.z();
				currentBlock.z() += step.z();
				side = 2;
			}

			//Check if ray hit
			if(currentBlock.x() >= 0 && currentBlock.x() < width) {
				if(currentBlock.y() >= 0 && currentBlock.y() < height) {
					if(currentBlock.z() >= 0 && currentBlock.z() < depth) {
						if(voxels[index(currentBlock.x(), currentBlock.y(), currentBlock.z())] > 0) {
							hit = true;
							rec.material = materials[voxels[index(currentBlock.x(), currentBlock.y(), currentBlock.z())] - 1];
						}
					}
				}
			}
		}

		// /*
		if(hit) {
			ivec3 normal(0);
			normal[side] = viewDir[side] > 0 ? -1 : 1;

			rec.front_face = true;
			rec.normal = vec3(normal.x(), normal.y(), normal.z());

			// const ivec3 targetBlock = currentBlock + normal;

			// DEBUG_RENRERER->box(
			// 		glm::vec3(targetBlock.x, targetBlock.y, targetBlock.z),
			// 		glm::vec3(targetBlock.x+1, targetBlock.y+1, targetBlock.z+1)
			// 	);

			// if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
			// 	const BlockPos blockPosAbsolute = BlockPos::compute(getVirtualOrigin(), targetBlock);

			// 	AABB blockAABB{targetBlock,
			// 			{targetBlock.x+1, targetBlock.y+1, targetBlock.z+1}};

			// 	if(!blockAABB.collidesWith(getAABB())) {
			// 		// const ChunkPos chunkPos{
			// 		// 	(int64_t)std::floor(targetBlock.x / 16.f),
			// 		// 	(int64_t)std::floor(targetBlock.y / 16.f),
			// 		// 	(int64_t)std::floor(targetBlock.z / 16.f) };

			// 		// Chunk *const chunk = world.getChunk(chunkPos);
			// 		Chunk *const chunk = world.getChunk(blockPosAbsolute.chunkPos());

			// 		if(chunk != nullptr) {
			// 			// const ivec3 blockPosRel(
			// 			// 	targetBlock.x - chunkPos.x() * 16,
			// 			// 	targetBlock.y - chunkPos.y() * 16,
			// 			// 	targetBlock.z - chunkPos.z() * 16);
			// 			const ivec3 blockPosRel = blockPosAbsolute.blockPos();

			// 			chunk->setBlock(world, blockPosRel, {BlockType::GRASS});
			// 		}
			// 	}
			// }
		}
		// */
		
		if(!hit)
			return false;
		
		return true;
	}
};
