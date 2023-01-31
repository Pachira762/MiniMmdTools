#pragma once
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <ranges>
#include <string>
#include <fstream>
#include <type_traits>
#include <cstddef>

// portable mmd library

namespace poml {
	//
	// Common
	//
	struct Interpolation {
		int8_t	x1;
		int8_t	x2;
		int8_t	y1;
		int8_t	y2;

		static constexpr Interpolation get_default() {
			return { 20,107,20,107 };
		}
	};

	template<unsigned N = 8>
	constexpr float calc_bezier(float x, float x1, float x2, float y1, float y2) {
		auto bezier = [](float t, float p1, float p2) {
			const float t2 = t * t;
			const float t3 = t2 * t;
			return (3.f * (p1 - p2) + 1.f) * t3 + 3.f * (-2.f * p1 + p2) * t2 + (3.f * p1) * t;
		};

		auto bezier1 = [](float t, float p1, float p2) {
			const float t2 = t * t;
			return 3.f * (3.f * (p1 - p2) + 1.f) * t2 + 6.f * (-2.f * p1 + p2) * t + (3.f * p1);
		};

		float t = 0.8f * x + 0.1f; // Avoid division by zero when x == 0.f && x1 == 0.f.
		for (unsigned i = 0; i < N; ++i) {
			float ft = bezier(t, x1, x2) - x;
			float ft1 = bezier1(t, x1, x2);
			t = t - (ft / ft1);
		}

		return bezier(t, y1, y2);
	}

	//
	// Pmx
	//

	enum class WeightKind : uint8_t {
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF,
	};

	enum class MorphKind : uint8_t {
		Group = 0,
		Vertex = 1,
		Bone = 2,
		UV = 3,
		ExUV1 = 4,
		ExUV2 = 5,
		ExUV3 = 6,
		ExUV4 = 7,
		Material = 8,
	};

	enum class MorphPanel : uint8_t {
		Eyeblow = 1,
		Eye = 2,
		Mouth = 3,
		Other = 4
	};

	enum class MaterialOp : uint8_t {
		Mult = 0,
		Add = 1,
	};

	enum class NodeKind : uint8_t {
		Bone = 0,
		Morph = 1
	};

	enum class BodyMode : uint8_t {
		Static = 0,
		Dynamic = 1,
		Combine = 2,
	};

	enum class JointKind : uint8_t {
		Spring6DOF = 0,
	};

	template<typename Vec2, typename Vec3, typename Vec4>
	struct PmxBase {
		using Text = std::wstring;

		static constexpr char Magic[4] = { 0x50,0x4d,0x58,0x20 };
		static constexpr float Version = 2.0f;

		struct Vertex {
			Vec3		position;
			Vec3		normal;
			Vec2		uv;
			Vec4		ex_uvs[4];
			WeightKind	weight_kind;
			int32_t		bone_indices[4];
			float		bone_weights[4];
			Vec3		sdef_c;
			Vec3		sdef_r0;
			Vec3		sdef_r1;
			float		edge;
		};

		using Texture = Text;

		struct Material {
			Text		name;
			Text		name_en;
			Vec4		diffuse;
			Vec4		specular;
			Vec3		ambient;
			bool		two_side;
			bool		cast_ground_shadow;
			bool		cast_self_shadow;
			bool		receive_self_shadow;
			bool		draw_edge;
			Vec4		edge_color;
			float		edge_size;
			int32_t		base_texture_index;
			int32_t		sphere_texture_index;
			uint8_t		sphere_mode;
			bool		use_shared_toon;
			int32_t		toon_texture_index;
			Text		note;
			int32_t		num_vertices;
		};

		struct Bone {
			struct IkLink {
				int32_t		index;
				bool		angle_limited;
				Vec3		angle_min;
				Vec3		angle_max;
			};

			Text		name;
			Text		name_en;
			Vec3		position;
			int32_t		parent_bone_index;
			int32_t		level;
			bool		has_tip_bone;
			bool		rotatable;
			bool		translatable;
			bool		visible;
			bool		operable;
			bool		is_ik;
			bool		local_driven;
			bool		driven_rotation;
			bool		driven_translation;
			bool		has_fixed_axis;
			bool		has_local_axis;
			bool		post_physics_transform;
			bool		external_transform;
			Vec3		tip_offset;
			int32_t		tip_bone_index;
			int32_t		drive_bone_index;
			float		drive_rate;
			Vec3		fixed_axis;
			Vec3		local_axis_x;
			Vec3		local_axis_z;
			int32_t		external_key;
			int32_t		ik_target_bone_index;
			int32_t		ik_iteration_count;
			float		ik_angle_limit;
			std::vector<IkLink>	ik_links;
		};

		struct VertexMorphData {
			int32_t		index;
			Vec3		offset;
		};

		struct UvMorphData {
			int32_t		index;
			Vec4		offset;
		};

		struct BoneMorphData {
			int32_t		index;
			Vec3		translation;
			Vec4		rotation;
		};

		struct MaterialMorphData {
			int32_t		index;
			MaterialOp	op;
			Vec4		diffuse;
			Vec4		specular;
			Vec3		ambient;
			Vec4		edge_color;
			float		edge_size;
			Vec4		tex;
			Vec4		sphere;
			Vec4		toon;
		};

		struct GroupMorphData {
			int32_t		index;
			float		rate;
		};

		template<typename Data>
		struct Morph {
			Text		name;
			Text		name_en;
			MorphPanel	panel;
			MorphKind	kind;
			std::vector<Data>	data;
		};

		using VertexMorph = Morph<VertexMorphData>;
		using UvMorph = Morph<UvMorphData>;
		using BoneMorph = Morph<BoneMorphData>;
		using MaterialMorph = Morph<MaterialMorphData>;
		using GroupMorph = Morph<GroupMorphData>;

		struct Node {
			struct Item {
				NodeKind	kind;
				int32_t		index;
			};

			Text		name;
			Text		name_en;
			bool		special;
			std::vector<Item>	items;
		};

		struct Body {
			Text		name;
			Text		name_en;
			int32_t		index;
			uint8_t		group;
			uint16_t	pass_group;
			uint8_t		shape;
			Vec3		size;
			Vec3		position;
			Vec3		rotation;
			float		mass;
			float		linear_damping;
			float		angular_damping;
			float		restitution;
			float		friction;
			BodyMode	mode;
		};

		struct Joint {
			Text		name;
			Text		name_en;
			JointKind	kind;
			int32_t		body_index_a;
			int32_t		body_index_b;
			Vec3		position;
			Vec3		rotation;
			Vec3		linear_min;
			Vec3		linear_max;
			Vec3		angular_min;
			Vec3		angular_max;
			Vec3		linear_spring_const;
			Vec3		angular_spring_const;
		};

		float		version;
		uint8_t		num_ex_uvs;
		uint8_t		vertex_index_size;
		uint8_t		texture_index_size;
		uint8_t		material_index_size;
		uint8_t		bone_index_size;
		uint8_t		morph_index_size;
		uint8_t		body_index_size;
		Text		name;
		Text		name_en;
		Text		comment;
		Text		comment_en;

		std::vector<Vertex>			vertices;
		std::vector<int32_t>		faces;
		std::vector<Texture>		textures;
		std::vector<Material>		materials;
		std::vector<Bone>			bones;
		std::vector<VertexMorph>	vertex_morphs;
		std::vector<UvMorph>		uv_morphs;
		std::vector<BoneMorph>		bone_morphs;
		std::vector<MaterialMorph>	material_morphs;
		std::vector<GroupMorph>		group_morphs;
		std::vector<Node>			nodes;
		std::vector<Body>			bodies;
		std::vector<Joint>			joints;
	};

	//
	// Vmd
	//

	template<typename Key>
	struct Track {
		std::vector<Key> keys;

		auto begin() {
			return keys.begin();
		}

		auto begin() const {
			return keys.begin();
		}

		auto end() {
			return keys.end();
		}

