#include "stdafx.h"
#include "ecs_character.h"
#include "graphics.h"
#include "random.h"

namespace ecs
{
	const int C3_LUT_COLORS = 48;
	const int C4_LUT_COLORS = 58;

	const int SKIN_COLORS = 18;
	const int HAIR_COLORS = 58;
	const int SOCK_COLORS = C3_LUT_COLORS;
	const int SHOE_COLORS = C3_LUT_COLORS;
	const int LOWERWEAR_COLORS = C3_LUT_COLORS;
	const int SHIRT_COLORS = C3_LUT_COLORS;
	const int GLOVES_COLORS = C3_LUT_COLORS;
	const int OUTERWEAR_COLORS = C3_LUT_COLORS;
	const int NECKWEAR_COLORS_1 = C3_LUT_COLORS;
	const int NECKWEAR_COLORS_2 = C4_LUT_COLORS;
	const int GLASSES_COLORS = C3_LUT_COLORS;
	const int HAT_COLORS_1 = C3_LUT_COLORS;
	const int HAT_COLORS_2 = C4_LUT_COLORS;

	extern entt::registry _registry;

	Character& emplace_character(entt::entity entity, const Character& ch) {
		return _registry.emplace<Character>(entity, ch);
	}

	Character& get_character(entt::entity entity) {
		return _registry.get<Character>(entity);
	}

	Character* try_get_character(entt::entity entity) {
		return _registry.try_get<Character>(entity);
	}

	bool remove_character(entt::entity entity) {
		return _registry.remove<Character>(entity);
	}

	bool has_character(entt::entity entity) {
		return _registry.all_of<Character>(entity);
	}

	void randomize_character(Character& ch)
	{
		ch.body = (Character::Body)(random::range_i(1, (int)Character::Body::Count - 1));
		ch.skin_color = random::range_i(0, SKIN_COLORS - 1);
		ch.sock = (Character::Sock)(random::range_i(0, (int)Character::Sock::Count - 1));
		ch.sock_color = random::range_i(0, SOCK_COLORS - 1);
		ch.shoe = (Character::Shoe)(random::range_i(0, (int)Character::Shoe::Count - 1));
		ch.shoe_color = random::range_i(0, SHOE_COLORS - 1);
		ch.lowerwear = (Character::Lowerwear)(random::range_i(0, (int)Character::Lowerwear::Count - 1));
		ch.lowerwear_color = random::range_i(0, LOWERWEAR_COLORS - 1);
		ch.shirt = (Character::Shirt)(random::range_i(0, (int)Character::Shirt::Count - 1));
		ch.shirt_color = random::range_i(0, SHIRT_COLORS - 1);
		ch.gloves = (Character::Gloves)(random::range_i(0, (int)Character::Gloves::Count - 1));
		ch.gloves_color = random::range_i(0, GLOVES_COLORS - 1);
		ch.outerwear = (Character::Outerwear)(random::range_i(0, (int)Character::Outerwear::Count - 1));
		ch.outerwear_color = random::range_i(0, OUTERWEAR_COLORS - 1);
		ch.neckwear = (Character::Neckwear)(random::range_i(0, (int)Character::Neckwear::Count - 1));
		ch.neckwear_color_1 = random::range_i(0, NECKWEAR_COLORS_1 - 1);
		ch.neckwear_color_2 = random::range_i(0, NECKWEAR_COLORS_2 - 1);
		ch.glasses = (Character::Glasses)(random::range_i(0, (int)Character::Glasses::Count - 1));
		ch.glasses_color = random::range_i(0, GLASSES_COLORS - 1);
		ch.hair = (Character::Hair)(random::range_i(0, (int)Character::Hair::Count - 1));
		ch.hair_color = random::range_i(0, HAIR_COLORS - 1);
		ch.hat = (Character::Hat)(random::range_i(0, (int)Character::Hat::Count - 1));
		ch.hat_color_1 = random::range_i(0, HAT_COLORS_1 - 1);
		ch.hat_color_2 = random::range_i(0, HAT_COLORS_2 - 1);
	}

