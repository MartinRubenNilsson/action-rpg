#include "stdafx.h"
#include "character.h"
#include "textures.h"
#include "shaders.h"
#include "random.h"

void Character::randomize()
{
	body = (Body)(random::range_i(1, (int)Body::Count - 1));
	skin_color = random::range_i(0, SKIN_COLORS - 1);
	sock = (Sock)(random::range_i(0, (int)Sock::Count - 1));
	sock_color = random::range_i(0, SOCK_COLORS - 1);
	shoe = (Shoe)(random::range_i(0, (int)Shoe::Count - 1));
	shoe_color = random::range_i(0, SHOE_COLORS - 1);
	lowerwear = (Lowerwear)(random::range_i(0, (int)Lowerwear::Count - 1));
	lowerwear_color = random::range_i(0, LOWERWEAR_COLORS - 1);
	shirt = (Shirt)(random::range_i(0, (int)Shirt::Count - 1));
	shirt_color = random::range_i(0, SHIRT_COLORS - 1);
	gloves = (Gloves)(random::range_i(0, (int)Gloves::Count - 1));
	gloves_color = random::range_i(0, GLOVES_COLORS - 1);
	outerwear = (Outerwear)(random::range_i(0, (int)Outerwear::Count - 1));
	outerwear_color = random::range_i(0, OUTERWEAR_COLORS - 1);
	neckwear = (Neckwear)(random::range_i(0, (int)Neckwear::Count - 1));
	//neckwear_color = random::range_i(0, NECKWEAR_COLORS - 1);
	glasses = (Glasses)(random::range_i(0, (int)Glasses::Count - 1));
	glasses_color = random::range_i(0, GLASSES_COLORS - 1);
	hair = (Hair)(random::range_i(0, (int)Hair::Count - 1));
	hair_color = random::range_i(0, HAIR_COLORS - 1);
	hat = (Hat)(random::range_i(0, (int)Hat::Count - 1));
	//hat_color = random::range_i(0, HAT_COLORS - 1);
}

