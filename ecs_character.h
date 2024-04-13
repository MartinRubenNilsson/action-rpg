#pragma once

namespace ecs
{
	struct Character
	{
		static constexpr int C3_LUT_COLORS = 48;
		static constexpr int C4_LUT_COLORS = 58;

		static constexpr int SKIN_COLORS = 18;
		static constexpr int HAIR_COLORS = 58;
		static constexpr int SOCK_COLORS = C3_LUT_COLORS;
		static constexpr int SHOE_COLORS = C3_LUT_COLORS;
		static constexpr int LOWERWEAR_COLORS = C3_LUT_COLORS;
		static constexpr int SHIRT_COLORS = C3_LUT_COLORS;
		static constexpr int GLOVES_COLORS = C3_LUT_COLORS;
		static constexpr int OUTERWEAR_COLORS = C3_LUT_COLORS;
		static constexpr int NECKWEAR_COLORS_1 = C3_LUT_COLORS;
		static constexpr int NECKWEAR_COLORS_2 = C4_LUT_COLORS;
		static constexpr int GLASSES_COLORS = C3_LUT_COLORS;
		static constexpr int HAT_COLORS_1 = C3_LUT_COLORS;
		static constexpr int HAT_COLORS_2 = C4_LUT_COLORS;

		enum class Body
		{
			None,
			Human,
			Count,
		};

		enum class Sock
		{
			None,
			SocksHigh,
			SocksLow,
			Stockings,
			Count,
		};

		enum class Shoe
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
		Sock sock = Sock::None;
		int sock_color = 0;
		Shoe shoe = Shoe::None;
		int shoe_color = 0;
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

		void randomize();
		std::shared_ptr<sf::Texture> bake_texture() const;
	};
}