		auto end() const {
			return keys.end();
		}

		decltype(auto) front() {
			return keys.front();
		}

		decltype(auto) front() const {
			return keys.front();
		}

		decltype(auto) back() {
			return keys.back();
		}

		decltype(auto) back() const {
			return keys.back();
		}

		bool empty() const {
			return keys.empty();
		}

		size_t size() const {
			return keys.size();
		}

		void resize(size_t size) {
			keys.resize(size);
		}

		Key& add() {
			return keys.emplace_back(Key());
		}

		template<typename T>
		Key& add(T&& key) {
			return keys.emplace_back(std::forward<T>(key));
		}

		std::pair<Key*, Key*> search(uint32_t frame) {
			if (keys.empty()) {
				return { nullptr, nullptr };
			}
			else if (keys.size() == 1) {
				return { keys.data(), keys.data() };
			}

			auto it = std::lower_bound(keys.begin(), keys.end(), frame, [](const Key& key0, const Key& key1) { return key0.frame < key1.frame;  });

			if (it == keys.begin()) {
				auto& key = keys.front();
				return { &key, &key };
			}
			else if (it == keys.end()) {
				auto& key = keys.back();
				return { &key, &key };
			}
			else {
				auto& key0 = *(it - 1);
				auto& key1 = *it;
				return { &key0, &key1 };
			}
		}

		std::pair<const Key*, const Key*> search(uint32_t frame) const {
			if (keys.empty()) {
				return { nullptr, nullptr };
			}
			else if (keys.size() == 1) {
				return { keys.data(), keys.data() };
			}

			auto it = std::ranges::lower_bound(keys, frame, {}, &Key::frame);

			if (it == keys.begin()) {
				auto& key = keys.front();
				return { &key, &key };
			}
			else if (it == keys.end()) {
				auto& key = keys.back();
				return { &key, &key };
			}
			else {
				auto& key0 = *(it - 1);
				auto& key1 = *it;
				return { &key0, &key1 };
			}
		}

		auto value_at(uint32_t frame) const {
			auto [key0, key1] = search(frame);
			return Key::interpolate(*key0, *key1, frame);
		}

		void sort() {
			std::ranges::sort(keys, {}, &Key::frame);
		}
	};

	template<typename Vec3, typename Vec4>
	struct VmdBase {
		using Text = std::string;

		static constexpr char Magic[30] = "Vocaloid Motion Data 0002\0\0\0\0";

		struct MotionKey {
			uint32_t		frame;
			Vec3			position;
			Vec4			orientation;
			Interpolation	ix;
			Interpolation	iy;
			Interpolation	iz;
			Interpolation	ir;
			int8_t			interpolation[64];
			bool			is_physics;

			static constexpr MotionKey get_default() {
				return {
					0,
					{},
					{},
					Interpolation::get_default(),
					Interpolation::get_default(),
					Interpolation::get_default(),
					Interpolation::get_default(),
					{
						20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107,
						20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 0,
						20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 0, 0,
						20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 107, 20, 0, 0, 0
					},
					false
				};
			}
		};

		struct MorphKey {
			uint32_t		frame;
			float			value;

			static float interpolate(const MorphKey& key0, const MorphKey& key1, uint32_t frame) {
				if (key0.frame == key1.frame) {
					return key0.value;
				}
				else {
					float time = static_cast<float>(frame - key0.frame) / static_cast<float>(key1.frame - key0.frame);
					return (1.f - time) * key0.value + time * key1.value;
				}
			}
		};

		struct CameraKey {
			uint32_t		frame;
			float			distance;
			Vec3			position;
			Vec3			rotation;
			Interpolation	ix;
			Interpolation	iy;
			Interpolation	iz;
			Interpolation	ir;
			Interpolation	id;
			Interpolation	iv;
			int32_t			view_angle;
			bool			orthographic;
		};

		struct LightKey {
			uint32_t	frame;
			Vec3		color;
			Vec3		position;
		};

		struct ShadowKey {
			uint32_t	frame;
			int8_t		type;
			float		distance;
		};

		struct IkKey {
			uint32_t	frame;
			bool		enable;
		};

		struct VisibilityKey {
			uint32_t	frame;
			bool		visible;
		};

		Text name;
		std::map<Text, Track<MotionKey>>	motion_tracks;
		std::map<Text, Track<MorphKey>>		morph_tracks;
		Track<CameraKey>					camera_track;
		Track<LightKey>						light_track;
		Track<ShadowKey>					shadow_track;
		Track<VisibilityKey>				visibility_track;
		std::map<Text, Track<IkKey>>		ik_tracks;
	};

	namespace io {
		template<int32_t N>
		struct VectorMarker {
			float e[N];
		};

		// 
		// Reader
		//

		struct BufferReaderBase {
			const std::byte* ptr;
			const std::byte* const end;

			template<typename T>
			bool equal(const T& value) {
				auto cur = ptr;
				ptr += sizeof(T);

				if (ptr <= end) {
					return std::memcmp(cur, &value, sizeof(T)) == 0;
				}
				else {
					return false;
				}
			}

			template<typename Src, typename Dst>
			void copy(Dst* dst) {
				auto cur = ptr;
				ptr += sizeof(Src);

				if (ptr <= end) {
					std::memcpy(dst, cur, sizeof(Src));
				}
			}

			template<typename Src, typename Dst>
			void cast(Dst* dst) {
				auto cur = ptr;
				ptr += sizeof(Src);

				if (ptr <= end) {
					Src tmp;
					std::memcpy(&tmp, cur, sizeof(Src));
					*dst = static_cast<Dst>(tmp);
				}
			}

			template<typename Char, int32_t N>
			void copy_text(std::basic_string<Char>* dst) {
				int32_t len = N;

				if constexpr (N == 0) {
					copy<int32_t>(&len);
					len /= sizeof(Char);
				}

				auto cur = ptr;
				ptr += sizeof(Char) * len;

				if (ptr <= end) {
					if constexpr (N != 0) {
						// Determine the actual string length.
						if constexpr (sizeof(Char) == 1) {
							len = static_cast<int32_t>(strnlen_s(reinterpret_cast<const char*>(cur), N));
						}
						else if constexpr (sizeof(Char) == 2) {
							len = static_cast<int32_t>(wcsnlen_s(reinterpret_cast<const wchar_t*>(cur), N));
						}
					}

					dst->assign(reinterpret_cast<const Char*>(cur), len);
				}
			}

			bool is_overflown() const {
				return ptr > end;
			}

			bool is_eof() const {
				return ptr >= end;
			}
		};

		template<typename Src = void, int32_t N = 0>
		struct BufferReader : BufferReaderBase {
			template<typename T>
			auto& as() {
				return *reinterpret_cast<BufferReader<T>*>(this);
			}

			auto& as_vec2() {
				return *reinterpret_cast<BufferReader<VectorMarker<2>>*>(this);
			}

			auto& as_vec3() {
				return *reinterpret_cast<BufferReader<VectorMarker<3>>*>(this);
			}

			auto& as_vec4() {
				return *reinterpret_cast<BufferReader<VectorMarker<4>>*>(this);
			}

			template<int32_t Len = 0>
			auto& as_texta() {
				return *reinterpret_cast<BufferReader<char, Len>*>(this);
			}

			template<int32_t Len = 0>
			auto& as_textw() {
				return *reinterpret_cast<BufferReader<wchar_t, Len>*>(this);
			}

			template<typename Dst>
			auto read() {
				Dst dst;

				copy<Dst>(&dst);

				return dst;
			}

			auto read_bool() {
				return static_cast<bool>(read<uint8_t>());
			}

			auto read_i8() {
				return read<int8_t>();
			}

			auto read_u8() {
				return read<uint8_t>();
			}

			auto read_i16() {
				return read<int16_t>();
			}

			auto read_u16() {
				return read<uint16_t>();
			}

			auto read_i32() {
				return read<int32_t>();
			}

