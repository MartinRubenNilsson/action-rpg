#include "stdafx.h"
#include "player_outfit.h"
#include "graphics.h"
#include "random.h"

namespace player
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

	void randomize_outfit(Outfit& outfit)
	{
		outfit.body = (Outfit::Body)(random::range_i(1, (int)Outfit::Body::Count - 1));
		outfit.skin_color = random::range_i(0, SKIN_COLORS - 1);
		outfit.sock = (Outfit::Sock)(random::range_i(0, (int)Outfit::Sock::Count - 1));
		outfit.sock_color = random::range_i(0, SOCK_COLORS - 1);
		outfit.shoe = (Outfit::Shoe)(random::range_i(0, (int)Outfit::Shoe::Count - 1));
		outfit.shoe_color = random::range_i(0, SHOE_COLORS - 1);
		outfit.lowerwear = (Outfit::Lowerwear)(random::range_i(0, (int)Outfit::Lowerwear::Count - 1));
		outfit.lowerwear_color = random::range_i(0, LOWERWEAR_COLORS - 1);
		outfit.shirt = (Outfit::Shirt)(random::range_i(0, (int)Outfit::Shirt::Count - 1));
		outfit.shirt_color = random::range_i(0, SHIRT_COLORS - 1);
		outfit.gloves = (Outfit::Gloves)(random::range_i(0, (int)Outfit::Gloves::Count - 1));
		outfit.gloves_color = random::range_i(0, GLOVES_COLORS - 1);
		outfit.outerwear = (Outfit::Outerwear)(random::range_i(0, (int)Outfit::Outerwear::Count - 1));
		outfit.outerwear_color = random::range_i(0, OUTERWEAR_COLORS - 1);
		outfit.neckwear = (Outfit::Neckwear)(random::range_i(0, (int)Outfit::Neckwear::Count - 1));
		outfit.neckwear_color_1 = random::range_i(0, NECKWEAR_COLORS_1 - 1);
		outfit.neckwear_color_2 = random::range_i(0, NECKWEAR_COLORS_2 - 1);
		outfit.glasses = (Outfit::Glasses)(random::range_i(0, (int)Outfit::Glasses::Count - 1));
		outfit.glasses_color = random::range_i(0, GLASSES_COLORS - 1);
		outfit.hair = (Outfit::Hair)(random::range_i(0, (int)Outfit::Hair::Count - 1));
		outfit.hair_color = random::range_i(0, HAIR_COLORS - 1);
		outfit.hat = (Outfit::Hat)(random::range_i(0, (int)Outfit::Hat::Count - 1));
		outfit.hat_color_1 = random::range_i(0, HAT_COLORS_1 - 1);
		outfit.hat_color_2 = random::range_i(0, HAT_COLORS_2 - 1);
	}

	Handle<graphics::Texture> create_outfit_texture(Outfit& outfit)
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
			const char* texture_path = nullptr;
			int lut1_type = -1;
			int lut1_y = -1;
			int lut2_type = -1;
			int lut2_y = -1;
		};

		std::vector<Layer> layers;

		// 00undr

		switch (outfit.neckwear) {
		case Outfit::Neckwear::CloakPlain:
			layers.emplace_back("00undr/fbas_00undr_cloakplain_00d.png", LUT_C3, outfit.neckwear_color_1, LUT_C4, outfit.neckwear_color_2);
			break;
		case Outfit::Neckwear::CloakWithMantlePlain:
			layers.emplace_back("00undr/fbas_00undr_cloakwithmantleplain_00b.png", LUT_C4, outfit.neckwear_color_1);
			break;
		}

		// 01body

		switch (outfit.body) {
		case Outfit::Body::Human:
			layers.emplace_back("01body/fbas_01body_human_00.png", LUT_SKIN, outfit.skin_color);
			break;
		}

		// 02sock

		switch (outfit.sock) {
		case Outfit::Sock::SocksHigh:
			layers.emplace_back("02sock/fbas_02sock_sockshigh_00a.png", LUT_C3, outfit.sock_color);
			break;
		case Outfit::Sock::SocksLow:
			layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, outfit.sock_color);
			break;
		case Outfit::Sock::Stockings:
			layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, outfit.sock_color);
			break;
		}

		// 03fot1

		switch (outfit.shoe) {
		case Outfit::Shoe::Boots:
			layers.emplace_back("03fot1/fbas_03fot1_boots_00a.png", LUT_C3, outfit.shoe_color);
			break;
		case Outfit::Shoe::Sandals:
			layers.emplace_back("03fot1/fbas_03fot1_sandals_00a.png", LUT_C3, outfit.shoe_color);
			break;
		case Outfit::Shoe::Shoes:
			layers.emplace_back("03fot1/fbas_03fot1_shoes_00a.png", LUT_C3, outfit.shoe_color);
			break;
		}

		// 04lwr1

		switch (outfit.lowerwear) {
		case Outfit::Lowerwear::LongPants:
			layers.emplace_back("04lwr1/fbas_04lwr1_longpants_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::Onepiece:
			layers.emplace_back("04lwr1/fbas_04lwr1_onepiece_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::OnepieceBoobs:
			layers.emplace_back("04lwr1/fbas_04lwr1_onepieceboobs_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::Shorts:
			layers.emplace_back("04lwr1/fbas_04lwr1_shorts_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::Undies:
			layers.emplace_back("04lwr1/fbas_04lwr1_undies_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		}

		// 05shrt

		switch (outfit.shirt) {
		case Outfit::Shirt::Bra:
			layers.emplace_back("05shrt/fbas_05shrt_bra_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::LongShirt:
			layers.emplace_back("05shrt/fbas_05shrt_longshirt_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::LongShirtBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_longshirtboobs_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::ShortShirt:
			layers.emplace_back("05shrt/fbas_05shrt_shortshirt_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::ShortShirtBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_shortshirtboobs_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::TankTop:
			layers.emplace_back("05shrt/fbas_05shrt_tanktop_00a.png", LUT_C3, outfit.shirt_color);
			break;
		case Outfit::Shirt::TankTopBoobs:
			layers.emplace_back("05shrt/fbas_05shrt_tanktopboobs_00a.png", LUT_C3, outfit.shirt_color);
			break;
		}

		// 06lwr2

		switch (outfit.lowerwear) {
		case Outfit::Lowerwear::Overalls:
			layers.emplace_back("06lwr2/fbas_06lwr2_overalls_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::OverallsBoobs:
			layers.emplace_back("06lwr2/fbas_06lwr2_overallsboobs_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::Shortalls:
			layers.emplace_back("06lwr2/fbas_06lwr2_shortalls_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::ShortallsBoobs:
			layers.emplace_back("06lwr2/fbas_06lwr2_shortallsboobs_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		}

		// 07fot2

		switch (outfit.shoe) {
		case Outfit::Shoe::CuffedBoots:
			layers.emplace_back("07fot2/fbas_07fot2_cuffedboots_00a.png", LUT_C3, outfit.shoe_color);
			break;
		case Outfit::Shoe::CurlyToeShoes:
			layers.emplace_back("07fot2/fbas_07fot2_curlytoeshoes_00a.png", LUT_C3, outfit.shoe_color);
			break;
		}

		// 08lwr3

		switch (outfit.lowerwear) {
		case Outfit::Lowerwear::FrillyDress:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillydress_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::FrillyDressBoobs:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillydressboobs_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::FrillySkirt:
			layers.emplace_back("08lwr3/fbas_08lwr3_frillyskirt_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::LongDress:
			layers.emplace_back("08lwr3/fbas_08lwr3_longdress_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::LongDressBoobs:
			layers.emplace_back("08lwr3/fbas_08lwr3_longdressboobs_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		case Outfit::Lowerwear::LongSkirt:
			layers.emplace_back("08lwr3/fbas_08lwr3_longskirt_00a.png", LUT_C3, outfit.lowerwear_color);
			break;
		}

		// 09hand

		switch (outfit.gloves) {
		case Outfit::Gloves::Gloves:
			layers.emplace_back("09hand/fbas_09hand_gloves_00a.png", LUT_C3, outfit.gloves_color);
			break;
		}

		// 10outr

		switch (outfit.outerwear) {
		case Outfit::Outerwear::Suspenders:
			layers.emplace_back("10outr/fbas_10outr_suspenders_00a.png", LUT_C3, outfit.outerwear_color);
			break;
		case Outfit::Outerwear::Vest:
			layers.emplace_back("10outr/fbas_10outr_vest_00a.png", LUT_C3, outfit.outerwear_color);
			break;
		}

		// 11neck

		switch (outfit.neckwear) {
		case Outfit::Neckwear::CloakPlain:
			layers.emplace_back("11neck/fbas_11neck_cloakplain_00d.png", LUT_C3, outfit.neckwear_color_1, LUT_C4, outfit.neckwear_color_2);
			break;
		case Outfit::Neckwear::CloakWithMantlePlain:
			layers.emplace_back("11neck/fbas_11neck_cloakwithmantleplain_00b.png", LUT_C4, outfit.neckwear_color_1);
			break;
		case Outfit::Neckwear::MantlePlain:
			layers.emplace_back("11neck/fbas_11neck_mantleplain_00b.png", LUT_C4, outfit.neckwear_color_1);
			break;
		case Outfit::Neckwear::Scarf:
			layers.emplace_back("11neck/fbas_11neck_scarf_00b.png", LUT_C4, outfit.neckwear_color_1);
			break;
		}

		// 12face

		switch (outfit.glasses) {
		case Outfit::Glasses::Glasses:
			layers.emplace_back("12face/fbas_12face_glasses_00a.png", LUT_C3, outfit.glasses_color);
			break;
		case Outfit::Glasses::Shades:
			layers.emplace_back("12face/fbas_12face_shades_00a.png", LUT_C3, outfit.glasses_color);
			break;
		}

		// 13hair

		bool headwear_replaces_hair = false;
		switch (outfit.hat) {
		case Outfit::Hat::Bandana:
		case Outfit::Hat::Headscarf:
			headwear_replaces_hair = true;
			break;
		}

		if (!headwear_replaces_hair) {
			switch (outfit.hair) {
			case Outfit::Hair::Afro:
				layers.emplace_back("13hair/fbas_13hair_afro_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::AfroPuffs:
				layers.emplace_back("13hair/fbas_13hair_afropuffs_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Bob1:
				layers.emplace_back("13hair/fbas_13hair_bob1_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Bob2:
				layers.emplace_back("13hair/fbas_13hair_bob2_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Dapper:
				layers.emplace_back("13hair/fbas_13hair_dapper_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Flattop:
				layers.emplace_back("13hair/fbas_13hair_flattop_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::LongWavy:
				layers.emplace_back("13hair/fbas_13hair_longwavy_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Ponytail1:
				layers.emplace_back("13hair/fbas_13hair_ponytail1_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Spiky1:
				layers.emplace_back("13hair/fbas_13hair_spiky1_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Spiky2:
				layers.emplace_back("13hair/fbas_13hair_spiky2_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Twintail:
				layers.emplace_back("13hair/fbas_13hair_twintail_00.png", LUT_HAIR, outfit.hair_color);
				break;
			case Outfit::Hair::Twists:
				layers.emplace_back("13hair/fbas_13hair_twists_00.png", LUT_HAIR, outfit.hair_color);
				break;
			}
		}

		// 14head

		switch (outfit.hat) {
		case Outfit::Hat::Bandana:
			layers.emplace_back("14head/fbas_14head_bandana_00b_e.png", LUT_C4, outfit.hat_color_1);
			break;
		case Outfit::Hat::BoaterHat:
			layers.emplace_back("14head/fbas_14head_boaterhat_00d.png", LUT_C3, outfit.hat_color_1, LUT_C4, outfit.hat_color_2);
			break;
		case Outfit::Hat::CowboyHat:
			layers.emplace_back("14head/fbas_14head_cowboyhat_00d.png", LUT_C3, outfit.hat_color_1, LUT_C4, outfit.hat_color_2);
			break;
		case Outfit::Hat::FloppyHat:
			layers.emplace_back("14head/fbas_14head_floppyhat_00d.png", LUT_C3, outfit.hat_color_1, LUT_C4, outfit.hat_color_2);
			break;
		case Outfit::Hat::Headscarf:
			layers.emplace_back("14head/fbas_14head_headscarf_00b_e.png", LUT_C4, outfit.hat_color_1);
			break;
		case Outfit::Hat::StrawHat:
			layers.emplace_back("14head/fbas_14head_strawhat_00d.png", LUT_C3, outfit.hat_color_1, LUT_C4, outfit.hat_color_2);
			break;
		}

		const std::string base_dir = "assets/textures/character/";

		// Load shader
		const Handle<graphics::Shader> shader = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/bake_character.frag");
		if (shader == Handle<graphics::Shader>()) return Handle<graphics::Texture>();

		// Aquire framebuffer
		const Handle<graphics::Framebuffer> framebuffer = graphics::get_temporary_framebuffer(1024, 1024);
		if (framebuffer == Handle<graphics::Framebuffer>()) return Handle<graphics::Texture>();

		int viewport[4];
		graphics::get_viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		graphics::set_viewport(0, 0, 1024, 1024);

		graphics::clear_framebuffer(framebuffer, 0.f, 0.f, 0.f, 0.f);
		graphics::bind_framebuffer(framebuffer);
		graphics::bind_shader(shader);

		for (const Layer& layer : layers) {
			const Handle<graphics::Texture> texture = graphics::load_texture(base_dir + layer.texture_path);
			if (texture == Handle<graphics::Texture>()) continue;

			Handle<graphics::Texture> lut1_texture;
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
			if (lut1_texture != Handle<graphics::Texture>()) {
				graphics::set_uniform_1i(shader, "lut1_type", layer.lut1_type);
				graphics::set_uniform_1i(shader, "lut1_y", layer.lut1_y);
			} else {
				graphics::set_uniform_1i(shader, "lut1_type", -1);
			}

			Handle<graphics::Texture> lut2_texture;
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
			if (lut2_texture != Handle<graphics::Texture>()) {
				graphics::set_uniform_1i(shader, "lut2_type", layer.lut2_type);
				graphics::set_uniform_1i(shader, "lut2_y", layer.lut2_y);
			} else {
				graphics::set_uniform_1i(shader, "lut2_type", -1);
			}

			graphics::bind_texture(0, texture);
			graphics::bind_texture(1, lut1_texture);
			graphics::bind_texture(2, lut2_texture);

			graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
		}

		Handle<graphics::Texture> texture = graphics::copy_texture(graphics::get_framebuffer_texture(framebuffer));
		graphics::release_temporary_framebuffer(framebuffer);
		graphics::set_viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		return texture;
	}
}