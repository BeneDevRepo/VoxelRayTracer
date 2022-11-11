#pragma once

#include <memory>

#include "RayTracing/vec.h"
#include "RayTracing/Ray.h"
#include "RayTracing/hit_record.h"

#include "hittable.h"

#include "RayTracing/Materials/Material.h"
#include "RayTracing/Materials/Lambertian.h"

class VoxelVolume : public hittable {
	std::vector<std::shared_ptr<Material>> materials;

	size_t voxels[8][8][8]; // 0 = air
	vec3 scale;

public:
	const vec3 p0, p1, p2;

public:
	VoxelVolume():
		materials{},
		voxels{} {
		scale = vec3(1.f);
		
		materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, .5f))); // purple
		materials.push_back(std::make_unique<Lambertian>(vec3(1.f, 0.f, 0.f))); // red
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 1.f, 0.f))); // green
		materials.push_back(std::make_unique<Lambertian>(vec3(0.f, 0.f, 1.f))); // blue

		// voxels[0][0][0] = true;

		voxels[3][3][3] = 1;
		voxels[3][3][4] = 1;
		voxels[3][4][3] = 1;
		voxels[3][4][4] = 1;
		voxels[4][3][3] = 1;
		voxels[4][3][4] = 1;
		voxels[4][4][3] = 1;
		voxels[4][4][4] = 1;

		voxels[2][3][3] = 2;
		voxels[2][3][4] = 2;
		voxels[2][4][3] = 2;
		voxels[2][4][4] = 2;

		voxels[3][2][3] = 3;
		voxels[3][2][4] = 3;
		voxels[4][2][3] = 3;
		voxels[4][2][4] = 3;

		voxels[3][3][2] = 4;
		voxels[3][4][2] = 4;
		voxels[4][3][2] = 4;
		voxels[4][4][2] = 4;
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
			if(currentBlock.x() >= 0 && currentBlock.x() < 8) {
				if(currentBlock.y() >= 0 && currentBlock.y() < 8) {
					if(currentBlock.z() >= 0 && currentBlock.z() < 8) {
						if(voxels[currentBlock.x()][currentBlock.y()][currentBlock.z()] > 0) {
							hit = true;
							rec.material = materials[voxels[currentBlock.x()][currentBlock.y()][currentBlock.z()] - 1];
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

		// rec.material = material;
		
		return true;

		// const float kEpsilon = .00000001;

		// const vec3& v0 = p0;
		// const vec3& v1 = p1;
		// const vec3& v2 = p2;
		// // const vec3& v1 = p2;
		// // const vec3& v2 = p1;

		// const vec3 v0v1 = v1 - v0;
		// const vec3 v0v2 = v2 - v0;

		// // no need to normalize
		// const vec3 N = cross(v0v1, v0v2); // N
		// const float area2 = N.length();

		// // Step 1: finding P
 
		// // check if ray and plane are parallel ?
		// const float NdotRayDirection = dot(N, r.dir); 
		// if (fabs(NdotRayDirection) < kEpsilon) // almost 0 
		// 	return false; // they are parallel so they don't intersect ! 
	
		// // compute d parameter using equation 2
		// const float d = -dot(N, v0);
	
		// // compute t (equation 3)
		// const float t = -(dot(N, r.orig) + d) / NdotRayDirection;
	
		// // check if the triangle is in behind the ray
		// if (t < 0.00001) return false; // the triangle is behind

		// if(t > t_max) return false; // Already found a closer object
	
		// // compute the intersection point using equation 1
		// const vec3 P = r.orig + t * r.dir; 
	
		// // Step 2: inside-outside test
		// vec3 C; // vector perpendicular to triangle's plane 

		// // edge 0
		// const vec3 edge0 = v1 - v0;
		// const vec3 vp0 = P - v0;
		// C = cross(edge0, vp0);
		// if (dot(N, C) < 0) return false; // P is on the right side 
	
		// // edge 1
		// const vec3 edge1 = v2 - v1; 
		// const vec3 vp1 = P - v1; 
		// C = cross(edge1, vp1); 
		// if (dot(N, C) < 0) return false; // P is on the right side 
	
		// // edge 2
		// const vec3 edge2 = v0 - v2;
		// const vec3 vp2 = P - v2;
		// C = cross(edge2, vp2); 
		// if (dot(N, C) < 0) return false; // P is on the right side; 

		// rec.set_face_normal(r, N / N.length());
		// // rec.set_face_normal(r, N);
		// rec.material = material;
		// rec.p = P;
		// rec.t = t;
	
		// return true; // this ray hits the triangle 



		// rec.t = root;
		// rec.p = r.at(rec.t);
		// vec3 outward_normal = (rec.p - p0) / .5;
		// rec.set_face_normal(r, outward_normal);
		// rec.material = material;

		// return true;
	}
};
