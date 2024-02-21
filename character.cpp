#include "stdafx.h"
#include "character.h"
#include "textures.h"
#include "random.h"

void Character::randomize()
{
	body = (Body)(random::range_i(1, (int)Body::Count - 1));
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
	std::vector<std::filesystem::path> paths;

	// 00undr

	switch (neckwear) {
	case Neckwear::CloakPlain:
		paths.push_back("00undr/fbas_00undr_cloakplain_00d.png");
		break;
	case Neckwear::CloakWithMantlePlain:
		paths.push_back("00undr/fbas_00undr_cloakwithmantleplain_00b.png");
		break;
	}

	// 01body

	switch (body) {
	case Body::Human:
		paths.push_back("01body/fbas_01body_human_00.png");
		break;
	}

	// 02sock

	switch (legwear) {
	case Legwear::SocksHigh:
		paths.push_back("02sock/fbas_02sock_sockshigh_00a.png");
		break;
	case Legwear::SocksLow:
		paths.push_back("02sock/fbas_02sock_sockslow_00a.png");
		break;
	case Legwear::Stockings:
		paths.push_back("02sock/fbas_02sock_stockings_00a.png");
		break;
	}

	// 03fot1

	switch (footwear) {
	case Footwear::Boots:
		paths.push_back("03fot1/fbas_03fot1_boots_00a.png");
		break;
	case Footwear::Sandals:
		paths.push_back("03fot1/fbas_03fot1_sandals_00a.png");
		break;
	case Footwear::Shoes:
		paths.push_back("03fot1/fbas_03fot1_shoes_00a.png");
		break;
	}

	// 04lwr1

	switch (lowerwear) {
	case Lowerwear::LongPants:
		paths.push_back("04lwr1/fbas_04lwr1_longpants_00a.png");
		break;
	case Lowerwear::Onepiece:
		paths.push_back("04lwr1/fbas_04lwr1_onepiece_00a.png");
		break;
	case Lowerwear::OnepieceBoobs:
		paths.push_back("04lwr1/fbas_04lwr1_onepieceboobs_00a.png");
		break;
	case Lowerwear::Shorts:
		paths.push_back("04lwr1/fbas_04lwr1_shorts_00a.png");
		break;
	case Lowerwear::Undies:
		paths.push_back("04lwr1/fbas_04lwr1_undies_00a.png");
		break;
	}

	// 05shrt

	switch (shirt) {
	case Shirt::Bra:
		paths.push_back("05shrt/fbas_05shrt_bra_00a.png");
		break;
	case Shirt::LongShirt:
		paths.push_back("05shrt/fbas_05shrt_longshirt_00a.png");
		break;
	case Shirt::LongShirtBoobs:
		paths.push_back("05shrt/fbas_05shrt_longshirtboobs_00a.png");
		break;
	case Shirt::ShortShirt:
		paths.push_back("05shrt/fbas_05shrt_shortshirt_00a.png");
		break;
	case Shirt::ShortShirtBoobs:
		paths.push_back("05shrt/fbas_05shrt_shortshirtboobs_00a.png");
		break;
	case Shirt::TankTop:
		paths.push_back("05shrt/fbas_05shrt_tanktop_00a.png");
		break;
	case Shirt::TankTopBoobs:
		paths.push_back("05shrt/fbas_05shrt_tanktopboobs_00a.png");
		break;
	}

	// 06lwr2

	switch (lowerwear) {
	case Lowerwear::Overalls:
		paths.push_back("06lwr2/fbas_06lwr2_overalls_00a.png");
		break;
	case Lowerwear::OverallsBoobs:
		paths.push_back("06lwr2/fbas_06lwr2_overallsboobs_00a.png");
		break;
	case Lowerwear::Shortalls:
		paths.push_back("06lwr2/fbas_06lwr2_shortalls_00a.png");
		break;
	case Lowerwear::ShortallsBoobs:
		paths.push_back("06lwr2/fbas_06lwr2_shortallsboobs_00a.png");
		break;
	}

	// 07fot2

	switch (footwear) {
	case Footwear::CuffedBoots:
		paths.push_back("07fot2/fbas_07fot2_cuffedboots_00a.png");
		break;
	case Footwear::CurlyToeShoes:
		paths.push_back("07fot2/fbas_07fot2_curlytoeshoes_00a.png");
		break;
	}

	// 08lwr3

	switch (lowerwear) {
	case Lowerwear::FrillyDress:
		paths.push_back("08lwr3/fbas_08lwr3_frillydress_00a.png");
		break;
	case Lowerwear::FrillyDressBoobs:
		paths.push_back("08lwr3/fbas_08lwr3_frillydressboobs_00a.png");
		break;
	case Lowerwear::FrillySkirt:
		paths.push_back("08lwr3/fbas_08lwr3_frillyskirt_00a.png");
		break;
	case Lowerwear::LongDress:
		paths.push_back("08lwr3/fbas_08lwr3_longdress_00a.png");
		break;
	case Lowerwear::LongDressBoobs:
		paths.push_back("08lwr3/fbas_08lwr3_longdressboobs_00a.png");
		break;
	case Lowerwear::LongSkirt:
		paths.push_back("08lwr3/fbas_08lwr3_longskirt_00a.png");
		break;
	}

	// 09hand

	switch (handwear) {
	case Handwear::Gloves:
		paths.push_back("09hand/fbas_09hand_gloves_00a.png");
		break;
	}

	// 10outr

	switch (outerwear) {
	case Outerwear::Suspenders:
		paths.push_back("10outr/fbas_10outr_suspenders_00a.png");
		break;
	case Outerwear::Vest:
		paths.push_back("10outr/fbas_10outr_vest_00a.png");
		break;
	}

	// 11neck

	switch (neckwear) {
	case Neckwear::CloakPlain:
		paths.push_back("11neck/fbas_11neck_cloakplain_00d.png");
		break;
	case Neckwear::CloakWithMantlePlain:
		paths.push_back("11neck/fbas_11neck_cloakwithmantleplain_00b.png");
		break;
	case Neckwear::MantlePlain:
		paths.push_back("11neck/fbas_11neck_mantleplain_00b.png");
		break;
	case Neckwear::Scarf:
		paths.push_back("11neck/fbas_11neck_scarf_00b.png");
		break;
	}

	// 12face

	switch (facewear) {
	case Facewear::Glasses:
		paths.push_back("12face/fbas_12face_glasses_00a.png");
		break;
	case Facewear::Shades:
		paths.push_back("12face/fbas_12face_shades_00a.png");
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
			paths.push_back("13hair/fbas_13hair_afro_00.png");
			break;
		case Hair::AfroPuffs:
			paths.push_back("13hair/fbas_13hair_afropuffs_00.png");
			break;
		case Hair::Bob1:
			paths.push_back("13hair/fbas_13hair_bob1_00.png");
			break;
		case Hair::Bob2:
			paths.push_back("13hair/fbas_13hair_bob2_00.png");
			break;
		case Hair::Dapper:
			paths.push_back("13hair/fbas_13hair_dapper_00.png");
			break;
		case Hair::Flattop:
			paths.push_back("13hair/fbas_13hair_flattop_00.png");
			break;
		case Hair::LongWavy:
			paths.push_back("13hair/fbas_13hair_longwavy_00.png");
			break;
		case Hair::Ponytail1:
			paths.push_back("13hair/fbas_13hair_ponytail1_00.png");
			break;
		case Hair::Spiky1:
			paths.push_back("13hair/fbas_13hair_spiky1_00.png");
			break;
		case Hair::Spiky2:
			paths.push_back("13hair/fbas_13hair_spiky2_00.png");
			break;
		case Hair::Twintail:
			paths.push_back("13hair/fbas_13hair_twintail_00.png");
			break;
		case Hair::Twists:
			paths.push_back("13hair/fbas_13hair_twists_00.png");
			break;
		}
	}

	// 14head

	switch (headwear) {
	case Headwear::Bandana:
		paths.push_back("14head/fbas_14head_bandana_00b_e.png");
		break;
	case Headwear::BoaterHat:
		paths.push_back("14head/fbas_14head_boaterhat_00d.png");
		break;
	case Headwear::CowboyHat:
		paths.push_back("14head/fbas_14head_cowboyhat_00d.png");
		break;
	case Headwear::FloppyHat:
		paths.push_back("14head/fbas_14head_floppyhat_00d.png");
		break;
	case Headwear::Headscarf:
		paths.push_back("14head/fbas_14head_headscarf_00b_e.png");
		break;
	case Headwear::StrawHat:
		paths.push_back("14head/fbas_14head_strawhat_00d.png");
		break;
	}

	// LOAD TEXTURES

	const std::filesystem::path dir = "assets/textures/character";
	std::vector<std::shared_ptr<sf::Texture>> textures;
	for (const std::filesystem::path& path : paths)
		if (std::shared_ptr<sf::Texture> texture = textures::get(dir / path))
			textures.push_back(texture);
	if (textures.empty()) return nullptr;

	// RENDER TO TEXTURE

	sf::RenderTexture render_texture;
	render_texture.create(1024, 1024);
	render_texture.clear(sf::Color::Transparent);
	for (const std::shared_ptr<sf::Texture>& texture : textures)
		render_texture.draw(sf::Sprite(*texture));
	render_texture.display();

	return std::make_shared<sf::Texture>(render_texture.getTexture());
}