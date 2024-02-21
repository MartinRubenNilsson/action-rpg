#pragma once

struct Character
{
	static constexpr int COLOR3_PALETTES = 48;
	static constexpr int COLOR4_PALETTES = 58;
	static constexpr int HAIR_PALETTES = 58;
	static constexpr int SKIN_PALETTES = 18;

	enum class Body
	{
		None,
		// 01body
		Human,
		Count,
	};

	enum class Legwear
	{
		None,
		// 02sock
		SocksHigh,
		SocksLow,
		Stockings,
		Count,
	};

	enum class Footwear
	{
		None,
		// 03fot1
		Boots,
		Sandals,
		Shoes,
		// 07fot2
		CuffedBoots,
		CurlyToeShoes,
		Count,
	};

	enum class Lowerwear
	{
		None,
		// 04lwr1
		LongPants,
		Onepiece,
		OnepieceBoobs,
		Shorts,
		Undies,
		// 06lwr2
		Overalls,
		OverallsBoobs,
		Shortalls,
		ShortallsBoobs,
		// 08lwr3
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
		// 05shrt
		Bra,
		LongShirt,
		LongShirtBoobs,
		ShortShirt,
		ShortShirtBoobs,
		TankTop,
		TankTopBoobs,
		Count,
	};

	enum class Handwear
	{
		None,
		// 09hand
		Gloves,
		Count,
	};

	enum class Outerwear
	{
		None,
		// 10outr
		Suspenders,
		Vest,
		Count,
	};

	enum class Neckwear
	{
		None,
		// 11neck & 00undr
		CloakPlain,
		CloakWithMantlePlain,
		// 11neck
		MantlePlain,
		Scarf,
		Count,
	};

	enum class Facewear
	{
		None,
		// 12face
		Glasses,
		Shades,
		Count,
	};

	enum class Hair
	{
		None,
		// 13hair
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

	enum class Headwear
	{
		None,
		// 14head
		Bandana, // replaces hair
		BoaterHat,
		CowboyHat,
		FloppyHat,
		Headscarf, // replaces hair
		StrawHat,
		Count,
	};

	Body body = Body::Human;
	int body_palette = 0;
	Legwear legwear = Legwear::None;
	Footwear footwear = Footwear::None;
	Lowerwear lowerwear = Lowerwear::None;
	Shirt shirt = Shirt::None;
	Handwear handwear = Handwear::None;
	Outerwear outerwear = Outerwear::None;
	Neckwear neckwear = Neckwear::None;
	Facewear facewear = Facewear::None;
	Hair hair = Hair::None;
	Headwear headwear = Headwear::None;

	void randomize();
	std::shared_ptr<sf::Texture> bake_texture() const;
};