			auto read_u32() {
				return read<uint32_t>();
			}

			auto read_f32() {
				return read<float>();
			}

			auto read_f64() {
				return read<double>();
			}

			template<typename Char, int32_t N>
			auto read_text() {
				std::basic_string<Char> tmp;
				copy_text<Char, N>(&tmp);
				return tmp;
			}

			template<int32_t N = 0>
			auto read_texta() {
				return read_text<char, N>();
			}

			template<int32_t N = 0>
			auto read_textw() {
				return read_text<wchar_t, N>();
			}
		};

		template<typename Src, int32_t N, typename Dst>
		Dst& operator<<(Dst& dst, BufferReader<Src, N>& buff) {
			static_assert(std::is_trivially_copyable_v<Src> && std::is_trivially_copyable_v<Dst>);

			if constexpr (std::is_same_v<Src, Dst>) {
				buff.copy<Src>(&dst);
			}
			else {
				buff.cast<Src>(&dst);
			}

			return dst;
		}

		template<typename Dst>
		Dst& operator<<(Dst& dst, BufferReader<void>& buff) {
			static_assert(std::is_trivially_copyable_v<Dst>);

			buff.copy<Dst>(&dst);

			return dst;
		}

		template<typename Dst, int32_t N>
		Dst& operator<<(Dst& dst, BufferReader<VectorMarker<N>>& buff) {
			buff.copy<VectorMarker<N>>(&dst);
			return dst;
		}

		template<typename Char, int32_t N>
		std::basic_string<Char>& operator<<(std::basic_string<Char>& dst, BufferReader<Char, N>& buff) {
			buff.copy_text<Char, N>(&dst);
			return dst;
		}

		//
		// Writer
		//

		struct BufferWriterBase {
			std::vector<std::byte> buff{};

			std::byte* require(size_t N) {
				auto contained = buff.size();
				buff.resize(contained + N);
				return buff.data() + contained;
			}

			void skip(size_t N) {
				if (N == 0) {
					return;
				}

				buff.resize(buff.size() + N);
			}

			template<typename Dst, typename Src>
			void write(const Src& src) {
				static_assert(sizeof(Dst) <= sizeof(Src));
				std::memcpy(require(sizeof(Dst)), &src, sizeof(Dst));
			}

			template<typename Src>
			void write_array(const Src* src, size_t N) {
				auto size = sizeof(Src) * N;
				std::memcpy(require(size), src, size);
			}

			template<typename Char, int32_t N>
			void write_text(const std::basic_string<Char>& src) {
				int32_t len = N;

				if constexpr (N == 0) {
					len = static_cast<int32_t>(src.length());
					write<int32_t>(static_cast<int32_t>(sizeof(Char) * len));
				}

				if constexpr (N == 0) {
					write_array(src.c_str(), len);
				}
				else {
					len = std::min(N, static_cast<int32_t>(src.length()));

					write_array(src.c_str(), len);
					skip(sizeof(Char) * (N - len));
				}
			}

			const std::byte* data() const {
				return buff.data();
			}

			size_t size() const {
				return buff.size();
			}
		};

		template<typename Dst = void, int32_t N = 0>
		struct BufferWriter : BufferWriterBase {
			auto& as_vec2() {
				return *reinterpret_cast<BufferWriter<VectorMarker<2>>*>(this);
			}

			auto& as_vec3() {
				return *reinterpret_cast<BufferWriter<VectorMarker<3>>*>(this);
			}

			auto& as_vec4() {
				return *reinterpret_cast<BufferWriter<VectorMarker<4>>*>(this);
			}

			template<int32_t N = 0>
			auto& as_texta() {
				return *reinterpret_cast<BufferWriter<char, N>*>(this);
			}

			template<int32_t N = 0>
			auto& as_textw() {
				return *reinterpret_cast<BufferWriter<wchar_t, N>*>(this);
			}
		};

		template<typename Src>
		const Src& operator<<(BufferWriter<void>& buff, const Src& src) {
			static_assert(std::is_trivially_copyable_v<Src>);
			buff.write<Src>(src);
			return src;
		}

		template<typename Src, int32_t N>
		const Src& operator<<(BufferWriter<VectorMarker<N>>& buff, const Src& src) {
			buff.write<VectorMarker<N>>(src);
			return src;
		}

		template<typename Char, int32_t N>
		const std::basic_string<Char>& operator<<(BufferWriter<Char, N>& buff, const std::basic_string<Char>& src) {
			buff.write_text<Char, N>(src);
			return src;
		}

		//
		// File IO
		//

		template<typename Path>
		inline std::vector<std::byte> load_binary(const Path& path) {
			auto ifs = std::ifstream(path, std::ios::binary);
			if (!ifs) {
				return {};
			}

			ifs.seekg(0, std::ios::end);
			auto end = ifs.tellg();
			ifs.seekg(0, std::ios::beg);
			auto beg = ifs.tellg();

			std::vector<std::byte> bin(end - beg);
			ifs.read((char*)bin.data(), end - beg);

			return bin;
		}

		template<typename Path>
		inline bool save_binary(const Path& path, const void* data, size_t size) {
			auto ofs = std::ofstream(path, std::ios::binary);
			if (!ofs) {
				return false;
			}

			ofs.write((const char*)data, size);
			return true;
		}

		template<typename Vec2, typename Vec3, typename Vec4>
		struct PmxImporter {
			using Pmx = PmxBase<Vec2, Vec3, Vec4>;

			Pmx& pmx;
			io::BufferReader<void, 0> buff;

			PmxImporter(Pmx& pmx, const void* buff, size_t size) :
				pmx(pmx),
				buff(io::BufferReader<void, 0>{ (const std::byte*)buff, (const std::byte*)buff + size }) {}

			bool is_valid_index_size(uint8_t index_size) {
				switch (index_size) {
				case 1:
				case 2:
				case 4:
					return true;
				default:
					return false;
				}
			}

			bool import_header() {
				if (!buff.equal(Pmx::Magic) ||
					!buff.equal(Pmx::Version) ||
					!buff.equal(static_cast<uint8_t>(8))) {
					return false;
				}

				uint8_t encode = buff.read_u8();
				pmx.num_ex_uvs << buff;
				pmx.vertex_index_size << buff;
				pmx.texture_index_size << buff;
				pmx.material_index_size << buff;
				pmx.bone_index_size << buff;
				pmx.morph_index_size << buff;
				pmx.body_index_size << buff;

				if (encode != 0 ||
					!is_valid_index_size(pmx.vertex_index_size) ||
					!is_valid_index_size(pmx.texture_index_size) ||
					!is_valid_index_size(pmx.material_index_size) ||
					!is_valid_index_size(pmx.bone_index_size) ||
					!is_valid_index_size(pmx.morph_index_size) ||
					!is_valid_index_size(pmx.body_index_size)) {
					return false;
				}

				pmx.name << buff.as_textw();
				pmx.name_en << buff.as_textw();
				pmx.comment << buff.as_textw();
				pmx.comment_en << buff.as_textw();

				return !buff.is_overflown();
			}

