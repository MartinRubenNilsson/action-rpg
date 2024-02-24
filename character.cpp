#include "stdafx.h"
#include "character.h"
#include "textures.h"
#include "shaders.h"
#include "random.h"

void Character::randomize()
{
	body = (Body)(random::range_i(1, (int)Body::Count - 1));
	skin_color = random::range_i(0, SKIN_COLORS - 1);
	legwear = (Legwear)(random::range_i(0, (int)Legwear::Count - 1));
	footwear = (Footwear)(random::range_i(0, (int)Footwear::Count - 1));
	lowerwear = (Lowerwear)(random::range_i(0, (int)Lowerwear::Count - 1));
	shirt = (Shirt)(random::range_i(0, (int)Shirt::Count - 1));
	handwear = (Handwear)(random::range_i(0, (int)Handwear::Count - 1));
	outerwear = (Outerwear)(random::range_i(0, (int)Outerwear::Count - 1));
	neckwear = (Neckwear)(random::range_i(0, (int)Neckwear::Count - 1));
	facewear = (Facewear)(random::range_i(0, (int)Facewear::Count - 1));
	hair = (Hair)(random::range_i(0, (int)Hair::Count - 1));
	headwear = (Headwear)(random::range_i(0, (int)Headwear::Count - 1));
}

