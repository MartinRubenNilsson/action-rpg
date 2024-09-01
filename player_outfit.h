#pragma once

namespace player
{
	struct Outfit
	{
		enum class Body
		{
			None,
			Human,
			Count,
		};

		enum class Socks
		{
			None,
			SocksHigh,
			SocksLow,
			Stockings,
			Count,
		};

		enum class Shoes
		{
			None,
			Boots,
			Sandals,
			Shoes,
			CuffedBoots,
			CurlyToeShoes,
			Count,
		};

		enum class Lowerwear
		{
			None,
			LongPants,
			Onepiece,
			OnepieceBoobs,
			Shorts,
			Undies,
			Overalls,
			OverallsBoobs,
			Shortalls,
			ShortallsBoobs,
			FrillyDress,
			FrillyDressBoobs,
			FrillySkirt,
			LongDress,
			LongDressBoobs,
			LongSkirt,
			Count,
		};

		enum class Shirt
		{
			None,
			Bra,
			LongShirt,
			LongShirtBoobs,
			ShortShirt,
			ShortShirtBoobs,
			TankTop,
			TankTopBoobs,
			Count,
		};

		enum class Gloves
		{
			None,
			Gloves,
			Count,
		};

		enum class Outerwear
		{
			None,
			Suspenders,
			Vest,
			Count,
		};

		enum class Neckwear
		{
			None,
			CloakPlain,
			CloakWithMantlePlain,
			MantlePlain,
			Scarf,
			Count,
		};

		enum class Glasses
		{
			None,
			Glasses,
			Shades,
			Count,
		};

		enum class Hair
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
			Count,
		};

		enum class Hat
		{
			None,
			Bandana, // replaces hair
			BoaterHat,
			CowboyHat,
			FloppyHat,
			Headscarf, // replaces hair
			StrawHat,
			Count,
		};

		Body body = Body::Human;
		int skin_color = 0;
		Socks socks = Socks::None;
		int socks_color = 0;
		Shoes shoes = Shoes::None;
		int shoes_color = 0;
		Lowerwear lowerwear = Lowerwear::None;
		int lowerwear_color = 0;
		Shirt shirt = Shirt::None;
		int shirt_color = 0;
		Gloves gloves = Gloves::None;
		int gloves_color = 0;
		Outerwear outerwear = Outerwear::None;
		int outerwear_color = 0;
		Neckwear neckwear = Neckwear::None;
		int neckwear_color_1 = 0;
		int neckwear_color_2 = 0;
		Glasses glasses = Glasses::None;
		int glasses_color = 0;
		Hair hair = Hair::None;
		int hair_color = 0;
		Hat hat = Hat::None;
		int hat_color_1 = 0;
		int hat_color_2 = 0;
	};

	void randomize_outfit(Outfit& outfit);
	Handle<graphics::Texture> create_outfit_texture(Outfit& outfit);
}