			template<typename BoneIndex>
			bool import_vertices() {
				pmx.vertices.resize(buff.read_i32());

				for (auto& vertex : pmx.vertices) {
					vertex.position << buff.as_vec3();
					vertex.normal << buff.as_vec3();
					vertex.uv << buff.as_vec2();

					for (int i = 0; i < pmx.num_ex_uvs; ++i) {
						vertex.ex_uvs[i] << buff.as_vec4();
					}

					switch (vertex.weight_kind << buff) {
					case WeightKind::BDEF1:
						vertex.bone_indices[0] << buff.as<BoneIndex>();
						vertex.bone_indices[1] = -1;
						vertex.bone_indices[2] = -1;
						vertex.bone_indices[3] = -1;

						vertex.bone_weights[0] = 1.f;
						vertex.bone_weights[1] = 0.f;
						vertex.bone_weights[2] = 0.f;
						vertex.bone_weights[3] = 0.f;
						break;

					case WeightKind::BDEF2:
						vertex.bone_indices[0] << buff.as<BoneIndex>();
						vertex.bone_indices[1] << buff.as<BoneIndex>();
						vertex.bone_indices[2] = -1;
						vertex.bone_indices[3] = -1;

						vertex.bone_weights[0] << buff;
						vertex.bone_weights[1] = 1.f - vertex.bone_weights[0];
						vertex.bone_weights[2] = 0.f;
						vertex.bone_weights[3] = 0.f;
						break;

					case WeightKind::BDEF4:
						vertex.bone_indices[0] << buff.as<BoneIndex>();
						vertex.bone_indices[1] << buff.as<BoneIndex>();
						vertex.bone_indices[2] << buff.as<BoneIndex>();
						vertex.bone_indices[3] << buff.as<BoneIndex>();

						vertex.bone_weights[0] << buff;
						vertex.bone_weights[1] << buff;
						vertex.bone_weights[2] << buff;
						vertex.bone_weights[3] << buff;
						break;

					case WeightKind::SDEF:
						vertex.bone_indices[0] << buff.as<BoneIndex>();
						vertex.bone_indices[1] << buff.as<BoneIndex>();
						vertex.bone_indices[2] = -1;
						vertex.bone_indices[3] = -1;

						vertex.bone_weights[0] << buff;
						vertex.bone_weights[1] = 0.f;
						vertex.bone_weights[2] = 0.f;
						vertex.bone_weights[3] = 0.f;

						vertex.sdef_c << buff.as_vec3();
						vertex.sdef_r0 << buff.as_vec3();
						vertex.sdef_r1 << buff.as_vec3();
						break;

					default:
						return false;
					}

					vertex.edge << buff;
				}

				return !buff.is_overflown();
			}

			template<typename VertexIndex>
			bool import_faces() {
				pmx.faces.resize(buff.read_i32());

				if (pmx.faces.size() % 3 != 0) {
					return false;
				}

				for (auto& index : pmx.faces) {
					index << buff.as<VertexIndex>();
				}

				return !buff.is_overflown();
			}

			bool import_textures() {
				pmx.textures.resize(buff.read_i32());

				for (auto& texture : pmx.textures) {
					texture << buff.as_textw();
				}

				return !buff.is_overflown();
			}

			template<typename TextureIndex>
			bool import_materials() {
				pmx.materials.resize(buff.read_i32());

				for (auto& material : pmx.materials) {
					material.name << buff.as_textw();
					material.name_en << buff.as_textw();
					material.diffuse << buff.as_vec4();
					material.specular << buff.as_vec4();
					material.ambient << buff.as_vec3();

					uint8_t flags = buff.read_u8();
					material.two_side = flags & 0x01;
					material.cast_ground_shadow = flags & 0x02;
					material.cast_self_shadow = flags & 0x04;
					material.receive_self_shadow = flags & 0x08;
					material.draw_edge = flags & 0x10;

					material.edge_color << buff.as_vec4();
					material.edge_size << buff;
					material.base_texture_index << buff.as<TextureIndex>();
					material.sphere_texture_index << buff.as<TextureIndex>();
					material.sphere_mode << buff;
					material.use_shared_toon << buff;
					if (material.use_shared_toon) {
						material.toon_texture_index << buff.as<uint8_t>();
					}
					else {
						material.toon_texture_index << buff.as<TextureIndex>();
					}
					material.note << buff.as_textw();
					material.num_vertices << buff;

					if (material.num_vertices % 3 != 0) {
						return false;
					}
				}

				return !buff.is_overflown();
			}

			template<typename BoneIndex>
			bool import_bones() {
				pmx.bones.resize(buff.read_i32());

				for (auto& bone : pmx.bones) {
					bone.name << buff.as_textw();
					bone.name_en << buff.as_textw();
					bone.position << buff.as_vec3();
					bone.parent_bone_index << buff.as<BoneIndex>();
					bone.level << buff;

					uint16_t flags = buff.read_u16();
					bone.has_tip_bone = flags & 0x0001;
					bone.rotatable = flags & 0x0002;
					bone.translatable = flags & 0x0004;
					bone.visible = flags & 0x0008;
					bone.operable = flags & 0x0010;
					bone.is_ik = flags & 0x0020;
					bone.local_driven = flags & 0x0080;
					bone.driven_rotation = flags & 0x0100;
					bone.driven_translation = flags & 0x0200;
					bone.has_fixed_axis = flags & 0x0400;
					bone.has_local_axis = flags & 0x0800;
					bone.post_physics_transform = flags & 0x1000;
					bone.external_transform = flags & 0x2000;

					if (bone.has_tip_bone) {
						bone.tip_offset = {};
						bone.tip_bone_index << buff.as<BoneIndex>();
					}
					else {
						bone.tip_offset << buff.as_vec3();
						bone.tip_bone_index = -1;
					}

					if (bone.driven_rotation || bone.driven_translation) {
						bone.drive_bone_index << buff.as<BoneIndex>();
						bone.drive_rate << buff;
					}
					else {
						bone.drive_bone_index = -1;
						bone.drive_rate = 0.f;
					}

					if (bone.has_fixed_axis) {
						bone.fixed_axis << buff.as_vec3();
					}
					else {
						bone.fixed_axis = {};
					}

					if (bone.has_local_axis) {
						bone.local_axis_x << buff.as_vec3();
						bone.local_axis_z << buff.as_vec3();
					}
					else {
						bone.local_axis_x = {};
						bone.local_axis_z = {};
					}

					if (bone.external_transform) {
						bone.external_key << buff;
					}
					else {
						bone.external_key = 0;
					}

					if (bone.is_ik) {
						bone.ik_target_bone_index << buff.as<BoneIndex>();
						bone.ik_iteration_count << buff;
						bone.ik_angle_limit << buff;

						bone.ik_links.resize(buff.read_i32());

						for (auto& ik_link : bone.ik_links) {
							ik_link.index << buff.as<BoneIndex>();
							ik_link.angle_limited << buff;

							if (ik_link.angle_limited) {
								ik_link.angle_min << buff.as_vec3();
								ik_link.angle_max << buff.as_vec3();
							}
						}
					}
					else {
						bone.ik_target_bone_index = -1;
					}
				}

				return !buff.is_overflown();
			}

			template<typename VertexIndex>
			void import_vertex_morph(const std::wstring& name, const std::wstring& name_en, MorphPanel panel) {
				std::vector<Pmx::VertexMorphData> morph_data(buff.read_i32());

				for (auto& data : morph_data) {
					data.index << buff.as<VertexIndex>();
					data.offset << buff.as_vec3();
				}

				pmx.vertex_morphs.emplace_back(name, name_en, panel, MorphKind::Vertex, std::move(morph_data));
			}

			template<typename VertexIndex>
			void import_uv_morph(const std::wstring& name, const std::wstring& name_en, MorphPanel panel, MorphKind kind) {
				std::vector<Pmx::UvMorphData> morph_data(buff.read_i32());

				for (auto& data : morph_data) {
					data.index << buff.as<VertexIndex>();
					data.offset << buff.as_vec4();
				}

				pmx.uv_morphs.emplace_back(name, name_en, panel, kind, std::move(morph_data));
			}

			template<typename BoneIndex>
			void import_bone_morph(const std::wstring& name, const std::wstring& name_en, MorphPanel panel) {
				std::vector<Pmx::BoneMorphData> morph_data(buff.read_i32());

				for (auto& data : morph_data) {
					data.index << buff.as<BoneIndex>();
					data.translation << buff.as_vec3();
					data.rotation << buff.as_vec4();
				}

				pmx.bone_morphs.emplace_back(name, name_en, panel, MorphKind::Bone, std::move(morph_data));
			}