std::shared_ptr<sf::Texture> Character::bake_texture() const
{
	enum Palette
	{
		PALETTE_SKIN,
		PALETTE_HAIR,
		PALETTE_A, // Single 3-color ramp
		PALETTE_B, // Single 4-color ramp
		PALETTE_C, // Two 3-color ramps
		PALETTE_D, // One 4-color ramp AND one 3-color ramp
	};

	struct Layer
	{
		std::filesystem::path texture_path;
		int palette = 0;
		int color = 0;
	};

	std::vector<Layer> layers;

	// 00undr

	switch (neckwear) {
	case Neckwear::CloakPlain:
		layers.emplace_back("00undr/fbas_00undr_cloakplain_00d.png", PALETTE_D);
		break;
	case Neckwear::CloakWithMantlePlain:
		layers.emplace_back("00undr/fbas_00undr_cloakwithmantleplain_00b.png", PALETTE_B);
		break;
	}

	// 01body

	switch (body) {
	case Body::Human:
		layers.emplace_back("01body/fbas_01body_human_00.png", PALETTE_SKIN, skin_color);
		break;
	}

	// 02sock

	switch (legwear) {
	case Legwear::SocksHigh:
		layers.emplace_back("02sock/fbas_02sock_sockshigh_00a.png", PALETTE_A);
		break;
	case Legwear::SocksLow:
		layers.emplace_back("02sock/fbas_02sock_sockslow_00a.png", PALETTE_A);
		break;
	case Legwear::Stockings:
		layers.emplace_back("02sock/fbas_02sock_stockings_00a.png", PALETTE_A);
		break;
	}

	// 03fot1

	switch (footwear) {
	case Footwear::Boots:
		layers.emplace_back("03fot1/fbas_03fot1_boots_00a.png", PALETTE_A);
		break;
	case Footwear::Sandals:
		layers.emplace_back("03fot1/fbas_03fot1_sandals_00a.png", PALETTE_A);
		break;
	case Footwear::Shoes:
		layers.emplace_back("03fot1/fbas_03fot1_shoes_00a.png", PALETTE_A);
		break;
	}

	// 04lwr1

	switch (lowerwear) {
	case Lowerwear::LongPants:
		layers.emplace_back("04lwr1/fbas_04lwr1_longpants_00a.png", PALETTE_A);
		break;
	case Lowerwear::Onepiece:
		layers.emplace_back("04lwr1/fbas_04lwr1_onepiece_00a.png", PALETTE_A);
		break;
	case Lowerwear::OnepieceBoobs:
		layers.emplace_back("04lwr1/fbas_04lwr1_onepieceboobs_00a.png", PALETTE_A);
		break;
	case Lowerwear::Shorts:
		layers.emplace_back("04lwr1/fbas_04lwr1_shorts_00a.png", PALETTE_A);
		break;
	case Lowerwear::Undies:
		layers.emplace_back("04lwr1/fbas_04lwr1_undies_00a.png", PALETTE_A);
		break;
	}

	// 05shrt

	switch (shirt) {
	case Shirt::Bra:
		layers.emplace_back("05shrt/fbas_05shrt_bra_00a.png", PALETTE_A);
		break;
	case Shirt::LongShirt:
		layers.emplace_back("05shrt/fbas_05shrt_longshirt_00a.png", PALETTE_A);
		break;
	case Shirt::LongShirtBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_longshirtboobs_00a.png", PALETTE_A);
		break;
	case Shirt::ShortShirt:
		layers.emplace_back("05shrt/fbas_05shrt_shortshirt_00a.png", PALETTE_A);
		break;
	case Shirt::ShortShirtBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_shortshirtboobs_00a.png", PALETTE_A);
		break;
	case Shirt::TankTop:
		layers.emplace_back("05shrt/fbas_05shrt_tanktop_00a.png", PALETTE_A);
		break;
	case Shirt::TankTopBoobs:
		layers.emplace_back("05shrt/fbas_05shrt_tanktopboobs_00a.png", PALETTE_A);
		break;
	}

	// 06lwr2

	switch (lowerwear) {
	case Lowerwear::Overalls:
		layers.emplace_back("06lwr2/fbas_06lwr2_overalls_00a.png", PALETTE_A);
		break;
	case Lowerwear::OverallsBoobs:
		layers.emplace_back("06lwr2/fbas_06lwr2_overallsboobs_00a.png", PALETTE_A);
		break;
	case Lowerwear::Shortalls:
		layers.emplace_back("06lwr2/fbas_06lwr2_shortalls_00a.png", PALETTE_A);
		break;
	case Lowerwear::ShortallsBoobs:
		layers.emplace_back("06lwr2/fbas_06lwr2_shortallsboobs_00a.png", PALETTE_A);
		break;
	}

	// 07fot2

	switch (footwear) {
	case Footwear::CuffedBoots:
		layers.emplace_back("07fot2/fbas_07fot2_cuffedboots_00a.png", PALETTE_A);
		break;
	case Footwear::CurlyToeShoes:
		layers.emplace_back("07fot2/fbas_07fot2_curlytoeshoes_00a.png", PALETTE_A);
		break;
	}

	// 08lwr3

	switch (lowerwear) {
	case Lowerwear::FrillyDress:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillydress_00a.png", PALETTE_A);
		break;
	case Lowerwear::FrillyDressBoobs:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillydressboobs_00a.png", PALETTE_A);
		break;
	case Lowerwear::FrillySkirt:
		layers.emplace_back("08lwr3/fbas_08lwr3_frillyskirt_00a.png", PALETTE_A);
		break;
	case Lowerwear::LongDress:
		layers.emplace_back("08lwr3/fbas_08lwr3_longdress_00a.png", PALETTE_A);
		break;
	case Lowerwear::LongDressBoobs:
		layers.emplace_back("08lwr3/fbas_08lwr3_longdressboobs_00a.png", PALETTE_A);
		break;
	case Lowerwear::LongSkirt:
		layers.emplace_back("08lwr3/fbas_08lwr3_longskirt_00a.png", PALETTE_A);
		break;
	}

	// 09hand

	switch (handwear) {
	case Handwear::Gloves:
		layers.emplace_back("09hand/fbas_09hand_gloves_00a.png", PALETTE_A);
		break;
	}

	// 10outr

	switch (outerwear) {
	case Outerwear::Suspenders:
		layers.emplace_back("10outr/fbas_10outr_suspenders_00a.png", PALETTE_A);
		break;
	case Outerwear::Vest:
		layers.emplace_back("10outr/fbas_10outr_vest_00a.png", PALETTE_A);
		break;
	}

	// 11neck

	switch (neckwear) {
	case Neckwear::CloakPlain:
		layers.emplace_back("11neck/fbas_11neck_cloakplain_00d.png", PALETTE_D);
		break;
	case Neckwear::CloakWithMantlePlain:
		layers.emplace_back("11neck/fbas_11neck_cloakwithmantleplain_00b.png", PALETTE_B);
		break;
	case Neckwear::MantlePlain:
		layers.emplace_back("11neck/fbas_11neck_mantleplain_00b.png", PALETTE_B);
		break;
	case Neckwear::Scarf:
		layers.emplace_back("11neck/fbas_11neck_scarf_00b.png", PALETTE_B);
		break;
	}

	// 12face

	switch (facewear) {
	case Facewear::Glasses:
		layers.emplace_back("12face/fbas_12face_glasses_00a.png", PALETTE_A);
		break;
	case Facewear::Shades:
		layers.emplace_back("12face/fbas_12face_shades_00a.png", PALETTE_A);
		break;
	}

	// 13hair

	bool headwear_replaces_hair = false;
	switch (headwear) {
	case Headwear::Bandana:
	case Headwear::Headscarf:
		headwear_replaces_hair = true;
		break;
	}

	if (!headwear_replaces_hair) {
		switch (hair) {
		case Hair::Afro:
			layers.emplace_back("13hair/fbas_13hair_afro_00.png", PALETTE_HAIR);
			break;
		case Hair::AfroPuffs:
			layers.emplace_back("13hair/fbas_13hair_afropuffs_00.png", PALETTE_HAIR);
			break;
		case Hair::Bob1:
			layers.emplace_back("13hair/fbas_13hair_bob1_00.png", PALETTE_HAIR);
			break;
		case Hair::Bob2:
			layers.emplace_back("13hair/fbas_13hair_bob2_00.png", PALETTE_HAIR);
			break;
		case Hair::Dapper:
			layers.emplace_back("13hair/fbas_13hair_dapper_00.png", PALETTE_HAIR);
			break;
		case Hair::Flattop:
			layers.emplace_back("13hair/fbas_13hair_flattop_00.png", PALETTE_HAIR);
			break;
		case Hair::LongWavy:
			layers.emplace_back("13hair/fbas_13hair_longwavy_00.png", PALETTE_HAIR);
			break;
		case Hair::Ponytail1:
			layers.emplace_back("13hair/fbas_13hair_ponytail1_00.png", PALETTE_HAIR);
			break;
		case Hair::Spiky1:
			layers.emplace_back("13hair/fbas_13hair_spiky1_00.png", PALETTE_HAIR);
			break;
		case Hair::Spiky2:
			layers.emplace_back("13hair/fbas_13hair_spiky2_00.png", PALETTE_HAIR);
			break;
		case Hair::Twintail:
			layers.emplace_back("13hair/fbas_13hair_twintail_00.png", PALETTE_HAIR);
			break;
		case Hair::Twists:
			layers.emplace_back("13hair/fbas_13hair_twists_00.png", PALETTE_HAIR);
			break;
		}
	}

	// 14head

	switch (headwear) {
	case Headwear::Bandana:
		layers.emplace_back("14head/fbas_14head_bandana_00b_e.png", PALETTE_B);
		break;
	case Headwear::BoaterHat:
		layers.emplace_back("14head/fbas_14head_boaterhat_00d.png", PALETTE_D);
		break;
	case Headwear::CowboyHat:
		layers.emplace_back("14head/fbas_14head_cowboyhat_00d.png", PALETTE_D);
		break;
	case Headwear::FloppyHat:
		layers.emplace_back("14head/fbas_14head_floppyhat_00d.png", PALETTE_D);
		break;
	case Headwear::Headscarf:
		layers.emplace_back("14head/fbas_14head_headscarf_00b_e.png", PALETTE_B);
		break;
	case Headwear::StrawHat:
		layers.emplace_back("14head/fbas_14head_strawhat_00d.png", PALETTE_D);
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
		std::shared_ptr<sf::Texture> ramps = textures::get(base_dir / "palettes/mana seed skin ramps.png");
		if (!ramps) continue;
		shader->setUniform("ramps_texture", *ramps);
		shader->setUniform("palette", layer.palette);
		shader->setUniform("color", layer.color);
		render_texture.draw(sf::Sprite(*texture), { shader.get() });
	}
	render_texture.display();

	return std::make_shared<sf::Texture>(render_texture.getTexture());
}