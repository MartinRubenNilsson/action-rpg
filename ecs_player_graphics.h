#pragma once

namespace ecs
{
	struct PlayerGraphics
	{
		enum class Headwear // 14head
		{
			None,
			Bandana,
			BoaterHat,
			CowboyHat,
			FloppyHat,
			Headscarf,
			StrawHat,
		};

		enum class Hair // 13hair
		{
			None,
			Afro,
			AfroPuffs,
			Bob1,
			Bob2,
			Dapper,
			Flattop,
			LongWavy,
			Ponytail1,
			Spiky1,
			Spiky2,
			Twintail,
			Twists,
		};

		enum class Facewear // 12face
		{
			None,
			Glasses,
			Shades,
		};

		enum class Neckwear // 11neck
		{
			None,
			CloakPlain,
			CloakWithMantlePlain,
			MantlePlain,
			Scarf,
		};

		enum class Outerwear // 10outr
		{
			None,
			Suspenders,
			Vest,
		};

		enum class Handwear // 09hand
		{
			None,
			Gloves,
		};

		enum class Lowerwear3 // 08lwr3
		{
			None,
			FrillyDress,
			FrillyDressBoobs,
			FrillySkirt,
			LongDress,
			LongDressBoobs,
			LongSkirt,
		};

		enum class Footwear2 // 07fot2
		{
			None,
			Boots,
			Shoes,
		};

		enum class Lowerwear2 // 06lwr2
		{
			None,
			Overalls,
			OverallsBoobs,
			Shortalls,
			ShortallsBoobs,
		};

		enum class Shirt // 05shrt
		{
			None,
			Bra,
			LongShirt,
			LongShirtBoobs,
			ShortShirt,
			ShortShirtBoobs,
			TankTop,
			TankTopBoobs,
		};

		enum class Lowerwear1 // 04lwr1
		{
			None,
			LongPants,
			Onepiece,
			OnepieceBoobs,
			Shorts,
			Undies,
		};

		enum class Footwear1 // 03fot1
		{
			None,
			Boots,
			Sandals,
			Shoes,
		};

		enum class Legwear // 02sock
		{
			None,
			SocksHigh,
			SocksLow,
			Stockings,
		};

		enum class Body // 01body
		{
			None,
			Human,
		};

		enum class Under // 00undr
		{
			None,
			CloakPlain,
			CloakWithMantlePlain,
		};

		Headwear headwear = Headwear::None;
		Hair hair = Hair::None;
		Facewear facewear = Facewear::None;
		Neckwear neckwear = Neckwear::None;
		Outerwear outerwear = Outerwear::None;
		Handwear handwear = Handwear::None;
		Lowerwear3 lowerwear3 = Lowerwear3::None;
		Footwear2 footwear2 = Footwear2::None;
		Lowerwear2 lowerwear2 = Lowerwear2::None;
		Shirt shirt = Shirt::None;
		Lowerwear1 lowerwear1 = Lowerwear1::None;
		Footwear1 footwear1 = Footwear1::None;
		Legwear legwear = Legwear::None;
		Body body = Body::Human;
		Under under = Under::None;
	};

	std::shared_ptr<sf::Texture> bake_player_texture(const PlayerGraphics& graphics);
}