			template<typename MaterialIndex>
			void import_material_morph(const std::wstring& name, const std::wstring& name_en, MorphPanel panel) {
				std::vector<Pmx::MaterialMorphData> morph_data(buff.read_i32());

				for (auto& data : morph_data) {
					data.index << buff.as<MaterialIndex>();
					data.op << buff;
					data.diffuse << buff.as_vec4();
					data.specular << buff.as_vec4();
					data.ambient << buff.as_vec4();
					data.edge_color << buff.as_vec4();
					data.edge_size << buff;
					data.tex << buff.as_vec4();
					data.sphere << buff.as_vec4();
					data.toon << buff.as_vec4();
				}

				pmx.material_morphs.emplace_back(name, name_en, panel, MorphKind::Material, std::move(morph_data));
			}

			template<typename MorphIndex>
			void import_group_morph(std::wstring& name, std::wstring& name_en, MorphPanel panel) {
				std::vector<Pmx::GroupMorphData> morph_data(buff.read_i32());

				for (auto& data : morph_data) {
					data.index << buff.as<MorphIndex>();
					data.rate << buff;
				}

				pmx.group_morphs.emplace_back(name, name_en, panel, MorphKind::Group, std::move(morph_data));
			}

			bool import_morphs() {
				const int32_t num_morphs = buff.read_i32();

				std::wstring name{}, name_en{};
				MorphPanel panel{};
				MorphKind kind{};

				for (int i = 0; i < num_morphs; ++i) {
					name << buff.as_textw();
					name_en << buff.as_textw();
					panel = buff.read<MorphPanel>();
					kind = buff.read<MorphKind>();

					switch (kind) {
					case MorphKind::Group:
						switch (pmx.morph_index_size) {
						case 1: import_group_morph<int8_t>(name, name_en, panel); break;
						case 2: import_group_morph<int16_t>(name, name_en, panel); break;
						case 4: import_group_morph<int32_t>(name, name_en, panel); break;
						}
						break;

					case MorphKind::Vertex:
						switch (pmx.vertex_index_size) {
						case 1: import_vertex_morph<uint8_t>(name, name_en, panel); break;
						case 2: import_vertex_morph<uint16_t>(name, name_en, panel); break;
						case 4: import_vertex_morph<int32_t>(name, name_en, panel); break;
						}
						break;

					case MorphKind::Bone:
						switch (pmx.bone_index_size) {
						case 1: import_bone_morph<int8_t>(name, name_en, panel); break;
						case 2: import_bone_morph<int16_t>(name, name_en, panel); break;
						case 4: import_bone_morph<int32_t>(name, name_en, panel); break;
						}
						break;

					case MorphKind::UV:
					case MorphKind::ExUV1:
					case MorphKind::ExUV2:
					case MorphKind::ExUV3:
					case MorphKind::ExUV4:
						switch (pmx.vertex_index_size) {
						case 1: import_uv_morph<uint8_t>(name, name_en, panel, kind); break;
						case 2: import_uv_morph<uint16_t>(name, name_en, panel, kind); break;
						case 4: import_uv_morph<int32_t>(name, name_en, panel, kind); break;
						}
						break;

					case MorphKind::Material:
						switch (pmx.material_index_size) {
						case 1: import_material_morph<int8_t>(name, name_en, panel); break;
						case 2: import_material_morph<int16_t>(name, name_en, panel); break;
						case 4: import_material_morph<int32_t>(name, name_en, panel); break;
						}
						break;
					}
				}

				return !buff.is_overflown();
			}

			template<typename BoneIndex, typename MorphIndex>
			bool import_nodes() {
				pmx.nodes.resize(buff.read_i32());

				for (auto& node : pmx.nodes) {
					node.name << buff.as_textw();
					node.name_en << buff.as_textw();

					node.special << buff;

					node.items.resize(buff.read_i32());
					for (auto& item : node.items) {
						switch (item.kind << buff) {
						case NodeKind::Bone: item.index << buff.as<BoneIndex>(); break;
						case NodeKind::Morph: item.index << buff.as<MorphIndex>(); break;
						default: return false;
						}
					}
				}

				return !buff.is_overflown();
			}

			template<typename BoneIndex>
			bool import_bodies() {
				pmx.bodies.resize(buff.read_i32());

				for (auto& body : pmx.bodies) {
					body.name << buff.as_textw();
					body.name_en << buff.as_textw();
					body.index << buff.as<BoneIndex>();
					body.group << buff;
					body.pass_group << buff;
					body.shape << buff;
					body.size << buff.as_vec3();
					body.position << buff.as_vec3();
					body.rotation << buff.as_vec3();
					body.mass << buff;
					body.linear_damping << buff;
					body.angular_damping << buff;
					body.restitution << buff;
					body.friction << buff;
					body.mode << buff;
				}

				return !buff.is_overflown();
			}

			template<typename BodyIndex>
			bool import_joints() {
				pmx.joints.resize(buff.read_i32());

				for (auto& joint : pmx.joints) {
					joint.name << buff.as_textw();
					joint.name_en << buff.as_textw();

					switch (joint.kind << buff) {
					case JointKind::Spring6DOF:
						joint.body_index_a << buff.as<BodyIndex>();
						joint.body_index_b << buff.as<BodyIndex>();
						joint.position << buff.as_vec3();
						joint.rotation << buff.as_vec3();
						joint.linear_min << buff.as_vec3();
						joint.linear_max << buff.as_vec3();
						joint.angular_min << buff.as_vec3();
						joint.angular_max << buff.as_vec3();
						joint.linear_spring_const << buff.as_vec3();
						joint.angular_spring_const << buff.as_vec3();
						break;

					default:
						return false;
					}
				}

				return !buff.is_overflown();
			}

			bool import_pmx() {
				bool ret = import_header();

				if (ret) {
					switch (pmx.bone_index_size) {
					case 1: ret = import_vertices<int8_t>(); break;
					case 2: ret = import_vertices<int16_t>(); break;
					case 4: ret = import_vertices<int32_t>(); break;
					}
				}

				if (ret) {
					switch (pmx.vertex_index_size) {
					case 1: ret = import_faces<uint8_t>(); break;
					case 2: ret = import_faces<uint16_t>(); break;
					case 4: ret = import_faces<int32_t>(); break;
					}
				}

				if (ret) {
					ret = import_textures();
				}

				if (ret) {
					switch (pmx.texture_index_size) {
					case 1: ret = import_materials<int8_t>(); break;
					case 2: ret = import_materials<int16_t>(); break;
					case 4: ret = import_materials<int32_t>(); break;
					}
				}

				if (ret) {
					switch (pmx.bone_index_size) {
					case 1: ret = import_bones<int8_t>(); break;
					case 2: ret = import_bones<int16_t>(); break;
					case 4: ret = import_bones<int32_t>(); break;
					}
				}

				if (ret) {
					ret = import_morphs();
				}

				if (ret) {
					switch (pmx.bone_index_size) {
					case 1:
						switch (pmx.morph_index_size) {
						case 1: ret = import_nodes<int8_t, int8_t>(); break;
						case 2: ret = import_nodes<int8_t, int16_t>(); break;
						case 4: ret = import_nodes<int8_t, int32_t>(); break;
						}
						break;

					case 2:
						switch (pmx.morph_index_size) {
						case 1: ret = import_nodes<int16_t, int8_t>(); break;
						case 2: ret = import_nodes<int16_t, int16_t>(); break;
						case 4: ret = import_nodes<int16_t, int32_t>(); break;
						}
						break;

					case 4:
						switch (pmx.morph_index_size) {
						case 1: ret = import_nodes<int32_t, int8_t>(); break;
						case 2: ret = import_nodes<int32_t, int16_t>(); break;
						case 4: ret = import_nodes<int32_t, int32_t>(); break;
						}
						break;
					}
				}

				if (ret) {
					switch (pmx.bone_index_size) {
					case 1: ret = import_bodies<int8_t>(); break;
					case 2: ret = import_bodies<int16_t>(); break;
					case 4: ret = import_bodies<int32_t>(); break;
					}
				}

				if (ret) {
					switch (pmx.body_index_size) {
					case 1: ret = import_joints<int8_t>(); break;
					case 2: ret = import_joints<int16_t>(); break;
					case 4: ret = import_joints<int32_t>(); break;
					}
				}

				return ret && buff.is_eof() && !buff.is_overflown();
			}
		};