	graphics::TextureHandle create_character_texture(const Character& ch)
	{
		enum LookupTextureType
		{
			LUT_SKIN,
			LUT_HAIR,
			LUT_C3, // 3-color
			LUT_C4, // 4-color
		};

		struct Layer
		{
			std::string texture_path;
			int lut1_type = -1;
			int lut1_y = -1;
			int lut2_type = -1;
			int lut2_y = -1;
		};

		std::vector<Layer> layers;

		// 00undr

		switch (ch.neckwear) {
		case Character::Neckwear::CloakPlain:
			layers.emplace_back("00undr/fbas_00undr_cloakplain_00d.png", LUT_C3, ch.neckwear_color_1, LUT_C4, ch.neckwear_color_2);
			break;
		case Character::Neckwear::CloakWithMantlePlain:
			layers.emplace_back("00undr/fbas_00undr_cloakwithmantleplain_00b.png", LUT_C4, ch.neckwear_color_1);
			break;
		}

		// 01body

		switch (ch.body) {
		case Character::Body::Human:
			layers.emplace_back("01body/fbas_01body_human_00.png", LUT_SKIN, ch.skin_color);
			break;
		}

		// 02sock

		switch (ch.sock) {
		case Character::Sock::SocksHigh:
			layers.emplace_back("02sock/fbas_02sock_sockshigh_00a.png", LUT_C3, ch.sock_color);
			break;
		case Character::Sock::SocksLow:
			layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, ch.sock_color);
			break;
		case Character::Sock::Stockings:
			layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, ch.sock_color);
			break;
		}

		// 03fot1

		switch (ch.shoe) {
		case Character::Shoe::Boots:
			layers.emplace_back("03fot1/fbas_03fot1_boots_00a.png", LUT_C3, ch.shoe_color);
			break;
		case Character::Shoe::Sandals:
			layers.emplace_back("03fot1/fbas_03fot1_sandals_00a.png", LUT_C3, ch.shoe_color);
			break;
		case Character::Shoe::Shoes:
			layers.emplace_back("03fot1/fbas_03fot1_shoes_00a.png", LUT_C3, ch.shoe_color);
			break;
		}

		// 04lwr1

		switch (ch.lowerwear) {
		case Character::Lowerwear::LongPants:
			layers.emplace_back("04lwr1/fbas_04lwr1_longpants_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::Onepiece:
			layers.emplace_back("04lwr1/fbas_04lwr1_onepiece_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::OnepieceBoobs:
			layers.emplace_back("04lwr1/fbas_04lwr1_onepieceboobs_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::Shorts:
			layers.emplace_back("04lwr1/fbas_04lwr1_shorts_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::Undies:
			layers.emplace_back("04lwr1/fbas_04lwr1_undies_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		}

		// 05shrt

		switch (ch.shirt) {
		case Character::Shirt::Bra:
			layers.emplace_back("05shrt/fbas_05shrt_bra_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::LongShirt:
			layers.emplace_back("05shrt/fbas_05shrt_longshirt_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::LongShirtBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_longshirtboobs_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::ShortShirt:
			layers.emplace_back("05shrt/fbas_05shrt_shortshirt_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::ShortShirtBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_shortshirtboobs_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::TankTop:
			layers.emplace_back("05shrt/fbas_05shrt_tanktop_00a.png", LUT_C3, ch.shirt_color);
			break;
		case Character::Shirt::TankTopBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_tanktopboobs_00a.png", LUT_C3, ch.shirt_color);
			break;
		}

		// 06lwr2

		switch (ch.lowerwear) {
		case Character::Lowerwear::Overalls:
			layers.emplace_back("06lwr2/fbas_06lwr2_overalls_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::OverallsBoobs:
			layers.emplace_back("06lwr2/fbas_06lwr2_overallsboobs_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::Shortalls:
			layers.emplace_back("06lwr2/fbas_06lwr2_shortalls_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::ShortallsBoobs:
			layers.emplace_back("06lwr2/fbas_06lwr2_shortallsboobs_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		}

		// 07fot2

		switch (ch.shoe) {
		case Character::Shoe::CuffedBoots:
			layers.emplace_back("07fot2/fbas_07fot2_cuffedboots_00a.png", LUT_C3, ch.shoe_color);
			break;
		case Character::Shoe::CurlyToeShoes:
			layers.emplace_back("07fot2/fbas_07fot2_curlytoeshoes_00a.png", LUT_C3, ch.shoe_color);
			break;
		}

		// 08lwr3

		switch (ch.lowerwear) {
		case Character::Lowerwear::FrillyDress:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillydress_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::FrillyDressBoobs:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillydressboobs_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::FrillySkirt:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillyskirt_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::LongDress:
			layers.emplace_back("08lwr3/fbas_08lwr3_longdress_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::LongDressBoobs:
			layers.emplace_back("08lwr3/fbas_08lwr3_longdressboobs_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		case Character::Lowerwear::LongSkirt:
			layers.emplace_back("08lwr3/fbas_08lwr3_longskirt_00a.png", LUT_C3, ch.lowerwear_color);
			break;
		}

		// 09hand

		switch (ch.gloves) {
		case Character::Gloves::Gloves:
			layers.emplace_back("09hand/fbas_09hand_gloves_00a.png", LUT_C3, ch.gloves_color);
			break;
		}

		// 10outr

		switch (ch.outerwear) {
		case Character::Outerwear::Suspenders:
			layers.emplace_back("10outr/fbas_10outr_suspenders_00a.png", LUT_C3, ch.outerwear_color);
			break;
		case Character::Outerwear::Vest:
			layers.emplace_back("10outr/fbas_10outr_vest_00a.png", LUT_C3, ch.outerwear_color);
			break;
		}

		// 11neck

		switch (ch.neckwear) {
		case Character::Neckwear::CloakPlain:
			layers.emplace_back("11neck/fbas_11neck_cloakplain_00d.png", LUT_C3, ch.neckwear_color_1, LUT_C4, ch.neckwear_color_2);
			break;
		case Character::Neckwear::CloakWithMantlePlain:
			layers.emplace_back("11neck/fbas_11neck_cloakwithmantleplain_00b.png", LUT_C4, ch.neckwear_color_1);
			break;
		case Character::Neckwear::MantlePlain:
			layers.emplace_back("11neck/fbas_11neck_mantleplain_00b.png", LUT_C4, ch.neckwear_color_1);
			break;
		case Character::Neckwear::Scarf:
			layers.emplace_back("11neck/fbas_11neck_scarf_00b.png", LUT_C4, ch.neckwear_color_1);
			break;
		}

		// 12face

		switch (ch.glasses) {
		case Character::Glasses::Glasses:
			layers.emplace_back("12face/fbas_12face_glasses_00a.png", LUT_C3, ch.glasses_color);
			break;
		case Character::Glasses::Shades:
			layers.emplace_back("12face/fbas_12face_shades_00a.png", LUT_C3, ch.glasses_color);
			break;
		}

		// 13hair

		bool headwear_replaces_hair = false;
		switch (ch.hat) {
		case Character::Hat::Bandana:
		case Character::Hat::Headscarf:
			headwear_replaces_hair = true;
			break;
		}

		if (!headwear_replaces_hair) {
			switch (ch.hair) {
			case Character::Hair::Afro:
				layers.emplace_back("13hair/fbas_13hair_afro_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::AfroPuffs:
				layers.emplace_back("13hair/fbas_13hair_afropuffs_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Bob1:
				layers.emplace_back("13hair/fbas_13hair_bob1_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Bob2:
				layers.emplace_back("13hair/fbas_13hair_bob2_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Dapper:
				layers.emplace_back("13hair/fbas_13hair_dapper_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Flattop:
				layers.emplace_back("13hair/fbas_13hair_flattop_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::LongWavy:
				layers.emplace_back("13hair/fbas_13hair_longwavy_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Ponytail1:
				layers.emplace_back("13hair/fbas_13hair_ponytail1_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Spiky1:
				layers.emplace_back("13hair/fbas_13hair_spiky1_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Spiky2:
				layers.emplace_back("13hair/fbas_13hair_spiky2_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Twintail:
				layers.emplace_back("13hair/fbas_13hair_twintail_00.png", LUT_HAIR, ch.hair_color);
				break;
			case Character::Hair::Twists:
				layers.emplace_back("13hair/fbas_13hair_twists_00.png", LUT_HAIR, ch.hair_color);
				break;
			}
		}

		// 14head

		switch (ch.hat) {
		case Character::Hat::Bandana:
			layers.emplace_back("14head/fbas_14head_bandana_00b_e.png", LUT_C4, ch.hat_color_1);
			break;
		case Character::Hat::BoaterHat:
			layers.emplace_back("14head/fbas_14head_boaterhat_00d.png", LUT_C3, ch.hat_color_1, LUT_C4, ch.hat_color_2);
			break;
		case Character::Hat::CowboyHat:
			layers.emplace_back("14head/fbas_14head_cowboyhat_00d.png", LUT_C3, ch.hat_color_1, LUT_C4, ch.hat_color_2);
			break;
		case Character::Hat::FloppyHat:
			layers.emplace_back("14head/fbas_14head_floppyhat_00d.png", LUT_C3, ch.hat_color_1, LUT_C4, ch.hat_color_2);
			break;
		case Character::Hat::Headscarf:
			layers.emplace_back("14head/fbas_14head_headscarf_00b_e.png", LUT_C4, ch.hat_color_1);
			break;
		case Character::Hat::StrawHat:
			layers.emplace_back("14head/fbas_14head_strawhat_00d.png", LUT_C3, ch.hat_color_1, LUT_C4, ch.hat_color_2);
			break;
		}

		const std::string base_dir = "assets/textures/character/";

		// Load shader
		const graphics::ShaderHandle shader = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/bake_character.frag");
		if (shader == graphics::ShaderHandle::Invalid) return graphics::TextureHandle::Invalid;

		// Aquire render target
		const graphics::RenderTargetHandle render_target = graphics::acquire_pooled_render_target(1024, 1024);
		if (render_target == graphics::RenderTargetHandle::Invalid) return graphics::TextureHandle::Invalid;

		int viewport[4];
		graphics::get_viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		graphics::set_viewport(0, 0, 1024, 1024);

		graphics::bind_render_target(render_target);
		graphics::clear_render_target(0.f, 0.f, 0.f, 0.f);

		graphics::bind_shader(shader);
		graphics::set_uniform_1i(shader, "tex", 0);
		graphics::set_uniform_1i(shader, "lut1", 1);
		graphics::set_uniform_1i(shader, "lut2", 2);

		for (const Layer& layer : layers) {
			const graphics::TextureHandle texture = graphics::load_texture(base_dir + layer.texture_path);
			if (texture == graphics::TextureHandle::Invalid) continue;

			graphics::TextureHandle lut1_texture = graphics::TextureHandle::Invalid;
			switch (layer.lut1_type) {
			case LUT_SKIN:
				lut1_texture = graphics::load_texture(base_dir + "palettes/mana seed skin ramps.png");
				break;
			case LUT_HAIR:
				lut1_texture = graphics::load_texture(base_dir + "palettes/mana seed hair ramps.png");
				break;
			case LUT_C3:
				lut1_texture = graphics::load_texture(base_dir + "palettes/mana seed 3-color ramps.png");
				break;
			case LUT_C4:
				lut1_texture = graphics::load_texture(base_dir + "palettes/mana seed 4-color ramps.png");
				break;
			}
			if (lut1_texture != graphics::TextureHandle::Invalid) {
				graphics::set_uniform_1i(shader, "lut1_type", layer.lut1_type);
				graphics::set_uniform_1i(shader, "lut1_y", layer.lut1_y);
			} else {
				graphics::set_uniform_1i(shader, "lut1_type", -1);
			}

			graphics::TextureHandle lut2_texture = graphics::TextureHandle::Invalid;
			switch (layer.lut2_type) {
			case LUT_SKIN:
				lut2_texture = graphics::load_texture(base_dir + "palettes/mana seed skin ramps.png");
				break;
			case LUT_HAIR:
				lut2_texture = graphics::load_texture(base_dir + "palettes/mana seed hair ramps.png");
				break;
			case LUT_C3:
				lut2_texture = graphics::load_texture(base_dir + "palettes/mana seed 3-color ramps.png");
				break;
			case LUT_C4:
				lut2_texture = graphics::load_texture(base_dir + "palettes/mana seed 4-color ramps.png");
				break;
			}
			if (lut2_texture != graphics::TextureHandle::Invalid) {
				graphics::set_uniform_1i(shader, "lut2_type", layer.lut2_type);
				graphics::set_uniform_1i(shader, "lut2_y", layer.lut2_y);
			} else {
				graphics::set_uniform_1i(shader, "lut2_type", -1);
			}

			graphics::bind_texture(0, texture);
			graphics::bind_texture(1, lut1_texture);
			graphics::bind_texture(2, lut2_texture);

			graphics::draw_triangles(3);
		}

		const graphics::TextureHandle texture =
			graphics::copy_texture(graphics::get_render_target_texture(render_target));
		graphics::release_pooled_render_target(render_target);
		graphics::set_viewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		return texture;
	}
}