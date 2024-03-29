Hi, thanks for picking up the Mana Seed Farmer Sprite System!

This is a complete paper doll system for your player sprite in a slice-of-life farming sim style game.
It features a variety of animations, hairstyles, outfits, props, and effects, as well as comprehensive guides to help you deploy them correctly.

You may add your own original hairstyle and outfit layers to this system for use in your own game.
If you would like to sell those original assets on itchio, please thoroughly read "Selling Mana Seed Compatible Assets.txt" for restrictions and contact info.


------------------------------------------------

Please note: this is not a quick and easy plug-and-play RPG Maker-type asset.
Experience with a more powerful game engine is required to be able to use the Mana Seed Farmer Sprite System.
Something like Unity, Godot, GameMaker Studio, Unreal Engine, or your own custom game engine.
You'll need to build animations with frames from different parts of the sheet (out of sequential order).
You'll need to flip some frames within the same animation.
You'll need to attach props and effects from separate sheets and line them up appropriately.
You'll need to move those props behind or in front of the chara_ecter sprite as necessary.
You'll need as many as 20 layers to use everything as intended.
You'll need to assemble a paper-doll system in your game engine if you want to be able to customize your outfits in-game.
You'll need to be able to swap color palettes at run-time if you want your players to customize their outfit/hair colors.
Etcetera...

If you're still new to game development, you can come visit the Mana Seed Discord server and speak with other devs using this asset.
The Mana Seed community is friendly and helpful (so long as you don't show up with a poor attitude).
Hope to see you there! https://selieltheshaper.weebly.com/discord.html


------------------------------------------------

This asset is meant to have everything you need to make a playable character for a Harvest Moon or Stardew Valley style farming life sim game.
Please consult the supporting files for help setting up the animations.

Slice the farmer base sheet into 64x64 cells, and use the cell reference sheet in combo with the animation guide.
Do the same with all the other (outfit) sheets, and layer them on top in the exact same arrangement.

If an effects sheet has something like "32x32" in the filename, that's what you should slice the sheet at.
Position the props and effects as demonstrated in the animation guide, placing them behind or in front of the sprite as necessary.
In the animation guide, props and effects with bright green text use a preconfigured layout, just place them over the character sprite like a clothing layer.
Props and effects NOT in bright green text on the animation guide are OPEN TO YOUR OWN INTERPRETATION!
Look at the animation guide and make your own judgment for these.
I didn't preconfigure everything because that would be both time consuming for me, and overly limiting to you.
I want you to see beyond the suggestions and realize the potential for unlisted animations.
Use any frames that makes sense to you, in any order you want, put the props and effects wherever you think they look good, and make your own animations!


------------------------------------------------

NAMING CONVENTIONS

Each Farmer Base sheet filename uses the same strict naming convention.
4 or 5 IDs, separated by underscores. For example: "fbas_14head_headscarf_00b_e"

	ID 1 (type)
		fbas (the farmer base, so you don't mix it up with other bases)


	ID 2 (layer, 6 digits)
		15over (over everything, top-most layer: front wing, magic effects)
		14head (headwear: hats, hoods, horns)
		13hair (hair)
		12face (facewear: glasses, masks)
		11neck (neckwear: over all the other clothes: cloak, scarf)
		10outr (outerwear: coats, jackets, vests, suspenders)
		09hand (handwear: gloves, bracers)
		08lwr3 (most skirts & dresses)
		07fot2 (footwear big enough go over pant legs)
		06lwr2 (pants with upper parts: overalls, shortalls)
		05shrt (most shirts & blouses)
		04lwr1 (most pants & shorts)
		03fot1 (footwear small enough to stay under the pants)
		02sock (legwear: socks, stockings, hose)
		01body (the body)
		00undr (under everything, bottom-most layer: back wing, cloak)

		Note:		Numbered versions should be distinct.
				For example: 04lwr1, 06lwr2, and 08lwr3 should all technically occupy the same "slot" on the player: You can't wear pants AND overalls.

		Another note:	Most hats can go right on top of the hair (all hairs will be designed to accomodate this without any fuss).
				But a few hats will completely replace any hairstyle (like the head scarf).
				These hats will include "_e" in the ID 5 string (see below for details).
		
		Yet another:	The 00undr layer is mostly used for the second part of complicated clothing elements.
				If something is in here, it most likely ALSO exists in another layer (currently just "cloakplain" at time of writing, in both 00undr and 11neck).
				Both of those layers must appear together for the clothing to work.

		Last note: 	These layers are only suggestions. If you have a system that works better for your game, do that!


	ID 3 (unique name, 16-character max)
		human
		undies
		bra
		tanktop
		onepiece
		longshirt
		longpants
		overalls
		shorts
		shoes
		cuffedboots
		frillydressboobs (IDs with "boobs" at the end are variants with noticable boob shapes in the chest area)
		twintail
		cowboyhat
		etcetera...


	ID 4 (version number and base palette)
		00, 01, 02, 03, etc (usually color swaps)
		I'm mostly just using version 00 right now, which will have a default universal color palette (see below).
		The single character that follows the two digits (only found on version 00) is the palette reference.
		If you're using a runtime color swapping function, you can program it to look for this character, then it'll know which colors to replace.
			00a	Single 3-color ramp
			00b	Single 4-color ramp
			00c	Two 3-color ramps
			00d	One 4-color ramp AND one 3-color ramp
		Please refer to "Mana Seed color ramps.png" in the "_supporting files" folder to know exactly what these ramps are, and which custom ramps you can replace them with.
	

	ID 5 (special ID: hair replacement)
		The 5th string is a special code only used on some specific clothing parts. On all other parts, there will only be 4 IDs.
		Right now, the only special ID is "_e" for hair replacement.
		This can be found on hats that cannot coexist with any hairstyles (like the headscarf).
		When a character equips a hat with a "_e" in ID 2, then the hair sprite in layer "13hair" (ID 2) should be removed.


------------------------------------------------

I'll add more to this little help file as necessary.
Please let me know if you have any issues, or notice any errors.

Thanks again!
Seliel