		template<typename Vec2, typename Vec3, typename Vec4>
		struct PmxExporter {
			using Pmx = PmxBase<Vec2, Vec3, Vec4>;

			const Pmx& pmx;
			io::BufferWriter<void, 0> buff;

			PmxExporter(const Pmx& pmx) :
				pmx(pmx) {}

			bool export_header() {
				buff << Pmx::Magic;
				buff << Pmx::Version;

				buff << static_cast<uint8_t>(8); // num ex data
				buff << static_cast<uint8_t>(0); // encode
				buff << static_cast<uint8_t>(pmx.num_ex_uvs);
				buff << static_cast<uint8_t>(4);
				buff << static_cast<uint8_t>(4);
				buff << static_cast<uint8_t>(4);
				buff << static_cast<uint8_t>(4);
				buff << static_cast<uint8_t>(4);
				buff << static_cast<uint8_t>(4);

				buff.as_textw() << pmx.name;
				buff.as_textw() << pmx.name_en;
				buff.as_textw() << pmx.comment;
				buff.as_textw() << pmx.comment_en;

				return true;
			}

			bool export_vertices() {
				buff << static_cast<int32_t>(pmx.vertices.size());

				for (auto& vertex : pmx.vertices) {
					buff.as_vec3() << vertex.position;
					buff.as_vec3() << vertex.normal;
					buff.as_vec2() << vertex.uv;

					for (int i = 0; i < pmx.num_ex_uvs; ++i) {
						buff.as_vec4() << vertex.ex_uvs[i];
					}

					switch (buff << vertex.weight_kind) {
					case WeightKind::BDEF1:
						buff << vertex.bone_indices[0];
						break;

					case WeightKind::BDEF2:
						buff << vertex.bone_indices[0];
						buff << vertex.bone_indices[1];

						buff << vertex.bone_weights[0];
						break;

					case WeightKind::BDEF4:
						buff << vertex.bone_indices[0];
						buff << vertex.bone_indices[1];
						buff << vertex.bone_indices[2];
						buff << vertex.bone_indices[3];

						buff << vertex.bone_weights[0];
						buff << vertex.bone_weights[1];
						buff << vertex.bone_weights[2];
						buff << vertex.bone_weights[3];
						break;

					case WeightKind::SDEF:
						buff << vertex.bone_indices[0];
						buff << vertex.bone_indices[1];

						buff << vertex.bone_weights[0];
						buff.as_vec3() << vertex.sdef_c;
						buff.as_vec3() << vertex.sdef_r0;
						buff.as_vec3() << vertex.sdef_r1;
						break;
					}

					buff << vertex.edge;
				}

				return true;
			}

			bool export_faces() {
				buff << static_cast<int32_t>(pmx.faces.size());
				buff.write_array<int32_t>(pmx.faces.data(), pmx.faces.size());
				return true;
			}

			bool export_textures() {
				buff << static_cast<int32_t>(pmx.textures.size());

				for (auto& texture : pmx.textures) {
					buff.as_textw() << texture;
				}

				return true;
			}

			bool export_materials() {
				buff << static_cast<int32_t>(pmx.materials.size());

				for (auto& material : pmx.materials) {
					buff.as_textw() << material.name;
					buff.as_textw() << material.name_en;

					buff.as_vec4() << material.diffuse;
					buff.as_vec4() << material.specular;
					buff.as_vec3() << material.ambient;

					uint8_t flags = 0;
					flags |= material.two_side << 0;
					flags |= material.cast_ground_shadow << 1;
					flags |= material.cast_self_shadow << 2;
					flags |= material.receive_self_shadow << 3;
					flags |= material.draw_edge << 4;
					buff << flags;

					buff.as_vec4() << material.edge_color;
					buff << material.edge_size;
					buff << material.base_texture_index;
					buff << material.sphere_texture_index;
					buff << material.sphere_mode;
					buff << material.use_shared_toon;

					if (material.use_shared_toon == 0) {
						buff << material.toon_texture_index;
					}
					else {
						buff << static_cast<int8_t>(material.toon_texture_index);
					}

					buff.as_textw() << material.note;
					buff << material.num_vertices;
				}

				return true;
			}

			bool export_bones() {
				buff << static_cast<int32_t>(pmx.bones.size());

				for (auto& bone : pmx.bones) {
					buff.as_textw() << bone.name;
					buff.as_textw() << bone.name_en;

					buff.as_vec3() << bone.position;
					buff << bone.parent_bone_index;
					buff << bone.level;

					uint16_t flags = 0;
					flags |= bone.has_tip_bone << 0;
					flags |= bone.rotatable << 1;
					flags |= bone.translatable << 2;
					flags |= bone.visible << 3;
					flags |= bone.operable << 4;
					flags |= bone.is_ik << 5;
					flags |= bone.local_driven << 7;
					flags |= bone.driven_rotation << 8;
					flags |= bone.driven_translation << 9;
					flags |= bone.has_fixed_axis << 10;
					flags |= bone.has_local_axis << 11;
					flags |= bone.post_physics_transform << 12;
					flags |= bone.external_transform << 13;
					buff << flags;

					if (bone.has_tip_bone) {
						buff << bone.tip_bone_index;
					}
					else {
						buff.as_vec3() << bone.tip_offset;
					}

					if (bone.driven_rotation || bone.driven_translation) {
						buff << bone.drive_bone_index;
						buff << bone.drive_rate;
					}

					if (bone.has_fixed_axis) {
						buff.as_vec3() << bone.fixed_axis;
					}

					if (bone.has_local_axis) {
						buff.as_vec3() << bone.local_axis_x;
						buff.as_vec3() << bone.local_axis_z;
					}

					if (bone.external_transform) {
						buff << bone.external_key;
					}

					if (bone.is_ik) {
						buff << bone.ik_target_bone_index;
						buff << bone.ik_iteration_count;
						buff << bone.ik_angle_limit;

						buff << static_cast<uint32_t>(bone.ik_links.size());
						for (auto& ik : bone.ik_links) {
							buff << ik.index;
							buff << ik.angle_limited;

							if (ik.angle_limited) {
								buff.as_vec3() << ik.angle_min;
								buff.as_vec3() << ik.angle_max;
							}
						}
					}
				}

				return true;
			}

			bool export_vertex_morphs() {
				for (auto& morph : pmx.vertex_morphs) {
					buff.as_textw() << morph.name;
					buff.as_textw() << morph.name_en;
					buff << morph.panel;
					buff << morph.kind;

					buff << static_cast<int32_t>(morph.data.size());
					for (auto& data : morph.data) {
						buff << data.index;
						buff.as_vec3() << data.offset;
					}
				}

				return true;
			}

			bool export_uv_morphs() {
				for (auto& morph : pmx.uv_morphs) {
					buff.as_textw() << morph.name;
					buff.as_textw() << morph.name_en;
					buff << morph.panel;
					buff << morph.kind;

					buff << static_cast<int32_t>(morph.data.size());
					for (auto& data : morph.data) {
						buff << data.index;
						buff.as_vec4() << data.offset;
					}
				}

				return true;
			}

			bool export_bone_morphs() {
				for (auto& morph : pmx.bone_morphs) {
					buff.as_textw() << morph.name;
					buff.as_textw() << morph.name_en;
					buff << morph.panel;
					buff << morph.kind;

					buff << static_cast<int32_t>(morph.data.size());
					for (auto& data : morph.data) {
						buff << data.index;
						buff.as_vec3() << data.translation;
						buff.as_vec4() << data.rotation;
					}
				}

				return true;
			}