std::shared_ptr<sf::Texture> Character::bake_texture() const
{
	enum LookupTextureType
	{
		LUT_SKIN,
		LUT_HAIR,
		LUT_C3, // Single 3-color ramp
		LUT_B, // Single 4-color ramp
		//LUT_C, // Two 3-color ramps
		LUT_D, // One 4-color ramp AND one 3-color ramp
	};

	struct Layer
	{
		std::filesystem::path texture_path;
		int lut_type = -1;
		int lut_row = -1;
	};

	std::vector<Layer> layers;

	// 00undr

	switch (neckwear) {
	case Neckwear::CloakPlain:
		layers.emplace_back("00undr/fbas_00undr_cloakplain_00d.png", LUT_D);
		break;
	case Neckwear::CloakWithMantlePlain:
		layers.emplace_back("00undr/fbas_00undr_cloakwithmantleplain_00b.png", LUT_B);
		break;
	}

	// 01body

	switch (body) {
	case Body::Human:
		layers.emplace_back("01body/fbas_01body_human_00.png", LUT_SKIN, skin_color);
		break;
	}

	// 02sock

	switch (sock) {
	case Sock::SocksHigh:
		layers.emplace_back("02sock/fbas_02sock_sockshigh_00a.png", LUT_C3, sock_color);
		break;
	case Sock::SocksLow:
		layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, sock_color);
		break;
	case Sock::Stockings:
		layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", LUT_C3, sock_color);
		break;
	}

	// 03fot1

	switch (shoe) {
	case Shoe::Boots:
		layers.emplace_back("03fot1/fbas_03fot1_boots_00a.png", LUT_C3, shoe_color);
		break;
	case Shoe::Sandals:
		layers.emplace_back("03fot1/fbas_03fot1_sandals_00a.png", LUT_C3, shoe_color);
		break;
	case Shoe::Shoes:
		layers.emplace_back("03fot1/fbas_03fot1_shoes_00a.png", LUT_C3, shoe_color);
		break;
	}

	// 04lwr1

	switch (lowerwear) {
	case Lowerwear::LongPants:
		layers.emplace_back("04lwr1/fbas_04lwr1_longpants_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::Onepiece:
		layers.emplace_back("04lwr1/fbas_04lwr1_onepiece_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::OnepieceBoobs:
		layers.emplace_back("04lwr1/fbas_04lwr1_onepieceboobs_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::Shorts:
		layers.emplace_back("04lwr1/fbas_04lwr1_shorts_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::Undies:
		layers.emplace_back("04lwr1/fbas_04lwr1_undies_00a.png", LUT_C3, lowerwear_color);
		break;
	}

	// 05shrt

	switch (shirt) {
	case Shirt::Bra:
		layers.emplace_back("05shrt/fbas_05shrt_bra_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::LongShirt:
		layers.emplace_back("05shrt/fbas_05shrt_longshirt_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::LongShirtBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_longshirtboobs_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::ShortShirt:
		layers.emplace_back("05shrt/fbas_05shrt_shortshirt_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::ShortShirtBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_shortshirtboobs_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::TankTop:
		layers.emplace_back("05shrt/fbas_05shrt_tanktop_00a.png", LUT_C3, shirt_color);
		break;
	case Shirt::TankTopBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_tanktopboobs_00a.png", LUT_C3, shirt_color);
		break;
	}

	// 06lwr2

	switch (lowerwear) {
	case Lowerwear::Overalls:
		layers.emplace_back("06lwr2/fbas_06lwr2_overalls_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::OverallsBoobs:
		layers.emplace_back("06lwr2/fbas_06lwr2_overallsboobs_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::Shortalls:
		layers.emplace_back("06lwr2/fbas_06lwr2_shortalls_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::ShortallsBoobs:
		layers.emplace_back("06lwr2/fbas_06lwr2_shortallsboobs_00a.png", LUT_C3, lowerwear_color);
		break;
	}

	// 07fot2

	switch (shoe) {
	case Shoe::CuffedBoots:
		layers.emplace_back("07fot2/fbas_07fot2_cuffedboots_00a.png", LUT_C3, shoe_color);
		break;
	case Shoe::CurlyToeShoes:
		layers.emplace_back("07fot2/fbas_07fot2_curlytoeshoes_00a.png", LUT_C3, shoe_color);
		break;
	}

	// 08lwr3

	switch (lowerwear) {
	case Lowerwear::FrillyDress:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillydress_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::FrillyDressBoobs:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillydressboobs_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::FrillySkirt:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillyskirt_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::LongDress:
		layers.emplace_back("08lwr3/fbas_08lwr3_longdress_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::LongDressBoobs:
		layers.emplace_back("08lwr3/fbas_08lwr3_longdressboobs_00a.png", LUT_C3, lowerwear_color);
		break;
	case Lowerwear::LongSkirt:
		layers.emplace_back("08lwr3/fbas_08lwr3_longskirt_00a.png", LUT_C3, lowerwear_color);
		break;
	}

	// 09hand

	switch (gloves) {
	case Gloves::Gloves:
		layers.emplace_back("09hand/fbas_09hand_gloves_00a.png", LUT_C3, gloves_color);
		break;
	}

	// 10outr

	switch (outerwear) {
	case Outerwear::Suspenders:
		layers.emplace_back("10outr/fbas_10outr_suspenders_00a.png", LUT_C3, outerwear_color);
		break;
	case Outerwear::Vest:
		layers.emplace_back("10outr/fbas_10outr_vest_00a.png", LUT_C3, outerwear_color);
		break;
	}

	// 11neck

	switch (neckwear) {
	case Neckwear::CloakPlain:
		layers.emplace_back("11neck/fbas_11neck_cloakplain_00d.png", LUT_D);
		break;
	case Neckwear::CloakWithMantlePlain:
		layers.emplace_back("11neck/fbas_11neck_cloakwithmantleplain_00b.png", LUT_B);
		break;
	case Neckwear::MantlePlain:
		layers.emplace_back("11neck/fbas_11neck_mantleplain_00b.png", LUT_B);
		break;
	case Neckwear::Scarf:
		layers.emplace_back("11neck/fbas_11neck_scarf_00b.png", LUT_B);
		break;
	}

	// 12face

	switch (glasses) {
	case Glasses::Glasses:
		layers.emplace_back("12face/fbas_12face_glasses_00a.png", LUT_C3, glasses_color);
		break;
	case Glasses::Shades:
		layers.emplace_back("12face/fbas_12face_shades_00a.png", LUT_C3, glasses_color);
		break;
	}

	// 13hair

	bool headwear_replaces_hair = false;
	switch (hat) {
	case Hat::Bandana:
	case Hat::Headscarf:
		headwear_replaces_hair = true;
		break;
	}

	if (!headwear_replaces_hair) {
		switch (hair) {
		case Hair::Afro:
			layers.emplace_back("13hair/fbas_13hair_afro_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::AfroPuffs:
			layers.emplace_back("13hair/fbas_13hair_afropuffs_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Bob1:
			layers.emplace_back("13hair/fbas_13hair_bob1_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Bob2:
			layers.emplace_back("13hair/fbas_13hair_bob2_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Dapper:
			layers.emplace_back("13hair/fbas_13hair_dapper_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Flattop:
			layers.emplace_back("13hair/fbas_13hair_flattop_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::LongWavy:
			layers.emplace_back("13hair/fbas_13hair_longwavy_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Ponytail1:
			layers.emplace_back("13hair/fbas_13hair_ponytail1_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Spiky1:
			layers.emplace_back("13hair/fbas_13hair_spiky1_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Spiky2:
			layers.emplace_back("13hair/fbas_13hair_spiky2_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Twintail:
			layers.emplace_back("13hair/fbas_13hair_twintail_00.png", LUT_HAIR, hair_color);
			break;
		case Hair::Twists:
			layers.emplace_back("13hair/fbas_13hair_twists_00.png", LUT_HAIR, hair_color);
			break;
		}
	}

	// 14head

	switch (hat) {
	case Hat::Bandana:
		layers.emplace_back("14head/fbas_14head_bandana_00b_e.png", LUT_B);
		break;
	case Hat::BoaterHat:
		layers.emplace_back("14head/fbas_14head_boaterhat_00d.png", LUT_D);
		break;
	case Hat::CowboyHat:
		layers.emplace_back("14head/fbas_14head_cowboyhat_00d.png", LUT_D);
		break;
	case Hat::FloppyHat:
		layers.emplace_back("14head/fbas_14head_floppyhat_00d.png", LUT_D);
		break;
	case Hat::Headscarf:
		layers.emplace_back("14head/fbas_14head_headscarf_00b_e.png", LUT_B);
		break;
	case Hat::StrawHat:
		layers.emplace_back("14head/fbas_14head_strawhat_00d.png", LUT_D);
		break;
	}

	std::shared_ptr<sf::Shader> shader = shaders::get("bake_character");
	if (!shader) return nullptr;

	const std::filesystem::path base_dir = "assets/textures/character";

	sf::RenderTexture render_texture;
	render_texture.create(1024, 1024);
	render_texture.clear(sf::Color::Transparent);
	for (const Layer& layer : layers) {
		std::shared_ptr<sf::Texture> texture = textures::get(base_dir / layer.texture_path);
		if (!texture) continue;
		std::shared_ptr<sf::Texture> lut;
		switch (layer.lut_type) {
		case LUT_SKIN:
			lut = textures::get(base_dir / "palettes/mana seed skin ramps.png");
			break;
		case LUT_HAIR:
			lut = textures::get(base_dir / "palettes/mana seed hair ramps.png");
			break;
		case LUT_C3:
			lut = textures::get(base_dir / "palettes/mana seed 3-color ramps.png");
			break;
		}
		if (lut) {
			shader->setUniform("lut", *lut);
			shader->setUniform("lut_type", layer.lut_type);
			shader->setUniform("lut_row", layer.lut_row);
		} else {
			shader->setUniform("lut_type", -1);
		}
		
		render_texture.draw(sf::Sprite(*texture), { shader.get() });
	}
	render_texture.display();

	return std::make_shared<sf::Texture>(render_texture.getTexture());
}