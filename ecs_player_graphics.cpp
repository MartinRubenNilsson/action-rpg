#include "stdafx.h"
#include "ecs_player_graphics.h"

namespace ecs
{
	struct PlayerClothes
	{
		//15over(over everything, top - most layer : front wing, magic effects)

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

		enum class Skirt // 08lwr3
		{
			None,
			MiniSkirt,
			PleatedSkirt,
		};

		//08lwr3(most skirts & dresses)
		//07fot2(footwear big enough go over pant legs)
		//06lwr2(pants with upper parts : overalls, shortalls)
		//05shrt(most shirts & blouses)
		//04lwr1(most pants & shorts)
		
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

		//00undr(under everything, bottom - most layer : back wing, cloak)
	
		Headwear headwear = Headwear::None;
		Hair hair = Hair::None;
		Facewear facewear = Facewear::None;
		Neckwear neckwear = Neckwear::None;
		Outerwear outerwear = Outerwear::None;
		Handwear handwear = Handwear::None;
		//TODO
		Footwear1 footwear1 = Footwear1::None;
		Legwear legwear = Legwear::None;
		Body body = Body::None;
	};
}