			bool export_material_morphs() {
				for (auto& morph : pmx.material_morphs) {
					buff.as_textw() << morph.name;
					buff.as_textw() << morph.name_en;
					buff << morph.panel;
					buff << morph.kind;

					buff << static_cast<int32_t>(morph.data.size());
					for (auto& data : morph.data) {
						buff << data.index;
						buff << data.op;
						buff.as_vec4() << data.diffuse;
						buff.as_vec4() << data.specular;
						buff.as_vec3() << data.ambient;
						buff.as_vec4() << data.edge_color;
						buff << data.edge_size;
						buff.as_vec4() << data.tex;
						buff.as_vec4() << data.sphere;
						buff.as_vec4() << data.toon;
					}
				}

				return true;
			}

			bool export_group_morphs() {
				for (auto& morph : pmx.group_morphs) {
					buff.as_textw() << morph.name;
					buff.as_textw() << morph.name_en;
					buff << morph.panel;
					buff << morph.kind;

					buff << static_cast<int32_t>(morph.data.size());
					for (auto& data : morph.data) {
						buff << data.index;
						buff << data.rate;
					}
				}

				return true;
			}

			bool export_morphs() {
				buff << static_cast<int32_t>(pmx.vertex_morphs.size() + pmx.uv_morphs.size() + pmx.bone_morphs.size() + pmx.material_morphs.size() + pmx.group_morphs.size());

				export_vertex_morphs();
				export_uv_morphs();
				export_bone_morphs();
				export_material_morphs();
				export_group_morphs();

				return true;
			}

			bool export_nodes() {
				buff << static_cast<int32_t>(pmx.nodes.size());

				for (auto& node : pmx.nodes) {
					buff.as_textw() << node.name;
					buff.as_textw() << node.name_en;
					buff << node.special;
					buff << static_cast<int32_t>(node.items.size());

					for (auto& item : node.items) {
						buff << item.kind;
						buff << item.index;
					}
				}

				return true;
			}

			bool export_bodies() {
				buff << static_cast<int32_t>(pmx.bodies.size());

				for (auto& body : pmx.bodies) {
					buff.as_textw() << body.name;
					buff.as_textw() << body.name_en;
					buff << body.index;
					buff << body.group;
					buff << body.pass_group;
					buff << body.shape;
					buff.as_vec3() << body.size;
					buff.as_vec3() << body.position;
					buff.as_vec3() << body.rotation;
					buff << body.mass;
					buff << body.linear_damping;
					buff << body.angular_damping;
					buff << body.restitution;
					buff << body.friction;
					buff << body.mode;
				}

				return true;
			}

			bool export_joints() {
				buff << static_cast<int32_t>(pmx.joints.size());

				for (auto& joint : pmx.joints) {
					buff.as_textw() << joint.name;
					buff.as_textw() << joint.name_en;

					switch (buff << joint.kind) {
					case JointKind::Spring6DOF:
						buff << joint.body_index_a;
						buff << joint.body_index_b;
						buff.as_vec3() << joint.position;
						buff.as_vec3() << joint.rotation;
						buff.as_vec3() << joint.linear_min;
						buff.as_vec3() << joint.linear_max;
						buff.as_vec3() << joint.angular_min;
						buff.as_vec3() << joint.angular_max;
						buff.as_vec3() << joint.linear_spring_const;
						buff.as_vec3() << joint.angular_spring_const;
						break;
					}
				}

				return true;
			}

			bool export_pmx() {
				bool ret = export_header();
				if (ret) ret = export_vertices();
				if (ret) ret = export_faces();
				if (ret) ret = export_textures();
				if (ret) ret = export_materials();
				if (ret) ret = export_bones();
				if (ret) ret = export_morphs();
				if (ret) ret = export_nodes();
				if (ret) ret = export_bodies();
				if (ret) ret = export_joints();

				return ret;
			}
		};

		template<typename Vec3, typename Vec4>
		struct VmdImporter {
			using Vmd = VmdBase<Vec3, Vec4>;

			Vmd& vmd;
			io::BufferReader<void, 0> buff;

			VmdImporter(Vmd& vmd, const void* data, size_t size) :
				vmd(vmd),
				buff(io::BufferReader<void, 0> { (const std::byte*)data, (const std::byte*)data + size }) {}

			bool import_header() {
				if (!buff.equal(Vmd::Magic)) {
					return false;
				}

				vmd.name << buff.as_texta<20>();

				return !buff.is_overflown();
			}

			bool import_motions() {
				for (uint32_t i = 0, num_keys = buff.read_u32(); i < num_keys; ++i) {
					auto name = buff.read_texta<15>();
					auto& key = vmd.motion_tracks[name].add();
					key.frame << buff;
					key.position << buff.as_vec3();
					key.orientation << buff.as_vec4();
					key.interpolation << buff;
					key.ix.x1 = key.interpolation[0 + 0];	key.ix.y1 = key.interpolation[0 + 4];	key.ix.x2 = key.interpolation[0 + 8];	key.ix.y2 = key.interpolation[0 + 12];
					key.iy.x1 = key.interpolation[16 + 0];	key.iy.y1 = key.interpolation[16 + 4];	key.iy.x2 = key.interpolation[16 + 8];	key.iy.y2 = key.interpolation[16 + 12];
					key.iz.x1 = key.interpolation[32 + 0];	key.iz.y1 = key.interpolation[32 + 4];	key.iz.x2 = key.interpolation[32 + 8];	key.iz.y2 = key.interpolation[32 + 12];
					key.ir.x1 = key.interpolation[48 + 0];	key.ir.y1 = key.interpolation[48 + 4];	key.ir.x2 = key.interpolation[48 + 8];	key.ir.y2 = key.interpolation[48 + 12];
					key.is_physics = !(key.interpolation[2] == 0x63 && key.interpolation[3] == 0x0f);
				}

				for (auto& [name, track] : vmd.motion_tracks) {
					track.sort();
				}

				return !buff.is_overflown();
			}

			bool import_morphs() {
				for (uint32_t i = 0, num_keys = buff.read_u32(); i < num_keys; ++i) {
					auto name = buff.read_texta<15>();
					auto& key = vmd.morph_tracks[name].add();
					key.frame << buff;
					key.value << buff;
				}

				// Remove unused track.
				for (auto it = vmd.morph_tracks.begin(); it != vmd.morph_tracks.end();) {
					auto& keys = it->second;

					if (keys.size() == 1 && (keys.front().frame == 0 && keys.front().value == 0.f)) {
						it = vmd.morph_tracks.erase(it);
					}
					else {
						++it;
					}
				}

				for (auto& [name, track] : vmd.morph_tracks) {
					track.sort();
				}

				return !buff.is_overflown();
			}

			bool import_cameras() {
				vmd.camera_track.resize(buff.read_u32());

				for (auto& key : vmd.camera_track) {
					key.frame << buff;
					key.distance << buff;
					key.position << buff.as_vec3();
					key.rotation << buff.as_vec3();
					key.ix << buff;
					key.iy << buff;
					key.iz << buff;
					key.ir << buff;
					key.id << buff;
					key.iv << buff;
					key.view_angle << buff;
					key.orthographic << buff;
				}

				vmd.camera_track.sort();

				return !buff.is_overflown();
			}

			bool import_lights() {
				vmd.light_track.resize(buff.read_u32());

				for (auto& key : vmd.light_track) {
					key.frame << buff;
					key.color << buff.as_vec3();
					key.position << buff.as_vec3();
				}

				vmd.light_track.sort();

				return !buff.is_overflown();
			}

			bool import_shadows() {
				vmd.shadow_track.resize(buff.read_u32());

				for (auto& key : vmd.shadow_track) {
					key.frame << buff;
					key.type << buff;
					key.distance << buff;
				}

				vmd.shadow_track.sort();

				return !buff.is_overflown();
			}

			bool import_ex_keys() {
				for (uint32_t i = 0, num_keys = buff.read_u32(); i < num_keys; ++i) {
					uint32_t frame = buff.read_u32();

					auto& visibility_key = vmd.visibility_track.add();
					visibility_key.frame = frame;
					visibility_key.visible << buff;

					for (uint32_t j = 0, num_iks = buff.read_u32(); j < num_iks; ++j) {
						auto name = buff.read_texta<20>();
						auto& ik_key = vmd.ik_tracks[name].add();
						ik_key.frame = frame;
						ik_key.enable << buff;
					}
				}

				vmd.visibility_track.sort();

				for (auto& [name, track] : vmd.ik_tracks) {
					track.sort();
				}

				return !buff.is_overflown();
			}

			bool import_vmd() {
				bool ret = import_header();
				if (ret) ret = import_motions();
				if (ret) ret = import_morphs();
				if (ret) ret = import_cameras();
				if (ret) ret = import_lights();
				if (ret) ret = import_shadows();
				if (ret) ret = import_ex_keys();

				return ret && buff.is_eof() && !buff.is_overflown();
			}
		};

		template<typename Vec3, typename Vec4>
		struct VmdExporter {
			using Vmd = VmdBase<Vec3, Vec4>;

			const Vmd& vmd;
			io::BufferWriter<void, 0> buff;

			VmdExporter(const Vmd& vmd) :
				vmd(vmd) {}

			bool export_header() {
				buff << Vmd::Magic;
				buff.as_texta<20>() << vmd.name;

				return true;
			}

			bool export_motions() {
				int8_t interp[64]{};

				uint32_t num_keys = 0;
				for (auto& [name, track] : vmd.motion_tracks) {
					num_keys += static_cast<uint32_t>(track.size());
				}
				buff << num_keys;

				for (auto& [name, track] : vmd.motion_tracks) {
					for (auto& key : track) {
						buff.as_texta<15>() << name;
						buff << key.frame;
						buff.as_vec3() << key.position;
						buff.as_vec4() << key.orientation;

						std::memcpy(interp, key.interpolation, 64);
						interp[0 + 0] = key.ix.x1;	interp[0 + 4] = key.ix.y1;	interp[0 + 8] = key.ix.x2;	interp[0 + 12] = key.ix.y2;
						interp[16 + 0] = key.iy.x1;	interp[16 + 4] = key.iy.y1;	interp[16 + 8] = key.iy.x2;	interp[16 + 12] = key.iy.y2;
						interp[32 + 0] = key.iz.x1;	interp[32 + 4] = key.iz.y1;	interp[32 + 8] = key.iz.x2;	interp[32 + 12] = key.iz.y2;
						interp[48 + 0] = key.ir.x1;	interp[48 + 4] = key.ir.y1;	interp[48 + 8] = key.ir.x2;	interp[48 + 12] = key.ir.y2;

						if (key.is_physics) {
							interp[2] = 0x63;
							interp[3] = 0x0f;
						}

						buff << interp;
					}
				}

				return true;
			}

			bool export_morphs() {
				uint32_t num_keys = 0;
				for (auto& [name, keys] : vmd.morph_tracks) {
					num_keys += static_cast<uint32_t>(keys.size());
				}
				buff << num_keys;

				for (auto& [name, track] : vmd.morph_tracks) {
					for (auto& key : track) {
						buff.as_texta<15>() << name;
						buff << key.frame;
						buff << key.value;
					}
				}

				return true;
			}

			bool export_cameras() {
				buff << static_cast<uint32_t>(vmd.camera_track.size());

				for (auto& key : vmd.camera_track) {
					buff << key.frame;
					buff << key.distance;
					buff.as_vec3() << key.position;
					buff.as_vec3() << key.rotation;
					buff << key.ix;
					buff << key.iy;
					buff << key.iz;
					buff << key.ir;
					buff << key.id;
					buff << key.iv;
					buff << key.view_angle;
					buff << static_cast<uint8_t>(key.orthographic);
				}

				return true;
			}

			bool export_lights() {
				buff << static_cast<uint32_t>(vmd.light_track.size());

				for (auto& key : vmd.light_track) {
					buff << key.frame;
					buff.as_vec3() << key.color;
					buff.as_vec3() << key.position;
				}

				return true;
			}

			bool export_shadows() {
				buff << static_cast<uint32_t>(vmd.shadow_track.size());

				for (auto& key : vmd.shadow_track) {
					buff << key.frame;
					buff << key.type;
					buff << key.distance;
				}

				return true;
			}

			bool export_ex_keys() {
				struct ExKey {
					bool visible = true;
					std::map<typename Vmd::Text, bool> iks;
				};
				std::map<uint32_t, ExKey> ex_keys{}; // frame - ex_key map

				// Integrate visibility and ik keys
				{
					for (auto& key : vmd.visibility_track) {
						ex_keys[key.frame].visible = key.visible;
					}

					for (auto& [name, track] : vmd.ik_tracks) {
						for (auto& key : track) {
							ex_keys[key.frame].iks[name] = key.enable;
						}
					}
				}

				for (auto& [frame, key] : ex_keys) {
					buff << frame;
					buff << key.visible;
					buff << static_cast<uint32_t>(key.iks.size());

					for (auto& [name, ik] : key.iks) {
						buff.as_texta<20>() << name;
						buff << static_cast<uint8_t>(ik);
					}
				}

				return true;
			}

			bool export_vmd() {
				bool ret = export_header();
				if (ret) ret = export_motions();
				if (ret) ret = export_morphs();
				if (ret) ret = export_cameras();
				if (ret) ret = export_lights();
				if (ret) ret = export_shadows();
				if (ret) ret = export_ex_keys();

				return ret;
			}
		};

	} // namespace io

	template<typename Pmx, typename Path>
	inline bool import_pmx(const Path& path, Pmx& pmx) {
		auto bin = io::load_binary(path);
		io::PmxImporter importer(pmx, bin.data(), bin.size());
		return importer.import_pmx();
	}

	template<typename Pmx>
	inline bool import_pmx(const void* data, size_t size, Pmx& pmx) {
		io::PmxImporter importer(pmx, data, size);
		return importer.import_pmx();
	}

	template<typename Pmx, typename Path>
	inline bool export_pmx(const Pmx& pmx, const Path& path) {
		io::PmxExporter exporter(pmx);
		if (!exporter.export_pmx()) {
			return false;
		}

		return io::save_binary(path, exporter.buff.data(), exporter.buff.size());
	}

	template<typename Pmx, typename Elem = std::byte>
	inline bool export_pmx(const Pmx& pmx, Track<Elem>& bin) {
		io::PmxExporter exporter(pmx);
		if (!exporter.export_pmx()) {
			return false;
		}

		bin.resize(exporter.buff.size());
		std::memcpy(bin.data(), exporter.buff.data(), exporter.buff.size());
		return true;
	}

	template<typename Vmd, typename Path>
	inline bool import_vmd(const Path& path, Vmd& vmd) {
		auto bin = io::load_binary(path);
		io::VmdImporter importer(vmd, bin.data(), bin.size());
		return importer.import_vmd();
	}

	template<typename Vmd>
	inline bool import_vmd(const void* data, size_t size, Vmd& vmd) {
		io::VmdImporter importer(vmd, data, size);
		return importer.import_vmd();
	}

	template<typename Vmd, typename Path>
	inline bool export_vmd(const Vmd& vmd, const Path& path) {
		io::VmdExporter exporter(vmd);
		if (!exporter.export_vmd()) {
			return false;
		}

		return io::save_binary(path, exporter.buff.data(), exporter.buff.size());
	}

	template<typename Vmd, typename Elem = std::byte>
	inline bool export_vmd(const Vmd& vmd, Track<Elem>& bin) {
		io::VmdExporter exporter(vmd);
		if (!exporter.export_vmd()) {
			return false;
		}

		bin.resize(exporter.buff.size());
		std::memcpy(bin.data(), exporter.buff.data(), exporter.buff.size());
		return true;
	}

} // namespace poml 
